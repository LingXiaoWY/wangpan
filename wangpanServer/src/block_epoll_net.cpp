#include<block_epoll_net.h>
#include<clogic.h>

bool block_epoll_net::InitNet(int port, void (*recv_callback)(int, char *, int))
{
    m_recv_callback = recv_callback;
    InitThreadPool();

    int flags = 1;
    int ret = 0;
    m_listenEv = new myevent_s(this);

    //1. 创建套接字
    m_listenfd = socket(AF_INET,SOCK_STREAM,0);
    if(m_listenfd == -1)
    {
        perror("create socket error");
        return false;
    }

    //set reuseraddr
    ret = setsockopt(m_listenfd,SOL_SOCKET,SO_REUSEADDR,(char*)&flags,sizeof(flags));
    if(ret == -1)
    {
        perror("setsockopt error");
        return false;
    }

    //监听套接字m_listenfd 采用LT非阻塞模式
    //set nonblock
    setNonBlockFd(m_listenfd);

    struct sockaddr_in local_addr;
    bzero(&local_addr,sizeof(sockaddr_in));

    //set address
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    //绑定 addr
    ret = bind(m_listenfd,(struct sockaddr*)&local_addr,sizeof(struct sockaddr));

    if(ret == -1)
    {
       perror("bind error");
       close(m_listenfd);
       return false;
    }

    ret = listen(m_listenfd,128);
    if(ret == -1)
    {
        perror("listen error");
        close(m_listenfd);
        return false;
    }

    //create epoll
    m_epoll_fd = epoll_create(MAX_EVENTS);

    m_listenEv->eventset(m_listenfd,m_epoll_fd);
    //将监听套接字 添加到epoll中，模式LT 非阻塞
    m_listenEv->eventadd(EPOLLIN);

    return true;
}

void block_epoll_net::EventLoop()
{
    cout<<"Server running"<<endl;
    while(true)
    {
        //等待事件发生
        int nfd = epoll_wait(m_epoll_fd,events,MAX_EVENTS + 1,1000);
        if(nfd < 0)
        {
            printf("epoll_wait error, exit\n");
            continue;
        }
        for(int i=0;i<nfd;i++)
        {
            struct myevent_s *ev = (struct myevent_s*)events[i].data.ptr;
            int fd = ev->fd;
            if((events[i].events & EPOLLIN))
            {
                if(fd == m_listenfd)
                    accept_event();
                else
                    recv_event(ev);
            }
            if((events[i].events & EPOLLOUT))
                epollout_event(ev);
        }
    }
}

int block_epoll_net::SendData(int fd, char *szbuf, int nlen)
{
    //发送不可分割 避免多线程并发
    //先发包大小 再发包内容 一次放入缓冲区
    /*
         +--------------+------------------+---------------------+
         |<-- 4bytes -->|<-- 4bytes协议头->|<--  协议其他内容 -->|
         +--------------+------------------+---------------------+
         |<- packsize ->|<------------ 数据包 struct ----------->|
     */
    int nPackSize = nlen + 4;
    vector<char>vecbuf(nPackSize,0);

    char *buf = & *vecbuf.begin();
    char *tmp = buf;
    *(int *)tmp = nlen; //按四个字节int写入
    tmp += sizeof(int);
    memcpy(tmp,szbuf,nlen);
    int res = send(fd,(const char *)buf,nPackSize,0);
    return res;
}

void block_epoll_net::setNonBlockFd(int fd)
{
    int flags = fcntl(fd,F_GETFL,0);
    int ret = fcntl(fd,F_SETFL,flags | O_NONBLOCK);
    if(ret == -1)
        perror("setNonBlockFd failed");
}

void block_epoll_net::setRecvBufSize(int fd)
{
    //接收缓冲区
    int nRecvBuf = 256 * 1024; //设置为256K
    setsockopt(fd,SOL_SOCKET,SO_RCVBUF,(const char *)&nRecvBuf,sizeof(int));
}

