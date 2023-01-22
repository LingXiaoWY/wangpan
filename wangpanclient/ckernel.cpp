#include "ckernel.h"
#include "md5.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

#define NetPackMap(a)  m_netPackFunMap[ a - DEF_PACK_BASE ]

//获得MD5函数
#define MD5_KEY 1234
static std::string getMD5(QString val)
{
    QString tmp = QString("%1_%2").arg(val).arg(MD5_KEY);
    MD5 md(tmp.toStdString());
    return md.toString();
}

CKernel::CKernel(QObject *parent) : QObject(parent),m_netPackFunMap(100),m_uid(2)
{
    ConfigSet();
    setNetPackMap();
    m_plogin = new login;
    m_pMainWindow = new MainWindow;
    m_plogin->show();
    connect(m_plogin,SIGNAL(SIG_Login(QString,QString)),this,SLOT(slot_LoginRq(QString,QString)));
    connect(m_plogin,SIGNAL(SIG_Register(QString,QString,QString)),this,SLOT(slot_RegisterRq(QString,QString,QString)));
    connect(m_pMainWindow,SIGNAL(SIG_getFile()),this,SLOT(slot_getFileRq()));
    connect(m_pMainWindow,SIGNAL(SIG_upLoad()),this,SLOT(slot_UpLoadFileHeaderRq()));
    connect(m_pMainWindow->m_pCSelectFile,SIGNAL(SIG_deleteFile(QString)),this,SLOT(slot_DeleteFileRq(QString)));
    connect(m_pMainWindow->m_pCSDownFile,SIGNAL(SIG_downloadFile(QString)),this,SLOT(slot_DownloadRq(QString)));

    m_client = new TcpClientMediator;
    m_client->OpenNet(m_serverIP,_DEF_TCP_PORT);
    connect(m_client,SIGNAL(SIG_ReadyData(uint,char*,int))
           ,this,SLOT(slot_ReadyData(uint,char*,int)));
}

CKernel::~CKernel()
{
    if(m_plogin)
        delete m_plogin;
    if(m_pMainWindow)
        delete m_pMainWindow;
    m_plogin = nullptr;
    m_pMainWindow = nullptr;
}

void CKernel::setNetPackMap()
{
    NetPackMap(_DEF_PACK_REGISTER_RQ) = &CKernel::slot_dealRegisterRq;
    NetPackMap(_DEF_PACK_LOGIN_RQ) = &CKernel::slot_dealLoginRq;
    NetPackMap(_DEF_PACK_GETFILELIST_RQ) = &CKernel::slot_dealgetFileRq;
    NetPackMap(_DEF_PACK_UPLOADFILE_RQ) = &CKernel::slot_dealgetUploadFileRq;
    NetPackMap(_DEF_PACK_UPLOADFILECONTENT_RQ) = &CKernel::slot_dealUpLoadFileContentRq;
    NetPackMap(_DEF_PACK_DELETEFILE_RQ) = &CKernel::slot_dealDeleteFileRq;
    NetPackMap(_DEF_PACK_DOWNLOADFILE_RQ) = &CKernel::slot_dealDownloadRq;
}

#include<QSettings>
#include<QCoreApplication>
#include<QFileInfo>
void CKernel::ConfigSet()
{
    //获取配置文件里的信息以及设置
    //.ini 配置文件
    //[net] 组名 GroupName
    //key = value
    //1. ip默认
    strcpy(m_serverIP,DEF_SERVER_IP);
    //2. 设置和获取配置文件 设置和exe同级的目录
    QString path = QCoreApplication::applicationDirPath() + "/config.ini";

    //查看路径是否存在
    QFileInfo info(path);
    if(info.exists())
    {
        //存在
        QSettings setting(path,QSettings::IniFormat,nullptr);
        //[net]组写入值
        setting.beginGroup("net");
        QVariant var = setting.value("ip");
        QString strip = var.toString();
        if(!strip.isEmpty())
            strcpy(m_serverIP,strip.toStdString().c_str());
        setting.endGroup();
    }
    else
    {
        //不存在
        QSettings setting(path,QSettings::IniFormat,nullptr);

        //[net]组写入值
        setting.beginGroup("net");
        setting.setValue("ip",QString::fromStdString(m_serverIP));
        setting.endGroup();
    }
}

