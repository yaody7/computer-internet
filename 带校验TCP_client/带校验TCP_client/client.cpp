#include<WinSock2.h>
#include<WS2tcpip.h>
#include<stdio.h>
#include<fstream>
#include<time.h>
#include<Windows.h>
#include<iostream>
#include<string>
#include<stdio.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
using namespace std;
#define PORT 8888

#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32.lib")

using namespace std;
int main() {
	cout << "Client is standby, press Enter to continue!" << endl;
	getchar();
	WSADATA wsaData;				//Socket ׼������
	SOCKET client;
	SOCKADDR_IN ServerAdd;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ServerAdd.sin_family = AF_INET;
	ServerAdd.sin_port = htons(PORT);
	ServerAdd.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	connect(client, (SOCKADDR*)& ServerAdd, sizeof(ServerAdd));

	short Checksum;

	FILE* fd;
	int count = 0;
	char buffer[1020];
	string wenjianming = "test.txt";
	fd = fopen(wenjianming.c_str(), "rb");					//��ð�����
	while (!feof(fd)) {
		char sendbuffer[1024];
		int wordcount = fread(sendbuffer, sizeof(char), 1020, fd);
		count++;
	}
	fclose(fd);

	string temp = to_string(count);			
	while (temp.size() != 6)
		temp = "0" + temp;
	string temptime = to_string(time(NULL));			//�����ļ����Լ���Ҫ��Ϣ�Ĵ����
	temp += temptime;
	temp += wenjianming;
	send(client, temp.c_str(), 1024, 0);

	struct _stat buf;
	char time[26];
	_stat(wenjianming.c_str(), &buf);
//	printf("%ld\n", buf.st_size);
	ctime_s(time, 26, &buf.st_ctime);
	send(client, time, 27, 0);					            	//����ʱ��
	send(client, to_string(buf.st_size).c_str(), 1024, 0);		//���ʹ�С

	fd = fopen(wenjianming.c_str(), "rb");
	short help = 0;
	while (!feof(fd)) {
		char sendbuffer[1024];
		int wordcount = fread(sendbuffer, sizeof(char), 1020, fd);			//��ȡ�ļ�����
		string tempcount = to_string(wordcount);
		while (tempcount.size() != 4)										//��ȡ���ݵĴ�С������4λ����4λ
			tempcount = "0" + tempcount;
		for (int i = 1020; i < 1024; i++) {									//�����ݴ�С���봫�����
			sendbuffer[i] = tempcount[i - 1020];							// ���ṹ��
		}																	//		|0...............1019|1020.......1023|
		for (int i = 0; i < 1024; i += 2) {									//		|      �ļ�����		 |    ���ݴ�С   |
			help += (short)(sendbuffer[i]);
			help += ((short)(sendbuffer[i + 1]) << 16);
		}
		send(client, sendbuffer, 1024, 0);
	}
	Checksum = ~help;

	//printf("%x\n", Checksum);

	/*
	printf("%x\n", tocheck[0]);
	printf("%x\n", tocheck[1]);
	*/
	char tocheck[16];

	for (int i = 0; i < 16; i++) {					//У�鲿��
		if (Checksum & 1) {
			tocheck[16 - 1 - i] = '1';
		}
		else {
			tocheck[16 - 1 - i] = '0';
		}
		Checksum >>= 1;
	}

/*	for (int i = 0; i < 16; i++)
		printf("%c", tocheck[i]);
	printf("\n");*/
	send(client, tocheck, 16, 0);
	fclose(fd);
	closesocket(client);			//�Ͽ����ӣ�����socket
	WSACleanup();
	system("pause");
	return 0;
}
