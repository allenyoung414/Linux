#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>
#include<iostream>
using namespace std;

void sys_err(const char *str)
{
    perror(str);
    exit(1);
}

// 信号处理函数
void sig_chld(int signo)
{
    pid_t pid;
    int stat; // 回收状态
    // 以非阻塞形式回收子进程
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated\n", pid);
    return;
}

int main(int argc, char* argv[])
{
    int lfd, cfd;
    pid_t pid;
    socklen_t clt_addr_len;
    struct sockaddr_in srv_addr, clt_addr;
    bzero(&srv_addr, 0);
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(8000);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    lfd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));

    bind(lfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));

    listen(lfd, 128);

    clt_addr_len = sizeof(clt_addr);

    signal(SIGCHLD, sig_chld);

    char buf[512];
    while(1)
    {
        cfd = accept(lfd, (struct sockaddr*)&clt_addr, &clt_addr_len);
        if(cfd==-1)
        {
            printf("connect failed\n");
            break;//这里还有点问题
        }
        else
        printf("client %s has connnected\n",inet_ntoa(clt_addr.sin_addr));

        if((pid == fork()) < 0)
        {
            sys_err("fork");
        }
        else if(pid == 0)
        {
            close(lfd);
            while(1)
            {
                memset(buf, 0, 512);

                int iret = recv(cfd, buf, sizeof(buf), 0);
                if (iret<=0) 
                {
                    printf("iret=%d\n",iret); break;  
                }
                printf("receive :%s\n",buf);

                int ret = strlen(buf);
                if(ret == 0)
                {
                    close(cfd);
                    exit(0);
                }

                for(int i = 0; i < ret; i++)
                {
                    buf[i] = toupper(buf[i]);
                }

                send(cfd, buf, ret, 0);
                write(STDOUT_FILENO, buf, ret);
                memset(buf, 0, 512);
                close(cfd);
                exit(0);
            }
        }
        else{
            close(cfd);
            continue;
        }
    }
    close(lfd);
    return 0;
}