CKernel::SendData(char *buf, int nlen)
{
    m_client->SendData(0,buf,nlen);
}

void CKernel::slot_getFileRq()
{
    STRU_GETFILELIST_RQ rq;
    rq.id = m_uid;
    SendData((char*)&rq,sizeof(rq));
}

void CKernel::slot_RegisterRq(QString name,QString tel,QString password)//发送注册请求
{
    STRU_REGISTER_RQ rq;
    strcpy(rq.name,name.toStdString().c_str());
    strcpy(rq.tel,tel.toStdString().c_str());
    strcpy(rq.password,getMD5(password).c_str());
    m_plogin->on_pb_clear_register_clicked();
    SendData((char*)&rq,sizeof(rq));
}

void CKernel::slot_LoginRq(QString tel,QString password)//发送登录请求
{
    STRU_LOGIN_RQ rq;
    strcpy(rq.tel,tel.toStdString().c_str());
    strcpy(rq.password,getMD5(password).c_str());
    SendData((char*)&rq,sizeof(rq));
}

void CKernel::slot_UpLoadFileHeaderRq()//上传文件头请求(文件名,id,路径,md5)
{
    STRU_UPLOADFILE_RQ rq;
    STRU_FILE_INFO *fileinfo = new STRU_FILE_INFO;
    QString file_path = QFileDialog::getOpenFileName(m_pMainWindow,tr("请选择要上传的文件"),".../","*.txt");
    QFileInfo info(file_path);
    if(info.size() == 0)
        return;
    QDateTime dateTime = QDateTime::currentDateTime();
    rq.uid = m_uid;//用户id
    fileinfo->filesize = rq.filesize = info.size();
    fileinfo->path = file_path;
    strcpy(rq.filename,info.fileName().toStdString().c_str());
    strcpy(rq.uploadtime,dateTime.toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str());
    strcpy(rq.MD5,getMD5(info.fileName()).c_str());//MD5
    strcpy(fileinfo->MD5,rq.MD5);
    m_uploadfileinfo[QString(rq.MD5)] = fileinfo;
    SendData((char*)&rq,sizeof(rq));
}

void CKernel::slot_UpLoadFileContentRq(char* szbuf , SOCKET sock)//上传文件内容请求
{
    STRU_UPLOADFILE_RS *rs = (STRU_UPLOADFILE_RS *)szbuf;
    STRU_UPLOADFILECONTENT_RQ rq;
    STRU_FILE_INFO *fileinfo = m_uploadfileinfo[QString(rs->MD5)];
    QFile file(fileinfo->path);
    bool isok = file.open(QIODevice::ReadOnly | QIODevice::Text); //只读模式打开
    if(isok == true)
    {
        //读文件
        while(file.atEnd() == false){
            rq.nlen = file.readLine(rq.data,1000);
            fileinfo->sendfilesize += rq.nlen;
            strcpy(rq.MD5,fileinfo->MD5);
            if(file.atEnd())
               rq.flag = true;
            SendData((char*)&rq,sizeof(rq));
        }
    }
    file.close();
}

void CKernel::slot_DownloadRq(QString filename)//下载文件请求
{
    m_pMainWindow->m_pCSDownFile->hide();
    STRU_DOWNLOADFILE_RQ rq;
    STRU_FILE_INFO *file = new STRU_FILE_INFO;
    rq.uid = m_uid;
    strcpy(rq.MD5,getMD5(filename).c_str());
    strcpy(file->MD5,rq.MD5);
    file->path = QFileDialog::getExistingDirectory(m_pMainWindow, "请选择文件保存路径", "./") + "/" + filename;
    m_downloadfileinfo[QString(rq.MD5)] = file;
    SendData((char*)&rq,sizeof(rq));
}

