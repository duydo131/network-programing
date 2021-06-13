// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>

#define BUFF_SIZE 2048
#pragma comment(lib, "Ws2_32.lib")

void process_signin();
void process_choose_mode();
void process_get_question();
void process_get_result();
void process_setup_room();
void process_get_info_room();
void process_access_room();
void process_submit();

int main(int argc, char* argv[]) {
	if (argc != 3)
	{
		printf("Bad input data in argv\n");
		return 0;
	}
	char* SERVER_ADDR = argv[1];
	u_short SERVER_PORT = atoi(argv[2]);

	// init WinSock 2.2
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not support");
		return 0;
	}

	// Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	// Construct Socket
	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		printf("Server %d : cannot create client socket.", WSAGetLastError());
		return 0;
	}

	// request connect
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error %d Cannot connect server.\n", WSAGetLastError());
		return 0;
	}

	char buff[BUFF_SIZE];
	int ret, messageLen;

	// comunicate with server
	while (1) {
		// Send message
		printf("Send to server : ");
		gets_s(buff, BUFF_SIZE);
		messageLen = strlen(buff);
		if (messageLen == 0) {
			send(client, buff, 0, 0);
			break;
		}

		// Send message to server
		ret = send(client, buff, messageLen, 0);
		if (ret == SOCKET_ERROR) {
			printf("Error %d :Cannot send data.\n", WSAGetLastError());
		}

		// receive message from server
		ret = recv(client, buff, BUFF_SIZE, 0);


		if (ret == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				printf("Time out!");
			}
			else {
				printf("Error %d : Cannot receive data.\n", WSAGetLastError());
			}
		}
		else if (strlen(buff) > 0) {
			// Echo result
			buff[ret] = 0;
			printf("%s\n", buff);
		}
	}


	// close socket
	closesocket(client);
	// terminate WinSock
	WSACleanup();
	return 0;
}