#! /bin/sh
if [ ! -d "/tmp/trace" ];then
mkdir /tmp/trace
fi

if [ -f "./hook.so" ];then
rm -f hook.so
fi

cd ./src
gcc -fPIC -shared -o ../hook.so tracer.c -ldl -luuid -pthread

cd ../

