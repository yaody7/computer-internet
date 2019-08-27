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
	WSADATA wsaData;			//Socket ׼���׶�
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
	Transfer = accept(Listening, (SOCKADDR*)& ClientAdd, &Clientlen);	//��������
	FILE* fd;
	int count = 0;
	char receiveBuffer[1024];
	recv(Transfer, receiveBuffer, 1024, 0);				//�����ļ�������Ҫ��Ϣ          �ṹ��
	cout << "�ļ����ƣ�" << receiveBuffer+16 << endl;	//����ļ���					|0........5|6...........15|16..............1023|
	char receiveBuffer1[1024];														//	|  ������  | ���俪ʼʱ�� |      �ļ�����      |
	recv(Transfer, receiveBuffer1, 27, 0);				//�����ļ�ʱ��
	cout << "�ļ�ʱ�䣺" << receiveBuffer1;				//����ļ�ʱ��
	recv(Transfer, receiveBuffer1, 1024, 0);			//�����ļ���С
	int filesize = 0;
	for (int i = 0; i < strlen(receiveBuffer1); i++) {
		filesize *= 10;
		filesize += receiveBuffer[i] - '0';
	}
	cout << "�ļ���С��" << filesize / 1024 << " KB" << endl;	//����ļ���С
	int packetcount = 0;
	for (int i = 0; i < 6; i++) {										//	�õ�packet����
		packetcount *= 10;											
		packetcount += (receiveBuffer[i] - '0');
	}
	long int starttime = 0;
	for (int i = 0; i < 10; i++) {										//  �õ���ʼʱ��
		starttime *= 10;
		starttime += (receiveBuffer[i + 6] - '0');
	}
	string tempbuffer = receiveBuffer;
	string wenjianming = tempbuffer.substr(16, tempbuffer.size() - 16);
	wenjianming = "./recv/" + wenjianming;
	fd = fopen(wenjianming.c_str(), "wb");				//���ļ�
	short help = 0;
	for (int i = 0; i < packetcount; i++) {
		recv(Transfer, receiveBuffer, 1024, 0);			//���հ�             ���ṹ��
		int hangnumber = 0;												//		|0...............1019|1020.......1023|
		for (int j = 0; j < 4; j++) {			//�õ����ݴ�С					|      �ļ�����		 |    ���ݴ�С   |
			hangnumber *= 10;
			hangnumber += receiveBuffer[j + 1020] - '0';
		}
		for (int i = 0; i < 1024; i += 2) {			
			help += (short)(receiveBuffer[i]);
			help += ((short)(receiveBuffer[i + 1]) << 16);
		}
		fwrite(receiveBuffer, hangnumber, 1, fd);		//д������ļ�
	}
	char tocheck[16];
	recv(Transfer, tocheck, 16, 0);

	short checksum = 0;					//���鲿��
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
		printf("У��ɹ���\n");
	else
		printf("У��ʧ��!\n");
	fclose(fd);
	long int endtime = time(NULL);
	cout << "����ʱ�䣺" << endtime - starttime << endl;
	if (endtime - starttime == 0)
		cout << "�����ٶȣ�NA(����ʱ���С)" << endl;
	else
		cout << "�����ٶȣ�" << packetcount  / (endtime - starttime) << "KB/s" << endl;
	closesocket(Transfer);			//����socket
	closesocket(Listening);
	WSACleanup();
	system("pause");
	return 0;
}
