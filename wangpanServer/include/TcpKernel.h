#ifndef TCPKERNEL_H
#define TCPKERNEL_H

#include <Mysql.h>
#include <block_epoll_net.h>
#include <packdef.h>
#include <sys/time.h>

class TcpKernel;
#define NetPackMap(a) TcpKernel::GetInstance()->m_NetPackMap[a - DEF_PACK_BASE]


class CLogic;
typedef int sock_fd;
typedef void (CLogic::*PFUN)(sock_fd, char *, int);


class TcpKernel
{
public:
    static TcpKernel *GetInstance()
    {
        static TcpKernel kernel;
        return &kernel;
    }
    int Open(int port);//开启核心服务
    void initRand();//初始化随机数
    void setNetPackMap();//设置协议映射
    void Close();//关闭核心服务
    static void DealData(sock_fd clientfd, char *szbuf, int nlen);//处理网络接收
    void EventLoop();//事件循环
    void SendData(sock_fd clientfd, char *szbuf, int nlen);//发送数据

public:
    CLogic *m_logic;
    PFUN m_NetPackMap[DEF_PACK_COUNT]; // 协议映射表
private:
    TcpKernel();
    ~TcpKernel();
    CMysql *m_sql;//数据库
    block_epoll_net *m_tcp;//网络
    friend class CLogic;
};

#endif
