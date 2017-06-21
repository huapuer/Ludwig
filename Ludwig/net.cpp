#include <stdio.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
#include "ludwig_net.h"

#pragma comment(lib,"ws2_32.lib")

SOCKET friedrich_socket;
SOCKET alan_socket;

struct net_config {
	char* ip_addr;
	int port;
};

DWORD WINAPI fiedrich_thread(LPVOID pM)
{
	net_config* cfg= (net_config*)pM;

    //初始化WSA
    WORD sockVersion = MAKEWORD(2,2);
    WSADATA wsaData;
    if(WSAStartup(sockVersion, &wsaData)!=0)
    {
        return 0;
    }

    //创建套接字
    SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(slisten == INVALID_SOCKET)
    {
        printf("socket error !");
        return 0;
    }

    //绑定IP和端口
    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(cfg->port);
    sin.sin_addr.S_un.S_addr = INADDR_ANY; 
    if(bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
    {
        printf("bind error !");
    }

    //开始监听
    if(listen(slisten, 5) == SOCKET_ERROR)
    {
        printf("listen error !");
        return 0;
    }

    //循环接收数据
    sockaddr_in remoteAddr;
    int nAddrlen = sizeof(remoteAddr);
    char revData[255]; 
    printf("等待连接...\n");
	alan_socket = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);
    if(alan_socket == INVALID_SOCKET)
    {
        printf("accept error !");
    }
	char IPdotdec[20];
	inet_ntop(AF_INET, &remoteAddr.sin_addr, IPdotdec, 16);
    printf("接受到一个连接：%s \r\n", IPdotdec);

	while (true) {
		//接收数据
		int ret = recv(alan_socket, revData, 255, 0);
		if (ret > 0)
		{
			revData[ret] = 0x00;
			if (friedrich_acts_table[(int)net_events::EVENT_TEST]) {
				friedrich_acts_table[net_events::EVENT_TEST](revData, ret);
			}
		}
	}
    
    closesocket(slisten);
    WSACleanup();
    return 0;
}

DWORD WINAPI alan_thread(LPVOID pM)
{
	net_config* cfg = (net_config*)pM;

	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}

	friedrich_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (friedrich_socket == INVALID_SOCKET)
	{
		printf("invalid socket !");
		return 0;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(cfg->port);
	inet_pton(AF_INET, cfg->ip_addr, &serAddr.sin_addr.S_un.S_addr);
	if (connect(friedrich_socket, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("connect error !");
		closesocket(friedrich_socket);
		return 0;
	}

	alan_says(net_events::EVENT_TEST, "Hello, Friedrich!", 17);

	while (true) {
		char recData[255];
		int ret = recv(friedrich_socket, recData, 255, 0);
		if (ret > 0)
		{
			recData[ret] = 0x00;
			if (alan_acts_table[(int)net_events::EVENT_TEST]) {
				alan_acts_table[net_events::EVENT_TEST](recData, ret);
			}
			printf(recData);
		}
	}
	closesocket(friedrich_socket);
	WSACleanup();
	return 0;
}

void friedrich_talking(int port) {
	net_config* cfg = new net_config();
	cfg->port = port;
	HANDLE handle = CreateThread(NULL, 0, fiedrich_thread, cfg, 0, NULL);
}

void alan_talking(char* ip_addr, int port) {
	net_config* cfg = new net_config();
	cfg->ip_addr = ip_addr;
	cfg->port = port;
	HANDLE handle = CreateThread(NULL, 0, alan_thread, cfg, 0, NULL);
}

void friedrich_acts(net_events ne, fp_event_callback ec) {
	friedrich_acts_table[ne] = ec;
}

void alan_acts(net_events ne, fp_event_callback ec) {
	alan_acts_table[ne] = ec;
}

void friedrich_says(net_events ne, char* c, int size) {
	send(alan_socket, c, size, 0);
}

void alan_says(net_events ne, char* c, int size) {
	send(friedrich_socket, c, size, 0);
}