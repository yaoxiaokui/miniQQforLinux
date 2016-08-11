/*************************************************************************
	> File Name: selectChatClient.cpp
	> Author: 
	> Mail: 
	> Created Time: Wed 10 Aug 2016 07:54:47 PM CST
 ************************************************************************/

#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <sys/select.h>
#include "clientSocket.h"
using namespace std;

int main(int argc, char * argv[])
{
    if(argc < 3){
        cerr << "input: " << argv[0] << ", ip, port" << endl;
        return -1;
    }


    const char *ip = argv[1];
    int port = atoi(argv[2]);

    //创建套接字
    clientSocket Socket = clientSocket(ip, port);
    int sockfd = Socket.getSockfd();
    if(sockfd < 0){
        cerr << "socket error" << endl;
        return -1;
    }


    //连接到服务器
    int ret = Socket.connectServer();
    if(ret < 0){
        cerr << "connect error" << endl;
        return -1;
    }

    char buffer[1024];

    fd_set readSet;

    while(1){

        FD_ZERO(&readSet);
        FD_SET(0, &readSet);
        FD_SET(sockfd, &readSet);

        int ret = select(sockfd+1, &readSet, NULL, NULL, 0);

        //如果终端上可读
        if(FD_ISSET(0, &readSet)){
            memset(buffer, 0, sizeof(buffer));

            //从终端读取数据
            int num = read(0, buffer, sizeof(buffer));

            if(buffer[num-1] == '\n'){
                buffer[num-1] = '\0';
            }

            if(strcmp(buffer, "quit") == 0){
                break;
            }

            //将数据写入到套接字
            write(sockfd, buffer, strlen(buffer));
        }

        //如果从套接字上接收到数据
        if(FD_ISSET(sockfd, &readSet)){
            memset(buffer, 0, sizeof(buffer));
            
            int num = recv(sockfd, buffer, sizeof(buffer), 0);
            if(num > 0){
                write(0, buffer, strlen(buffer));
                cout << endl;
            }
        }
        
    }


    return 0; 
}
