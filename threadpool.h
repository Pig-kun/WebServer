#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <exception>
#include <list>
#include "locker.h"
#include <cstdio>

// 线程池类，定义成模版类为了代码的复用性
template<typename T>
class threadpool {
public:
    threadpool(int thread_number=8, int max_requests = 10000);
    ~threadpool();
    bool append(T* request); // 添加请求到请求队列

private:
    static void* worker(void* arg); // 工作线程，处理请求
    void run(); // 运行线程池

private:
    // 线程的数量
    int m_thread_number;
    
    // 线程池数组，大小为线程数量
    pthread_t* m_threads;

    // 请求队列最多允许的，等待处理的请求数量
    int m_max_requests;
    // 请求队列
    std::list<T*> m_workqueue;

    // 互斥锁
    locker m_queuelocker;

    // 信号量，用于判断是否有任务需要处理
    sem m_queuestat;

    // 线程池是否停止
    bool m_stop;
};

template<typename T>
threadpool<T>::threadpool(int thread_number, int max_requests):
    m_thread_number(thread_number),m_max_requests(max_requests),
    m_stop(false), mthread(NULL){

    if(thread_number <= 0 || max_requests <= 0){
        throw std::exception(); // 线程数量和请求数量必须大于0
    }
    m_threads = new pthread_t[m_thread_number]; // 创建线程池数组
    if(!m_threads){
        throw std::exception(); // 创建线程池数组失败
    }

    // 创建 thread_number 个线程，并将他们设置为线程脱离
    for(int i=0; i < m_thread_number; ++i){
        printf("create the %dth thread\n", i);

        if(pthread_create(m_threads + i, NULL, worker, this) != 0){
            delete[] m_threads; // 删除线程池数组
            throw std::exception(); // 创建线程失败
        }

        if(pthread_detach(m_threads[i]) != 0){ // 设置线程为脱离线程
            delete[] m_threads; // 删除线程池数组
            throw std::exception(); // 设置线程为脱离线程失败
        }
    }
}

template<typename T>
threadpool<T>::~threadpool(){
    delete[] m_threads; // 删除线程池数组
    m_stop = true; // 设置线程池停止
}

template<typename T>
bool threadpool<T>::append(T* request){
    m_queuelocker.lock(); // 上锁
    if(m_workqueue.size() > m_max_requests){ // 请求队列满了
        m_queuelocker.unlock(); // 解锁
        return false; // 返回失败
    }
    m_workqueue.push_back(request); // 将请求加入队列
    m_queuelocker.unlock(); // 解锁
    m_queuestat.post(); // 信号量加1
    return true; // 返回成功
}

template<typename T>
void* threadpool<T>::worker(void* arg){
    threadpool* pool = (threadpool*)arg; // 获取线程池对象
    pool->run(); // 运行线程池
    return pool;
}

template<typename T>
void threadpool<T>::run(){
    while(!m_stop){ // 如果线程池没有停止
        m_queuestat.wait(); 
        m_queuelocker.lock(); // 上锁
        if(m_workqueue.empty()){ // 如果请求队列为空
            m_queuelocker.unlock(); // 解锁
            continue; // 继续循环
        }

        T* request = m_workqueue.front(); // 获取请求队列的第一个请求
        m_workqueue.pop_front(); // 删除第一个请求
        m_queuelocker.unlock(); // 解锁

        if(!request){ // 如果请求为空
            continue; // 继续循环
        }

        request->process(); // 处理请求
    }  
}


#endif