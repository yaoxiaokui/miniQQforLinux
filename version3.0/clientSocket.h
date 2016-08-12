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


//客户端的套接字类
class clientSocket{
private:
    int sockfd;
    struct sockaddr_in remoteAddr;
    struct sockaddr_in localAddr;

public:
    clientSocket(const char * ip, int port);
    ~clientSocket();
    int connectServer();
    int getSockfd() const;
};


/****************************************************************
*   函数名称：clientSocket
*   功能描述: 构造函数 
*   参数列表: ip是要连接的服务器IP地址，port连接服务器的端口号 
*   返回结果：无
*****************************************************************/
clientSocket::clientSocket(const char *ip, int port)
{
    //创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        cerr << "socket error" << endl;
        return;
    }

    //初始化地址
    bzero(&remoteAddr, sizeof(remoteAddr));
    remoteAddr.sin_port = htons(port);
    remoteAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &remoteAddr.sin_addr);
}


/****************************************************************
*   函数名称：clientSocket
*   功能描述: 析构函数
*   参数列表: 
*   返回结果：无
*****************************************************************/
clientSocket::~clientSocket()
{
    if(sockfd > 0)
        close(sockfd);
}


/****************************************************************
*   函数名称：connectServer
*   功能描述: 请求连接服务器
*   参数列表: 
*   返回结果：0表示连接成功，-1表示连接失败
*****************************************************************/
int clientSocket::connectServer()
{
    int ret = connect(sockfd, (struct sockaddr*)&remoteAddr, sizeof(remoteAddr));
    if(ret < 0){
        cerr << "connect error" << endl;
        return -1;
    }
    
    return ret;
}

/****************************************************************
*   函数名称：getSockfd
*   功能描述: 获得套接字描述符
*   参数列表: 
*   返回结果：0表示连接成功，-1表示连接失败
*****************************************************************/
int clientSocket::getSockfd() const
{
    return sockfd;
}
