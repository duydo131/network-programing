// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include "Client_utils.h"
#include "Client_module.h"
#include "process.h"
#include <future>
#include <thread>

using namespace std;

#define BUFF_SIZE 2048
#pragma comment(lib, "Ws2_32.lib")

string result = "";

SOCKET client;
char buff[BUFF_SIZE];
int ret, seek = 0, length_time_practice = 0, question_size_practice = 0;
Room room;
string user = "", time_exam = "";
boolean flag = false, done = false, show_question = true, fetch_question = true;
vector<Room> info_room;
future<void> compute_thread;

void process_signup();
void process_signin();
void process_signout();
void process_practice();
void process_get_question();
void process_get_info_room();
void process_access_room();
void process_get_result();
void process_setup_room();
void process_get_result_of_room();
int get_question(int);

int menu() {
	char choice[100];
	int c;
	while (true)
	{
		printf("------------------------- MENU -------------------------\n");
		if (flag) cout << "             Welcome " << user << endl;
		printf("1. Sign Up\n");
		printf("2. Sign In\n");
		printf("3. Sign Out\n");
		printf("4. Practice\n");
		printf("5. Get Info Room List\n");
		printf("6. Access Room\n");
		printf("7. Setup Room\n");
		printf("8. Get Result Room\n");
		printf("Enter your choice: ");
		gets_s(choice, 100);

		c = (int)(choice[0] - 48);

		if (strlen(choice) > 1 || c < 1 || c > 8)
		{
			printf("Your choice is incorrect. Please, try again!\n");
		}
		else break;
	}

	return (int)(choice[0] - 48);
}

void fetch(int number_of_question) {
	while (true && fetch_question) {
		int ret = get_question(seek);
		if (ret < 0) {
			cout << "Error Server!! Try again!!\n";
			return;
		}
		if (seek >= number_of_question) break;
	}
	return;
}

void show_info_room(string info) {
	system("CLS");
	if (info == "") {
		cout << "No Rooms Available!!!!!\n";
		return;
	}
	vector<string> payloads = split(info, Q_DELIMITER);
	for (int j = 0; j < payloads.size() - 1; j++) {
		vector<string> room_info = split(payloads[j], A_DELIMITER);
		cout << (j + 1) << ". Room ID: " << room_info[0] << endl;
		cout << "\tNumber of question : " << room_info[1] << endl;
		cout << "\tLengh time : " << room_info[2] << endl;
		cout << "\tStart time : " << formatTime(room_info[3]) << endl << endl;
	}
}

void show_questions(string payload) {
	system("CLS");
	vector<string> payloads = split(payload, A_DELIMITER);
	for (int j = 0; j < payloads.size() - 1; j++) {
		vector<string> question = split(payloads[j], Q_DELIMITER);
		cout << "ID " << question[0] << ":" <<  question[1] << endl;
		cout << question[2] << endl;// A
		cout << question[3] << endl;// B
		cout << question[4] << endl;// C
		cout << question[5] << endl;// D
		seek++;
		string in;
		while (show_question) {
			cout << "\nChoose 1 answer : ";
			getline(cin, in);
			if (validate_result(in)) break;
		}
		result += in + A_DELIMITER;
		system("CLS");
	}
}

void show_result(string payload) {
	vector<string> rs = split(payload, SPACE_DELIMITER);
	cout << "Result: " << endl;
	cout << "Correct: " << rs[0] << endl;
	cout << "Wrong: " << rs[1] << endl;
}

void show_info_result(string room_id, string data) {
	cout << endl << "Room ID: " << room_id << endl;
	string tab = "    ";
	vector<string> results = split(data, A_DELIMITER);
	if (results.size() <= 1) {
		cout << "Room have not result" << endl;
		return;
	}
	for (int j = 0; j < results.size() - 1; j++) {
		vector<string> info = split(results[j], Q_DELIMITER);
		cout << tab << "User: " << info[0] << endl;
		cout << tab << tab << "Correct: " << info[1] << endl;
		cout << tab << tab << "Wrong: " << info[2] << endl;
		cout << tab << tab << "User: " << formatTime(info[3]) << endl << endl;
	}
}

