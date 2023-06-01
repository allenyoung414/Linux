#include<iostream>
#include<thread>
#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")

using namespace std;

void Send(SOCKET client, char send_buf[])
{
	while(true)
	{
		cin >> send_buf;
		send(client, send_buf, 100, 0);
	}
}
void Recv(SOCKET client, char recv_buf[])
{
	while (true)
	{
		recv(client, recv_buf, 100, 0);
		cout << recv_buf << endl;
	}

}

int main()
{
	int len;

	char send_buf[100];
	char recv_buf[100];

	thread thread_send;
	thread thread_recv;

	SOCKET server;
	SOCKET client;

	SOCKADDR_IN server_addr;
	SOCKADDR_IN client_addr;

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	server = socket(AF_INET, SOCK_STREAM, 0);

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(1234);
	bind(server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));

	listen(server, 10);
	cout << "等待连接......" << endl;

	len = sizeof(SOCKADDR);
	client = accept(server, (SOCKADDR*)&client_addr, &len);
	cout << "连接成功" << endl;

	thread_send = thread(Send, client, send_buf);
	thread_recv = thread(Recv, client, recv_buf);

	thread_send.join();
	thread_recv.join();

	closesocket(server);
	closesocket(client);

	WSACleanup();
	return 0;



}
