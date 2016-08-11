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
using namespace std;


//保存客户端信息的结构体
struct ClientData
{
    int fd;
    struct sockaddr_in addr;
};

//向聊天室的其它所有人发送消息
void sendMsgToOthers(struct ClientData *client, int self, const char * buff);
//获得要发送到其它成员的信息
string getSendMsg(struct ClientData *client, int currentClientNum, int flag);

//聊天室的大小
int ClientNum = 10;


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


    //创建套接字
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0){
        cerr << "socket error" << endl;
    }
    
    //绑定套接字和地址,先获得地址
    sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));

    serverAddr.sin_port = htons(port);
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &serverAddr.sin_addr);

    //绑定套接字和地址
    int ret = bind(listenfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if(ret < 0){
        cerr << "bind error" << endl;
        close(listenfd);
        return -1;
    }

    //监听套接字
    ret = listen(listenfd, 5);
    if(ret < 0){
        cerr << "listen error" << endl;
        close(listenfd);
        return -1;
    }

    //保存客户端的信息
    struct ClientData client[ClientNum];
    bzero(client, sizeof(client));

    int currentClientNum = 0;
    int maxSockfd = listenfd;

    fd_set readSet;

    //存放输入的数据
    char buffer[1024];
    //存放客户的地址和端口号 
    string clientInfo;

    //使用select实现IO复用
    while(1){

        FD_ZERO(&readSet);

        FD_SET(listenfd, &readSet);

        //设置每个客户端连接的套接字
        for(int i = 0; i < ClientNum; ++i){
            if(client[i].fd != 0){
                FD_SET(client[i].fd, &readSet);

                if(maxSockfd < client[i].fd)
                    maxSockfd = client[i].fd;
            }
        }

        int ret = select(maxSockfd + 1, &readSet, NULL, NULL, 0);
        if(ret < 0){
            ;//cerr << "select error" << endl;
        }

        //有请求到来
        //首先判断是否是一个新的连接请求
        if(FD_ISSET(listenfd, &readSet)){
            struct sockaddr_in clientAddr;
            bzero(&clientAddr, sizeof(clientAddr));
            socklen_t clientAddrLen;

            int connfd = accept(listenfd, (struct sockaddr*)&clientAddr, &clientAddrLen);
            if(connfd < 0){
                cerr << "accept error" << endl;
            }
            else{
                if(currentClientNum == ClientNum){
                    cout << "聊天室已满!" << endl;
                }
                else{
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

                            break;
                        }
                    }
                }
                    
            }
        }

        //再依次判断每个客户端连接
        for(int i = 0; i < ClientNum; ++i){
            memset(buffer, 0, sizeof(buffer));
            clientInfo.clear();

            if(FD_ISSET(client[i].fd, &readSet)){
                int ret = recv(client[i].fd, buffer, sizeof(buffer), 0);
                if(ret <= 0){
                    //如果客户端离开聊天室
                    close(client[i].fd);
                    FD_CLR(client[i].fd, &readSet);
                    client[i].fd = 0;
                    currentClientNum--;

                    //获得要发送到其它成员的信息
                    clientInfo = getSendMsg(&client[i], currentClientNum, 2);

                    //输出要终端
                    cout << clientInfo << endl;

                    //发送其它人
                    sendMsgToOthers(client, i, clientInfo.c_str());
                }
                else{
                    if(buffer[ret-1]=='\n'){
                        buffer[ret-1] = '\0';
                    }

                    //获得要发送到其它成员的信息
                    clientInfo = getSendMsg(&client[i], currentClientNum, 0);

                    //输出客户端写入的内容,输出到终端
                    cout << clientInfo;
                    cout << buffer << endl;


                    //发送其它人
                    sendMsgToOthers(client, i, clientInfo.c_str());
                    //发送其它人
                    sendMsgToOthers(client, i, buffer);
                }
            }
        }

    }


    return 0;
}

//获得要发送到其它成员的信息
string getSendMsg(struct ClientData *client, int currentClientNum, int flag)
{
    string clientInfo = "";

    clientInfo.append(inet_ntoa((*client).addr.sin_addr));
    clientInfo.append(":");
    string portNum = to_string(ntohs((*client).addr.sin_port));
    clientInfo.append(portNum);

    if(flag == 0){//表示发送消息
        clientInfo.append("-->");
        return clientInfo;
    }
    else if(flag == 1){//表示进入聊天室
        clientInfo.append("进入聊天室!");
    }
    else if(flag == 2){//表示离开聊天室
        clientInfo.append("离开聊天室!");
    }

    clientInfo.append("当前人数为");
    string personNum = to_string(currentClientNum);
    clientInfo.append(personNum);
    
    return clientInfo;
}

//向聊天室的其它所有人发送消息
void sendMsgToOthers(struct ClientData *client, int self, const char * buff)
{
    for(int i = 0; i < ClientNum; ++i){
        if(client[i].fd != 0 && i != self){
            //cout << "发送其它人: " << buff << endl;
            send(client[i].fd, buff, strlen(buff), 0);
        }
    }
}
