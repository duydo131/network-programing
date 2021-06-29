// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include "Client_utils.h"
#include "Client_module.h"
 
#define BUFF_SIZE 2048
#pragma comment(lib, "Ws2_32.lib") 

Message process_signup();
Message process_signin();
Message process_signout();
Message process_practice();
Message process_get_question();
Message process_get_info_room();
Message process_access_room();
void process_get_result();
void process_setup_room();
void process_submit();

int menu() {
	char choice[100];
	int c;
	while (true)
	{
		printf("------------------------- MENU -------------------------\n");
		printf("1. Sign Up\n");
		printf("2. Sign In\n");
		printf("3. Sign Out\n");
		printf("4. Practice\n");
		printf("5. Get Info Room List\n");
		printf("6. Access Room\n");
		printf("Enter your choice: ");
		gets_s(choice, 100);

		c = (int)(choice[0] - 48);

		if (strlen(choice) > 1 || c < 1 || c > 6)
		{
			printf("Your choice is incorrect. Please, try again!\n");
		}
		else break;
	}

	return (int)(choice[0] - 48);
}

void show_info_room(string info) {
	cout <<"info" <<info;
	vector<string> payloads = split(info, SPACE_DELIMITER);
	vector<string> rooms_info = split(payloads[1], Q_DELIMITER);
	for (int i = 0; i < rooms_info.size(); i++) {
		vector<string> room_info = split(rooms_info[i], A_DELIMITER);
		cout << "Room " << i << endl;
		cout << "\tNumber of question : " << room_info[0] << endl;
		cout << "\tLengh time : " << room_info[1] << endl;
		cout << "\tStart time : " << room_info[2] << endl;
	}
}

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
	int ret;
	Message message;

	// comunicate with server
	while (1) {

		switch (menu())
		{
		case 1: {
			// Sign Up Process
			message = process_signup();
			break;
		}
		case 2: {
			// Sign In Process
			message = process_signin();
			break;
		}
		case 3: {
			// Sign Out Process
			message = process_signout();
			break;
		}
		case 4: {
			// Practice Process
			message = process_practice();
			break;
		}
		case 5: {
			// Get Info Room List Process
			message = process_get_info_room();
			break;
		}
		case 6: {
			// Access Room Process
			message = process_access_room();
			break;
		}
		default:
			break;
		}

		encodeMessage(message, buff);

		// send message to server
		ret = Send(client, buff, 0);

		// receive message from server
		ret = Receive(client, buff, 0);
		
		message = decodeMessage(buff);
		cout << message.opcode;
		string resCode = split(message.payload, SPACE_DELIMITER)[0];
		switch (stoi(resCode))
		{
		case 0: {
			cout << ">> Success" << endl;
			if (message.opcode == 9) {
				show_info_room(message.payload);
			}
			break;
		}
		case 1: {
			cout << ">> Account exist!!!" << endl;
			break;
		}
		case 101: {
			cout << ">> Account is logined!!!" << endl;
			break;
		}
		case 102: {
			cout << ">> Username or Password is incorrect!!!" << endl;
			break;
		}
		case 103: {
			cout << ">> Account is locked!!!" << endl;
			break;
		}
		case 104: {
			cout << ">> You are logined!!!" << endl;
			break;
		}
		case 201: {
			cout << ">> You are not login!!!" << endl;
			break;
		}
		case 404: {
			cout << ">> Error UNKNOW!!!" << endl;
			break;
		}
		default:
			break;
		}
	}

	// close socket
	closesocket(client);
	// terminate WinSock
	WSACleanup();
	return 0;
}

Message process_signup() {
	Message message;
	message.opcode = 2;
	string username, password, passwordConfirm, payload;

	cout << "Sign Up\n";
	cout << "Enter username: ";
	getline(cin, username);

	while (true)
	{
		cout << "Enter password: ";
		getline(cin, password);
		cout << "Enter password again: ";
		getline(cin, passwordConfirm);

		if (password == passwordConfirm) {
			break;
		}
		else {
			cout << "Two password not match!!! Enter your password again!!!\n";
		}
	}

	payload = username;
	payload.append(Q_DELIMITER);
	payload.append(password);

	message.length = payload.size();
	message.payload = payload;

	return message;
};

Message process_signin() {
	Message message;
	message.opcode = 3;
	string username, password, payload;

	cout << "Sign In\n";
	cout << "Enter username: ";
	getline(cin, username);
	cout << "Enter password: ";
	getline(cin, password);

	payload = username;
	payload.append(Q_DELIMITER);
	payload.append(password);

	message.length = payload.size();
	message.payload = payload;

	return message;
};

Message process_signout() {
	Message message;
	message.opcode = 4;
	message.length = 0;
	message.payload = "";

	return message;
};

Message process_practice() {
	Message message;
	message.opcode = 5;
	message.length = 0;
	message.payload = "";

	return message;
};

Message process_get_question() {
	Message message;

	return message;
};

Message process_get_info_room() {
	Message message;
	message.opcode = 9;
	message.length = 0;
	message.payload = "";

	return message;
};

Message process_access_room() {
	Message message;
	message.opcode = 10;
	string room, payload;

	cout << "Access room\n";
	cout << "Enter room: ";
	getline(cin, room);
	payload = room;

	message.length = payload.size();
	message.payload = payload;

	return message;
};

