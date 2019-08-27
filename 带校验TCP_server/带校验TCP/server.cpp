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

int main() {
	cout << "Server is standby, press Enter to continue!" << endl;
	getchar();
	WSADATA wsaData;			//Socket 准备阶段
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
	Transfer = accept(Listening, (SOCKADDR*)& ClientAdd, &Clientlen);	//握手连接
	FILE* fd;
	int count = 0;
	char receiveBuffer[1024];
	recv(Transfer, receiveBuffer, 1024, 0);				//接收文件名及必要信息          结构：
	cout << "文件名称：" << receiveBuffer+16 << endl;	//输出文件名					|0........5|6...........15|16..............1023|
	char receiveBuffer1[1024];														//	|  包数量  | 传输开始时间 |      文件名称      |
	recv(Transfer, receiveBuffer1, 27, 0);				//接收文件时间
	cout << "文件时间：" << receiveBuffer1;				//输出文件时间
	recv(Transfer, receiveBuffer1, 1024, 0);			//接收文件大小
	int filesize = 0;
	for (int i = 0; i < strlen(receiveBuffer1); i++) {
		filesize *= 10;
		filesize += receiveBuffer[i] - '0';
	}
	cout << "文件大小：" << filesize / 1024 << " KB" << endl;	//输出文件大小
	int packetcount = 0;
	for (int i = 0; i < 6; i++) {										//	得到packet数量
		packetcount *= 10;											
		packetcount += (receiveBuffer[i] - '0');
	}
	long int starttime = 0;
	for (int i = 0; i < 10; i++) {										//  得到开始时间
		starttime *= 10;
		starttime += (receiveBuffer[i + 6] - '0');
	}
	string tempbuffer = receiveBuffer;
	string wenjianming = tempbuffer.substr(16, tempbuffer.size() - 16);
	wenjianming = "./recv/" + wenjianming;
	fd = fopen(wenjianming.c_str(), "wb");				//打开文件
	short help = 0;
	for (int i = 0; i < packetcount; i++) {
		recv(Transfer, receiveBuffer, 1024, 0);			//接收包             包结构：
		int hangnumber = 0;												//		|0...............1019|1020.......1023|
		for (int j = 0; j < 4; j++) {			//得到内容大小					|      文件内容		 |    内容大小   |
			hangnumber *= 10;
			hangnumber += receiveBuffer[j + 1020] - '0';
		}
		for (int i = 0; i < 1024; i += 2) {			
			help += (short)(receiveBuffer[i]);
			help += ((short)(receiveBuffer[i + 1]) << 16);
		}
		fwrite(receiveBuffer, hangnumber, 1, fd);		//写入接收文件
	}
	char tocheck[16];
	recv(Transfer, tocheck, 16, 0);

	short checksum = 0;					//检验部分
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
	if (~(checksum + help) == 0)
		printf("校验成功！\n");
	else
		printf("校验失败!\n");
	fclose(fd);
	long int endtime = time(NULL);
	cout << "传输时间：" << endtime - starttime << endl;
	if (endtime - starttime == 0)
		cout << "传输速度：NA(传输时间过小)" << endl;
	else
		cout << "传输速度：" << packetcount  / (endtime - starttime) << "KB/s" << endl;
	closesocket(Transfer);			//清理socket
	closesocket(Listening);
	WSACleanup();
	system("pause");
	return 0;
}
