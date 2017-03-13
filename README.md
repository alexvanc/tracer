# tracer
tracer-beta

##1、tracer.c
* Use the following command to compile the tracer, and use **LD_PROLOAD** to intercept network requests.

	`gcc -fPIC -shared -o hook.so tracer.c -ldl -luuid -lcurl`
	
* if you want to test the tracer on your cluster, please replace the ip address in tracer.c(which is 10.211.55.38) with your own controller service(the port of controller service should still be 80).

###To Do
	1、Case 2: message queue based requests tracing
	2、sendmsg/recvmsg, sendto/recvfrom function interception.
	3、add connection info interception and send it to the trace database
	4、Automatically generate the trace of a request from the trace database
	
	

##2、controller.php
	Redis service should be set up as the backend of controller

##3、tracedb.sql
Mysql5.5 is used as the database of tracing data
