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
#include <sys/time.h>
 
using namespace std;
 
// server port
#define SERVER_PORT 8888
 
//epoll 支持的最大并发量
#define EPOLL_SIZE 5000
 
//message buffer size
#define BUF_SIZE 0xFFFF
 
// exit
#define EXIT "exit"
 
//设置sockfd，pipefd非阻塞
int setnonblocking(int sockfd){
	fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
	return 0;
}

int addfd(int epollfd, int fd, bool enable_et = true){
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN; //输入出发epoll-event
	if(enable_et){
		ev.events = EPOLLIN | EPOLLET;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
	setnonblocking(fd);
}

unsigned int getTime()
{
    struct timeval time;
 
    /* 获取时间，理论到us */
    gettimeofday(&time, NULL);
    return (time.tv_sec*1000 + time.tv_usec/1000);
 
}

int main(int argc, char *argv[])
{ 
    int i;
    int pid;
    //管道
    int pipe_fd[2];
    //socket实例
    int sock;
    char message[BUF_SIZE];

    //server 地址信息结构体
    struct sockaddr_in sockAddr;
    //连接方式、ip、端口
    sockAddr.sin_family = AF_INET;
    printf("请输入server地址\n");
    char server_ip[BUF_SIZE];
    fgets(server_ip, BUF_SIZE, stdin);
    const char* servInetAddr = server_ip;
	inet_pton(AF_INET, servInetAddr, &sockAddr.sin_addr);
    sockAddr.sin_port = htons(SERVER_PORT);

    //epoll实例
    int epfd;
    //ready list event数量
    int epoll_events_count;
    //epoll监听events
    static struct epoll_event events[2];
    int isClientWork = true;

    //创建socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("create socket fail.\n");
        exit(-1);
    }
    //连接 server socket
    if (connect(sock, (struct sockaddr *)&sockAddr, sizeof(sockAddr)) < 0) {
        printf("connect server fail.\n");
        exit(-1);
    }
    //创建管道用于子进程写数据，父进程读取数据
    if (pipe(pipe_fd) < 0) {
        printf("create pipe fail.\n");
        exit(-1);
    }
    //创建epoll instance
    epfd = epoll_create(EPOLL_SIZE);
    if (epfd < 0) {
        printf("create epoll instance fail.\n");    
        exit(-1);
    }
    //添加sockt 和 管道读文件描述符到 interest list   
    addfd(epfd, sock);
    addfd(epfd, pipe_fd[0]);

    unsigned int start_time;
    int bytes_sent = 0;
    int bytes_received = 0;
    
    pid = fork();
    if (pid < 0) {
        printf("fork fail.\n");    
        exit(-1);
    } else if (pid == 0) {
        //这里是子进程    
        //子进程关闭读端
        close(pipe_fd[0]);
        printf("input 'exit' if you want quit chat room.\n");
        while (isClientWork) {
            bzero(&message, BUF_SIZE);
            //从标准输入读取数据
            fgets(message, BUF_SIZE, stdin);
            //判断是否输入了'exit'
            if (strncasecmp(message, "exit", 4) == 0) {
                isClientWork = false;    
            } else {
                //从标准输入读取的数据写入管道    
                if (write(pipe_fd[1], message, strlen(message) - 1) < 0) {
                    printf("client write pip fail.\n");
                    exit(-1);
                }
            }
        }
    } else {
        //这里是父进程    
        //父进程关闭写端
        close(pipe_fd[1]);
        while (isClientWork) {
            
                epoll_events_count = epoll_wait(epfd, events, 2, -1);               
            //处理事件
            for (i = 0; i < epoll_events_count; i++) {
                bzero(&message, BUF_SIZE);
                //服务端发来的消息    
                if (events[i].data.fd == sock) {
                    if (recv(sock, message, BUF_SIZE, 0) == 0) {
                        printf("server error\n");
                        isClientWork = false;
                        close(sock);
                    } else {
                        sleep(1);
                        printf("%s\n", message);
                        bytes_received += sizeof(message); 
                        cout<<start_time<<endl;
                        unsigned int duration = getTime() - start_time;
                        duration = (double)duration / 1000;
                        cout<<duration<<endl;
                        double speed = (double) bytes_received / duration / 1024 / 1024;
                        cout << "Test finished in " << duration << " s, speed: " << speed << " Mbps" << endl;
                    }   
                } else {
                    //子进程写管道事件    
                    //从管道中读取数据
                    if (read(events[i].data.fd, message, BUF_SIZE) == 0) {
                        printf("parent read pipe fail\n");
                        isClientWork = false;
                    } else {
                        start_time = getTime();
                        cout<<start_time<<endl;
                        send(sock, message, BUF_SIZE, 0);
                        bytes_sent += sizeof(message);    
                    }
                }
            

            }
            
        }
    }
    if (pid > 0) {
        close(pipe_fd[0]);
        close(sock);
    } else {
        close(pipe_fd[1]);    
    }
    return 0;
}