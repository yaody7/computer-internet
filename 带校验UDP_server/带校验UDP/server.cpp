#include<WinSock2.h>
#include<WS2tcpip.h>
#include<stdio.h>
#include<time.h>
#include<string>
#include<fstream>
#include<Windows.h>
#include<iostream>
#include<cstring>
#pragma warning(disable : 4996)
#pragma comment(lib,"ws2_32.lib")
using namespace std;
long int ACK = -1;

bool ischeck(char tochar[]) {
	for (int i = 0; i < 16; i++)
		if (tochar[i] != '0' && tochar[i] != '1')
			return false;
	return true;
}

int main(void) {
	WORD wVersionRequested;		//Socket ׼������
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
	SOCKET sockServer = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockServer == INVALID_SOCKET) {
		cout << "Failed in socket() with error: " << WSAGetLastError() << endl;
		return -2;
	}

	SOCKADDR_IN addrServer;
	memset(&addrServer, 0, sizeof(addrServer));
	addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(3000);            //3000???

	err = bind(sockServer, (SOCKADDR*)& addrServer, sizeof(SOCKADDR));
	if (err != 0) {
		cout << "Fails in binding with error:" << WSAGetLastError();
		return -3;
	}

	SOCKADDR_IN addrClient;
	memset(&addrClient, 0, sizeof(addrClient));
	int length = sizeof(SOCKADDR);
	char receiveBuffer[1024];
	cout << "Wait for data!" << endl;

	short checksum = 0;
	short help = 0;

	recvfrom(sockServer, receiveBuffer, 100, 0, (SOCKADDR*)& addrClient, &length);		//�����ļ���
	cout << "�ļ����ƣ�"<<receiveBuffer+16 << endl;										//����ļ���
	char receiveBuffer1[1024];	
	recvfrom(sockServer, receiveBuffer1, 27, 0, (SOCKADDR*)& addrClient, &length);		//����ʱ��ʱ��
	cout << "�ļ�ʱ�䣺"<<receiveBuffer1;												//����ļ�ʱ��
	recvfrom(sockServer, receiveBuffer1, 1024, 0, (SOCKADDR*)& addrClient, &length);	//�����ļ���С
	int filesize = 0;
	for (int i = 0; i < strlen(receiveBuffer1); i++) {
		filesize *= 10;
		filesize += receiveBuffer1[i] - '0';
	}
	cout << "�ļ���С��"<<filesize/1024<<" KB" << endl;									//����ļ���С
	int packetcount = 0;
	for (int i = 0; i < 6; i++) {
		packetcount *= 10;
		packetcount += (receiveBuffer[i] - '0');
	}
	long int starttime = 0;								//���㴫�俪ʼʱ��
	for (int i = 0; i < 10; i++) {
		starttime *= 10;
		starttime += (receiveBuffer[i + 6] - '0');
	}
	string tempbuffer = receiveBuffer;														
	string wenjianming = tempbuffer.substr(16, tempbuffer.size() - 16);	//�洢�ļ�����
	wenjianming = wenjianming.substr(7, wenjianming.size() - 7);
	wenjianming = "./recv/" + wenjianming;
	FILE * fd = fopen(wenjianming.c_str(), "wb");			//�򿪽����ļ�
	while (1) {																					//���ṹ��
		recvfrom(sockServer, receiveBuffer, 1024, 0, (SOCKADDR*)& addrClient, &length);			//|0........................1013|1014......1017|1018....1023|
		if (strcmp(receiveBuffer, "End Connect") == 0) {				//У�鲿��				  |        ʵ���ļ�����			| �ļ����ݴ�С |   �����   |		
			if (!(checksum + help) == 0)
				printf("У��ɹ�!\n");
			else
				printf("У��ʧ��!\n");
			closesocket(sockServer);
			WSACleanup();
			break;
		}
		else if (ischeck(receiveBuffer)) {
			for (int i = 0; i < 16; i++) {
				checksum <<= 1;
				checksum |= (receiveBuffer[i] - '0');
			}
		}
		else {
			string back = "";					//�յ�ȷ����Ϣ
			int packetnumber = 0;
			for (int i = 0; i < 6; i++) {				
				packetnumber *= 10;
				packetnumber += (receiveBuffer[i + 1018] - '0');			
				back += receiveBuffer[i + 1018];
			}
			if (packetnumber == ACK + 1) {
				int number = 0;
				for (int j = 0; j < 4; j++) {
					number *= 10;
					number += receiveBuffer[j + 1014] - '0';
				}

				for (int i = 0; i < 1024; i += 2) {
					help += (short)(receiveBuffer[i]);
					help += ((short)(receiveBuffer[i + 1]) << 16);
				}

				fwrite(receiveBuffer, number, 1, fd);
				ACK = packetnumber;
				sendto(sockServer, back.c_str(), back.size() + 1, 0, (SOCKADDR*)& addrClient, sizeof(SOCKADDR));			//�����յ�ȷ����Ϣ
			}
		}
	}

	fclose(fd);			//�ر��ļ�
	long int endtime = time(NULL);		
	long long int size = packetcount;
	int shijiancha = endtime - starttime;
	cout << "����ʱ�䣺" << endtime - starttime << "s" << endl;
	if (shijiancha == 0)
		cout << "�����ٶȣ�NA(����ʱ�����)" << endl;
	else {
		//	cout << "�����ٶȣ�" << size / (endtime - starttime) << "KB /s" << endl;
		cout << "�����ٶȣ�" << size / (endtime - starttime) << "KB/s" << endl;
	}
	if (ACK == packetcount - 1)
		cout << "����ɹ���" << endl;
	else
		cout << "����ʧ�ܣ�" << endl;

	cout << "Client close its socket, press Enter to quit." << endl;

	getchar();
}

