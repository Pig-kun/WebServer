#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <exception>
#include <semaphore.h>

// 线程同步机制封装类

// 互斥锁类
class locker {
public:
    locker() {
       if (pthread_mutex_init(&m_mutex, NULL) != 0){
        throw std::exception(); // 初始化互斥锁失败
       }; // 初始化互斥锁
    }

    ~locker() {
        pthread_mutex_destroy(&m_mutex); // 销毁互斥锁
    }

    bool lock() {
        return pthread_mutex_lock(&m_mutex) == 0; // 加锁
    }

    bool unlock() {
        return pthread_mutex_unlock(&m_mutex) == 0; // 解锁
    }

    pthread_mutex_t* get() {
        return &m_mutex; // 获取互斥锁
    }

private:
    pthread_mutex_t m_mutex; // 互斥锁
};


// 条件变量类
class cond {
public:
    cond() {
        if (pthread_cond_init(&m_cond, NULL) != 0) {
            throw std::exception(); // 初始化条件变量失败
        } // 初始化条件变量
    }

    ~cond() {
        pthread_cond_destroy(&m_cond); // 销毁条件变量
    }

    bool wait(pthread_mutex_t* m_mutex) {
        return pthread_cond_wait(&m_cond, m_mutex) == 0; // 等待条件变量
    }

    bool timedwait(pthread_mutex_t* m_mutex, struct timespec t) {
        return pthread_cond_timedwait(&m_cond, m_mutex, &t) == 0; // 等待条件变量，带超时
    }

    bool signal() {
        return pthread_cond_signal(&m_cond) == 0; // 唤醒一个等待线程
    }

    bool broadcast() {
        return pthread_cond_broadcast(&m_cond) == 0; // 唤醒所有等待线程
    }
private:
    pthread_cond_t m_cond; // 条件变量
};

// 信号量类
class sem {
public:
    sem() {
        if (sem_init(&m_sem, 0, 0) != 0) {
            throw std::exception(); // 初始化信号量失败
        } // 初始化信号量
    }

    sem(int num) {
        if (sem_init(&m_sem, 0, num) != 0) {
            throw std::exception(); // 初始化信号量失败
        } // 初始化信号量
    }

    ~sem() {
        sem_destroy(&m_sem); // 销毁信号量
    }

    bool wait() {
        return sem_wait(&m_sem) == 0; // 等待信号量
    }

    bool post() {
        return sem_post(&m_sem) == 0; // 增加信号量
    }
private:
    sem_t m_sem; // 信号量
};

#endif