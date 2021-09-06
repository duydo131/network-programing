#pragma once
#include "iostream"
#include "fstream"
#include "string"
#include "vector"
#include "time.h"
#include "ws2tcpip.h"
#include "winsock2.h"

using namespace std;

/**
* Receive message of SOCKET
* @param s: SOCKET receive message
* @param buff: [OUT] char pointer to contain message receive
* @param flags: flags for recv() function
* @param session: Session pointer, session of client connect to server
* @retruns bytes received of last message
*/
int Receive(SOCKET s, char *buff, int flags) {
	char recvBuff[BUFF_SIZE], tmpBuff[BUFF_SIZE] = "";
	char clientIP[INET_ADDRSTRLEN];
	int res, clientPort, i = 0, n = 0;

	Message message;

	// receive message
	while (true)
	{
		res = recv(s, recvBuff, BUFF_SIZE, flags);
		if (res <= 0) {
			printf("Error %d: Cannot receive data from server\n", WSAGetLastError());
			break;
		}
		else {
			recvBuff[res] = 0;
			//printf("Receive from server: %s\n", recvBuff);
			strcat_s(tmpBuff, recvBuff);

			// get message to get length of message
			if (i == 0) {
				message = decodeMessage(recvBuff);
				n += message.payload.length();
			}
			else {
				n += res;
			}

			// check bytes received
			if (n >= message.length) {
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

