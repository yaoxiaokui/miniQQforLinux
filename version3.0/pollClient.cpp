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
#include <poll.h>
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

    pollfd pollfds[2];
    pollfds[0].fd = STDIN_FILENO;
    pollfds[0].events = POLLIN;

    pollfds[1].fd = sockfd;
    pollfds[1].events = POLLIN;


    while(1){

        int ret = poll(pollfds, 2, 0);
        if(ret <= 0)
            continue;

        //如果终端上可读
        if(pollfds[0].revents & POLLIN){
            memset(buffer, 0, sizeof(buffer));

            //从终端读取数据
            int num = read(STDIN_FILENO, buffer, sizeof(buffer));

            /*
            if(buffer[num-1] == '\n'){
                buffer[num-1] = '\0';
            }
            */

            if(strcmp(buffer, "quit\n") == 0){
                break;
            }

            //将数据写入到套接字
            send(sockfd, buffer, strlen(buffer), 0);
        }

        //如果从套接字上接收到数据
        if(pollfds[1].revents & POLLIN){
            memset(buffer, 0, sizeof(buffer));
            
            int num = recv(sockfd, buffer, sizeof(buffer), 0);
            if(num > 0){
                if(buffer[num-1] == '\n'){
                    buffer[num-1] = '\0';
                }

                write(STDOUT_FILENO, buffer, strlen(buffer));
                cout << endl;
            }
        }
        
    }


    close(sockfd);
    return 0; 
}
