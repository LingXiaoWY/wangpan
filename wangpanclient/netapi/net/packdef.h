#pragma once

typedef int SOCKET;

#define DEF_SERVER_IP ("192.168.3.133")

#define _DEF_BUFFER  ( 4096 )
#define _DEF_CONTENT_SIZE	(1024)
#define MAX_SIZE (40)

//自定义协议   先写协议头 再写协议结构
//登录 注册 获取好友信息 添加好友 聊天 发文件 下线请求
#define DEF_PACK_BASE	(10000)
#define _DEF_PROTOCOL_COUNT (100)

//注册请求结果
#define tel_is_exist		(0)
#define register_success	(1)
#define name_is_exist       (2)

//返回的结果
//登录请求的结果
#define user_not_exist		(0)
#define password_error		(1)
#define login_success		(2)

//添加好友的结果
#define no_this_user		(0)
#define user_refuse			(1)
#define user_offline		(2)
#define add_success			(3)

typedef int PackType;

//协议结构
//注册
#define _DEF_PACK_REGISTER_RQ	(DEF_PACK_BASE + 0 )
typedef struct STRU_REGISTER_RQ
{
    STRU_REGISTER_RQ():type(_DEF_PACK_REGISTER_RQ)
    {
        memset( tel  , 0, sizeof(tel));
        memset( name  , 0, sizeof(name));
        memset( password , 0, sizeof(password) );
    }
    //需要手机号码 , 密码, 昵称
    PackType type;
    char tel[MAX_SIZE];
    char name[MAX_SIZE];
    char password[MAX_SIZE];

}STRU_REGISTER_RQ;

typedef struct STRU_REGISTER_RS
{
    //回复结果
    STRU_REGISTER_RS(): type(_DEF_PACK_REGISTER_RQ) , result(register_success){}
    PackType type;
    int result;
}STRU_REGISTER_RS;

//登录
#define _DEF_PACK_LOGIN_RQ	(DEF_PACK_BASE + 1)
typedef struct STRU_LOGIN_RQ
{
    //登录需要: 手机号 密码
    STRU_LOGIN_RQ():type(_DEF_PACK_LOGIN_RQ)
    {
        memset( tel , 0, sizeof(tel) );
        memset( password , 0, sizeof(password) );
    }
    PackType type;
    char tel[MAX_SIZE];
    char password[MAX_SIZE];

}STRU_LOGIN_RQ;

typedef struct STRU_LOGIN_RS
{
    // 需要 结果 , 用户的id
    STRU_LOGIN_RS(): type(_DEF_PACK_LOGIN_RQ) , result(login_success),userid(0)
    {
        memset(name,0,sizeof(name));
    }
    PackType type;
    int userid;
    int result;
    char name[MAX_SIZE];
}STRU_LOGIN_RS;

//获取文件列表
#define _DEF_PACK_GETFILELIST_RQ	(DEF_PACK_BASE + 2)
#define GET_SUCCESS (0)
#define GET_FAILED  (1)
typedef struct STRU_GETFILELIST_RQ
{
    STRU_GETFILELIST_RQ():type(_DEF_PACK_GETFILELIST_RQ)
    {
        id = 0;
    }
    //需要手机号码,密码
    PackType type;
    int id;
}STRU_GETFILELIST_RQ;

typedef struct filecon
{
    filecon()
    {
        memset(filename,0,sizeof(filename));
        memset(filesize,0,sizeof(filesize));
        memset(uploadtime,0,sizeof(uploadtime));
    }
    char filename[100];
    char filesize[10];
    char uploadtime[20];
}filecon;

typedef struct STRU_GETFILELIST_RS
{
    //回复结果
    STRU_GETFILELIST_RS(): type(_DEF_PACK_GETFILELIST_RQ) , result(GET_SUCCESS),filenum(0){}
    PackType type;
    int result;
    int filenum;
    filecon resVec[20];
}STRU_GETFILELIST_RS;

//上传文件
#define _DEF_PACK_UPLOADFILE_RQ	(DEF_PACK_BASE + 3)
#define UPLOADRQ_SUCCESS (0)
#define UPLOADRQ_FAILED  (1)
typedef struct STRU_UPLOADFILE_RQ
{
    STRU_UPLOADFILE_RQ():type(_DEF_PACK_UPLOADFILE_RQ)
    {
        uid = 0;
        memset(filename,0,sizeof(filename));
        memset(uploadtime,0,sizeof(uploadtime));
        filesize = 0;
        memset(MD5,0,sizeof(MD5));
        memset(path,0,sizeof(path));
    }
    //需要id、文件名、上传时间、文件大小、MD5值
    PackType type;
    int uid;
    char filename[100];
    char uploadtime[20];
    int filesize;
    char MD5[40];
    char path[100];
}STRU_UPLOADFILE_RQ;

