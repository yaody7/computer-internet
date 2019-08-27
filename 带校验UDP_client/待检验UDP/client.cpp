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
#pragma warning(disable : 4996)
#pragma comment(lib,"ws2_32.lib")
using namespace std;
long int ACK = -1;		//使用GBD确认方法，初始化ACK为-1
int main(void) {
	//获取数据包多少
	int count = 0;						//记录传输包个数
	string wenjianming = "test.txt";	//设置文件名
	char sendBuffer[1024];				//传输缓存
	FILE* fd = fopen(wenjianming.c_str(), "rb");		//打开文件
	while (!feof(fd)) {									//获得文件大小
		int wordcount = fread(sendBuffer, sizeof(char), 1014, fd);
		count++;
	}
	fclose(fd);											//关闭文件



	WORD wVersionRequested;								//Socket 准备过程
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		cout << "WSAStartup failed with error: " << err << endl;
		return 1;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		cout << "Winsock.dll Version Error" << endl;
		WSACleanup();
		return 1;
	}
	else {
		cout << "Winsock 2.2 dll is standby." << endl;
	}
	unsigned long ul = 1;

	SOCKET socketClient = socket(AF_INET, SOCK_DGRAM, 0);
	ioctlsocket(socketClient, FIONBIO, (unsigned long*)& ul);		//设置成非阻塞模式。 
	SOCKADDR_IN addrServer;
	memset(&addrServer, 0, sizeof(addrServer));
	addrServer.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");		//设置IP地址及各网络参数
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(3000);								
	cout << "Begin to send data, press Enter to continue." << endl;
	getchar();
	char receiveBuffer[100];			//接受缓存
	int length = sizeof(SOCKADDR);
	int sendcount = 0;					


	string temp = to_string(count);
	while (temp.size() != 6) {
		temp = "0" + temp;
	}
	string temptime = to_string(time(NULL));
	temp += temptime;
	temp += wenjianming;
	sendto(socketClient, temp.c_str(), 1024, 0, (SOCKADDR*)& addrServer, sizeof(SOCKADDR));
	Sleep(20);
	struct _stat buf;
	char time[26];
	_stat("test.txt", &buf);
	printf("%ld\n", buf.st_size);
	ctime_s(time, 26, &buf.st_ctime);
	sendto(socketClient, time, 27, 0, (SOCKADDR*)& addrServer, sizeof(SOCKADDR));
	sendto(socketClient, to_string(buf.st_size).c_str(), 1024, 0, (SOCKADDR*)& addrServer, sizeof(SOCKADDR));



	short checksum = 0;
	short help = 0;
	char tocheck[1024];			//GBN算法确认窗口

	for (int j = 0; j < 10; j++) {			//若发生丢包，只重发10次
		help = 0;
		checksum = 0;
		bool complete = true;
		FILE* fd = fopen(wenjianming.c_str(), "rb");
		if (ACK == count - 1)													//   包结构：
			break;																//   |0........................1013|1014......1017|1018....1023|
		for (int k = 0; k < ACK; k++) {											//   |        实际文件内容	       | 文件内容大小 |   包编号   |	
			int wordcount = fread(sendBuffer, sizeof(char), 1014, fd);
		}
		for (int m = ACK + 1; m < count; m++) {
			int wordcount = fread(sendBuffer, sizeof(char), 1014, fd);			//读取文件内容
			string tempcount = to_string(wordcount);
			while (tempcount.size() != 4)
				tempcount = "0" + tempcount;
			for (int i = 1014; i < 1018; i++) {									//设置文件内容大小，即该包中实际内容大小
				sendBuffer[i] = tempcount[i - 1014];
			}
			string temp = to_string(m);
			while (temp.size() != 6)
				temp = "0" + temp;
			for (int j = 1018; j < 1024; j++) {									//设置包编号
				sendBuffer[j] = temp[j - 1018];
			}
			Sleep(5 * j);
			sendto(socketClient, sendBuffer, 1024, 0, (SOCKADDR*)& addrServer, sizeof(SOCKADDR));
			checksum = ~help;
			for (int i = 0; i < 16; i++) {
				if (checksum & 1) {
					tocheck[16 - 1 - i] = '1';
				}
				else {
					tocheck[16 - 1 - i] = '0';
				}
				checksum >>= 1;
			}
		}
		fclose(fd);
		while (1) {			//GBN算法，确认接收到第几个包
			int recvsize = recvfrom(socketClient, receiveBuffer, 100, 0, (SOCKADDR*)& addrServer, &length);
			if (recvsize < 0)
				break;
			else {
				int temp = 0;
				for (int i = 0; i < 6; i++) {
					temp *= 10;
					temp += (receiveBuffer[i] - '0');

				}
				if (temp > ACK)
					ACK = temp;
			}
		}
	}
	sendto(socketClient, tocheck, 2, 0, (SOCKADDR*)& addrServer, sizeof(SOCKADDR));
	sendto(socketClient, "End Connect", strlen("End Connect") + 1, 0, (SOCKADDR*)& addrServer, sizeof(SOCKADDR));
	cout << "Succeed! Press Enter to quit!" << endl;			
	getchar();
	closesocket(socketClient);			//断开连接，清理socket
	WSACleanup();
	return 0;
}
