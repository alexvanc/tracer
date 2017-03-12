#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <arpa/inet.h>
#include <sys/timeb.h>
#include <pthread.h>
#include <uuid/uuid.h>
#include <curl/curl.h>
#include <string.h>


typedef ssize_t(*RECV)(int sockfd, void *buf, size_t len, int flags);
typedef ssize_t(*SEND)(int sockfd, const void *buf, size_t len, int flags);
typedef ssize_t(*WRITE)(int fd, const void *buf, size_t count);
typedef ssize_t(*READ)(int fd, void *buf, size_t count);
typedef ssize_t(*SENDMSG)(int sockfd, const struct msghdr *msg, int flags);
typedef ssize_t(*RECVMSG)(int sockfd, struct msghdr * msg, int flags);
typedef ssize_t(*SENDTO)(int socket, char *buf, int buflen, int flags,struct sockaddr *addr, int addrlen);
typedef ssize_t(*RECVFROM)(int socket, char *buf, int buflen, int flags,struct sockaddr *addr, int *addrlen);

// typedef int(*CONN)(int socket, const struct sockaddr *addr, socklen_t length);
// typedef int(*CLOSE)(int fd);

struct string {
  char *ptr;
  size_t len;
};

//generate 37bytes uuid
char *random_uuid( char*);

//extract 37bytes uuid from message
int get_uuid(char*,char*,int);

//unused, message buffer in local db
int push_to_local_database(char*,int,pid_t,pthread_t,char*,long long,char,int);

//send the message to the central db directly
int push_to_database(char*,int,pid_t,pthread_t,char*,long long,char,int);

//check wether there is a message has been sent but not received on central controller
int check_send_status(char*);

//check wether this message is waiting for being received on central controller
int check_recv_status(char*);

//send the message to the central controller
int mark_send(char*);

//mark this message as received on central controller
int mark_recv(char*);

//get local timestamp
long long gettime();

//log trace into local files, for debug and buffer/cache
void push_to_local_file(char*,int,pid_t,pthread_t,char*,long long,char,int);

//log info into local files, only for debug
void record_to_local_file(char*,int,char*,char*);


//to get the http response
int get_response(char* post_parameter);
void init_string(struct string *s);
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s);
// char *ptr, size_t size, size_t nmemb, void *userdata


ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
    static void *handle = NULL;
    static RECV old_recv = NULL;
    struct sockaddr_in sin; //local socket info
    struct sockaddr_in son; //remote socket into 
    socklen_t s_len = sizeof(sin);
    if( !handle )
    {
        handle = dlopen("libc.so.6", RTLD_LAZY);
        old_recv = (RECV)dlsym(handle, "recv");
    }
    if ((getsockname(sockfd, (struct sockaddr *)&sin, &s_len) != -1) &&(getpeername(sockfd, (struct sockaddr *)&son, &s_len) != -1))
    {
        unsigned short int in_port;
        unsigned short int on_port;
        char *in_ip;
        char *on_ip;
        in_port=ntohs(sin.sin_port);
        on_port=ntohs(son.sin_port);
        in_ip=inet_ntoa(sin.sin_addr);
        on_ip=inet_ntoa(son.sin_addr);

        // filter of our own service, db and controller
        if ((in_port==80) || (on_port==80)){ 
            return old_recv(sockfd, buf, len, flags);
        }

        char result[len];
        char id[39];
        int n;
        int if_id;
        int check_status;
        long long ttime;
        n=old_recv(sockfd, result, len, flags);
        // printf("RECV bytes:%d\n",n);
        if_id=get_uuid(id,result,n);
        if (if_id==0){// id can be extracted

            while (1){
                check_status=check_recv_status(id);
                if (check_status==0){// this message is waiting for being received
                    ttime=gettime();

                    //for debug
                    push_to_local_file(in_ip,in_port,getpid(),pthread_self(),id,ttime,0,n); //0 means recv,1==send

                    memmove(buf,result,n-39);
                    mark_recv(id);//mark it in the controller as received
                    push_to_database(in_ip,in_port,getpid(),pthread_self(),id,ttime,0,n);
                    return n-39;
                }
                sleep(1);

            }
        }else{
            printf("RECV cannot extrat id\n");
            record_to_local_file(in_ip,in_port,"recv","sock");
            memmove(buf,result,n);
            return n;
        }

    }
    return old_recv(sockfd, buf, len, flags);

}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{

    static void *handle = NULL;
    static SEND old_send = NULL;
    struct sockaddr_in sin;
    struct sockaddr_in son;
    socklen_t s_len = sizeof(son);
    if( !handle )
    {
        handle = dlopen("libc.so.6", RTLD_LAZY);
        old_send = (SEND)dlsym(handle, "send");
    }

    if  ((getsockname(sockfd, (struct sockaddr *)&sin, &s_len) != -1) &&(getpeername(sockfd, (struct sockaddr *)&son, &s_len) != -1)){
        unsigned short int in_port;
        unsigned short int on_port;
        char *in_ip;
        char *on_ip;
        in_port=ntohs(sin.sin_port);
        in_ip=inet_ntoa(sin.sin_addr);
        on_port=ntohs(son.sin_port);
        on_ip=inet_ntoa(son.sin_addr);

         // filter our own service, db and controller
        if ((in_port==80) || (on_port==80)){
            return old_send(sockfd, buf, len, flags);
        }

        int check_status;
        int new_len=len+39;
        char target[new_len];
        memmove(target,buf,len);

        //add uuid at the end of data
        char id[39];
        random_uuid(id);
        int i;
        for (i=0;i<39;i++){
            target[len+i]=id[i];
        }

        while (1){
            check_status=check_send_status(id);
            if(check_status==0){ //no message is waiting for receiving and the message queue is available
                long long ttime;
                ttime=gettime();

                //for debug
                push_to_local_file(in_ip,in_port,getpid(),pthread_self(),id,ttime,1,new_len);

                mark_send(id);
                push_to_database(in_ip,in_port,getpid(),pthread_self(),id,ttime,1,new_len);
                return old_send(sockfd, target, new_len, flags);
            }else{
                //other cases, to do
            }

            sleep(1);
        }
        return old_send(sockfd, buf, len, flags);  

    }
    
    return old_send(sockfd, buf, len, flags);

}

