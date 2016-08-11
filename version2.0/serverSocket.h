/*************************************************************************
	> File Name: mySocket.h 
	> Author: 
	> Mail: 
	> Created Time: Wed 10 Aug 2016 07:45:44 PM CST
 ************************************************************************/
#include <iostream>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <string>
using namespace std;


//服务器的套接字类
class serverSocket{
private:
    int listenfd;
    //struct sockaddr_in remoteAddr;
    struct sockaddr_in localAddr;

public:
    serverSocket(const char * ip, int port);
    ~serverSocket();
    int acceptConnection(struct sockaddr_in *remoteAddr, socklen_t* remoteAddrLen);
    struct sockaddr_in getRemoteAddr() const;
    int getListenfd() const;
};

/****************************************************************
*   函数名称：serverSocket
*   功能描述: 构造函数 
*   参数列表: ip监听的IP地址，port监听的端口号 
*   返回结果：无
*****************************************************************/
serverSocket::serverSocket(const char *ip, int port)
{
    //创建套接字
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0){
        cerr << "socket error" << endl;
        return;
    }

    //初始化地址
    bzero(&localAddr, sizeof(localAddr));
    localAddr.sin_port = htons(port);
    localAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &localAddr.sin_addr);
    
    //绑定套接字和地址
    int ret = bind(listenfd, (struct sockaddr*)&localAddr, sizeof(localAddr));
    if(ret < 0){
        cerr << "bind error" << endl;
        return;
    }

    //监听套接字
    ret = listen(listenfd, 5);
    if(ret < 0){
        cerr << "listen error" << endl;
        return;
    }

}

/****************************************************************
*   函数名称：serverSocket
*   功能描述: 析构函数
*   参数列表: 
*   返回结果：无
*****************************************************************/
serverSocket::~serverSocket()
{
    if(listenfd > 0)
        close(listenfd);
}


/****************************************************************
*   函数名称：acceptConnection
*   功能描述: 获取请求连接
*   参数列表: 
*   返回结果：返回一个连接套接字
*****************************************************************/
int serverSocket::acceptConnection(struct sockaddr_in *remoteAddr, socklen_t* remoteAddrLen)
{
    int connfd = accept(listenfd, (struct sockaddr*)remoteAddr, remoteAddrLen);
    if(connfd < 0){
        cerr << "class accept error" << endl;
        return -1;
    }
    

    return connfd;
}


/****************************************************************
*   函数名称：getListenfd
*   功能描述: 获得本地的监听套接字
*   参数列表: 
*   返回结果：返回本地的监听套接字
*****************************************************************/
int serverSocket::getListenfd() const
{
    return listenfd; 
}