void CKernel::slot_DeleteFileRq(QString filename)//删除文件请求
{
    m_pMainWindow->m_pCSelectFile->hide();
    STRU_DELETEFILE_RQ rq;
    rq.uid = m_uid;
    strcpy(rq.MD5,getMD5(filename).c_str());
    SendData((char*)&rq,sizeof(rq));
}

void CKernel::slot_dealRegisterRq(char *szbuf,SOCKET sock)//处理注册回复
{
    //解析数据包
    STRU_REGISTER_RS * rs = (STRU_REGISTER_RS *)szbuf;

    //根据结果弹窗
    switch(rs->result)
    {
    case tel_is_exist:
        QMessageBox::about(this->m_plogin,"注册提示","注册失败，手机号已存在");
        break;
    case name_is_exist:
        QMessageBox::about(this->m_plogin,"注册提示","注册失败，昵称已存在");
        break;
    case register_success:
        QMessageBox::about(this->m_plogin,"注册提示","注册成功");
        break;
    default:
        QMessageBox::about(this->m_plogin,"注册提示","注册异常");
        break;
    }
}

void CKernel::slot_dealLoginRq(char *szbuf,SOCKET sock)//处理登录回复
{
    //拆包
    STRU_LOGIN_RS * rs = (STRU_LOGIN_RS *)szbuf;
    //根据不同结果显示
    switch(rs->result)
    {
    case user_not_exist:
        QMessageBox::about(m_plogin,"提示","用户不存在，登录失败");
        break;
    case password_error:
        QMessageBox::about(m_plogin,"提示","密码错误，登录失败");
        break;
    case login_success:
        //ui 切换
        m_plogin->hide();
        m_pMainWindow->show();
        //存储
        m_uid = rs->userid;
        m_userName = QString::fromStdString(rs->name);
        break;
    default:
        QMessageBox::about(m_plogin,"提示","登录异常");
        break;
    }

}

void CKernel::slot_ReadyData(unsigned int lSendIP, char *buf, int nlen)
{
    //协议映射表
    PackType type = *(int *)buf; //*(int *)按照整型取数据
    if(type >= DEF_PACK_BASE && type < DEF_PACK_BASE + _DEF_PROTOCOL_COUNT)
    {
        //根据协议头跳到对应的函数中
        PFUN pf = NetPackMap(type);
        (this->*pf)(buf,nlen);
    }
    //buf 要回收
    delete[] buf;
    buf = nullptr;
}

void CKernel::slot_dealgetFileRq(char *szbuf, SOCKET sock)
{
    STRU_GETFILELIST_RS * rs = (STRU_GETFILELIST_RS *)szbuf;
    if(rs->result == GET_FAILED)
    {
        QMessageBox::about(m_pMainWindow,"提示","获取文件列表失败，请检查网络");
        exit(-1);
    }
    else
    {
        if(rs->filenum == 0 || rs->filenum != m_pMainWindow->m_tableModel1->rowCount())
            for(int i=0;i<m_pMainWindow->m_tableModel1->rowCount();i++)
                m_pMainWindow->m_tableModel1->removeRow(i);
        m_filenameVec.resize(rs->filenum);
        for(int i=0;i<rs->filenum;i++)
        {
            m_filenameVec[i] = rs->resVec[i].filename;
            m_pMainWindow->m_tableModel1->setItem(i,0,new QStandardItem(rs->resVec[i].filename));
            m_pMainWindow->m_tableModel1->setItem(i,1,new QStandardItem(rs->resVec[i].filesize));
            m_pMainWindow->m_tableModel1->setItem(i,2,new QStandardItem(rs->resVec[i].uploadtime));
        }
        m_pMainWindow->setFileNum(rs->filenum);
        m_pMainWindow->m_pCSelectFile->slot_SelectFile(m_filenameVec);
        m_pMainWindow->m_pCSDownFile->slot_SelectFile(m_filenameVec);
    }
}

