#include<WinSock2.h>
#include<WS2tcpip.h>
#include<stdio.h>
#include<time.h>
#include<string>
#include<fstream>
#include<Windows.h>
#include<iostream>
#include<cstring>
#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32.lib")
#define PORT 8888
using namespace std;
FILE* fd;
long int starttime = 0;
int packetcount = 0;
bool oktobegin = false;
int turn = 0;
bool ishelp = false;
short help0 = 0;
short help1 = 0;
short help = 0;
DWORD WINAPI transfer(LPVOID param) {		//�̰߳󶨺���
	DWORD socketid = *(DWORD*)param;		//Socket׼������
	//printf("%d\n", socketid);
	WSADATA wsaData;
	SOCKET Listening;
	SOCKET Transfer;
	SOCKADDR_IN ServerAdd;
	SOCKADDR_IN ClientAdd;
	int Clientlen = sizeof(ClientAdd);
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	Listening = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ServerAdd.sin_family = AF_INET;
	ServerAdd.sin_port = htons(PORT + socketid);
	ServerAdd.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	bind(Listening, (SOCKADDR*)& ServerAdd, sizeof(ServerAdd));
	listen(Listening, 10);
	Transfer = accept(Listening, (SOCKADDR*)& ClientAdd, &Clientlen);

	int count = 0;
	char receiveBuffer[1024];
	recv(Transfer, receiveBuffer, 1024, 0);
	if (socketid == 0) {						//��0���߳� ���й���Ԫ��Ϣ�����
		cout << "�ļ����ƣ�" << receiveBuffer + 16 << endl;		//����ļ�����
		char receiveBuffer1[1024];
		recv(Transfer, receiveBuffer1, 27, 0);		//����ʱ��
		cout << "�ļ�ʱ�䣺" << receiveBuffer1;		//���ʱ��
		recv(Transfer, receiveBuffer1, 1024, 0);		//�����ļ���С
		int filesize = 0;
		for (int i = 0; i < strlen(receiveBuffer1); i++) {
			filesize *= 10;
			filesize += (receiveBuffer1[i] - '0');
		}
		cout << "�ļ���С��" << filesize / 1024 << " KB" << endl;		//����ļ���С
	}
	string tempbuffer = receiveBuffer;
	string wenjianming = tempbuffer.substr(16, tempbuffer.size() - 16);
	wenjianming = "./recv/" + wenjianming;
	if (socketid == 0) {							//�ṹ��
													//	|0........5|6...........15|16..............1023|
		for (int i = 0; i < 10; i++) {				//	|  ������  | ���俪ʼʱ�� |      �ļ�����      |
			starttime *= 10;
			starttime += (receiveBuffer[i + 6] - '0');
		}

		for (int i = 0; i < 6; i++) {
			packetcount *= 10;
			packetcount += (receiveBuffer[i] - '0');
		}
		fd = fopen(wenjianming.c_str(), "ab+");
		oktobegin = true;
	}
	while (oktobegin == false);
	for (int i = 0; i < packetcount; i++) {
		recv(Transfer, receiveBuffer, 1024, 0);		//���հ�
		string queren = "END";						//���ṹ��
		if (queren == receiveBuffer)				//	|0...............1019|1020.......1023|
			break;									//	|      �ļ�����		 |    ���ݴ�С   |
		int hangnumber = 0;
		for (int j = 0; j < 4; j++) {
			hangnumber *= 10;
			hangnumber += receiveBuffer[j + 1020] - '0';
		}
		if (socketid == 0) {					//���ʻ�����
			while (turn == 1);
		}
		else
			while (turn == 0);

		for (int i = 0; i < 1024; i += 2) {
			if (socketid == 0) {
				help0 += (short)(receiveBuffer[i]);
				help0 += ((short)(receiveBuffer[i + 1]) << 16);
			}
			else {
				help1 += (short)(receiveBuffer[i]);
				help1 += ((short)(receiveBuffer[i + 1]) << 16);
			}
		}

		fwrite(receiveBuffer, hangnumber, 1, fd);
		if (socketid == 0)					//�˳�������
			turn = 1;
		else
			turn = 0;
	}
	closesocket(Transfer);
	closesocket(Listening);
	WSACleanup();
	return 0;
}
int main() {
	cout << "Server is ready! Press Enter to continue!" << endl;
	getchar();
	//�����̣߳�
	DWORD ThreadID[2];
	HANDLE ThreadHandle[2];
	int Param0 = 0;
	int Param1 = 1;
	ThreadHandle[0] = CreateThread(				//�����߳�
		NULL,
		0,
		transfer,
		&Param0,
		0,
		&ThreadID[0]
	);
	ThreadHandle[1] = CreateThread(
		NULL,
		0,
		transfer,
		&Param1,
		0,
		&ThreadID[1]);
	for (int i = 0; i < 2; i++) {				//�ȴ��߳̽���
		if (ThreadHandle[i] != NULL) {
			WaitForSingleObject(ThreadHandle[i], INFINITE);
			CloseHandle(ThreadHandle[i]);
		}
	}

	char tocheck[16];

	WSADATA wsaData;
	SOCKET Listening;
	SOCKET Transfer;
	SOCKADDR_IN ServerAdd;
	SOCKADDR_IN ClientAdd;
	int Clientlen = sizeof(ClientAdd);
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	Listening = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ServerAdd.sin_family = AF_INET;
	ServerAdd.sin_port = htons(PORT);
	ServerAdd.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	bind(Listening, (SOCKADDR*)& ServerAdd, sizeof(ServerAdd));
	listen(Listening, 10);
	Transfer = accept(Listening, (SOCKADDR*)& ClientAdd, &Clientlen);

	recv(Transfer, tocheck, 16, 0);				//У�鲿��
	short checksum = 0;
	for (int i = 0; i < 16; i++) {
		checksum <<= 1;
		checksum |= (tocheck[i] - '0');
	}
	/*
	printf("%x\n", checksum);

	for (int i = 0; i < 16; i++)
		printf("%c", tocheck[i]);
	printf("\n");
	*/
	help = help0 + help1;

	if (~(help + checksum) == 0)
		printf("У��ɹ�!\n");
	else
		printf("У��ʧ��!\n");

	long int endtime = time(NULL);
	cout << "����ʱ�䣺" << endtime - starttime << endl;
	if (endtime - starttime == 0)
		cout << "�����ٶȣ�NA(����ʱ���С)" << endl;
	else
		cout << "�����ٶȣ�" << packetcount  / (endtime - starttime) << "KB/s" << endl;
	system("pause");
	fclose(fd);
	return 0;
}
