#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
using namespace std;

list<int> clients_list;

#define SERVER_PORT 8888

#define EPOLL_SIZE 5000

#define BUF_SIZE 0xFFFF

#define SERVER_WELCOME "欢迎来到聊天室！你的ID是用户#%d"

#define SERVER_MESSAGE "用户 %d 说 >> %s"

#define EXIT "EXIT"

#define CAUTION "聊天室只有你一个人！"

int setnonblocking(int sockfd){//非阻塞
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
    return 0;
}

void addfd(int epollfd, int fd, bool enable_et){
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if(enable_et){
        ev.events = EPOLLIN | EPOLLET;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);//添加监控事件ev，把事件挂到红黑树上
    setnonblocking(fd);
}

int sendmessege(int clientfd){
    char buf[BUF_SIZE], messege[BUF_SIZE];

    bzero(buf, BUF_SIZE);
    bzero(messege, BUF_SIZE);

    //printf("从用户(用户ID = %d)读取\n", clientfd);
    int len = recv(clientfd, buf, BUF_SIZE, 0);

    if(len == 0){
        close(clientfd);
        clients_list.remove(clientfd);
         printf("用户 %d 关闭.\n 现在有 %d 个用户在聊天室中\n", clientfd, (int)clients_list.size());
    }
    else{
        sprintf(messege, SERVER_MESSAGE, clientfd, buf);//把recv到的内容读取到messege中
        if(send(clientfd, messege, BUF_SIZE, 0) < 0){
            perror("出错啦！");
            exit(-1);
        }
    }
}

int main(int argc, char* argv[]){

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if(listenfd < 0){
        perror("listenfd");
    	exit(-1);
    }
    printf("监听 socket 创建成功\n");

    if(bind(listenfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
        perror("bind error");
        exit(-1);
    }

    int ret = listen(listenfd, 5);
    if(ret < 0){
        perror("listen error");
        exit(-1);
    }
    int epfd = epoll_create(EPOLL_SIZE);//创建红黑树
    if(epfd < 0){
        perror("epfd error");
        exit(-1);
    }
    printf("epoll 创建成功, epoll 大小 = %d\n", epfd);
    static struct epoll_event events[EPOLL_SIZE];

    addfd(epfd, listenfd, true);

    while(1)
    {
        int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
        if(epoll_events_count < 0){
            perror("epoll failure");
            break;
        }
        printf("epoll 事件数量 = %d\n", epoll_events_count);

        for(int i = 0; i < epoll_events_count; i++){
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd){
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(struct sockaddr_in);
                int clientfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
                
                printf("用户连接来自于: %s : % d(IP : port), clientfd = %d \n", inet_ntoa(client_address.sin_addr),
                ntohs(client_address.sin_port),
                clientfd);

                addfd(epfd, clientfd, true);

                clients_list.push_back(clientfd);
                printf("增加新的 clientfd = %d 到 epoll\n", clientfd);
                printf("现在有 %d 用户在聊天室中\n", (int)clients_list.size());
 
                //服务端发送欢迎消息
                char message[BUF_SIZE];
                bzero(message, BUF_SIZE);
                sprintf(message, SERVER_WELCOME, clientfd);
                int ret = send(clientfd, message, BUF_SIZE, 0);
                if(ret < 0){
                    perror("error");
                    exit(-1);
                }
            }
            else{
                int ret = sendmessege(sockfd);
                if(ret < 0){
                    perror("error");
                    exit(-1);
                }
            }

        }   
    }
    close(listenfd);
    close(epfd);
    return 0;
}
