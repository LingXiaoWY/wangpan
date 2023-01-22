#ifndef _BLOCK_EPOLL_NET_H
#define _BLOCK_EPOLL_NET_H

#include<packdef.h>
#include<netinet/tcp.h>
class block_epoll_net;

//阻塞IO epoll

//数据缓存
struct DataBuffer
{
    DataBuffer(block_epoll_net *pNet,int sockfd,char *buf, int nlen)
        :pNet(pNet),sockfd(sockfd),buf(buf),nlen(nlen){}

    block_epoll_net *pNet;
    int sockfd;
    char *buf;
    int nlen;
};


template<class K,class V>
struct MyMap
{
public:
    MyMap(){
        pthread_mutex_init(&m_lock,nullptr);
    }

    //获取的结果 找不到 如果是对象 v 如果是指针  应该是nullptr 规定该函数使用时，调用是确保一定有
    bool find(K k, V &v)
    {
        pthread_mutex_lock(&m_lock);
        if(m_map.count(k) == 0)
        {
            pthread_mutex_unlock(&m_lock);
            return false;
        }
        v = m_map[k];
        pthread_mutex_unlock(&m_lock);
        return true;
    }

    //向事件表中插入事件
    void insert( K k, V v)
    {
        pthread_mutex_lock(&m_lock);
        m_map[k] = v;
        pthread_mutex_unlock(&m_lock);
    }

    //删除事件表中事件
    void erase(K k)
    {
        pthread_mutex_lock(&m_lock);
        m_map.erase(k);
        pthread_mutex_unlock(&m_lock);
    }

    bool IsExist(K k)
    {
        pthread_mutex_lock(&m_lock);
        if(m_map.count(k) > 0)
            return true;
        pthread_mutex_unlock(&m_lock);
        return false;
    }

private:
    pthread_mutex_t m_lock;
    map<K,V>m_map;//epoll就绪事件表
};


//事件结构
struct myevent_s
{
    int fd; // cfd listenfd
    int epoll_fd; //epoll_create 句柄
    int events;
    int status; //1 表示在监听事件，0表示不在
    block_epoll_net *pNet;
    myevent_s(block_epoll_net *pNet)
    {
        this->pNet = pNet;
    }

    void eventset(int fd,int efd/*epoll_create返回的句柄*/)
    {
        this->fd = fd;
        this->events = 0;
        this->status = 0;
        epoll_fd = efd;
    }
    //监听树上添加节点
    void eventadd(int events)
    {
        int op = EPOLL_CTL_MOD;
        struct epoll_event epv = {0,{0}};
        epv.data.ptr = this;
        epv.events = this->events = events;
        if(this->status != 1)
        {
            this->status = 1;
            op = EPOLL_CTL_ADD;
        }
        int ret = epoll_ctl(epoll_fd,op,this->fd,&epv);
        if(ret < 0)
            printf("event add failed [fd=%d], events[%d]\n",this->fd,events);
    }
    //监听树上删除节点
    void eventdel()
    {
        struct epoll_event epv = {0,{0}};
        if(this->status != 1)
            return;
        epv.data.ptr = this;
        this->status = 0;
        epoll_ctl(epoll_fd,EPOLL_CTL_DEL,this->fd,&epv);
    }
};

class block_epoll_net
{
public:
    block_epoll_net(){}
    ~block_epoll_net(){}
    bool InitNet(int port , void (*recv_callback)( int , char* , int ));//初始化 采用回调的方式, 解决数据接收处理
    void EventLoop();//epoll事件循环
    int SendData(int fd, char* szbuf , int nlen );//发送数据
    static void setNonBlockFd( int fd);
    static void setRecvBufSize(int fd);
    static void setSendBufSize(int fd);
    static void setNoDelay(int fd);

private:
    void (*m_recv_callback)( int , char* , int );//接收处理回调函数
    bool InitThreadPool();//初始线程池
    static void *Buffer_Deal(void *arg);//线程函数 处理数据包
    static void *recv_task(void *arg);//线程函数 接收数据
    void accept_event();//epoll事件处理
    void recv_event(myevent_s *ev); //接收: 事件到来recv_event --> 接收数据 recv_task -> 处理 Buffer_Deal
    void epollout_event(myevent_s *ev);
    myevent_s * m_listenEv;//监听套接字对应的是事件
    int m_listenfd;//监听套接字
    int m_epoll_fd;// epoll_create 句柄
    struct epoll_event events[MAX_EVENTS+1];//每一个套接字 对应一个事件结构
    thread_pool *m_threadpool;//线程池相关

    MyMap<int,myevent_s*>m_mapSockfdToEvent;
    pool_t *m_pool;
};

#endif
