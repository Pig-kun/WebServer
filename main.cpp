#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <fcntl.h>
#include <sys/epoll.h>
#include <signal.h>
#include "locker.h"
#include "threadpool.h"
#include "http_conn.h"

#define MAX_FD 65534 // 最大文件描述符数量
#define MAX_EVENT_NUMBER 10000 // 监听的最大事件数量


// 添加信号捕捉
void addsig(int sig, void(handler)(int)){
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;

    sigfillset(&sa.sa_mask); // 填充信号集
    if(sigaction(sig, &sa, NULL) < 0){ // 注册信号处理函数
        perror("sigaction");
        exit(-1);
    }
}

// 添加文件描述符到epoll对象中
extern void addfd(int epollfd, int fd, bool one_shot);
// 从epoll对象中删除文件描述符
extern void removefd(int epollfd, int fd);
// 修改文件描述符
extern void modfd(int epollfd, int fd, int ev);



int main(int argc, char* argv[]){

    if(argc <= 1){
        printf("按照如下格式运行：%s port_number\n", basename(argv[0]));
        exit(-1);
    }

    // 获取端口号
    int port = atoi(argv[1]);

    // 对SIGPIPE信号进行处理
    addsig(SIGPIPE, SIG_IGN); // 忽略SIGPIPE信号

    // 创建线程池,初始化线程池
    threadpool<http_conn>* pool = NULL;
    try{
        pool = new threadpool<http_conn>; // 创建线程池
    } catch(...){
        exit(-1); // 创建线程池失败
    }

    // 创建数组，用于保存所有客户端信息
    http_conn* users = new http_conn[ MAX_FD ]; // 创建客户端信息数组
    if(!users){
        exit(-1); // 创建客户端信息数组失败
    }

    int listenfd = socket(PF_INET, SOCK_STREAM, 0); // 创建监听套接字
    if(listenfd < 0){
        perror("socket");
        exit(-1); // 创建监听套接字失败
    }

    // 设置端口复用
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)); // 设置端口复用

    // 绑定端口
    struct sockaddr_in address;
    address.sin_family = AF_INET; // IPv4
    address.sin_addr.s_addr = INADDR_ANY; // 监听所有IP地址
    address.sin_port = htons(port); // 端口号
    if(bind(listenfd, (struct sockaddr*)&address, sizeof(address)) < 0){ // 绑定端口
        perror("bind");
        exit(-1); // 绑定端口失败
    }

    // 监听端口
    listen(listenfd, 5); // 监听端口

    // 创建epoll对象，事件数组，添加
    epoll_event events[MAX_EVENT_NUMBER]; // 创建事件数组
    int epollfd = epoll_create(5); // 创建epoll对象
    if(epollfd < 0){
        perror("epoll_create");
        exit(-1); // 创建epoll对象失败
    }

    // 将监听的文件描述符添加到epoll对象中
    addfd(epollfd, listenfd, false); // 添加监听文件描述符到epoll对象中
    http_conn::m_epollfd = epollfd; // 设置http_conn类的epollfd

    while(true){
        int num = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1); // 等待事件发生
        if(num < 0 && errno != EINTR){ // 等待事件失败
            printf("epoll failure\n");
            break; // 退出循环
        }

        // 遍历所有发生的事件
        for (int i = 0; i < num; ++i){
            int sockfd = events[i].data.fd; // 获取文件描述符
            if(sockfd == listenfd){ // 如果是监听文件描述符
                // 有客户端链接进来
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address); // 客户端地址长度
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength); // 接受连接请求
                if(connfd < 0){
                    perror("accept");
                    continue; // 继续循环
                }

                if(http_conn::m_user_count >= MAX_FD){ // 如果用户数量超过最大值
                    // 目前连接满了
                    // 给客户端写一个信息：服务器内部正忙
                    close(connfd); // 关闭连接
                    continue;
                }
                // 将新的客户数据初始化，放到数组中
                users[connfd].init(connfd, client_address); // 初始化客户端信息
            } else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                //对方异常断开或者错误事件
                users[sockfd].close_conn();
            } else if(events[i].events & EPOLLIN){ // 如果是可读事件
                if(users[sockfd].read()){ // 读取数据成功
                    // 一次性把所有数据都读完
                    pool->append(users + sockfd); // 将请求加入线程池
                } else {
                    users[sockfd].close_conn(); // 关闭连接
                }
            } else if(events[i].events & EPOLLOUT){ // 如果是可写事件
                // 一次性写入所有数据
                if(!users[sockfd].write()){ // 写入数据失败
                    users[sockfd].close_conn(); // 关闭连接
                }
            } else {
                printf("something else happened\n");
            }
        }
    }

    close(epollfd);
    close(listenfd);
    delete [] users;
    delete pool;

    return 0;
}