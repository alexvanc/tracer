# REP
## REPTrace 安装及使用
### 1. 环境准备
1）安装依赖库
**Ubuntu**

```bash
apt-get install uuid-dev
apt-get install libcurl4-openssl-dev
```
**CentOS**
```bash
yum install e2fsprogs-devel
yum install uuid-devel
yum install libuuid-devel
yum install curl 
yum install curl-devel.x86_64
```
2）编译

  a)  修改src/tracer.c文件
 
第11、12行的IP改为本机IP(如果是容器，则改为容器IP)
第18行的*legal_ip_list*改为需要追踪的机器的IP(如有多个，依次添加)
**注意 :** 第17行的*ip_list_length*需要与第18行的*legal_ip_list*数组长度一致。

  b) 编译

```bash
    ./install.sh
```
编译完成后，当前目录下会生成hook.so文件。



### 2. 运行REPTrace
1）禁用IPV6
```bash
sudo vim /etc/sysctl.conf
最后面加三行：
net.ipv6.conf.all.disable_ipv6 = 1
net.ipv6.conf.default.disable_ipv6 = 1
net.ipv6.conf.lo.disable_ipv6 = 1
保存文件后执行
sudo sysctl -p
```
2）追踪应用程序执行过程
LD_PRELOAD=/home/trace/hook.so 应用运行命令
如运行testcase1

```bash
cd ./testcase/case1
gcc -o server server.c
gcc -o client1 client1.c
在两个窗口中分别运行：（需要将路径改为本地hook.so的路径）
LD_PRELOAD=/home/trace/hook.so server（先运行）
LD_PRELOAD=/home/trace/hook.so client1 
```
生成的traceData位于**/tmp/trace/**目录下



## REPTrace 因果关系分析
### 1. 环境准备
python 2.7+
mysql 5.5+

### 2. 修改配置文件
```bash
cd ./REP
vim trace.config
{
 "mysql_host": "localhost", #mysql主机
 "mysql_db": "trace", #用于存储trace数据的数据库名称
 "mysql_usr": "root",#mysql账号
 "mysql_pwd": "root",#mysql密码
 "TraceDataPath": "/path/to/traceData.dat", #traceData数据所在目录
 "TraceSqlPath": "./tracedb.sql",
 "ProjectPath": "/path/to/trace",#trace所在目录
 "PythonVersion": "3" #python版本
 }
```

### 3. 生成请求执行路径
```bash
python3 ./start.py
```

1）请求执行路径

```
因果关系存在mysql中
a) 不同data_unit_id间因果关系：根据abstract1表中的parent_unit字段关联因果关系
(即A的parent_unit为B的data_unit_id，则B为A的父节点)
b) 同一data_unit_id内因果关系：根据combination1表中的data_unit_id、ktid、source_ip字段进行分组，每组内按照时间顺序确定因果关系。
```
2）日志文件
```
/path/to/trace/output/dbname/trace.log
```

