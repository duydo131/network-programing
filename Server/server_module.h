#pragma once

#include "stdafx.h"
#include "stdio.h"
#include "server_utils.h"
#include <string>
#include "fstream"
#include "ws2tcpip.h"
#include "winsock2.h"
#include "list"
#include <stdlib.h>     /* srand, rand */
using namespace std;

/*
* Function to get all rooms in database
* @returns list of room in database
*/
vector<Room> getAllRooms(string rooms_path) {
	vector<Room> rooms;
	string line;
	ifstream file(rooms_path);
	while (!file.eof())
	{
		getline(file, line);
		if (line == "") continue;
		Room room;
		vector<string> data = split(line, SPACE_DELIMITER);
		room.id = data[0];
		room.number_of_question = stoi(data[1]);
		room.length_time = stoi(data[2]);
		room.start_time = data[3];
		rooms.push_back(room);
	}
	file.close();
	return rooms;
}

string ACCOUNTS_PATH = "accounts.txt";
string QUESTIONS_PATH = "questions.txt";
string ROOMS_PATH = "rooms.txt";

vector<Account> accounts = getAllAccounts(ACCOUNTS_PATH);
vector<Question> questions = getAllQuestions(QUESTIONS_PATH);
vector<Room> rooms = getAllRooms(ROOMS_PATH);

struct Session {
	SOCKET s;
	sockaddr_in addr;
	char username[50];
	bool login;
	char clientIP[INET_ADDRSTRLEN];
	int clientPort;
	int seek;
};

time_t to_time_t(const string& timestamp) // throws on bad timestamp
{
	tm tm{};
	tm.tm_year = stoi(timestamp.substr(0, 4)) - 1900;
	tm.tm_mon = stoi(timestamp.substr(4, 2)) - 1;
	tm.tm_mday = stoi(timestamp.substr(6, 2));
	tm.tm_hour = stoi(timestamp.substr(8, 2));
	tm.tm_min = stoi(timestamp.substr(10, 2));
	tm.tm_sec = stoi(timestamp.substr(12, 2));

	return mktime(addressof(tm));
}

int comparetime(time_t time1, time_t time2) {
	return difftime(time1, time2) > 0.0 ? 1 : -1;
}

bool check_status_room(Room room) {
	time_t current_time = time(0);
	if (comparetime(to_time_t(room.start_time), current_time) == 1) return true;
	else return false;
}
bool checkAccountExist(string username) {
	int number_accounts = accounts.size();
	for (int i = 0; i < number_accounts; i++) {
		if (accounts[i].username == username && accounts[i].status == 1) {
			return true;
		}
	}
	return false;
}

// ------------
string decode_question(Question q) {
	string payload = "";
	string id = to_string(q.id);
	string question = q.question;
	string options;
	options.append(q.options[0]);
	options.append(Q_DELIMITER);
	options.append(q.options[1]);
	options.append(Q_DELIMITER);
	options.append(q.options[2]);
	options.append(Q_DELIMITER);
	options.append(q.options[3]);
	payload.append(id);
	payload.append(Q_DELIMITER);
	payload.append(question);
	payload.append(Q_DELIMITER);
	payload.append(options);
	payload.append(A_DELIMITER);
	return payload;
}

int decode_room(Room &room, string payload) {
	vector<string> info_room = split(payload, A_DELIMITER);
	try {
		room.id = info_room[0];
		room.number_of_question = stoi(info_room[1]);
		room.length_time = stoi(info_room[2]);
		room.start_time = info_room[3];
		return 1;
	}
	catch (exception ex) {
		return -1;
	}
}

int get_room(string id) {
	for (int i = 0; i < rooms.size(); i++) {
		if (rooms[i].id == id) return i;
	}
	return -1;
}

string generate_id(Session* session) {
	time_t seconds = time(NULL);
	char id[100];
	sprintf_s(id, 100, "%s%d%ld", session->clientIP, session->clientPort, (long)seconds);
	return convertToString(id, strlen(id));
}

int inline random_index(int max) {
	return rand() % max;
}

vector<int> random_question(int number_of_question) {
	vector<int> rs;
	list<int> temp;
	int length_of_all_question = questions.size();
	for (int i = 0; i < length_of_all_question; i++) {
		temp.push_back(i);
	}
	int index;
	list<int>::iterator it;
	for (int i = 0; i < number_of_question; i++) {
		index = random_index(temp.size());
		it = temp.begin();
		advance(it, index);
		rs.push_back(*it);
		temp.erase(it);
	}
	return rs;
}

void load_question(Room &room) {
	vector<int> ramdom_intdex_question = random_question(room.number_of_question);
	room.questions.clear();
	for (int i : ramdom_intdex_question) room.questions.push_back(questions[i]);
}

