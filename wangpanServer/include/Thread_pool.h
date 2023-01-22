#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <packdef.h>
#include <err_str.h>
typedef struct
{
    void *(*task)(void *);
    void *arg;
} st_task; // 任务结构体

typedef struct
{
    int thread_max;           // 最大线程数
    int thread_min;           // 最小线程数
    int thread_alive;         // 线程存活数量
    int thread_busy;          // 处于工作的线程数
    int thread_wait;          // 处于等待的线程数
    bool thread_shutdown;      // 线程关闭
    int queue_max;            // 等待队列的最大数量
    int queue_cur;            // 等待队列中现有的线程数量
    int queue_front;          // 队列头位置
    int queue_rear;           // 队列尾位置
    pthread_cond_t not_full;  // 未满信号量
    pthread_cond_t not_empty; // 非空信号量
    pthread_mutex_t lock;     // 锁
    st_task *queue_task;      // 任务队列
    pthread_t *tids;          // 线程数组
    pthread_t manager_tid;    // 管理者线程
} pool_t;

class thread_pool
{
public:
    pool_t *Pool_create(int, int, int);                    // 线程池创建函数
    int pool_destroy(pool_t *);                            // 线程池销毁函数
    int Producer_add(pool_t *, void *(*)(void *), void *); // 生产者添加
    static void *Custom(void *);                           // 消费
    static void *Manager(void *);                          // 调度，管理
    static int if_thread_alive(pthread_t);                 // 查看线程是否存活
};

#endif
