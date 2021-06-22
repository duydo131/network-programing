#pragma once
#include "iostream"
#include "fstream"
#include "string"
#include "vector"
#include "ws2tcpip.h"
#include "winsock2.h"

using namespace std;

struct Question {
	string question;
	string options[4];
	string answer;
}; 

typedef struct Room {
	int number_of_question;
	int length_time;
	long long start_time;
} Room;

/*
bool decode_room(string in, Room &r) {
	int space = 0;
	int index[2], inx;
	for (int i = 0; i < in.length(); i++) {
		if (in[i] == ' ') {
			space++;
			if (space > 2) return false;
			index[inx++] = i;
		}
	}
	if (space != 2) return false;
	r.number_of_question = atoi(in.substr(0, index[0]).c_str());
	r.length_time = atoi(in.substr(index[0], index[1]).c_str());
	r.start_time = atoi(in.substr(index[1], in.length()).c_str());
	return true;
}
*/


// duy
/*
void generate_response(char* out, char opcode, int length, const char* payload) {
	*(out) = opcode;
	add_length(length, out);
	if (payload != NULL) memcpy(out + 3, payload, length);
}


change int to number base 256
@Param Input
length : number
buff[] : char array storage length
@Param Output : no return value
void add_length(int length, char* buff) {
	*(buff + 1) = length / 256;
	*(buff + 2) = length % 256;
}
*/

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
			printf("Receive from server: %s\n", recvBuff);
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