ssize_t write(int fd, const void *buf, size_t count)
{
    static void *handle = NULL;
    static WRITE old_write = NULL;
    struct sockaddr_in sin;
    struct sockaddr_in son;
    socklen_t s_len = sizeof(son);
    if( !handle )
    {
        handle = dlopen("libc.so.6", RTLD_LAZY);
        old_write = (WRITE)dlsym(handle, "write");
    }

    if  ((getsockname(fd, (struct sockaddr *)&sin, &s_len) != -1) &&(getpeername(fd, (struct sockaddr *)&son, &s_len) != -1)){
        unsigned short int in_port;
        unsigned short int on_port;
        char *in_ip;
        char *on_ip;
        in_port=ntohs(sin.sin_port);
        in_ip=inet_ntoa(sin.sin_addr);
        on_port=ntohs(son.sin_port);
        on_ip=inet_ntoa(son.sin_addr);

         // filter our own service, db and controller
        if ((in_port==80) || (on_port==80)){
            return old_write(fd, buf, count);
        }

        int check_status;
        int new_len=count+39;
        char target[new_len];
        memmove(target,buf,count);

        //add uuid at the end of data
        char id[39];
        random_uuid(id);
        int i;
        for (i=0;i<39;i++){
            target[count+i]=id[i];
        }

        while (1){
            check_status=check_send_status(id);
            if(check_status==0){ //no message is waiting for receiving and the message queue is available
                long long ttime;
                ttime=gettime();

                //for debug
                push_to_local_file(in_ip,in_port,getpid(),pthread_self(),id,ttime,2,new_len);

                mark_send(id);
                push_to_database(in_ip,in_port,getpid(),pthread_self(),id,ttime,2,new_len);
                return old_write(fd, target, new_len);
            }else{
                //other cases, to do
            }

            sleep(1);
        }
        return old_write(fd, buf, count);  

    }
    
    return old_write(fd, buf, count);

}

