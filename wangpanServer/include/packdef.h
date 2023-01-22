#ifndef _PACKDEF_H
#define _PACKDEF_H

#define TRUE 1
#define FALSE 0

#include<cstdlib>
#include<iostream>
#include<fstream>
#include<pthread.h>
#include<signal.h>
#include<errno.h>
#include<cstring>
#include<unistd.h>
#include<sys/epoll.h>
#include<sys/stat.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<ctime>
#include<list>
#include<map>
#include<vector>
using namespace std;

#include<Thread_pool.h>

typedef int PackType;

#define MAX_SIZE (40)

#define MAX_EVENTS 4096

#define DEF_TIMEOUT 10
#define DEF_COUNT   10
#define DEF_PACK_COUNT 100
#define DEF_SIZE 45
#define DEF_PORT 8000
#define DEF_SERVERIP "0.0.0.0"

/*------------数据库信息------------*/
#define DEF_DB_NAME    "wangpan"
#define DEF_DB_IP      "localhost"
#define DEF_DB_USER    "guo"
#define DEF_DB_PWD     "123456"
/*--------------------------------*/


#define DEF_PACK_BASE 10000


#define DEF_LISTEN 128
#define DEF_EPOLLSIZE 4096
#define DEF_IPSIZE 16
#define DEF_SQLLEN 400


//用户信息
typedef struct UserInfo
{
  UserInfo()
  {
    m_sockfd = 0;
    m_id = 0;
    memset(m_userName,0,sizeof(m_userName));
  }
  int m_sockfd;
  int m_id;
  char m_userName[MAX_SIZE];
  char m_passwd[128];
}UserInfo;


//注册
#define _DEF_PACK_REGISTER_RQ	(DEF_PACK_BASE + 0 )

//登录
#define _DEF_PACK_LOGIN_RQ	(DEF_PACK_BASE + 1)


//注册请求结果
#define tel_is_exist		(0)
#define register_success	(1)
#define name_is_exist       (2)
#define register_fail       (3)


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
    STRU_LOGIN_RS(): type(_DEF_PACK_LOGIN_RQ) , userid(0),result(login_success)
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
//请求上传，获得上传路径
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

typedef struct STRU_FILE_INFO
{
    STRU_FILE_INFO():filesize(0),sendfilesize(0){
        memset(path,0,sizeof(path));
        memset(MD5,0,sizeof(MD5));
    }
    int filesize;//文件大小
    int sendfilesize;//已发送文件大小
    char path[100];//路径
    char MD5[40];//MD5值
}STRU_FILE_INFO;

typedef struct STRU_FILE_CON //文件属性
{
    STRU_FILE_CON(){
        memset(f_name,0,sizeof(f_name));
        memset(f_uploadtime,0,sizeof(f_uploadtime));
        memset(f_filesize,0,sizeof(f_filesize));
    }
    char f_name[100];
    char f_uploadtime[20];
    char f_filesize[100];
}STRU_FILE_CON;

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
#endif
