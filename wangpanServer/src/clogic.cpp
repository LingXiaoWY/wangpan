#include<clogic.h>



CLogic::CLogic(TcpKernel *pkernel):m_netPackFunMap(100)
{
    m_pKernel = pkernel;
    m_sql = pkernel->m_sql;
    m_tcp = pkernel->m_tcp;
    pthread_mutex_init(&m_roomListMutex,nullptr);
}
void CLogic::GetFileListRq(sock_fd clientfd, char*szbuf, int nlen)//获取用户文件信息列表
{
    //解析数据包
    STRU_GETFILELIST_RQ* rq = (STRU_GETFILELIST_RQ*)szbuf;
    STRU_GETFILELIST_RS rs;
    //根据用户id查看数据库中文件列表信息
    list<string>lstRes;
    char sqlStr[1000] = "";
    sprintf(sqlStr,"select F_name,F_size,F_uploadtime from file inner join user_file on U_id = %d and file.f_id = user_file.F_id;",rq->id);
    m_sql->SelectMysql(sqlStr,3,lstRes);
    if(lstRes.size() > 0)
    {
        rs.result = GET_SUCCESS;
        int filenum{},cnt{};
        for(auto i:lstRes)
        {
            switch (cnt) {
            case 0:
                strcpy(rs.resVec[filenum].filename,i.c_str());
            break;
            case 1:
                strcpy(rs.resVec[filenum].filesize,i.c_str());
            break;
            case 2:
            {
                strcpy(rs.resVec[filenum].uploadtime,i.c_str());
                ++filenum;
            }
            break;
            }
            cnt = (cnt + 1) % 3;
        }
        rs.filenum = lstRes.size() / 3;
    }
    //返回结果
    SendData(clientfd,(char *)&rs,sizeof(rs));
}
void CLogic::slot_dealRegisterRq(sock_fd clientfd, char*szbuf, int nlen)//处理注册请求
{
    //解析数据包 获取tel passwd name
    STRU_REGISTER_RQ* rq = (STRU_REGISTER_RQ *)szbuf;
    STRU_REGISTER_RS rs;
    //根据tel 查数据库 看有没有
    list<string>lstRes;
    char sqlStr[1000] = "";
    sprintf(sqlStr,"select U_tel from user where U_tel = '%s';",rq->tel);
    m_sql->SelectMysql(sqlStr,1,lstRes);
    //有 返回结果
    if(lstRes.size()>0)
    {
      rs.result = tel_is_exist;
    }

    else{
        //没有 接下来看昵称 有没有
        lstRes.clear();
        sprintf(sqlStr,"select U_name from user where U_name = %s;",rq->name);
        m_sql->SelectMysql(sqlStr,1,lstRes);
        //有返回结果
        if(lstRes.size()>0)
        rs.result = name_is_exist;
        //没有 注册成功 更新数据库 写表
        else
        {
            rs.result = register_success;
            sprintf(sqlStr,"insert into user(U_name,U_password,U_tel) values('%s','%s','%s');",rq->name,rq->password,rq->tel);
            int res = m_sql->UpdataMysql(sqlStr);
            if(!res)
            {
              rs.result = register_fail;
              cout<<"register failed"<<endl;
            }
        }
    }
    //返回结果
    SendData(clientfd,(char *)&rs,sizeof(rs));
}
void CLogic::slot_dealLoginRq(sock_fd clientfd, char*szbuf, int nlen)//处理登录请求
{
    //拆包 获取 tel password
    STRU_LOGIN_RQ* rq = (STRU_LOGIN_RQ*)szbuf;
    STRU_LOGIN_RS rs;
    //根据 tel 查 id password name
    char sqlstr[1000] = "";
    list<string>lstRes;
    sprintf(sqlstr,"select U_id,U_password,U_name from user where U_tel = '%s';",rq->tel);
    m_sql->SelectMysql(sqlstr,3,lstRes);

    if(lstRes.size() == 0)
    {
        //没有 返回结果
        rs.result = user_not_exist;
    }
    else{
        int id = atoi(lstRes.front().c_str());
        lstRes.pop_front();
        string strPasswd = lstRes.front();
        lstRes.pop_front();
        string strName = lstRes.front();
        lstRes.pop_front();
        //有 看密码是否一致
        if(strcmp(rq->password,strPasswd.c_str())!=0)
        {
            //不一致 返回结果
            rs.result = password_error;
        }
        else
        {
            //一致
            rs.result = login_success;
            UserInfo *info = nullptr;
            //如果之前有用户信息 强制下线 先回收
            if(m_mapIdToUserInfo.find(id,info))
            {
                //强制下线

                //回收
                m_mapIdToUserInfo.erase(id);
                delete info;
            }
            //把id和套接字捆绑在一起
            //保存用户信息
            info = new UserInfo;
            info->m_id = id;
            info->m_sockfd = clientfd;
            strcpy(info->m_userName,strName.c_str());
            strcpy(rs.name,strName.c_str());
            rs.userid = id;
            //把id和套接字 捆绑在一起
            m_mapIdToUserInfo.insert(id,info);
            //返回结果 id name result
            SendData(clientfd,(char *)&rs,sizeof(rs));
            return;
        }

    }
    SendData(clientfd,(char *)&rs,sizeof(rs));
}
void CLogic::slot_dealUpLoadFileHeaderRq(sock_fd clientfd, char*szbuf, int nlen)//上传文件头请求(文件名,id,路径,md5)
{
    STRU_UPLOADFILE_RQ *rq = (STRU_UPLOADFILE_RQ *)szbuf;
    STRU_UPLOADFILE_RS rs;
    STRU_FILE_INFO *fileinfo = new STRU_FILE_INFO;
    //1. 查找数据库中对应用户uid，是否上传过该文件，上传过则返回
    char sqlstr[1000] = "";
    list<string>lstRes;
    sprintf(sqlstr,"select file.f_id from file inner join user_file on U_id = %d and file.f_id = user_file.F_id and F_name = '%s';",rq->uid,rq->filename);
    m_sql->SelectMysql(sqlstr,1,lstRes);
    //2. 该用户未上传过该文件，则允许上传，并将文件信息录入数据库
    if(lstRes.size() == 0 && rq->filesize > 0)
    {
        char path[100];
        memset(path,0,sizeof(path));
        strcpy(path,"/home/guo/Downloads/");
        strcat(path,rq->filename);
        strcpy(fileinfo->path,path);
        fileinfo->filesize = rq->filesize;
        strcpy(fileinfo->MD5,rq->MD5);
        strcpy(rs.MD5,rq->MD5);
        memset(sqlstr,0,sizeof(sqlstr));
        sprintf(sqlstr,"insert into file(F_name,F_uploadtime,F_size,F_path,F_MD5) values('%s','%s','%d','%s','%s');",rq->filename,rq->uploadtime,rq->filesize,path,rq->MD5);
        int res = m_sql->UpdataMysql(sqlstr);
        if(!res)
        {
          rs.result = UPLOADRQ_FAILED;
          cout<<"UPLOAD_FAILED1"<<endl;
          SendData(clientfd,(char *)&rs,sizeof(rs));
          return;
        }
        sleep(1);
        memset(sqlstr,0,sizeof(sqlstr));
        lstRes.clear();
        sprintf(sqlstr,"select f_id from file where F_MD5 = '%s';",rq->MD5);
        m_sql->SelectMysql(sqlstr,1,lstRes);
        if(lstRes.size() != 0){
            memset(sqlstr,0,sizeof(sqlstr));
            sprintf(sqlstr,"insert into user_file values(%d,'%s');",rq->uid,(*lstRes.begin()).c_str());
            res = m_sql->UpdataMysql(sqlstr);
        }
        else
        {
          rs.result = UPLOADRQ_FAILED;
          cout<<"UPLOAD_FAILED2"<<endl;
        }
        m_uploadingFile[rq->MD5] = fileinfo;
    }
    else
    {
        rs.result = UPLOADRQ_FAILED;
    }
    //3. 向客户端发送返回信息
    SendData(clientfd,(char *)&rs,sizeof(rs));
}
void CLogic::slot_dealUpLoadFileContentRq(sock_fd clientfd, char*szbuf, int nlen)//上传文件内容请求
{
    STRU_UPLOADFILECONTENT_RQ *rq = (STRU_UPLOADFILECONTENT_RQ *)szbuf;
    STRU_UPLOADFILECONTENT_RS rs;
    STRU_FILE_INFO *fileinfo = m_uploadingFile[rq->MD5];
    if(fileinfo == nullptr)
        return;
    fstream file;
    file.open(fileinfo->path,ios::app);
    file<<rq->data;
    int nRealWriteNum = rq->nlen;
    if(nRealWriteNum > 0)
    {
        fileinfo->sendfilesize += nRealWriteNum;
        if(rq->flag)
        {
            file.close();
            STRU_UPLOADFILECONTENT_RS rs;
            rs.result = UPLOADRQ_SUCCESS;
            strcpy(rs.MD5,fileinfo->MD5);
            SendData(clientfd,(char*)&rs,sizeof(rs));
            //删除文件映射表中的文件信息
            auto ite = m_uploadingFile.begin();
            while(ite != m_uploadingFile.end())
            {
                if(ite->first == fileinfo->MD5)
                {
                    delete fileinfo;
                    fileinfo = nullptr;
                    ite = m_uploadingFile.erase(ite);
                    break;
                }
                ++ite;
            }
        }
    }
    else
    {
        STRU_UPLOADFILECONTENT_RS rs;
        rs.result = UPLOADRQ_FAILED;
        SendData(clientfd,(char*)&rs,sizeof(rs));
        file.close();
    }
}
void CLogic::slot_dealDownloadRq(sock_fd clientfd, char*szbuf, int nlen)//下载文件请求
{
    //1. 拆包，获得文件信息
    STRU_DOWNLOADFILE_RQ *rq = (STRU_DOWNLOADFILE_RQ *)szbuf;
    STRU_DOWNLOADFILE_RS rs;
    STRU_FILE_INFO *file = new STRU_FILE_INFO;
    strcpy(file->MD5,rq->MD5);
    strcpy(rs.MD5,rq->MD5);
    m_downloadFile[file->MD5] = file;
    //2. 读取文件
    char sqlstr[1000];
    list<string>lstRes;
    sprintf(sqlstr,"select F_path from file where F_MD5 = '%s';",rq->MD5);
    m_sql->SelectMysql(sqlstr,1,lstRes);
    if(lstRes.size() > 0)
    {
        string path = (*lstRes.begin());
        lstRes.pop_front();
        FILE *pfile = nullptr;
        pfile = fopen(path.c_str(),"r");
        if(!pfile) return ;
        struct stat statbuf;
        stat(path.c_str(),&statbuf);
        rs.filesize = statbuf.st_size;
        while(rs.sendfilesize < rs.filesize)
        {
            memset(rs.data,0,sizeof(rs.data));
            rs.sendfilesize += fread(rs.data,sizeof(char),1000,pfile);
            if(rs.sendfilesize == rs.filesize)
                rs.flag = true;
            SendData(clientfd,(char *)&rs,sizeof(rs));
        }
        fclose(pfile);
        return;
    }
    else
    {
        rs.result = DOWNLOAD_FAILED;
    }
    SendData(clientfd,(char *)&rs,sizeof(rs));
}
void CLogic::slot_dealDeleteFileRq(sock_fd clientfd, char *szbuf, int nlen)//删除文件请求
{
    STRU_DELETEFILE_RQ *rq = (STRU_DELETEFILE_RQ *)szbuf;
    STRU_DELETEFILE_RS rs;
    char sqlstr[1000];
    list<string>lstRes;
    sprintf(sqlstr,"select F_id from user_file where F_id = (select f_id from file where F_MD5 = '%s') and U_id = %d;",rq->MD5,rq->uid);
    m_sql->SelectMysql(sqlstr,1,lstRes);
    if(lstRes.size() > 0)
    {
        string fileid = (*lstRes.begin());
        lstRes.pop_front();
        sprintf(sqlstr,"select F_path from file where f_id = %s;",fileid.c_str());
        m_sql->SelectMysql(sqlstr,1,lstRes);
        if(lstRes.size() > 0)
        {
            STRU_FILE_CON filecon;
            string str = (*lstRes.begin());
            lstRes.pop_front();
            strcpy(rq->path,str.c_str());
            sprintf(sqlstr,"delete from file where f_id = %s;",fileid.c_str());
            int res = m_sql->UpdataMysql(sqlstr);
            sprintf(sqlstr,"select F_name,F_uploadtime,F_size from file where f_id = %s;",fileid.c_str());
            m_sql->SelectMysql(sqlstr,3,lstRes);
            int cnt = 0;
            for(auto i:lstRes)
            {
                switch(cnt)
                {
                    case 0:strcpy(filecon.f_name,i.c_str());break;
                    case 1:strcpy(filecon.f_uploadtime,i.c_str());break;
                    case 2:strcpy(filecon.f_filesize,i.c_str());break;
                }
                cnt++;
            }
            lstRes.clear();
            if(!res)
            {
                rs.result = DELETE_FAILED;
                sprintf(sqlstr,"insert into file values(%s,'%s','%s','%s','%s','%s');",fileid.c_str(),filecon.f_name,filecon.f_uploadtime,filecon.f_filesize,rq->path,rq->MD5);
                m_sql->UpdataMysql(sqlstr);
                SendData(clientfd,(char*)&rs,sizeof(rs));
                return;
            }
            sprintf(sqlstr,"delete from user_file where F_id = '%s';",fileid.c_str());
            res = m_sql->UpdataMysql(sqlstr);
            if(!res)
            {
                rs.result = DELETE_FAILED;
                sprintf(sqlstr,"insert into file values(%s,'%s','%s','%s','%s','%s');",fileid.c_str(),filecon.f_name,filecon.f_uploadtime,filecon.f_filesize,rq->path,rq->MD5);
                m_sql->UpdataMysql(sqlstr);
                sprintf(sqlstr,"insert into user_file values(%d,'%s');",rq->uid,fileid.c_str());//操作失败回滚
                m_sql->UpdataMysql(sqlstr);
                SendData(clientfd,(char*)&rs,sizeof(rs));
                return;
            }
            unlink(rq->path);
        }
        else
        {
            rs.result = DELETE_FAILED;
        }
    }
    else
    {
        rs.result = DELETE_FAILED;
    }
    SendData(clientfd,(char*)&rs,sizeof(rs));
}
void CLogic::SendData(sock_fd clientfd, char*szbuf, int nlen)//发送数据
{
    m_pKernel->SendData(clientfd,szbuf,nlen);
}
void CLogic::setNetPackMap()
{
    NetPackMap(_DEF_PACK_REGISTER_RQ) = &CLogic::slot_dealRegisterRq;
    NetPackMap(_DEF_PACK_LOGIN_RQ) = &CLogic::slot_dealLoginRq;
    NetPackMap(_DEF_PACK_GETFILELIST_RQ) = &CLogic::GetFileListRq;
    NetPackMap(_DEF_PACK_UPLOADFILE_RQ) = &CLogic::slot_dealUpLoadFileHeaderRq;
    NetPackMap(_DEF_PACK_UPLOADFILECONTENT_RQ) = &CLogic::slot_dealUpLoadFileContentRq;
    NetPackMap(_DEF_PACK_DELETEFILE_RQ) = &CLogic::slot_dealDeleteFileRq;
    NetPackMap(_DEF_PACK_DOWNLOADFILE_RQ) = &CLogic::slot_dealDownloadRq;
}