void CKernel::slot_dealgetUploadFileRq(char *szbuf, SOCKET sock)
{
    STRU_UPLOADFILE_RS *rs = (STRU_UPLOADFILE_RS *)szbuf;
    switch(rs->result)
    {
    case UPLOADRQ_SUCCESS:
        slot_UpLoadFileContentRq(szbuf,sock);
        break;
    case UPLOADRQ_FAILED:
    {
        QMessageBox::about(m_pMainWindow,"提示","请检查网络，重新上传");
        m_uploadfileinfo.erase(QString(rs->MD5));
    }
    break;
    default:
        QMessageBox::about(m_pMainWindow,"提示","上传异常");
        break;
    }
}

void CKernel::slot_dealUpLoadFileContentRq(char *szbuf, SOCKET sock)
{
    STRU_UPLOADFILECONTENT_RS *rs = (STRU_UPLOADFILECONTENT_RS *)szbuf;
    switch (rs->result) {
    case UPLOADRQ_SUCCESS:
    {
        auto ite = m_uploadfileinfo.begin();
        while(ite != m_uploadfileinfo.end())
        {
            if(ite->first == QString(rs->MD5))
            {
                delete m_uploadfileinfo[QString(rs->MD5)];
                m_uploadfileinfo[QString(rs->MD5)] = nullptr;
                ite = m_uploadfileinfo.erase(ite);
                break;
            }
            ++ite;
        }
        QMessageBox::about(m_pMainWindow,"提示","上传成功");
    }break;
    case UPLOADRQ_FAILED:
        QMessageBox::about(m_pMainWindow,"提示","上传失败，请选择续传文件或者重新选择上传文件");
        break;
    default:
        break;
    }
}

void CKernel::slot_dealDeleteFileRq(char *szbuf, SOCKET sock)
{
    STRU_UPLOADFILE_RS *rs = (STRU_UPLOADFILE_RS *)szbuf;

    switch(rs->result)
    {
    case DELETE_SUCCESS:
    {
        QMessageBox::about(m_pMainWindow,"提示","删除成功");
        if(m_uploadfileinfo.count(QString(rs->MD5)))
            m_uploadfileinfo.erase(QString(rs->MD5));
    }break;
    case DELETE_FAILED:
        QMessageBox::about(m_pMainWindow,"提示","请检查网络，重新选择删除");
        break;
    default:
        QMessageBox::about(m_pMainWindow,"提示","删除异常");
        break;
    }
}

void CKernel::slot_dealDownloadRq(char *szbuf, SOCKET sock)
{
    STRU_DOWNLOADFILE_RS *rs = (STRU_DOWNLOADFILE_RS*)szbuf;
    if(rs->result == DOWNLOAD_FAILED)
    {
        QMessageBox::about(m_pMainWindow,"提示","下载失败，请检查网络");
        return;
    }
    else
    {

        STRU_FILE_INFO * pfile = m_downloadfileinfo[QString(rs->MD5)];
        QFile file(pfile->path);
        bool isok = file.open(QIODevice::WriteOnly| QIODevice::Append | QIODevice::Text); //只写模式打开
        if(isok == true)
        {
            QTextStream streamFileOut(&file);
            streamFileOut.setCodec("UTF-8");
            streamFileOut << QString(rs->data);
            streamFileOut.flush();
        }
        file.close();
        if(rs->flag)
        {
            QMessageBox::about(m_pMainWindow,"提示","下载成功");
            auto ite = m_downloadfileinfo.begin();
            while(ite != m_downloadfileinfo.end())
            {
                if(ite->first == QString(rs->MD5))
                {
                    delete m_downloadfileinfo[QString(rs->MD5)];
                    m_downloadfileinfo[QString(rs->MD5)] = nullptr;
                    ite = m_downloadfileinfo.erase(ite);
                    break;
                }
                ite++;
            }
        }
    }
}
