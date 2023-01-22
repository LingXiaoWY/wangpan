#include<TcpKernel.h>
#include<clogic.h>

int TcpKernel::Open(int port)
{
    initRand();

    m_sql = new CMysql;

    //数据库 使用127.0.0.1地址 用户名guo 密码123456 数据库 wangpan 没有的话进行创建
    if(!m_sql->ConnectMysql(DEF_DB_IP,DEF_DB_USER,DEF_DB_PWD,DEF_DB_NAME))
    {
        printf("Connect Mysql Failed...\n");
        return false;
    }
    else
    {
        cout<<"MySQL Connect Success...\n"<<endl;
    }
    //初始化网络
    m_tcp = new block_epoll_net;
    bool res = m_tcp->InitNet(port,&TcpKernel::DealData);
    if(!res)
        err_str("net init fail",-1);
    m_logic = new CLogic(this);
    setNetPackMap();

    return true;
}

void TcpKernel::initRand()
{
    struct timeval time;
    gettimeofday(&time,nullptr);
    srand(time.tv_sec + time.tv_usec);
}

void TcpKernel::setNetPackMap()
{
    //清空映射
    bzero(m_NetPackMap,sizeof(m_NetPackMap));
    //协议映射赋值setNetPackMap
    m_logic->setNetPackMap();
}

void TcpKernel::Close()
{
    m_sql->DisConnect();
}

void TcpKernel::DealData(sock_fd clientfd, char *szbuf, int nlen)
{
    PackType type = *(PackType*)szbuf;
    if((type >= DEF_PACK_BASE) && (type < DEF_PACK_BASE + DEF_PACK_COUNT))
    {
        PFUN pf = NetPackMap(type);
        if(pf)
            (TcpKernel::GetInstance()->m_logic->*pf)(clientfd,szbuf,nlen);
    }
    return;
}

void TcpKernel::EventLoop()
{
    cout<<"event loop"<<endl;
    m_tcp->EventLoop();
}

void TcpKernel::SendData(sock_fd clientfd, char *szbuf, int nlen)
{
    m_tcp->SendData(clientfd,szbuf,nlen);
}

TcpKernel::TcpKernel()
{

}

TcpKernel::~TcpKernel()
{
    if(m_logic)delete m_logic;
}

