#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <thread>
#pragma comment(lib, "ws2_32.lib")
// �������ݱ��ĵĸ�ʽ���壬��Ҫ�ͷ������˱���һ��

// ��Ϣ����
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};

// ��ͷ������������Ϣ���Ĵ�С���������ݵ�����
struct DataHeader
{
	short dataLength;
	short cmd;
};

// ���壺 ����
struct Login :public DataHeader // ��Ϣ���ṹֱ�Ӽ̳а�ͷ�ṹ����������ֱ���ڹ��캯��ֱ�ӳ�ʼ����ͷ
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char password[32];
};

struct LoginResult :public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0; // 0 Ϊ��¼����1 Ϊ��¼�ɹ�
	}
	int result;
};

struct Logout :public DataHeader
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutResult :public DataHeader
{
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0; // 0 Ϊ�ǳ�����1 Ϊ�ǳ��ɹ�
	}
	int result;
};

struct NewUserJoin :public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_NEW_USER_JOIN;
		socketId = 0;
	}
	int socketId;
};

int handleSocket(SOCKET _sock)
{
	// �û��������������� Ϊ����Ӧ�̳����ݺͱ䳤���ݡ����ڻ�������ճ�����ְ�������
	char szRecz[4096] = {};
	// �ȶ�ȡ��ͷ���ݣ��ж���Ϣ�����͡�
	int nLen = recv(_sock, szRecz, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecz;

	// �� len ����ʱ����� recv ���� 0��˵�����ӵ�����һ�˷�����һ�� FIN ���ݰ�����ŵû�и�����Ҫ���͵����ݡ�
	if (nLen <= 0)
	{
		printf("��������Ͽ����ӣ��������\n");
		return -1;
	}

	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		// �����Ѿ������˰�ͷ��buffer ��Ҫ�Ӱ��忪ͷ��ȡ��ȡ�������ݡ����ݳ��� len Ӧ���������ݰ���С��ȥ��ͷ��С
		recv(_sock, szRecz + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult* loginResult = (LoginResult*)szRecz;
		printf("���յ�����������Ϣ��CMD_LOGIN_RESULT�����ݳ��ȣ�%d\n", loginResult->dataLength);
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		recv(_sock, szRecz + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LogoutResult* logoutResult = (LogoutResult*)szRecz;
		printf("���յ�����������Ϣ��CMD_LOGOUT_RESULT�����ݳ��ȣ�%d\n", logoutResult->dataLength);
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		recv(_sock, szRecz + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		NewUserJoin* joinResult = (NewUserJoin*)szRecz;
		printf("���յ�����������Ϣ��CMD_NEW_USER_JOIN�����ݳ��ȣ�%d\n", joinResult->dataLength);
	}
	break;
	}
	return 0;
}


bool g_bRun = true;

// ������������߼��ŵ����߳��У������Ļ���Ҫ g_bRun ȫ�ֱ������жϳ����Ƿ�Ҫ�˳�
void cmdThread(SOCKET _sock)
{
	while (true)
	{
		char cmdBuf[128] = {};
		// scanf ��������������Ҫ�������߳���ʹ�ã������������ѭ��
		scanf("%s", cmdBuf);

		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("�˳�����\n");
			g_bRun = false;
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "admin");
			strcpy(login.password, "pwd");

			send(_sock, (char*)&login, sizeof(Login), 0);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "admin");

			send(_sock, (char*)&logout, sizeof(Logout), 0);
		}
		else
		{
			printf("��֧�ֵ����\n");
		}
	}
}

int main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(ver, &data);

	//-- ���� TCP �ͻ���

	// 1. ����һ�� socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		printf("Error: socket �����׽���ʧ��\n");
	}
	else
	{
		printf("socket �����׽��ֳɹ�\n");
	}
	// 2. ���ӷ����� connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	// ��������ַ
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	// ���� connect ����ͨ����Ŀ���������ͳ�ʼ SYN ���ݰ������� TCP ����
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("Error: connect �����׽���ʧ��\n");
	}
	else
	{
		printf("connect �����׽��ֳɹ�\n");
	}

	// �����̣߳��ڶ��������ܸ���������
	std::thread t1(cmdThread, _sock);
	// �����߳������̷߳��룬�������߳��˳������߳���Ȼ attach ���̻߳����
	t1.detach();

	while (g_bRun)
	{
		// ת�� select ����ģ��
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(_sock, &fdRead);
		// ������ timeval �����ã��ȴ�һ�ι̶�ʱ�䡣����һ��������׼���� I/O ʱ���أ����ǲ������ɸò�����ָ��� timeval �ṹ��ָ����������΢������
		// Ҳ����˵�븳ֵ 0 ��ȣ����������������ͻ��ˣ����� select ��ͬʱ�����������顣
		timeval t = { 1,0 };
		int ret = select(_sock, &fdRead, 0, 0, &t);
		if (ret < 0)
		{
			printf("select ���������\n");
			break;
		}

		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);
			if (-1 == handleSocket(_sock))
			{
				printf("select �����������������Ͽ����ӡ�\n");
				break;
			}
		}


		// Sleep(1000); // ֻ������ Windows
	}

	// 7. �ر��׽��� socket
	closesocket(_sock);

	WSACleanup();
	printf("�ͻ������˳������������");
	// ��ֹ�����д���ִ����͹ر�
	getchar();
	return 0;
}