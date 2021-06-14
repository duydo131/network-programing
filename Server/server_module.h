#pragma once

#include "stdafx.h"
#include "stdio.h"
#include "server_utils.h"
#include "ws2tcpip.h"
#include "winsock2.h"
#define BUFF_SIZE 2048 // 2 MB
using namespace std;

string ACCOUNTS_PATH = "accounts.txt";
string QUESTIONS_PATH = "questions.txt";

vector<Account> accounts = getAllAccounts(ACCOUNTS_PATH);
vector<Question> questions = getAllQuestions(QUESTIONS_PATH);

enum Mode {
	PRACTICE = 1,
	TEST = 2
};

enum ResponseCode {
	// Login
	SUCCESS = 0,
	ACCOUNT_EXISTED = 1,
	ACCOUNT_LOGGED = 101,
	INCORRECT_ACCOUNT = 102,
	ACCOUNT_LOCKED = 103,
	HAVE_ACCOUNT = 104,
	NO_LOGIN = 201,
	BAD_REQUEST = 301,
	// Command Error
	COMMAND_ERROR = 404,
};


struct Room {
	int number_of_question;
	vector<Question> questions;
	int length_time;
	long long start_time;
};

struct Session {
	SOCKET s;
	sockaddr_in addr;
	string username;
	bool login;
};

bool checkAccountExist(string username) {
	int number_accounts = accounts.size();
	for (int i = 0; i < number_accounts; i++) {
		if (accounts[i].username == username && accounts[i].status == 1) {
			return true;
		}
	}
	return false;
}

/**
* Function for client registry account
* @param message: message to handle
* @retruns response message for client
*/
Message registry(Message message) {
	Message response;
	response.opcode = message.opcode;

	Account account;
	vector<string> data = split(message.payload, SPACE_DELIMITER);
	account.username = data[0];
	account.password = data[1];
	account.status = 1;

	ResponseCode resCode;
	if (!checkAccountExist(account.username)) {
		saveAccount(account, ACCOUNTS_PATH);
		accounts.push_back(account);
		resCode = SUCCESS;
	}
	else {
		resCode = ACCOUNT_EXISTED;
	}

	response.payload = to_string(resCode);
	response.length = response.payload.length();

	return response;
}

/**
* Function for client login
* @param message: message to handle
* @param session: [IN/OUT] pointer session of client
* @retruns response message for client
*/
Message login(Message message, Session *session) {
	Message response;
	response.opcode = message.opcode;

	Account account;
	vector<string> data = split(message.payload, SPACE_DELIMITER);
	account.username = data[0];
	account.password = data[1];

	ResponseCode resCode = INCORRECT_ACCOUNT;
	
	int number_accounts = accounts.size();
	for (int i = 0; i < number_accounts; i++) {
		if (accounts[i].username == account.username && accounts[i].password == account.password)
		{
			if (accounts[i].status == 1) {
				if (accounts[i].login) {
					resCode = ACCOUNT_LOGGED;
				}
				else {
					accounts[i].login = true;
					session->login = true;
					session->username = account.username;
					resCode = SUCCESS;
				}
			}
			else {
				resCode = ACCOUNT_LOCKED;
			}
			break;
		}
	}

	response.payload = to_string(resCode);
	response.length = response.payload.length();

	return response;
}

/**
* Function for client logout
* @param message: message to handle
* @param session: [IN/OUT] pointer session of client
* @retruns response message for client
*/
Message logout(Message message, Session *session) {
	Message response;
	response.opcode = message.opcode;

	ResponseCode resCode = BAD_REQUEST;

	if (session->login)
	{	
		int number_accounts = accounts.size();
		for (int i = 0; i < number_accounts; i++) {
			if (accounts[i].username == session->username && accounts[i].login == true && accounts[i].status == 1) {
				accounts[i].login = false;
				session->username = "";
				resCode = SUCCESS;
				break;
			}
		}
	}
	else
	{
		resCode = NO_LOGIN;
	}

	response.payload = to_string(resCode);
	response.length = response.payload.length();

	return response;
}

/**
* Function to handle message
* @param message: message to handle
* @param session: [IN/OUT] pointer session of client to handle
* @retruns response message for client
*/
Message handleMessage(Message message, Session *session) {
	Message response;

	switch (message.opcode)
	{
		case 1: {
			// registry account process 
			response = registry(message);
			break;
		}
		case 2: {
			// login process
			response = login(message, session);
			break;
		}
		case 3: {
			// logout process
			response = logout(message, session);
			break;
		}
		case 4: {
			break;
		}
		case 5: {
			break;
		}
		case 6: {
			break;
		}
		case 7: {
			break;
		}
		case 8: {
			break;
		}
		case 9: {
			break;
		}
		case 10: {
			break;
		}
		default:
			break;
	}
	return response;
}

/**
* Receive message of SOCKET
* @param s: SOCKET receive message
* @param buff: [OUT] char pointer to contain message receive
* @param flags: flags for recv() function
* @param session: Session pointer, session of client connect to server
* @retruns bytes received of last message
*/
int Receive(SOCKET s, char *buff, int flags, Session *session) {
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