typedef struct STRU_UPLOADFILE_RS
{
    //回复结果
    STRU_UPLOADFILE_RS(): type(_DEF_PACK_UPLOADFILE_RQ) , result(UPLOADRQ_SUCCESS){memset(MD5,0,sizeof(MD5));}
    PackType type;
    int result;
    char MD5[40];
}STRU_UPLOADFILE_RS;

#define _DEF_PACK_UPLOADFILECONTENT_RQ	(DEF_PACK_BASE + 4)

typedef struct STRU_UPLOADFILECONTENT_RQ
{
    STRU_UPLOADFILECONTENT_RQ():type(_DEF_PACK_UPLOADFILECONTENT_RQ),flag(false)
    {
        nlen = 0;
        memset(MD5,0,sizeof(MD5));
        memset(data,0,sizeof(data));
    }
    //需要用户id、文件名、上传时间、文件大小、MD5值
    PackType type;
    int nlen;
    char MD5[40];
    char data[1000];
    bool flag;
}STRU_UPLOADFILECONTENT_RQ;

typedef struct STRU_UPLOADFILECONTENT_RS
{
    //回复结果
    STRU_UPLOADFILECONTENT_RS(): type(_DEF_PACK_UPLOADFILECONTENT_RQ) , result(UPLOADRQ_SUCCESS){memset(MD5,0,sizeof(MD5));}
    PackType type;
    int result;
    char MD5[40];
}STRU_UPLOADFILECONTENT_RS;

#include<QString>
typedef struct STRU_FILE_INFO
{
    STRU_FILE_INFO():filesize(0),sendfilesize(0){
        memset(MD5,0,sizeof(MD5));
    }
    int filesize;//文件大小
    int sendfilesize;//已发送文件大小
    QString path;//路径
    char MD5[40];//MD5值
}STRU_FILE_INFO;

//下载文件
#define _DEF_PACK_DOWNLOADFILE_RQ	(DEF_PACK_BASE + 6)
#define DOWNLOAD_SUCCESS (0)
#define DOWNLOAD_FAILED  (1)
typedef struct STRU_DOWNLOADFILE_RQ
{
    STRU_DOWNLOADFILE_RQ():type(_DEF_PACK_DOWNLOADFILE_RQ)
    {
        uid = 0;
        memset(MD5,0,sizeof(MD5));
    }
    PackType type;
    int uid;//用户ID
    char MD5[40];//MD5值
}STRU_DOWNLOADFILE_RQ;

typedef struct STRU_DOWNLOADFILE_RS
{
    //回复结果
    STRU_DOWNLOADFILE_RS():type(_DEF_PACK_DOWNLOADFILE_RQ),
        result(DOWNLOAD_SUCCESS),
        filesize(0),
        sendfilesize(0),
        flag(false)
    {
        memset(MD5,0,sizeof(MD5));
        memset(data,0,sizeof(data));
    }
    PackType type;
    int result;
    int filesize;//文件大小
    int sendfilesize;//已发送文件大小
    bool flag;//文件下载完成标记
    char MD5[40];
    char data[1000];//文件内容
}STRU_DOWNLOADFILE_RS;

//删除文件
#define _DEF_PACK_DELETEFILE_RQ	(DEF_PACK_BASE + 5)
#define DELETE_SUCCESS (0)
#define DELETE_FAILED  (1)
typedef struct STRU_DELETEFILE_RQ
{
    STRU_DELETEFILE_RQ():type(_DEF_PACK_DELETEFILE_RQ)
    {
        uid = 0;
        memset(MD5,0,sizeof(MD5));
        memset(path,0,sizeof(path));
    }
    PackType type;
    int uid;
    char MD5[40];
    char path[100];
}STRU_DELETEFILE_RQ;

typedef struct STRU_DELETEFILE_RS
{
    //回复结果
    STRU_DELETEFILE_RS(): type(_DEF_PACK_DELETEFILE_RQ) , result(DELETE_SUCCESS){}
    PackType type;
    int result;
}STRU_DELETEFILE_RS;

//上传续传
//下载续传

