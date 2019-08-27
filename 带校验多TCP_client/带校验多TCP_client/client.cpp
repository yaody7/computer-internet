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
FILE* fd;
string wenjianming = "test2.txt";
int packetcount = 0;
int turn = 0;

short help0 = 0;
short help1 = 0;
short help = 0;
short checksum = 0;
char tocheck[16];

DWORD WINAPI transfer(LPVOID param) {			//线程所绑定的函数
	DWORD socketid = *(DWORD*)param;			//Socket准备过程
	WSADATA wsaData;
	SOCKET client;
	SOCKADDR_IN ServerAdd;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ServerAdd.sin_family = AF_INET;
	ServerAdd.sin_port = htons(PORT + socketid);
	ServerAdd.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	connect(client, (SOCKADDR*)& ServerAdd, sizeof(ServerAdd));

	string temp = to_string(packetcount);
	while (temp.size() != 6)
		temp = "0" + temp;
	string temptime = to_string(time(NULL));
	temp += temptime;
	temp += wenjianming;
	send(client, temp.c_str(), 1024, 0);						//文件名与必要信息传输，结构：
	if (socketid == 0) {			//由0号线程准备管理元						|0........5|6...........15|16..............1023|	
		struct _stat buf;													//	|  包数量  | 传输开始时间 |      文件名称      |
		char time[26];
		_stat(wenjianming.c_str(), &buf);
	//	printf("%ld\n", buf.st_size);
		ctime_s(time, 26, &buf.st_ctime);
		send(client, time, 27, 0);
		send(client, to_string(buf.st_size).c_str(), 1024, 0);
	}
	int packetid = 0;
	while (!feof(fd)) {					
		if (socketid == 0)			//线程互斥实现过程，使用自旋锁技术
			while (turn == 1);
		else
			while (turn == 0);
		char sendbuffer[1024];												//包结构：
		int wordcount = fread(sendbuffer, sizeof(char), 1020, fd);			//		|0...............1019|1020.......1023|
		if (socketid == 0)													//		|      文件内容		 |    内容大小   |
			turn = 1;
		else
			turn = 0;
		packetid++;
		string tempcount = to_string(wordcount);
		while (tempcount.size() != 4)
			tempcount = "0" + tempcount;
		for (int i = 1020; i < 1024; i++) {				//制作文件内容大小部分
			sendbuffer[i] = tempcount[i - 1020];
		}

		for (int i = 0; i < 1024; i += 2) {			//校验部分
			if (socketid == 0) {
				help0 += (short)(sendbuffer[i]);
				help0 += ((short)(sendbuffer[i + 1]) << 16);
			}
			else {
				help1 += (short)(sendbuffer[i]);
				help1 += ((short)(sendbuffer[i + 1]) << 16);
			}
		}

		send(client, sendbuffer, 1024, 0);

	}
	string endcall = "END";
	send(client, endcall.c_str(), 4, 0);
	closesocket(client);
	WSACleanup();
	return 0;
}
int main() {
	fd = fopen(wenjianming.c_str(), "rb");
	while (!feof(fd)) {           //获得包数量
		char sendbuffer[1024];
		int wordcount = fread(sendbuffer, sizeof(char), 1020, fd);
		packetcount++;
	}
	fclose(fd);
	fd = fopen(wenjianming.c_str(), "rb");
	//创建线程
	DWORD ThreadID[2];
	HANDLE ThreadHandle[2];
	int Param0 = 0;
	int Param1 = 1;
	cout << "Client is ready! Press Enter to continue!" << endl;
	getchar();
	ThreadHandle[0] = CreateThread(			//线程的创建
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
	for (int i = 0; i < 2; i++) {			//等待线程结束
		if (ThreadHandle[i] != NULL) {
			WaitForSingleObject(ThreadHandle[i], INFINITE);
			CloseHandle(ThreadHandle[i]);
		}
	}
	help = help0 + help1;
	checksum = ~help;
	short t = checksum;						//校验部分
	for (int i = 0; i < 16; i++) {
		if (checksum & 1) {
			tocheck[16 - 1 - i] = '1';
		}
		else {
			tocheck[16 - 1 - i] = '0';
		}
		checksum >>= 1;
	}

	WSADATA wsaData;
	SOCKET client;
	SOCKADDR_IN ServerAdd;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ServerAdd.sin_family = AF_INET;
	ServerAdd.sin_port = htons(PORT);
	ServerAdd.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	connect(client, (SOCKADDR*)& ServerAdd, sizeof(ServerAdd));
	/*
	for (int i = 0; i < 16; i++)
		printf("%c", tocheck[i]);
	printf("\n");
	printf("%x\n", help);
	*/
	send(client, tocheck, 16, 0);




	system("pause");
	fclose(fd);
	return 0;
}
