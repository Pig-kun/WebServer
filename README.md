# WebServer
A Simple WebServer
# 普通版

```bash
cd .
g++ *.cpp -o run -pthread
./run [端口号]
```
# 定时检测非活跃版
```bash
cd noactive/
g++ *.cpp -o run -pthread
./run [端口号]
```
# 服务器压力测试
```bash
cd test_pressure/webbench-1.5/
make
./webbench -c 1000 -t 5 http://192.168.211.129:端口号/index.html
```