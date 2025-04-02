#include "http_conn.h"


int http_conn::m_epollfd = -1;
int http_conn::m_user_count = 0;

// 设置文件描述符非阻塞 
void setnonblocking(int fd){
    int old_flag = fcntl(fd, F_GETFL);
    int new_flag = old_flag | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_flag);
}

// 向epoll对象中添加文件描述符
void addfd(int epollfd, int fd, bool one_shot){
    epoll_event event;
    event.data.fd = fd; // 设置事件的文件描述符
    event.events = EPOLLIN | EPOLLRDHUP; // 设置事件的类型

    if(one_shot){ // 如果是单次事件
        event.events |= EPOLLONESHOT; // 设置为单次事件
    }

    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event); // 添加事件到epoll对象中
    // 设置文件描述符非阻塞
    setnonblocking(fd);
}

// 从epoll对象中删除文件描述符
void removefd(int epollfd, int fd){
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0); // 删除事件
    close(fd); // 关闭文件描述符
}

// 修改文件描述符， 重置socket的EPOLLONESHOT事件,以确保下一次可读时，EPOLLIN事件能被触发
void modfd(int epollfd, int fd, int ev){
    epoll_event event;
    event.data.fd = fd; // 设置事件的文件描述符
    event.events = ev | EPOLLONESHOT | EPOLLRDHUP; // 设置事件的类型

    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event); // 修改事件
}

// 初始化连接
void http_conn::init(int sockfd, const sockaddr_in& addr){
    m_sockfd = sockfd; 
    m_address = addr; 
    int reuse = 1;
    setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)); // 设置端口复用
    
    // 添加socket到epoll对象中
    addfd(m_epollfd, sockfd, true); 
    m_user_count++; // 总用户数量加1
}

// 关闭连接
void http_conn::close_conn(){
    if(m_sockfd != -1){
        removefd(m_epollfd, m_sockfd);
        m_sockfd = -1;
        m_user_count--; // 关闭一个连接，客户总数量-1
    }
}

bool http_conn::read(){
    printf("read once\n");
    return true;
}

bool http_conn::write(){
    printf("write once\n");
    return true;
}

//由线程池中工作线程调用，这是处理HTTP请求的入口函数
void http_conn::process(){

    // 解析HTTP请求

    printf("parse request\n");
    
    modfd(m_epollfd, m_sockfd, EPOLLIN);
    //生成响应
}