int save_room(string rooms_path, Room room) {
	ofstream outfile(rooms_path, ios::app);
	if (outfile.fail()) return -1;
	outfile << endl << room.id << SPACE_DELIMITER 
					<< room.number_of_question << SPACE_DELIMITER 
					<< room.length_time << SPACE_DELIMITER 
					<< room.start_time;
	outfile.close();
	return 1;
}

bool validate_result(vector<string> rs) {
	for (string r : rs) {
		if (r != "A" && r != "B" && r != "C" && r != "D") return false;
	}
	return true;
}

Message process_check_result(Message message, Session *session) {
	Message response;
	vector<string> rs = split(message.payload, A_DELIMITER);
	string payload;
	if (validate_result(rs)) {
		int correct = 0, wrong = 0;
		for (int i = 0; i < rs.size(); i++) {
			if (rs[i] != questions[i].answer) wrong++;
			else correct++;
		}
		string res = to_string(correct) + SPACE_DELIMITER + to_string(wrong);
		response.opcode = SUCCESS;
		response.payload = res;
		response.length = response.payload.length();
	}
	else {
		response.opcode = ERROR_CODE;
		response.payload = to_string(ERROR_RESULT);
		response.length = response.payload.length();
	}

	return response;
}

Message process_setup_room(Message message, Session *session) {
	Message response;
	response.opcode = ERROR_CODE;
	response.payload = to_string(ERROR_SETUP_ROOM);
	response.length = response.payload.length();
	Room room;
	vector<string> data = split(message.payload, R_DELIMITER);
	if (data.size() == 3) {
		try {
			int number_of_question = stoi(data[0]);
			int length_time = stoi(data[1]);
			string start_time = data[2];
			room.number_of_question = number_of_question;
			room.length_time = length_time;
			room.start_time = start_time;
			room.id = generate_id(session);
			int res = save_room(ROOMS_PATH, room);
			if (res < 0) throw 1;
			rooms.push_back(room);
			response.opcode = SUCCESS;
		}
		catch (exception &ex) {
			// pass
		}
	}
	return response;
}

/**
* Function for client registry account
* @param message: message to handle
* @retruns response message for client
*/
Message registry(Message message) {
	Message response;

	Account account;
	vector<string> data = split(message.payload, Q_DELIMITER);
	account.username = data[0];
	account.password = data[1];
	account.status = 1;

	ResponseCode resCode;
	if (!checkAccountExist(account.username)) {
		saveAccount(account, ACCOUNTS_PATH);
		accounts.push_back(account);
		response.opcode = SUCCESS;
	}
	else {
		response.opcode = ERROR_CODE;
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
	response.opcode = ERROR_CODE;

	Account account;
	vector<string> data = split(message.payload, Q_DELIMITER);
	account.username = data[0];
	account.password = data[1];

	ResponseCode resCode = INCORRECT_ACCOUNT;

	if (session->login) {
		resCode = LOGGED;
	}
	else {
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
						strcpy_s(session->username, account.username.length() + 1, account.username.c_str());
						response.opcode = SUCCESS;
					}
				}
				else {
					resCode = ACCOUNT_LOCKED;
				}
				break;
			}
		}
	}
	if (response.opcode != SUCCESS) {
		response.payload = to_string(resCode);
		response.length = response.payload.length();
	}

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
	response.opcode = ERROR_CODE;

	ResponseCode resCode = BAD_REQUEST;

	if (session->login)
	{
		int number_accounts = accounts.size();
		for (int i = 0; i < number_accounts; i++) {
			if (accounts[i].username == convertToString(session->username, strlen(session->username)) && accounts[i].login == true && accounts[i].status == 1) {
				accounts[i].login = false;
				session->login = false;
				session->username[0] = 0;
				response.opcode = SUCCESS;
				break;
			}
		}
	}
	else
	{
		resCode = NO_LOGIN;
	}
	if (response.opcode != SUCCESS) {
		response.payload = to_string(resCode);
		response.length = response.payload.length();
	}
	return response;
}

/**
* Function for client practice
* @param message: message to handle
* @param session: [IN/OUT] pointer session of client
* @retruns response message for client
*/
Message practice(Message message, Session *session) {
	Message response;
	response.opcode = ERROR_CODE;

	ResponseCode resCode = BAD_REQUEST;

	if (session->login)
	{
		response.opcode = SUCCESS;
	}
	else
	{
		resCode = NO_LOGIN;
	}

	if (response.opcode != SUCCESS) {
		response.payload = to_string(resCode);
		response.length = response.payload.length();
	}
	return response;
}

