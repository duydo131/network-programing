// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "string"
#include "server_module.h"
#include "ws2tcpip.h"
#include "winsock2.h"

#define SERVER_PORT 5000
#define SERVER_ADDR "127.0.0.1"

#pragma comment(lib, "Ws2_32.lib")
using namespace std;

int main(int argc, char *argv[])
{
	// Inittiate WinSock
	WSADATA wsaDATA;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaDATA)) {
		printf("Winsock 2.2 is not supported\n");
		return 0;
	}

	// Construct socket
	SOCKET listenSocket;
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket.", WSAGetLastError());
		return 0;
	}

	// Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	if (bind(listenSocket, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error %d: Cannot bind this address.", WSAGetLastError());
		return 0;
	}

	if (listen(listenSocket, 10)) {
		printf("Error %d: Cannot place server socket in state LISTEN.", WSAGetLastError());
		return 0;
	}

	printf("Server started!\n");

	// Load database
	SOCKET client[FD_SETSIZE], connSock;
	fd_set readfds, initfds; //use initfds to initiate readfds at the begining of every loop step
	sockaddr_in clientAddr;
	Session sessions[FD_SETSIZE];
	
	int nEvents, clientAddrLen = sizeof(clientAddr), clientPort, ret;
	char buff[BUFF_SIZE], clientIP[INET_ADDRSTRLEN];

	for (int i = 0; i < FD_SETSIZE; i++)
		client[i] = 0;	// 0 indicates available entry

	FD_ZERO(&initfds);
	FD_SET(listenSocket, &initfds);

	Message msgRecv, msgRes;

	// Communicate with clients
	while (1) {
		readfds = initfds;
		nEvents = select(0, &readfds, 0, 0, 0);
		if (nEvents < 0) {
			printf("Error! Cannot poll sockets: %d\n", WSAGetLastError());
			break;
		}

		//new client connection
		if (FD_ISSET(listenSocket, &readfds)) {
			if ((connSock = accept(listenSocket, (sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
				printf("Error! Cannot accept new connection: %d\n", WSAGetLastError());
				break;
			}
			else {
				inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
				clientPort = ntohs(clientAddr.sin_port);
				printf("New connection from client %s:%d\n", clientIP, clientPort);

				int i;
				for (i = 0; i < FD_SETSIZE; i++)
					if (client[i] == 0) {
						client[i] = connSock;
						FD_SET(client[i], &initfds);
						Session session = { connSock, clientAddr, "", false };
						sessions[i] = session;
						break;
					}

				if (i == FD_SETSIZE) {
					printf("\nToo many clients.");
					closesocket(connSock);
				}

				if (--nEvents == 0)
					continue; //no more event
			}
		}

		//receive data from clients
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (client[i] == 0)
				continue;

			if (FD_ISSET(client[i], &readfds)) {
				ret = Receive(client[i], buff, 0, &sessions[i]);
				if (ret <= 0 ) {
					FD_CLR(client[i], &initfds);
					closesocket(client[i]);
					client[i] = 0;
					sessions[i] = {};
				}
				else {
					msgRecv = decodeMessage(buff);
					msgRes = handleMessage(msgRecv, &sessions[i]);
					encodeMessage(msgRes, buff);
					Send(client[i], buff, 0);
				}
			}

			if (--nEvents <= 0)
				continue; //no more event
		}

	}

	// Close socket
	closesocket(listenSocket);

	// Terminate Winsock
	WSACleanup();


	return 0;
}

