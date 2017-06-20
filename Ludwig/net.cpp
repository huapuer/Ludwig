#include <stdio.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>

#pragma comment(lib,"ws2_32.lib")

SOCKET friedrich_socket;
SOCKET alan_socket;

DWORD WINAPI server_thread(LPVOID pM)
{
	SOCKET* accepted_socket = (SOCKET*)pM;

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
    sin.sin_port = htons(8888);
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
		*accepted_socket = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);
        if(*accepted_socket == INVALID_SOCKET)
        {
            printf("accept error !");
        }
		char IPdotdec[20];
		inet_ntop(AF_INET, &remoteAddr.sin_addr, IPdotdec, 16);
        printf("接受到一个连接：%s \r\n", IPdotdec);
        
		while (true) {
			//接收数据
			int ret = recv(*accepted_socket, revData, 255, 0);
			if (ret > 0)
			{
				revData[ret] = 0x00;
				printf(revData);
			}
		}
    
    closesocket(slisten);
    WSACleanup();
    return 0;
}

DWORD WINAPI client_thread(LPVOID pM)
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}

	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		printf("invalid socket !");
		return 0;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8888);
	inet_pton(AF_INET, "127.0.0.1", &serAddr.sin_addr.S_un.S_addr);
	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("connect error !");
		closesocket(sclient);
		return 0;
	}
	char * sendData = "你好，TCP服务端，我是客户端!\n";
	send(sclient, sendData, strlen(sendData), 0);

	while (true) {
		char recData[255];
		int ret = recv(sclient, recData, 255, 0);
		if (ret > 0)
		{
			recData[ret] = 0x00;
			printf(recData);
		}
	}
	closesocket(sclient);
	WSACleanup();
	return 0;
}

void make_friedrich_talk() {
	HANDLE handle = CreateThread(NULL, 0, server_thread, NULL, 0, NULL);
}

void make_alan_talk() {
	HANDLE handle = CreateThread(NULL, 0, server_thread, NULL, 0, NULL);
}