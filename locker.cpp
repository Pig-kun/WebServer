#include "locker.h"

locker::locker() {
    if (pthread_mutex_init(&m_mutex, NULL) != 0){
     throw std::exception(); // 初始化互斥锁失败
    }; // 初始化互斥锁
}

locker::~locker() {
     pthread_mutex_destroy(&m_mutex); // 销毁互斥锁
}

bool locker::lock() {
     return pthread_mutex_lock(&m_mutex) == 0; // 加锁
}

bool locker::unlock() {
     return pthread_mutex_unlock(&m_mutex) == 0; // 解锁
}

pthread_mutex_t* locker::get() {
     return &m_mutex; // 获取互斥锁
}


cond::cond() {
    if (pthread_cond_init(&m_cond, NULL) != 0) {
        throw std::exception(); // 初始化条件变量失败
    } // 初始化条件变量
}

cond::~cond() {
    pthread_cond_destroy(&m_cond); // 销毁条件变量
}

bool cond::wait(pthread_mutex_t* m_mutex) {
    return pthread_cond_wait(&m_cond, m_mutex) == 0; // 等待条件变量
}

bool cond::timedwait(pthread_mutex_t* m_mutex, struct timespec t) {
    return pthread_cond_timedwait(&m_cond, m_mutex, &t) == 0; // 等待条件变量，带超时
}

bool cond::signal() {
    return pthread_cond_signal(&m_cond) == 0; // 唤醒一个等待线程
}

bool cond::broadcast() {
    return pthread_cond_broadcast(&m_cond) == 0; // 唤醒所有等待线程
}



sem::sem() {
    if (sem_init(&m_sem, 0, 0) != 0) {
        throw std::exception(); // 初始化信号量失败
    } // 初始化信号量
}

sem::sem(int num) {
    if (sem_init(&m_sem, 0, num) != 0) {
        throw std::exception(); // 初始化信号量失败
    } // 初始化信号量
}

sem::~sem() {
    sem_destroy(&m_sem); // 销毁信号量
}

bool sem::wait() {
    return sem_wait(&m_sem) == 0; // 等待信号量
}

bool sem::post() {
    return sem_post(&m_sem) == 0; // 增加信号量
}