void block_epoll_net::setSendBufSize(int fd)
{
    //发送缓冲区
    int nSendBuf = 128 * 1024;
    setsockopt(fd,SOL_SOCKET,SO_SNDBUF,(const char *)&nSendBuf,sizeof(int));
}

void block_epoll_net::setNoDelay(int fd)
{
    int value = 1;
    setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char *)&value,sizeof(int));
}

bool block_epoll_net::InitThreadPool()
{
    m_pool = nullptr;
    m_threadpool = new thread_pool;

    //创建拥有10个线程的线程池 最大线程数200 环形队列最大值50000
    m_pool = m_threadpool->Pool_create(200,10,50000);
    if(m_pool == nullptr)
    {
        perror("create Thread_Pool Failed");
        exit(-1);
    }
    return true;
}

void *block_epoll_net::Buffer_Deal(void *arg)
{
    DataBuffer *buffer = (DataBuffer *)arg;
    if(!buffer)return nullptr;
    buffer->pNet->m_recv_callback(buffer->sockfd,buffer->buf,buffer->nlen);

    if(buffer->buf != nullptr)
    {
        delete[] buffer->buf;
        buffer->buf = nullptr;
    }
    delete buffer;
    return 0;
}

void *block_epoll_net::recv_task(void *arg)
{
    myevent_s *ev = (myevent_s*)arg;

    //利用全局指针 方便操作
    block_epoll_net *pthis = ev->pNet;

    //接收和处理分离epoll_ctl
    int nRelReadNum = 0;
    int nPackSize = 0;
    char *pSzBuf = nullptr;
    nRelReadNum = read(ev->fd,&nPackSize,sizeof(nPackSize));
    if(nRelReadNum <= 0)
    {
        ev->eventdel();
        close(ev->fd);
        //回收event结构
        pthis->m_mapSockfdToEvent.erase(ev->fd);
        delete ev;
    }
    else
    {
        pSzBuf = new char[nPackSize];
        int nOffSet = 0;
        nRelReadNum = 0;
        while(nPackSize)
        {
            nRelReadNum = recv(ev->fd,pSzBuf+nOffSet,nPackSize,0);
            if(nRelReadNum <= 0)
                break;
            nOffSet += nRelReadNum;
            nPackSize -= nRelReadNum;
        }
        //接收和处理分离 跑线程池里其他线程处理，避免处理影响接收
        DataBuffer *buffer = new DataBuffer(ev->pNet,ev->fd,pSzBuf,nOffSet);
        pthis->m_threadpool->Producer_add(pthis->m_pool,Buffer_Deal,(void*)buffer);

        //这次接收完 要重新注册事件 此时epoll mode -> epollin | epolloneshot 没有修改，使用重复值
        ev->eventadd(ev->events);
        return 0;
    }
    return nullptr;
}

void block_epoll_net::accept_event()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int clientfd = accept(m_listenfd,(struct sockaddr *)&addr,&len);
    if(clientfd == -1)
    {
        printf("%s :accept,%s\n",__func__,strerror(errno));
        return;
    }

    //设置接收缓冲区大小
    setRecvBufSize(clientfd);
    //设置发送缓冲区大小
    setSendBufSize(clientfd);
    //设置 无延迟
    setNoDelay(clientfd);

    myevent_s *clientEv = new myevent_s(this);
    clientEv->eventset(clientfd,m_epoll_fd);
    //使用EPOLLONESHOT epoll监听不会重复检测 避免多线程并发 使得同一个套接字接收是排队的
    clientEv->eventadd(EPOLLIN|EPOLLONESHOT);

    m_mapSockfdToEvent.insert(clientfd,clientEv);

    printf("new connect [%s:%d][time:%ld]\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),time(nullptr));
    return;
}

void block_epoll_net::recv_event(myevent_s *ev)
{
    //接收事件采用线程池，多线程处理，由于使用EPOLLONESHOT，避免同一套接字的线程并发问题
    m_threadpool->Producer_add(m_pool,recv_task,(void *)ev);
}

void block_epoll_net::epollout_event(myevent_s *ev)
{

}