ssize_t read(int fd, void *buf, size_t count)
{
    static void *handle = NULL;
    static READ old_read = NULL;
    struct sockaddr_in sin; //local socket info
    struct sockaddr_in son; //remote socket into 
    socklen_t s_len = sizeof(sin);
    if( !handle )
    {
        handle = dlopen("libc.so.6", RTLD_LAZY);
        old_read = (READ)dlsym(handle, "read");
    }
    if ((getsockname(fd, (struct sockaddr *)&sin, &s_len) != -1) &&(getpeername(fd, (struct sockaddr *)&son, &s_len) != -1))
    {
        unsigned short int in_port;
        unsigned short int on_port;
        char *in_ip;
        char *on_ip;
        in_port=ntohs(sin.sin_port);
        on_port=ntohs(son.sin_port);
        in_ip=inet_ntoa(sin.sin_addr);
        on_ip=inet_ntoa(son.sin_addr);

        // filter our own service, db and controller
        if ((in_port==80) || (on_port==80)){ 
            return old_read(fd, buf, count);
        }

        char result[count];
        char id[39];
        int n;
        int if_id;
        int check_status;
        long long ttime;
        n=old_read(fd, result, count);
        // printf("RECV bytes:%d\n",n);
        if_id=get_uuid(id,result,n);
        if (if_id==0){// id can be extracted

            while (1){
                check_status=check_recv_status(id);
                if (check_status==0){// this message is waiting for being received
                    ttime=gettime();

                    //for debug
                    push_to_local_file(in_ip,in_port,getpid(),pthread_self(),id,ttime,3,n); //0 means recv,1==send

                    memmove(buf,result,n-39);
                    mark_recv(id);//mark it in the controller as received
                    push_to_database(in_ip,in_port,getpid(),pthread_self(),id,ttime,3,n);
                    return n-39;
                }
                sleep(1);

            }
        }else{
            printf("RECV cannot extrat id\n");
            record_to_local_file(in_ip,in_port,"read","sock");
            memmove(buf,result,n);
            return n;
        }

    }
    return old_read(fd, buf, count);
}


ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags){
    static void *handle = NULL;
    static RECVMSG old_recvmsg = NULL;
    struct sockaddr_in sin; //local socket info
    struct sockaddr_in son; //remote socket into 
    socklen_t s_len = sizeof(sin);
    if( !handle )
    {
        handle = dlopen("libc.so.6", RTLD_LAZY);
        old_recvmsg = (RECVMSG)dlsym(handle, "recvmsg");
    }
    if ((getsockname(sockfd, (struct sockaddr *)&sin, &s_len) != -1) &&(getpeername(sockfd, (struct sockaddr *)&son, &s_len) != -1))
    {
        unsigned short int in_port;
        unsigned short int on_port;
        char *in_ip;
        char *on_ip;
        in_port=ntohs(sin.sin_port);
        on_port=ntohs(son.sin_port);
        in_ip=inet_ntoa(sin.sin_addr);
        on_ip=inet_ntoa(son.sin_addr);

        // filter our own service, db and controller
        if ((in_port==80) || (on_port==80)){ 
            return old_recvmsg(sockfd, msg, flags);
        }
        //To do
        return old_recvmsg(sockfd, msg, flags);

    }
    return old_recvmsg(sockfd, msg, flags);
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags){
    static void *handle = NULL;
    static SENDMSG old_sendmsg = NULL;
    struct sockaddr_in sin;
    struct sockaddr_in son;
    socklen_t s_len = sizeof(son);
    if( !handle )
    {
        handle = dlopen("libc.so.6", RTLD_LAZY);
        old_sendmsg = (SENDMSG)dlsym(handle, "sendmsg");
    }

    if  ((getsockname(sockfd, (struct sockaddr *)&sin, &s_len) != -1) &&(getpeername(sockfd, (struct sockaddr *)&son, &s_len) != -1)){
        unsigned short int in_port;
        unsigned short int on_port;
        char *in_ip;
        char *on_ip;
        in_port=ntohs(sin.sin_port);
        in_ip=inet_ntoa(sin.sin_addr);
        on_port=ntohs(son.sin_port);
        on_ip=inet_ntoa(son.sin_addr);

         // filter our own service, db and controller
        if ((in_port==80) || (on_port==80)){
            return old_sendmsg(sockfd, msg, flags);
        }

        return old_sendmsg(sockfd, msg, flags);  
    }
    
    return old_sendmsg(sockfd, msg, flags);
}

