#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
#pragma once
#include <iostream>
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
    static const int READ_BUFFER_SIZE = 2048; // 读缓冲区大小
    static const int WRITE_BUFFER_SIZE = 1024;  // 写缓冲区大小
    static const int FILENAME_LEN = 200;        // 文件名的最大长度

    // HTTP请求方法，这里只支持GET
    enum METHOD {GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT};
    
    /*
        解析客户端请求时，主状态机的状态
        CHECK_STATE_REQUESTLINE:当前正在分析请求行
        CHECK_STATE_HEADER:当前正在分析头部字段
        CHECK_STATE_CONTENT:当前正在解析请求体
    */
    enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT };
    
    /*
        服务器处理HTTP请求的可能结果，报文解析的结果
        NO_REQUEST          :   请求不完整，需要继续读取客户数据
        GET_REQUEST         :   表示获得了一个完成的客户请求
        BAD_REQUEST         :   表示客户请求语法错误
        NO_RESOURCE         :   表示服务器没有资源
        FORBIDDEN_REQUEST   :   表示客户对资源没有足够的访问权限
        FILE_REQUEST        :   文件请求,获取文件成功
        INTERNAL_ERROR      :   表示服务器内部错误
        CLOSED_CONNECTION   :   表示客户端已经关闭连接了
    */
    enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };
    
    // 从状态机的三种可能状态，即行的读取状态，分别表示
    // 1.读取到一个完整的行 2.行出错 3.行数据尚且不完整
    enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };

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
    char m_read_buf[READ_BUFFER_SIZE];
    int m_read_idx; // 标识读缓冲区已经读入的客户端数据的最后一个字节的下一个位置

    int m_checked_index; // 当前正在分析的字符在读缓冲区的位置
    int m_start_line; // 当前正在解析的行的起始位置

    char * m_url; //请求目标文件的文件名
    char * m_version; //协议版本，只支持HTTP1.1
    METHOD m_method; //请求方法
    char m_real_file[ FILENAME_LEN ];       // 客户请求的目标文件的完整路径，其内容等于 doc_root + m_url, doc_root是网站根目录
    char * m_host; //主机名
    bool m_linger; //HTTP请求是否保持连接

    int m_content_length;
    
    CHECK_STATE m_check_state;

    char m_write_buf[ WRITE_BUFFER_SIZE ];  // 写缓冲区
    int m_write_idx;                        // 写缓冲区中待发送的字节数
    char* m_file_address;                   // 客户请求的目标文件被mmap到内存中的起始位置
    struct stat m_file_stat;                // 目标文件的状态。通过它我们可以判断文件是否存在、是否为目录、是否可读，并获取文件大小等信息
    struct iovec m_iv[2];                   // 我们将采用writev来执行写操作，所以定义下面两个成员，其中m_iv_count表示被写内存块的数量。
    int m_iv_count;

    void init();

    HTTP_CODE process_read(); // 解析HTTP请求
    HTTP_CODE parse_request_line(char * text); // 解析首行
    HTTP_CODE parse_header(char * text); // 解析头
    HTTP_CODE parse_content(char * text); // 解析内容

    LINE_STATUS parse_line();
    HTTP_CODE do_request();

    bool process_write(HTTP_CODE ret);

    // 这一组函数被process_write调用以填充HTTP应答。
    void unmap();
    bool add_response( const char* format, ... );
    bool add_content( const char* content );
    bool add_content_type();
    bool add_status_line( int status, const char* title );
    bool add_headers( int content_length );
    bool add_content_length( int content_length );
    bool add_linger();
    bool add_blank_line();

    char * get_line() { return m_read_buf + m_start_line; }
};



#endif