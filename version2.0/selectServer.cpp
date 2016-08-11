/*************************************************************************
	> File Name: selectChatServer.cpp
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
#include "serverSocket.h"
using namespace std;


//保存客户端信息的结构体
struct ClientData
{
    int fd;
    struct sockaddr_in addr;
};

//向聊天室的其它所有人发送消息
void sendMsgToOthers(ClientData *client, int self, const char * buff);
//获得要发送到其它成员的信息
string getSendMsg(ClientData *client, int currentClientNum, int flag);
//处理新的连接请求
void handleConnection(int connfd, ClientData * client, sockaddr_in clientAddr, int &currentClientNum);
//处理客户请求
void handleMessage(ClientData * client, int currIndex, char * buffer, fd_set &readSet, int & currentClientNum);

//聊天室的大小
int ClientNum = 10;
//缓冲区的最大值
const int MAXBUFFSIZE = 1024;


//主函数
int main(int argc, char * argv[])
{
    if(argc < 4){
        cout << "input: " << argv[0] << ", ip, port, ClientNum" << endl;
        return -1;
    }

    const char * ip = argv[1];
    int port = atoi(argv[2]);
    ClientNum = atoi(argv[3]);

    //创建一个服务器套接字对象
    serverSocket Socket = serverSocket(ip, port);
    int listenfd = Socket.getListenfd();


    //保存客户端的信息
    struct ClientData client[ClientNum];
    bzero(client, sizeof(client));

    int currentClientNum = 0;
    int maxSockfd = listenfd;

    fd_set readSet;

    //存放输入的数据
    char buffer[MAXBUFFSIZE];

    //使用select实现IO复用
    while(1){

        FD_ZERO(&readSet);

        //设置监听套接字
        FD_SET(listenfd, &readSet);

        //设置每个客户端连接的套接字
        for(int i = 0; i < ClientNum; ++i){
            if(client[i].fd != 0){
                FD_SET(client[i].fd, &readSet);

                if(maxSockfd < client[i].fd)
                    maxSockfd = client[i].fd;
            }
        }

        //使用select实现对多个套接字进行监听
        int ret = select(maxSockfd + 1, &readSet, NULL, NULL, 0);
        if(ret < 0){
            continue;
        }

        //首先判断是否是一个新的连接请求
        if(FD_ISSET(listenfd, &readSet)){
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen;

            //接受新的连接请求
            int connfd = Socket.acceptConnection(&clientAddr, &clientAddrLen);
            if(connfd < 0){
                cerr << "accept error" << endl;
            }
            else{
                //处理新的连接请求
                handleConnection(connfd, client, clientAddr, currentClientNum);
            }
        }

        //再依次判断每个客户端连接
        for(int i = 0; i < ClientNum; ++i){
            memset(buffer, 0, sizeof(buffer));

            if(FD_ISSET(client[i].fd, &readSet)){
                //处理客户发的消息
                handleMessage(client, i, buffer, readSet, currentClientNum);
            }
        }

    }


    return 0;
}

//获得要发送到其它成员的信息
string getSendMsg(ClientData *client, int currentClientNum, int flag)
{
    //存放客户的地址和端口号 
    string clientInfo;

    clientInfo.append(inet_ntoa((*client).addr.sin_addr));
    clientInfo.append(":");
    string portNum = to_string(ntohs((*client).addr.sin_port));
    clientInfo.append(portNum);

    if(flag == 0){//表示发送消息
        clientInfo.append("-->");
        return clientInfo;
    }
    else if(flag == 1){
        clientInfo.append("进入聊天室!");
    }
    else if(flag == 2){
        clientInfo.append("离开聊天室!");
    }

    clientInfo.append("当前人数为");
    string personNum = to_string(currentClientNum);
    clientInfo.append(personNum);
    
    return clientInfo;
}

//向聊天室的其它所有人发送消息
void sendMsgToOthers(ClientData *client, int self, const char * buff)
{
    for(int i = 0; i < ClientNum; ++i){
        if(client[i].fd != 0 && i != self){
            //cout << "发送其它人: " << buff << endl;
            send(client[i].fd, buff, strlen(buff), 0);
        }
    }
}

//处理新的连接请求
void handleConnection(int connfd, ClientData * client, sockaddr_in clientAddr, int & currentClientNum)
{

    //存放客户的地址和端口号 
    string clientInfo;

    if(currentClientNum == ClientNum){
        cout << "聊天室已满!" << endl;
        send(connfd, "聊天室已满!", 100, 0);
        return;
    }

    //如果聊天室没有满
    for(int i = 0; i < ClientNum; ++i){
        if(client[i].fd == 0){
            client[i].fd = connfd;
            client[i].addr = clientAddr;
            currentClientNum++;

            //获得要发送到其它成员的信息
            clientInfo = getSendMsg(&client[i], currentClientNum, 1);

            //输出到终端
            cout << clientInfo << endl;

            //发送其它人
            sendMsgToOthers(client, i, clientInfo.c_str());

            send(connfd, "         欢迎进入聊天室!", 100, 0);

            break;
        }
    }

}


//处理客户请求
void handleMessage(ClientData * client, int currIndex, char * buffer, fd_set &readSet, int & currentClientNum)
{
    //存放客户的地址和端口号 
    string clientInfo;

    int ret = recv(client[currIndex].fd, buffer, MAXBUFFSIZE, 0);
    if(ret <= 0){
        //如果客户端离开聊天室
        close(client[currIndex].fd);
        FD_CLR(client[currIndex].fd, &readSet);
        client[currIndex].fd = 0;
        currentClientNum--;

        //获得要发送到其它成员的信息
        clientInfo = getSendMsg(client, currentClientNum, 2);

        //输出要终端
        cout << clientInfo << endl;

        //发送其它人
        sendMsgToOthers(client, currIndex, clientInfo.c_str());
    }
    else{
        if(buffer[ret-1]=='\n'){
            buffer[ret-1] = '\0';
        }

        //获得要发送到其它成员的信息
        clientInfo = getSendMsg(&client[currIndex], currentClientNum, 0);

        //输出客户端写入的内容,输出到终端
        cout << clientInfo;
        cout << buffer << endl;


        //发送其它人
        sendMsgToOthers(client, currIndex, clientInfo.c_str());
        //发送其它人
        sendMsgToOthers(client, currIndex, buffer);
    }
}
    
