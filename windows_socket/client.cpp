#include<iostream>
#include<thread>
#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")

using namespace std;

void Send(SOCKET client, char send_buf[]) {
    while (true) {
        cin >> send_buf;
        send(client, send_buf, 100, 0);
    }
}

void Recv(SOCKET client, char recv_buf[]) {
    while (true) {
        recv(client, recv_buf, 100, 0);
        cout << recv_buf << endl;
    }
}

int main()
{
    char send_buf[100];//发送缓冲区
    char recv_buf[100];//接收缓冲区

    thread thread_send;
    thread thread_recv;

    SOCKET client;

    SOCKADDR_IN client_addr;//使用结构体 SOCKADDR_IN 存储配置

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    client = socket(AF_INET, SOCK_STREAM, 0);

    memset(&client_addr, 0, sizeof(client_addr)); //清零
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //server端ip地址
    client_addr.sin_port = htons(1234); //监听端口

    connect(client, (SOCKADDR*)&client_addr, sizeof(SOCKADDR));

    thread_send = thread(Send, client, send_buf);
    thread_recv = thread(Recv, client, recv_buf);

    thread_send.join();
    thread_recv.join();

    //关闭套接字
    closesocket(client);

    WSACleanup();
    return 0;
}
