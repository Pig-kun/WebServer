#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
#include <sys/epoll.h>
#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include "locker.h"


class http_conn {
public:

    static int m_epollfd; // 所有的socket上的事件都在这个epoll对象上注册
    static int m_user_count; // 当前连接的用户数量

    http_conn() {} // 构造函数
    ~http_conn() {} // 析构函数

    void process(); // 处理客户端请求
    void init(int sockfd, const sockaddr_in& addr); // 初始化新接受的连接
    void close_conn();
    bool read(); // 非阻塞的读
    bool write(); // 非阻塞的写

private:
    int m_sockfd; // 该HTTP连接的socket
    sockaddr_in m_address; // 客户端的socket地址信息


};



#endif