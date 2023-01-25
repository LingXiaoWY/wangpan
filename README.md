# wangpan
1.开发环境：客户端：Windows下QT，服务端：Ubuntu20.04 LTS下，均为C/C++开发</br></br>
2.简介：局域网下的网盘存储，实现的功能：登录注册系统、上传文件、下载文件、删除文件，该系统可以注册用户，已有的用户可以直接登录进入网盘主界面，可以对本地文件进行上传，对于属于用户的存储在服务器中的文件可以进行下载与删除。</br></br>
3.协议：自定义协议，传输层TCP、UDP</br></br>
4.服务器基于高并发Server，使用了epoll的LT模式、线程池、单例模式，能够处理万次请求</br></br>
5.登录系统实现：client通过与server进行交互，查询数据库，验证登录/注册用户是否存在，注册时，MD5加密算法将用户密码进行加密，把明文变密文</br></br>
6.上传文件：在客户端上点击上传按钮准备上传文件，弹窗上选择要上传的文件，确认后，获取文件的文件名和路径，根据文件生成MD5，将MD5值作为文件识别码，客户端向服务器发送文件信息，服务器将文件信息录入MySQL数据库中，然后接收客户端发送的数据，将文件数据通过提前设置好的文件路径写入文件中。下载文件：客户端在界面里面点击下载按钮，下载选中项，服务器通过路径读取文件内容发送给客户端，客户端接收后写入本地文件。删除文件：删除为两端删除，客户端从列表删除信息, 服务器用户文件删除列表里的文件。</br></br>
</br>
暂未完成功能：上传续传，下载续传</br>

Client类图
-------------
<div align=center><img src="https://img.fwfly.com/img/2023/01/22/12vtedi.png" /> </div>

Server类图
-------------
<div align=center><img src="https://img.fwfly.com/img/2023/01/22/12vtpoi.png" /> </div>

流程图
-------------
<div align=center><img src="https://img.fwfly.com/img/2023/01/22/12vtjgu.png" height="765"/> </div>

登录&注册界面
-------------
<div align=center><img src="https://img.fwfly.com/img/2023/01/23/21lun.jpg" height="765"/> </div>
<div align=center><img src="https://img.fwfly.com/img/2023/01/23/21ms1.jpg" height="765"/> </div>

主界面
-------------
<div align=center><img src="https://img.fwfly.com/img/2023/01/23/21mti.jpg" height="765"/> </div>

Client类图直链网址：https://img.fwfly.com/img/2023/01/22/12vtedi.png</br>
Server类图直链网址：https://img.fwfly.com/img/2023/01/22/12vtpoi.png</br>
流程图直链网址：https://img.fwfly.com/img/2023/01/22/12vtjgu.png</br>
