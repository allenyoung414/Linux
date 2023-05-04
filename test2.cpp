#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include<assert.h>
#include<stdio.h>
#include<string.h>

static bool stop = false;
//SIGTERM信号的处理函数，触发时结束主程序中的循环
static void handle_term(int sig)
{
    stop = true;
}

int main(int argc, char* argv[])
{
    signal(SIGTERM, handle_term);

     if( argc <= 3 )
    {
        printf( "usage: %s ip_address port_number backlog\n", basename( argv[0] ) );
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int backlog = atoi(argv[3]);

    int sock = socket(PF_INET, SOCK_STREAM, 0);//PF——INET是协议族，tcpipv4，SOCK——STREAM是流服务，0指默认协议
    //相应的还有数据报服务
    assert(sock >= 0);

    //创建一个ipv4 socket地址
    struct sockaddr_in address;
    bzero(&address, sizeof(address));//置0
    address.sin_family = AF_INET;//tcpipv4地址族
    inet_pton(AF_INET, ip, &address.sin_addr);//将ip地址指向的点分十进制ipv4地址转换为网络字节序的地址并存在address.sinaddr中
    address.sin_port = htons(port);//小端存储

    int ret = bind(sock, (struct sockaddr*) & address, sizeof(address));//命名socket
    //将sock与address进行绑定，bind成功时返回0
    assert(ret != -1);

    ret = listen(sock, backlog);//backlog参数提示监听内核监听队列的最大长度
    assert(ret != -1);

    //循环等待连接直到有信号把他中断
    while( !stop)
    {
        sleep(1);
    }

    close(sock);
    return 0;

}