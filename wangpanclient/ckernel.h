#ifndef CKERNEL_H
#define CKERNEL_H

#include "login.h"
#include "mainwindow.h"
#include <QObject>
#include <vector>
#include "INetMediator.h"
#include "packdef.h"
#include <QDebug>
#include <QString>
#include <list>
#include "TcpClientMediator.h"
#include <QFileDialog>
#include <QDateTime>
#include <map>


class CKernel;
typedef void (CKernel::*PFUN)(char *szbuf,SOCKET sock);

class CKernel : public QObject
{
    Q_OBJECT
private:
    explicit CKernel(QObject *parent = 0);
    ~CKernel();
    void setNetPackMap();
    void ConfigSet();
public:
    static CKernel *getInstance()
    {
        static CKernel ckernel;
        return &ckernel;
    }
    SendData(char *buf, int nlen);//发送数据

private:
    login *m_plogin;//登录窗口指针
    MainWindow *m_pMainWindow;//主窗口指针
    std::vector<PFUN>m_netPackFunMap;//协议映射表 协议头与处理函数的对应关系
    INetMediator *m_client;
    char m_serverIP[20];
    QString m_userName;
    int m_uid;//用户ID
    std::map<QString,STRU_FILE_INFO*>m_uploadfileinfo;
    std::map<QString,STRU_FILE_INFO*>m_downloadfileinfo;
    std::vector<QString>m_filenameVec;
signals:

public slots:
    void slot_getFileRq();//发送获取文件列表请求
    void slot_RegisterRq(QString name,QString tel,QString password);//发送注册请求
    void slot_LoginRq(QString tel,QString password);//发送登录请求
    void slot_UpLoadFileHeaderRq();//上传文件头请求(文件名,id,路径,md5)
    void slot_UpLoadFileContentRq(char *szbuf , SOCKET sock);//上传文件内容请求
    void slot_DownloadRq(QString filename);//下载文件请求
    void slot_DeleteFileRq(QString filename);//删除文件
    void slot_dealRegisterRq(char *szbuf,SOCKET sock);//处理注册回复
    void slot_dealLoginRq(char *szbuf,SOCKET sock);//处理登录回复
    void slot_dealgetFileRq(char *szbuf,SOCKET sock);//处理文件列表回复
    void slot_dealgetUploadFileRq(char *szbuf,SOCKET sock);//处理上传文件请求
    void slot_dealUpLoadFileContentRq(char *szbuf,SOCKET sock);//处理上传文件内容请求
    void slot_dealDeleteFileRq(char *szbuf,SOCKET sock);//处理删除请求
    void slot_dealDownloadRq(char *szbuf,SOCKET sock);//处理下载请求
    void slot_ReadyData(unsigned int lSendIP, char *buf, int nlen);
};

#endif // CKERNEL_H
