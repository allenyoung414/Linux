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

pthread_mutex_t mutex;

void *client_process(void *arg)
{
    char buf[1024] = "";
    int cfd = *(int *)arg;
    pthread_mutex_unlock(&mutex);

    recv(cfd, buf, sizeof(buf), 0);
    int ret = strlen(buf);
    if(ret == 0)
    {
        close(cfd);
        exit(1);
    }
    for(int i = 0; i < ret; i++)
    {
        buf[i] = toupper(buf[i]);
    }

    send(cfd, buf, ret, 0);

    write(STDOUT_FILENO, buf, ret);
    close(cfd);
    return NULL;
}

int main(int argc, char* argv[])
{
    int lfd, cfd;
    socklen_t clt_addr_len;
    struct sockaddr_in srv_addr, clt_addr;

    pthread_mutex_init(&mutex, NULL);

    bzero(&srv_addr, 0);
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(8000);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    lfd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));

    bind(lfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

    listen(lfd, 128);

    clt_addr_len = sizeof(clt_addr);

    pthread_t thread_id;

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
        if(cfd > 0)
        {
            pthread_create(&thread_id, NULL, client_process, (void *)&cfd);
            pthread_detach(thread_id);
        }
    }
    close(lfd);
    return 0;
}