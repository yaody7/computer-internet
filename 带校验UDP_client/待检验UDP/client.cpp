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
long int ACK = -1;		//ʹ��GBDȷ�Ϸ�������ʼ��ACKΪ-1
int main(void) {
	//��ȡ���ݰ�����
	int count = 0;						//��¼���������
	string wenjianming = "test.txt";	//�����ļ���
	char sendBuffer[1024];				//���仺��
	FILE* fd = fopen(wenjianming.c_str(), "rb");		//���ļ�
	while (!feof(fd)) {									//����ļ���С
		int wordcount = fread(sendBuffer, sizeof(char), 1014, fd);
		count++;
	}
	fclose(fd);											//�ر��ļ�



	WORD wVersionRequested;								//Socket ׼������
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
	ioctlsocket(socketClient, FIONBIO, (unsigned long*)& ul);		//���óɷ�����ģʽ�� 
	SOCKADDR_IN addrServer;
	memset(&addrServer, 0, sizeof(addrServer));
	addrServer.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");		//����IP��ַ�����������
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(3000);								
	cout << "Begin to send data, press Enter to continue." << endl;
	getchar();
	char receiveBuffer[100];			//���ܻ���
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
	char tocheck[1024];			//GBN�㷨ȷ�ϴ���

	for (int j = 0; j < 10; j++) {			//������������ֻ�ط�10��
		help = 0;
		checksum = 0;
		bool complete = true;
		FILE* fd = fopen(wenjianming.c_str(), "rb");
		if (ACK == count - 1)													//   ���ṹ��
			break;																//   |0........................1013|1014......1017|1018....1023|
		for (int k = 0; k < ACK; k++) {											//   |        ʵ���ļ�����	       | �ļ����ݴ�С |   �����   |	
			int wordcount = fread(sendBuffer, sizeof(char), 1014, fd);
		}
		for (int m = ACK + 1; m < count; m++) {
			int wordcount = fread(sendBuffer, sizeof(char), 1014, fd);			//��ȡ�ļ�����
			string tempcount = to_string(wordcount);
			while (tempcount.size() != 4)
				tempcount = "0" + tempcount;
			for (int i = 1014; i < 1018; i++) {									//�����ļ����ݴ�С�����ð���ʵ�����ݴ�С
				sendBuffer[i] = tempcount[i - 1014];
			}
			string temp = to_string(m);
			while (temp.size() != 6)
				temp = "0" + temp;
			for (int j = 1018; j < 1024; j++) {									//���ð����
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
		while (1) {			//GBN�㷨��ȷ�Ͻ��յ��ڼ�����
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
	closesocket(socketClient);			//�Ͽ����ӣ�����socket
	WSACleanup();
	return 0;
}
