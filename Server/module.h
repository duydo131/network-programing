#pragma once
#include "ws2tcpip.h"
#include "winsock2.h"
#include <time.h>

#include "util.h"

string R_DELIMITER = "#%#";
string SPACE_DELIMITER = " ";

enum status {
	NO_CONNECT = 10,

	AUTH = 20,
	NOT_AUTH = 21,
};

enum mode {
	PRACTICE = 1,
	TEST = 2
};

enum ReturnCode {
	// Login
	ACCOUNT_LOGGED = 101,
	HAVE_NOT_ACCOUNT = 102,
	ACCOUNT_LOCKED = 103,
	HAVE_ACCOUNT = 104,

	NO_LOGIN = 201,

	BAD_REQUEST = 301,

	// Command Error
	COMMAND_ERROR = 404,
};

// struct save account 
typedef struct Account {
	string name;
	int status;
} Account;

typedef struct Session {
	char clientIP[INET_ADDRSTRLEN];
	int clientPort;
	int status;
	int mode;
} Session;

typedef struct Room {
	int number_of_question;
	int length_time;
	long long start_time;
} Room;

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