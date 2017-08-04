#include "ludwig_net_sync.h"
#ifdef NET_SYNC

#include <stdio.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>

#pragma comment(lib,"ws2_32.lib")

SOCKET friedrich_socket;
SOCKET alan_socket;

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

SOCKET friedrich_listen;

void friedrich_talking(int port) {
	//初始化WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		exit(1);
	}

	//创建套接字
	friedrich_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (friedrich_listen == INVALID_SOCKET)
	{
		printf("socket error !");
		exit(1);
	}

	//绑定IP和端口
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(friedrich_listen, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("bind error !");
		exit(1);
	}

	//开始监听
	if (listen(friedrich_listen, 5) == SOCKET_ERROR)
	{
		printf("listen error !");
		exit(1);
	}
}

void friedrich_hearing() {
	//循环接收数据
	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);
	printf("等待连接...\n");
	alan_socket = accept(friedrich_listen, (SOCKADDR *)&remoteAddr, &nAddrlen);
	if (alan_socket == INVALID_SOCKET)
	{
		printf("accept error !");
	}
	char IPdotdec[20];
	inet_ntop(AF_INET, &remoteAddr.sin_addr, IPdotdec, 16);
	printf("接受到一个连接：%s \r\n", IPdotdec);

	char recvData[255];
	net_buffer* nb = new(net_buffer);

	//接收数据
	int ret = recv(alan_socket, recvData, 255, 0);
	if (ret > 0)
	{
		unpack(nb, recvData, ret, friedrich_acts_table);
	}
}

void friedrich_quiet() {
	closesocket(friedrich_listen);
	WSACleanup();
}

void alan_talking(char* ip_addr, int port) {
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		exit(1);
	}

	friedrich_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (friedrich_socket == INVALID_SOCKET)
	{
		printf("invalid socket !");
		exit(1);
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip_addr, &serAddr.sin_addr.S_un.S_addr);
	while (connect(friedrich_socket, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR);
}

void alan_hearing() {
	char recvData[255];
	net_buffer* nb = new(net_buffer);
	int ret = recv(friedrich_socket, recvData, 255, 0);
	if (ret > 0)
	{
		unpack(nb, recvData, ret, alan_acts_table);
	}
}

void alan_quiet() {
	closesocket(friedrich_socket);
	WSACleanup();
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

#endif