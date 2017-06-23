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

struct net_buffer {
	char* buff;
	unsigned long buff_len = 0;
	unsigned long recv_len = 0;
	unsigned long decl_len = 0;
};

unsigned long chars2ulong(char* src) {
	unsigned long ret = 0;
	for (int i = 0; i < sizeof(unsigned long); i++) {
		ret |= src[i] << ((sizeof(unsigned long) - i -1) * 8);
	}
	return ret;
}

unsigned int chars2uint(char* src) {
	unsigned int ret = 0;
	for (int i = 0; i < sizeof(unsigned int); i++) {
		ret |= src[i] << ((sizeof(unsigned int) - i - 1) * 8);
	}
	return ret;
}

int test_count = 0;

void unpack(net_buffer* buff, char* recv_content, unsigned long len, fp_event_callback* callbacks) {
	unsigned long total_len = buff->recv_len + len;
	if (total_len > buff->buff_len) {
		char* old_buff = buff->buff;
		buff->buff = (char*)malloc(total_len);
		if (buff->buff_len > 0) {
			memcpy(buff->buff, old_buff, buff->recv_len);
			free(old_buff);
		}
		buff->buff_len = total_len;
	}

	memcpy(buff->buff+buff->recv_len, recv_content, len);
	buff->recv_len = total_len;

	unsigned long header_len = sizeof(unsigned long) + sizeof(unsigned int);
	while (buff->recv_len > header_len) {
		if (buff->decl_len == 0) {
			buff->decl_len = chars2ulong(buff->buff);
		}

		if (buff->recv_len >= buff->decl_len) {
			unsigned int event_no = chars2uint(buff->buff + sizeof(unsigned long));
			callbacks[event_no](buff->buff + header_len, buff->decl_len- header_len);

			buff->recv_len -= buff->decl_len;
			memcpy(buff->buff, buff->buff + buff->decl_len, buff->recv_len);
			buff->decl_len = 0;
		}
		else {
			break;
		}
	}
}

void ulong2chars(unsigned long val, char* dst) {
	for (int i = 0; i < sizeof(unsigned long); i++) {
		dst[i] = val >> ((sizeof(unsigned long) - i - 1) * 8);
	}
}

void uint2chars(unsigned int val, char* dst) {
	for (int i = 0; i < sizeof(unsigned int); i++) {
		dst[i] = val >> ((sizeof(unsigned int) - i - 1) * 8);
	}
}

void pack(net_events ne, char* c, unsigned long size, SOCKET socket) {
	unsigned long header_len = sizeof(unsigned long) + sizeof(unsigned int);
	char* p = (char*)malloc(header_len + size);

	ulong2chars(header_len + size, p);
	uint2chars(ne, p + sizeof(unsigned long));

	memcpy(p + header_len, c, size);
	
	send(socket, p, header_len + size, 0);
}

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
    printf("等待连接...\n");
	alan_socket = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);
    if(alan_socket == INVALID_SOCKET)
    {
        printf("accept error !");
    }
	char IPdotdec[20];
	inet_ntop(AF_INET, &remoteAddr.sin_addr, IPdotdec, 16);
    printf("接受到一个连接：%s \r\n", IPdotdec);

	char recvData[255];
	net_buffer* nb = new(net_buffer);
	while (true) {
		//接收数据
		int ret = recv(alan_socket, recvData, 255, 0);
		if (ret > 0)
		{
			unpack(nb, recvData, ret, friedrich_acts_table);
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

	for (int i = 0; i < 10000; i++) {
		alan_says(net_events::EVENT_TEST, "Hello, Friedrich!", 17);
	}

	char recvData[255];
	net_buffer* nb = new(net_buffer);
	while (true) {
		int ret = recv(friedrich_socket, recvData, 255, 0);
		if (ret > 0)
		{
			unpack(nb, recvData, ret, alan_acts_table);
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

void friedrich_says(net_events ne, char* c, unsigned long size) {
	pack(ne, c, size, alan_socket);
}

void alan_says(net_events ne, char* c, unsigned long size) {
	pack(ne, c, size, friedrich_socket);
}