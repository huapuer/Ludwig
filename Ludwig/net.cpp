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

    //��ʼ��WSA
    WORD sockVersion = MAKEWORD(2,2);
    WSADATA wsaData;
    if(WSAStartup(sockVersion, &wsaData)!=0)
    {
        return 0;
    }

    //�����׽���
    SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(slisten == INVALID_SOCKET)
    {
        printf("socket error !");
        return 0;
    }

    //��IP�Ͷ˿�
    sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(8888);
    sin.sin_addr.S_un.S_addr = INADDR_ANY; 
    if(bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
    {
        printf("bind error !");
    }

    //��ʼ����
    if(listen(slisten, 5) == SOCKET_ERROR)
    {
        printf("listen error !");
        return 0;
    }

    //ѭ����������
    sockaddr_in remoteAddr;
    int nAddrlen = sizeof(remoteAddr);
    char revData[255]; 
        printf("�ȴ�����...\n");
		*accepted_socket = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);
        if(*accepted_socket == INVALID_SOCKET)
        {
            printf("accept error !");
        }
		char IPdotdec[20];
		inet_ntop(AF_INET, &remoteAddr.sin_addr, IPdotdec, 16);
        printf("���ܵ�һ�����ӣ�%s \r\n", IPdotdec);
        
		while (true) {
			//��������
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
	char * sendData = "��ã�TCP����ˣ����ǿͻ���!\n";
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