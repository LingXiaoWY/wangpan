#ifndef _CLOGIC_H
#define _CLOGIC_H

#include "TcpKernel.h"


class CLogic
{
public:
    CLogic(TcpKernel *pkernel);
    void GetFileListRq(sock_fd clientfd, char*szbuf, int nlen);//获取用户文件信息列表
    void slot_dealRegisterRq(sock_fd clientfd, char*szbuf, int nlen);//发送注册请求
    void slot_dealLoginRq(sock_fd clientfd, char*szbuf, int nlen);//发送登录请求
    void slot_dealUpLoadFileHeaderRq(sock_fd clientfd, char*szbuf, int nlen);//上传文件头请求(文件名,id,路径,md5)
    void slot_dealUpLoadFileContentRq(sock_fd clientfd, char*szbuf, int nlen);//上传文件内容请求
    void slot_dealDownloadRq(sock_fd clientfd, char*szbuf, int nlen);//下载文件请求
    void slot_dealUploadContinueRq(sock_fd clientfd, char*szbuf, int nlen);//续传文件请求
    void slot_dealDeleteFileRq(sock_fd clientfd, char*szbuf, int nlen);//删除文件请求
    void SendData(sock_fd clientfd, char*szbuf, int nlen);//发送数据
    void setNetPackMap();

private:
    TcpKernel* m_pKernel;
    CMysql * m_sql;
    block_epoll_net * m_tcp;
    MyMap<int,UserInfo*>m_mapIdToUserInfo;
    vector<list<int>>m_roomUserlist;
    pthread_mutex_t m_roomListMutex;
    vector<PFUN>m_netPackFunMap;
    map<string,STRU_FILE_INFO*>m_uploadingFile;
    map<string,STRU_FILE_INFO*>m_downloadFile;
};

#endif
