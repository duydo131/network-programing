#pragma once

// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "server_utils.h"
#include "ws2tcpip.h"
#include "winsock2.h"

#define BUFF_SIZE 2048 // 2 MB

#pragma comment(lib, "Ws2_32.lib")
using namespace std;

struct ClientSession {
	SOCKET s;
	sockaddr_in addr;
	string username;
	bool login;
};

/**
* Receive message of SOCKET
* @param s: SOCKET receive message
* @param buff: [OUT] char pointer to contain message receive
* @param flags: flags for recv() function
* @param session: ClientSession pointer, session of client connect to server
* @retruns bytes received of last message
*/
int Receive(SOCKET s, char *buff, int flags, ClientSession *session) {
	char recvBuff[BUFF_SIZE], tmpBuff[BUFF_SIZE] = "";
	char clientIP[INET_ADDRSTRLEN];
	int res, clientPort, i = 0, n = 0;

	// get ip and port of client;
	inet_ntop(AF_INET, &session->addr.sin_addr, clientIP, sizeof(clientIP));
	clientPort = ntohs(session->addr.sin_port);

	Message message;

	// receive message
	while (true)
	{
		res = recv(s, recvBuff, BUFF_SIZE, flags);
		if (res == SOCKET_ERROR) {
			printf("Error %d: Cannot receive data from client[%s:%d]\n", WSAGetLastError(), clientIP, clientPort);
			break;
		}
		else if (res == 0) {
			printf("Client[%s:%d] disconnects\n", clientIP, clientPort);
			break;
		}
		else {
			recvBuff[res] = 0;
			printf("Receive from client[%s:%d]: %s\n", clientIP, clientPort, recvBuff);
			strcat_s(tmpBuff, recvBuff);
			n += res;
			// get message to get length of message
			if (i == 0) {
				message = decodeMessage(recvBuff);
			}

			// check bytes received
			if (n >= message.length + 3) {
				memcpy(buff, tmpBuff, BUFF_SIZE);
				break;
			}
		}
		i++;
	}

	return res;
}

/**
* Send message of SOCKET
* @param s: SOCKET send message
* @param buff: char pointer to contain message send
* @param flags: flags for send() function
* @retruns bytes sent of last message
*/
int Send(SOCKET s, char *buff, int flags) {
	int msglen, nLeft, idx;
	int res;

	msglen = strlen(buff);
	nLeft = msglen;
	idx = 0;

	while (nLeft > 0)
	{
		res = send(s, buff + idx, nLeft, 0);
		if (res == SOCKET_ERROR) {
			printf("Error %d: Cannot send data.", WSAGetLastError());
			res = false;
			break;
		}
		nLeft -= res;
		idx += res;
	}

	return res;
}