// ssize_t sendto(int socket, char *buf, int buflen, int flags,struct sockaddr *addr, int addrlen){

// }
// ssize_t recvfrom(int socket, char *buf, int buflen, int flags,struct sockaddr *addr, int *addrlen){

// }

int get_uuid(char* result,char* buf,int start){
    int i=start-39;
    for (i = 0; i < 39; ++i)
    {
        result[i]=buf[start-39+i];
    }
    // printf("CMP Test: %c,%c\n",result[37],result[38]);
    if (result[37]=='^' && result[38]=='^'){
        printf("CMP Test:\n");
        return 0;
    }else{
        return 1;
    }

}

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

// to start post and return the response
int getresponse(char* post_parameter){
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if(curl) {
    struct string s;
    init_string(&s);

    curl_easy_setopt(curl, CURLOPT_URL, "10.211.55.38/controller.php");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_parameter);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    res = curl_easy_perform(curl);

    
    // char result[s.len];
    // memcpy(result,s.ptr,s.len);
    int result_flag;
    printf("%s\n", s.ptr);
    record_to_local_file("11.11.11.11",1111,post_parameter,s.ptr);
    if(strcmp(s.ptr, "0") == 0){
        result_flag=0;
    }else{
        result_flag=1;
    }

    free(s.ptr);

    /* always cleanup */
    curl_easy_cleanup(curl);
    return result_flag;
  }
}

 /**
  * Create random UUID
  *
  * @param buf - buffer to be filled with the uuid string
  */
  char *random_uuid( char buf[39] )
  {
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, buf);
    buf[37]='^';
    buf[38]='^';
    return buf;
}

long long gettime(){
    struct timeb t;
    ftime(&t);
    return 1000 * t.time + t.millitm;
}

void push_to_local_file(char* ip,int port,pid_t pid,pthread_t tid,char* uuid,long long time,char type,int length){
    FILE *logFile;
    // printf("here\n" );
    if((logFile=fopen("/tmp/testlog.txt","a+"))!=NULL){
        fprintf(logFile,"ip: %s\tport: %d\t pid: %u\t tid: %u\t uuid: %s\ttime: %lld\ttype: %d\tlength:%d\n", ip,port,(unsigned int)pid,(unsigned int)tid,uuid,time,type,length);
        fclose(logFile);

    }else{
        printf("Cannot open file!");

    }
}

void record_to_local_file(char* ip,int port,char* func,char* socktype){
  FILE *logFile;
    // printf("here\n" );
  if((logFile=fopen("/tmp/testlog.txt","a+"))!=NULL){
    fprintf(logFile,"%s\t%s\t%s\t%d\n", func,socktype,ip,port);
    fclose(logFile);

}else{
    printf("Cannot open file!");

}

}

//type=20
int check_send_status(char* id){
    char * parameter_tmp="uuid=%s&rtype=%d";
    char parameter[1024];
    sprintf(parameter,parameter_tmp,id,20);
    return getresponse(parameter);
}

//type=21
int check_recv_status(char* id){
// To do
    char * parameter_tmp="uuid=%s&rtype=%d";
    char parameter[1024];
    sprintf(parameter,parameter_tmp,id,22);
    return getresponse(parameter);
}

//type=22
int mark_send(char* id){
// To do
    char * parameter_tmp="uuid=%s&rtype=%d";
    char parameter[1024];
    sprintf(parameter,parameter_tmp,id,21);
    return getresponse(parameter);
}
//type=23
int mark_recv(char* id){
// To do
    char * parameter_tmp="uuid=%s&rtype=%d";
    char parameter[1024];
    sprintf(parameter,parameter_tmp,id,23);
    return getresponse(parameter);
}

//type=24
int push_to_database(char* ip,int port,pid_t pid,pthread_t tid,char* uuid,long long time,char type,int length){
// To do
    char * parameter_tmp="ip=%s&port=%d&pid=%u&tid=%u&uuid=%s&ttime=%lld&type=%d&length=%d&rtype=%d";
    char parameter[1024];
    sprintf(parameter,parameter_tmp,ip,port,pid,tid,uuid,time,type,length,24);
    return getresponse(parameter);

}