/**
* Function for client get info room
* @param message: message to handle
* @param session: [IN/OUT] pointer session of client
* @retruns response message for client
*/
Message get_info_room(Message message, Session *session) {
	Message response;
	response.opcode = SUCCESS;
	string payload = "";
	for (int i = 0; i < rooms.size(); i++) {
		string id = rooms[i].id;
		string number_question = to_string(rooms[i].number_of_question);
		string length_time = to_string(rooms[i].length_time);
		string start_time = rooms[i].start_time;
		payload.append(id);
		payload.append(A_DELIMITER);
		payload.append(number_question);
		payload.append(A_DELIMITER);
		payload.append(length_time);
		payload.append(A_DELIMITER);
		payload.append(start_time);
		payload.append(Q_DELIMITER);
	}
	response.payload = payload;
	response.length = response.payload.length();

	return response;
}

/**
* Function for client access room
* @param message: message to handle
* @param session: [IN/OUT] pointer session of client
* @retruns response message for client
*/
Message access_room(Message message, Session *session) {
	Message response;
	response.opcode = ERROR_CODE;

	ResponseCode resCode = BAD_REQUEST;
	if (session->login)
	{
		string id_room = message.payload;
		int index_room = get_room(id_room);
		if (index_room >= 0) {
			Room room = rooms[index_room];
			if (check_status_room(room)) {
				string payload; 
				payload.append(room.id);
				payload.append(A_DELIMITER);
				payload.append(to_string(room.number_of_question));
				payload.append(A_DELIMITER);
				payload.append(to_string(room.length_time));
				payload.append(A_DELIMITER);
				payload.append(room.start_time);
				response.payload = payload;
				response.length = payload.length();
				response.opcode = SUCCESS;
			}
			else {
				resCode = ROOM_STARTED;
			}
		}
		else {
			resCode = ROOM_NO_EXIST;
		}
	}
	else
	{
		resCode = NO_LOGIN;
	}
	if (response.opcode != SUCCESS) {
		response.payload = to_string(resCode);
		response.length = response.payload.length();
	}
	return response;
}

/**
* Function for client get questions
* @param message: message to handle
* @param session: [IN/OUT] pointer session of client
* @retruns response message for client
*/
Message get_question(Message message, Session *session) {
	Message response;
	if (session->login)
	{
		string payload = "";
		vector<string> info = split(message.payload, Q_DELIMITER);
		int seek = stoi(info[0]);
		string id_room = info[1];
		if (id_room == PRACTICE) {
			if (seek == 0) session->seek = random_index(questions.size());
			for (int i = 0; i < QUESTION_SIZE; i++) {
				if (seek + i >= QUESTION_SIZE_PRACTICE) break;
				int index = (seek + session->seek + i) % questions.size();
				payload.append(decode_question(questions[index]));
			}
			response.opcode = SUCCESS;
			response.payload = payload;
			response.length = response.payload.length();
			return response;
		}
		int index_room = get_room(id_room);
		
		if (index_room >= 0) {
			Room room = rooms[index_room];
			if (room.questions.size() == 0) load_question(room);
			int max_question = min(seek + QUESTION_SIZE, room.number_of_question);
			for (int i = seek; i < max_question; i++) {
				payload.append(decode_question(room.questions[i]));
			}
			response.opcode = SUCCESS;
			response.payload = payload;
			response.length = response.payload.length();
			return response;
		}
		response.opcode = ERROR_CODE;
		response.payload = to_string(ROOM_NO_EXIST);
		response.length = response.payload.length();
		return response;
	}
	response.opcode = ERROR_CODE;
	response.payload = to_string(NO_LOGIN);
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
	case 2: {
		// registry account process 
		response = registry(message);
		break;
	}
	case 3: {
		// login process
		response = login(message, session);
		break;
	}
	case 4: {
		// logout process
		response = logout(message, session);
		break;
	}
	case 5: {
		// practice process
		response = practice(message, session);
		break;
	}
	case 6: {
		// practice process
		response = get_question(message, session);
		break;
	}
	case 7: {
		// practice process
		response = process_check_result(message, session);
		break;
	}
	case 8: {
		// practice process
		response = process_setup_room(message, session);
		break;
	}
	case 9: {
		// Get Info Room List Process
		response = get_info_room(message, session);
		break;
	}
	case 10: {
		// Access Room Process
		response = access_room(message, session);
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
		if (res <= 0) {
			printf("Client[%s:%d] disconnects\n", clientIP, clientPort);

			// logout client
			if (session->login) {
				int number_accounts = accounts.size();
				for (int j = 0; j < number_accounts; j++) {
					if (accounts[j].username == convertToString(session->username, strlen(session->username)))
					{
						accounts[j].login = false;
						session->login = false;
						session->username[0] = 0;
						break;
					}
				}
			}

			break;
		}
		else {
			recvBuff[res] = 0;
			printf("Receive from client[%s:%d]: %s\n", clientIP, clientPort, recvBuff);
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
