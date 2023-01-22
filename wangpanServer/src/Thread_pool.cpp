#include<Thread_pool.h>


pool_t *thread_pool::Pool_create(int max, int min, int que_max)
{
    pool_t *p = nullptr;
    p = new pool_t;
    if(p == nullptr)
        err_str("new thread_pool error",-1);
    //初始化线程池结构体变量
    p->thread_max   =  max;
    p->thread_min   =  min;
    p->thread_alive =  0;
    p->thread_busy  =  0;
    p->thread_wait  =  0;
    p->thread_shutdown = true; //默认线程池开启
    p->queue_max    =  que_max;
    p->queue_cur    =  0;
    p->queue_front  =  0;
    p->queue_rear   =  0;
    p->queue_task = nullptr; //队列任务
    p->tids = nullptr;//线程数组

    //1. 信号量，锁，初始化，pthread_cond_init,pthread_mutex_init
    if(pthread_cond_init(&p->not_full,nullptr)
       ||pthread_cond_init(&p->not_empty,nullptr)
       ||pthread_mutex_init(&p->lock,nullptr))
        err_str("cond or lock init error",-1);
    //2. 给线程池分配空间
    p->tids = new pthread_t[max];
    if(p->tids == nullptr)
        err_str("new thread pool error",-1);
    //3. 初始化线程池
    bzero(p->tids,sizeof(pthread_t)*max);
    //4. 给任务队列分配空间
    p->queue_task = new st_task[p->queue_max];
    if(!p->queue_task)
        err_str("task_queue malloc failed",-1);
    //5. 创建线程 pthread_create
    int err = 0;
    for(int i = 0;i < min;i++)//按最小值创建
    {
        err = pthread_create(&p->tids[i],nullptr,Custom,(void*)p);
        if(err > 0)
            err_str("create custom error",-1);
        ++p->thread_alive;//存活线程数量+1
    }
    //6. 给管理者线程创建空间
    err = pthread_create(&p->manager_tid,nullptr,Manager,(void*)p);
    if(err > 0)
        err_str("create Manager thread failed",-1);
    //7. 返回创建好的线程池
    return p;
}

int thread_pool::pool_destroy(pool_t *p)
{
    //1. 回收任务队列空间
    delete[] p->queue_task;
    p->queue_task = nullptr;
    //2. 回收p->tids空间
    for(int i=0;i<p->thread_alive;i++)
    {
        if(if_thread_alive(p->tids[i]))
            pthread_join(p->tids[i],nullptr);
    }
    delete[] p->tids;
    p->tids = nullptr;
    //3. 回收p
    delete p;
    p = nullptr;
    return 0;
}

int thread_pool::Producer_add(pool_t *p, void *(task)(void *arg), void *arg)
{
    //1. 上锁
    //进入临界区
    pthread_mutex_lock(&p->lock);
    //2. 判断任务队列满了没
    while(p->queue_cur == p->queue_max && p->thread_shutdown)
        pthread_cond_wait(&p->not_full,&p->lock);//等待not_full条件变量被唤醒
    //3. 线程池是否关闭
    if(!p->thread_shutdown)
    {
        pthread_mutex_unlock(&p->lock);
        return -1;
    }
    p->queue_task[p->queue_front].task = task;
    p->queue_task[p->queue_front].arg = arg;
    p->queue_front = (p->queue_front + 1) % p->queue_max;
    ++p->queue_cur;
    //4. 设置not_empty条件变量,解锁，返回
    pthread_cond_signal(&p->not_empty);
    pthread_mutex_unlock(&p->lock);
    return 0;
}

void *thread_pool::Custom(void *arg)
{
    pool_t *p = (pool_t*)arg;
    st_task task;
    while(p->thread_shutdown)
    {
        //1. 上锁，判断任务队列是否为空，为空,循环等待not_empty条件变量
        pthread_mutex_lock(&p->lock);
        while(p->queue_cur == 0 && p->thread_shutdown)
            pthread_cond_wait(&p->not_empty,&p->lock);
        //2. 判断线程池是否处于开启状态
        if(!p->thread_shutdown)
        {
            pthread_mutex_unlock(&p->lock);
            pthread_exit(nullptr);
        }

        //3. 判断等待的线程数量，判断存活的线程数量
        if(p->thread_wait > 0 && p->thread_alive > p->thread_min)
        {
            --p->thread_wait;
            --p->thread_alive;
            pthread_mutex_unlock(&p->lock);
            pthread_exit(nullptr);
        }

        //4. 取出任务队列中的任务 取出任务，参数，队列中的任务数-1，发送not_full条件变量，处于正在工作的线程状态数-1
        task.task = p->queue_task[p->queue_rear].task;
        task.arg = p->queue_task[p->queue_rear].arg;
        p->queue_rear = (p->queue_rear + 1) % p->queue_max;
        --p->queue_cur;
        pthread_cond_signal(&p->not_full);
        ++p->thread_busy;
        //5. 解锁，执行核心任务
        pthread_mutex_unlock(&p->lock);
        (*task.task)(task.arg);
        //6. 执行任务完毕，在临界区中处于忙状态的线程数-1
        pthread_mutex_lock(&p->lock);
        --p->thread_busy;
        pthread_mutex_unlock(&p->lock);
    }
    return 0;
}

void *thread_pool::Manager(void *arg)
{
    pool_t* p = (pool_t *)arg;
    int alive{},cur{},busy{};
    while(p->thread_shutdown)
    {
        //1. 在临界区中获取存活线程数，正在工作线程数，任务队列中的任务数
        pthread_mutex_lock(&p->lock);
        alive = p->thread_alive;
        cur = p->queue_cur;
        busy = p->thread_busy;
        pthread_mutex_unlock(&p->lock);
        //2. 如果线程池中的正在工作的线程数大于80%，线程池扩容
        bool tmp = (float)busy / (float)alive * 100 >= 80.0 ? true : false;
        if((cur > alive - busy || tmp ) || p->thread_max > alive)
        {
            for(int i=0;i<p->thread_min;i++)
            {
                for(int j=0;j<p->thread_max;i++)
                {
                    if(p->tids[j] == 0 || !if_thread_alive(p->tids[j]))
                    {
                        pthread_mutex_lock(&p->lock);
                        pthread_create(&p->tids[j],nullptr,Custom,(void *)p);
                        ++p->thread_alive;
                        pthread_mutex_unlock(&p->lock);
                        break;
                    }
                }
            }
        }
        //3. 如果 busy * 2 < alive - busy 且 alive > p->thread_min
        //活跃线程数太低，恢复的默认的线程数
        if( busy * 2 < alive - busy && alive > p->thread_min)
        {
            pthread_mutex_lock(&p->lock);
            p->thread_wait = DEF_COUNT;
            pthread_mutex_unlock(&p->lock);
            for(int i=0;i<DEF_COUNT;i++)
                pthread_cond_signal(&p->not_empty);
        }
        //4. 挂起默认时间
        sleep(DEF_TIMEOUT);
    }
    return 0;
}

int thread_pool::if_thread_alive(pthread_t tid)
{
    if((pthread_kill(tid,0))==-1)
    {
        if(errno == ESRCH) //ESRCH 没有这样的进程
            return false;
    }
    return true;
}
