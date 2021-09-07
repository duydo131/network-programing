#pragma once

#include "stdafx.h"
#include "stdio.h"
#include "ws2tcpip.h"
#include "winsock2.h"
#include "list"
#include "stdlib.h"

#include "server_utils.h"

using namespace std;

struct Session {
	SOCKET s;
	sockaddr_in addr;
	char username[50];
	bool login;
	char clientIP[INET_ADDRSTRLEN];
	int clientPort;
	int seek;
	Account* account = nullptr;
};

vector<Account> accounts = getAllAccounts(ACCOUNTS_PATH);
vector<Question> questions = getAllQuestions(QUESTIONS_PATH);
vector<Room> rooms = getAllRooms(ROOMS_PATH);
map<string, vector<Result> > results = get_result(RESULT_PATH);

bool checkAccountExist(string username) {
	int number_accounts = accounts.size();
	for (int i = 0; i < number_accounts; i++) {
		if (accounts[i].username == username && accounts[i].status == 1) {
			return true;
		}
	}
	return false;
}

// log
void log_activity(Session* session, char* request, Message response, string filename, time_t now) {
	string res = "";
	if (response.opcode == SUCCESS) res += to_string(SUCCESS);
	else res += to_string(ERROR_CODE) + " " + response.payload;

	// define pre_log "ClientIP:ClientPort"
	char* pre_log = get_pre_log(session->clientIP, session->s);

	// define pre_log "[dd/mm/yyyy hh:mm:ss]"
	char* time_request = get_time_request(now);

	char log_data[BUFF_SIZE];
	sprintf_s(log_data, "%s %s $ %s $ %s", pre_log, time_request, request, res.c_str());

	// log 
	log(filename, log_data);

	free(time_request);
	free(pre_log);
}

// get room for room_id
int get_room(string id) {
	for (int i = 0; i < rooms.size(); i++) {
		if (rooms[i].id == id) return i;
	}
	return -1;
}

// generate id for room
string generate_id(Session* session) {
	time_t seconds = time(NULL);
	char id[50];
	char* IP = format_client_IP(session->clientIP);
	sprintf_s(id, 50, "%d%s%d%ld", random_range(10000, 99999), IP, session->clientPort, (long)seconds);
	free(IP);
	return convertToString(id, strlen(id));
}

/**
* load question for room (lazy : when client request, load_question 1 time)
*/
void load_question(Room &room) {
	vector<int> ramdom_intdex_question = random_question(room.number_of_question, questions.size());
	room.questions.clear();
	for (int i : ramdom_intdex_question) room.questions.push_back(questions[i]);
}

/**
* Function for client check answer
* @param message: message to handle
* @param session: [IN/OUT] pointer session of client
* @retruns response message for client
*/
Message process_check_result(Message message, Session *session) {
	Message response;
	// check login
	if (session->login) {
		vector<string> info = split(message.payload, M_DELIMITER);
		string id_room = info[0];
		int correct = 0, wrong = 0;
		vector<string> rs = split(info[1], A_DELIMITER);

		if (id_room == PRACTICE) {
			if (session->seek < 0) {
				response.opcode = ERROR_CODE;
				response.payload = to_string(BAD_REQUEST);
				response.length = response.payload.length();
				return response;
			}
			if (info[1] == "") {
				correct = 0;
				wrong = QUESTION_SIZE_PRACTICE;
			}
			else {
				for (int i = 0; i < rs.size(); i++) {
					if (rs[i] != questions[i + session->seek].answer) wrong++;
					else correct++;
				}
				wrong += QUESTION_SIZE_PRACTICE - rs.size();
			}
			response.opcode = SUCCESS;
			session->seek = -1;
		}
		int index_room = get_room(id_room);

		if (index_room >= 0) {
			Room room = rooms[index_room];
			if (room.questions.size() == 0) load_question(room);
			if (info[1] == "") {
				correct = 0;
				wrong = room.questions.size();
			}
			else {
				for (int i = 0; i < rs.size(); i++) {
					if (rs[i] != room.questions[i].answer) wrong++;
					else correct++;
				}
				wrong += room.questions.size() - rs.size();
			}
			response.opcode = SUCCESS;
			Result result;
			result.user = session->username;
			result.right = correct;
			result.wrong = wrong;
			result.time = info[2];
			save_result(RESULT_PATH, result, room.id);
			if (results.find(room.id) != results.end()) results[room.id].push_back(result);
			else {
				vector<Result> r;
				r.push_back(result);
				results[room.id] = r;
			}
		}
		if (response.opcode == SUCCESS) {
			string res = to_string(correct) + SPACE_DELIMITER + to_string(wrong);
			response.payload = res;
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
* Function for client create room
* @param message: message to handle
* @param session: [IN/OUT] pointer session of client
* @retruns response message for client
*/
Message process_setup_room(Message message, Session *session) {
	Message response;
	if (session->login) {
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
				response.length = 0;
				return response;
			}
			catch (int ex) {}
			catch (exception &ex) {}
		}
		response.opcode = ERROR_CODE;
		response.payload = to_string(ERROR_SETUP_ROOM);
		response.length = response.payload.length();
		return response;
	}
	response.opcode = ERROR_CODE;
	response.payload = to_string(NO_LOGIN);
	response.length = response.payload.length();
	return response;
}

/**
* Function for client get result of room
* @param message: message to handle
* @param session: [IN/OUT] pointer session of client
* @retruns response message for client
*/
Message process_get_result_of_room(Message message, Session *session) {
	Message response;
	if (session->login) {
		Room room;
		string room_id = message.payload;
		int room_index = get_room(room_id);
		if (room_index < 0) {
			response.opcode = ERROR_CODE;
			response.payload = to_string(ROOM_NO_EXIST);
			response.length = response.payload.length();
			return response;
		}
		vector<Result> result = results[room_id];
		string payload = "";
		for (Result r : result) {
			payload += encode_result(r);
		}
		response.opcode = SUCCESS;
		response.payload = payload;
		response.length = payload.length();
		return response;
	}
	response.opcode = ERROR_CODE;
	response.payload = to_string(NO_LOGIN);
	response.length = response.payload.length();
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
		response.length = 0;
	}
	else {
		response.opcode = ERROR_CODE;
		resCode = ACCOUNT_EXISTED;
	}
	if (response.opcode != SUCCESS) {
		response.payload = to_string(resCode);
		response.length = response.payload.length();
	}

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
						response.length = 0;
						session->account = &accounts[i];
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
				response.length = 0;
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
		string payload = to_string(LENGTH_TIME_PRACTICE) + A_DELIMITER + to_string(QUESTION_SIZE_PRACTICE);
		response.opcode = SUCCESS; 
		response.payload = payload;
		response.length = payload.length();
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
				resCode = ROOM_NOT_STARTED;
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
	case 11: {
		// Access Room Process
		response = process_get_result_of_room(message, session);
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