void save_info_room(string payload) {
	info_room.clear();
	vector<string> payloads = split(payload, Q_DELIMITER);
	for (int j = 0; j < payloads.size() - 1; j++) {
		vector<string> info = split(payloads[j], A_DELIMITER);
		Room r;
		r.id = info[0];
		r.number_of_question = stoi(info[1]);
		r.length_time = stoi(info[2]);
		r.start_time = info[3];
		info_room.push_back(r);
	}
}

void get_info_room(bool f) {
	Message message;
	message.opcode = 9;
	message.length = 0;
	message.payload = "";

	encodeMessage(message, buff);
	// send message to server
	ret = Send(client, buff, 0);

	// receive message from server
	ret = Receive(client, buff, 0);
	message = decodeMessage(buff);
	if (message.opcode == SUCCESS) {
		if (f) show_info_room(message.payload);
		save_info_room(message.payload);
	}
	else {
		switch (stoi(message.payload))
		{
		case NO_LOGIN:
			cout << "You are not logged in!!";
			break;
		default:
			cout << "Error Server";
		}
	}
}

void save_room(string payload) {
	vector<string> info = split(payload, A_DELIMITER);
	room.id = info[0];
	room.number_of_question = stoi(info[1]);
	room.length_time = stoi(info[2]);
	room.start_time = info[3];
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
	system("CLS");
	// comunicate with server
	while (1) {

		switch (menu())
		{
		case 1: {
			// Sign Up Process
			process_signup();
			break;
		}
		case 2: {
			// Sign In Process
			process_signin();
			break;
		}
		case 3: {
			// Sign Out Process
			process_signout();
			break;
		}
		case 4: {
			// Practice Process
			process_practice();
			break;
		}
		case 5: {
			// Get Info Room List Process
			process_get_info_room();
			break;
		}
		case 6: {
			// access room process
			process_access_room();
			break;
		}
		case 7: {
			process_setup_room();
			break; 
		}
		case 8: {
			process_get_result_of_room();
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

void process_signup() {
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

	encodeMessage(message, buff);
	// send message to server
	ret = Send(client, buff, 0);

	// receive message from server
	ret = Receive(client, buff, 0);
	message = decodeMessage(buff);

	if (message.opcode == SUCCESS) cout << "Sigup Success!!";
	else {
		switch (stoi(message.payload))
		{
		case ACCOUNT_EXISTED:
			cout << "Account existed";
			break;
		default:
			cout << "Error Server";
		}
	}
	cout << "\nPress key to continue!!\n";
	_getch();
	system("CLS");
};

void process_signin() {
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

	encodeMessage(message, buff);
	// send message to server
	ret = Send(client, buff, 0);

	// receive message from server
	ret = Receive(client, buff, 0);
	message = decodeMessage(buff);
	if (message.opcode == SUCCESS) {
		cout << "Login Success!!";
		user = username;
		flag = true;
	}
	else {
		switch (stoi(message.payload))
		{
		case LOGGED:
			cout << "Account logged";
			break;
		case INCORRECT_ACCOUNT:
			cout << "Account incorrect";
			break;
		case ACCOUNT_LOCKED:
			cout << "Account locked";
			break;
		case ACCOUNT_LOGGED:
			cout << "Account login elsewhere";
			break;
		default:
			cout << "Error Server";
		}
	}
	cout << "\nPress key to continue!!\n";
	_getch();
	system("CLS");
};

void process_signout() {
	Message message;
	message.opcode = 4;
	message.length = 0;
	message.payload = "";

	encodeMessage(message, buff);
	// send message to server
	ret = Send(client, buff, 0);

	// receive message from server
	ret = Receive(client, buff, 0);
	message = decodeMessage(buff);
	if (message.opcode == SUCCESS) {
		cout << "Logout Success!!";
		flag = false;
	}
	else {
		switch (stoi(message.payload))
		{
		case NO_LOGIN:
			cout << "You are not logged in!!";
			break;
		default:
			cout << "Error Server";
		}
	}
	cout << "\nPress key to continue!!\n";
	_getch();
	system("CLS");
};

void process_practice() {
	Message message;
	message.opcode = 5;
	message.length = 0;
	message.payload = "";

	encodeMessage(message, buff);
	// send message to server
	ret = Send(client, buff, 0);

	// receive message from server
	ret = Receive(client, buff, 0);
	message = decodeMessage(buff);
	if (message.opcode == SUCCESS) {
		room.id = PRACTICE;
		vector<string> info = split(message.payload, A_DELIMITER);
		length_time_practice = stoi(info[0]);
		question_size_practice = stoi(info[1]);
		process_get_question();
	}
	else {
		switch (stoi(message.payload))
		{
		case NO_LOGIN:
			cout << "You are not logged in!!";
			break;
		default:
			cout << "Error Server";
		}
	}
	cout << "\nPress key to continue!!\n";
	_getch();
	system("CLS");
};

int get_question(int seek) {
	Message message;
	message.opcode = 6;
	
	string payload = to_string(seek) + Q_DELIMITER + room.id;
	message.payload = payload;
	message.length = payload.length();

	encodeMessage(message, buff);
	// send message to server
	ret = Send(client, buff, 0);

	// receive message from server
	ret = Receive(client, buff, 0);
	message = decodeMessage(buff);
	if (message.opcode == SUCCESS) {
		show_questions(message.payload);
	}
	else {
		switch (stoi(message.payload))
		{
		case NO_LOGIN:
			cout << "You are not logged in!!";
			break;
		case ROOM_NO_EXIST:
			cout << "Room not exist!!\n";
			break;
		default:
			cout << "Error Server";
		}
		return -1;
	}
	return 1;
}

void process_get_question() {
	result = "";
	seek = 0;
	time_exam = get_time(time(0));
	show_question = true;
	fetch_question = true;
	int ret;
	if(room.id == NOT_ACCESS_ROOM) cout << "NOT ACCESS ROOM!!!!!!\n";
	else{
		if (room.id == PRACTICE) {
			auto timeToWait = chrono::system_clock::now() + chrono::milliseconds(length_time_practice * 1000);
			compute_thread = async(fetch, question_size_practice);
			future_status status = compute_thread.wait_until(timeToWait);
			if (status != future_status::ready) {
				cout << "\nOver!!!!\n";
				show_question = false;
				fetch_question = false;
			}
		}
		else {
			auto timeToWait = chrono::system_clock::now() + chrono::milliseconds(room.length_time * 1000);
			compute_thread = async(fetch, room.number_of_question);
			future_status status = compute_thread.wait_until(timeToWait);
			if (status != future_status::ready) {
				cout << "\nOver!!!!\n";
				show_question = false;
				fetch_question = false;
			}
		}
		result = result.substr(0, result.length() - A_DELIMITER.length());
		process_get_result();
		room.id = PRACTICE;
	}	
};

void process_get_info_room() {
	get_info_room(true);
	cout << "\nPress key to continue!!\n";
	_getch();
	system("CLS");
};

void process_access_room() {
	get_info_room(false);
	Message message;
	message.opcode = 10;
	string payload;
	int index_room;

	cout << "Access room\n";
	if (info_room.size() <= 0) {
		cout << "No Rooms Available!!!!!\n";
		cout << "\nPress key to continue!!\n";
		_getch();
		system("CLS");
		return;
	}
	index_room = get_line("Enter room", 1, info_room.size());
	payload = info_room[index_room - 1].id;

	message.length = payload.size();
	message.payload = payload;

	encodeMessage(message, buff);
	// send message to server
	ret = Send(client, buff, 0);

	// receive message from server
	ret = Receive(client, buff, 0);
	message = decodeMessage(buff);
	if (message.opcode == SUCCESS) {
		save_room(message.payload);
		process_get_question();
	}
	else {
		switch (stoi(message.payload))
		{
		case NO_LOGIN:
			cout << "You are not logged in!!";
			break;
		case ROOM_STARTED:
			cout << "Room started!!";
			break;
		case ROOM_NO_EXIST:
			cout << "Room not exist!!";
			break;
		default:
			cout << "Error Server";
		}
	}
	cout << "\nPress key to continue!!\n";
	_getch();
	system("CLS");
};


void process_get_result() {
	Message message;
	message.opcode = 7;
	message.payload = room.id + M_DELIMITER + result + M_DELIMITER + time_exam;
	message.length = message.payload.length();

	encodeMessage(message, buff);
	// send message to server
	ret = Send(client, buff, 0);

	// receive message from server
	ret = Receive(client, buff, 0);
	message = decodeMessage(buff);
	if (message.opcode == SUCCESS) {
		show_result(message.payload);
	}
	else {
		switch (stoi(message.payload))
		{
		case ERROR_RESULT:
			cout << "Bad request";
			break;
		default:
			cout << "Error Server";
		}
	}
}

void process_setup_room() {
	Message message;
	message.opcode = 8;
	int number_of_question, length_time;
	string start_time;

	cout << "Setup Room\n";
	number_of_question = get_line("Number of question");
	length_time = get_line("Length time");
	start_time = get_start_time();
	string payload = to_string(number_of_question) + R_DELIMITER + to_string(length_time) + R_DELIMITER + start_time;
	message.payload = payload;
	message.length = message.payload.length();

	encodeMessage(message, buff);
	// send message to server
	ret = Send(client, buff, 0);

	// receive message from server
	ret = Receive(client, buff, 0);
	message = decodeMessage(buff);
	if (message.opcode == SUCCESS) {
		cout << "Setup Room Success!!" << endl;
		//cout << "Room " << message.payload << endl;
		cout << "\tNumber of question : " << number_of_question << endl;
		cout << "\tLengh time : " << length_time << endl;
		cout << "\tStart time : " << formatTime(start_time) << endl << endl;
	}
	else {
		switch (stoi(message.payload))
		{
		case NO_LOGIN:
			cout << "You are not logged in!!";
			break;
		case ERROR_SETUP_ROOM:
			cout << "Setup room error";
			break;
		default:
			cout << "Error Server";
		}
	}
	cout << "\nPress key to continue!!\n";
	_getch();
	system("CLS");
}

void process_get_result_of_room() {
	get_info_room(false);
	Message message;
	message.opcode = 11;
	string payload;
	int index_room;

	cout << "Position room\n";
	if (info_room.size() <= 0) {
		cout << "No Rooms Available!!!!!\n";
		cout << "\nPress key to continue!!\n";
		_getch();
		system("CLS");
		return;
	}
	index_room = get_line("Enter room", 1, info_room.size());
	payload = info_room[index_room - 1].id;

	message.length = payload.size();
	message.payload = payload;

	encodeMessage(message, buff);
	// send message to server
	ret = Send(client, buff, 0);

	// receive message from server
	ret = Receive(client, buff, 0);
	message = decodeMessage(buff);
	if (message.opcode == SUCCESS) {
		show_info_result(payload, message.payload);
	}
	else {
		switch (stoi(message.payload))
		{
		case NO_LOGIN:
			cout << "You are not logged in!!";
			break;
		case ROOM_NO_EXIST:
			cout << "Room not exist!!";
			break;
		default:
			cout << "Error Server";
		}
	}
	cout << "\nPress key to continue!!\n";
	_getch();
	system("CLS");
}