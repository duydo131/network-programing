#pragma once

#include "string"

using namespace std;

#define BUFF_SIZE 2048 // 2 KB

string Q_DELIMITER = "#%#";
string A_DELIMITER = "$%$";
string SPACE_DELIMITER = " ";
string R_DELIMITER = "&%&";
string M_DELIMITER = "%$%";

string PRACTICE = "PRACTICE";

int QUESTION_SIZE = 4;
int QUESTION_SIZE_PRACTICE = 4;

int LENGTH_TIME_PRACTICE = 10;

string ACCOUNTS_PATH = "accounts.txt";
string QUESTIONS_PATH = "questions.txt";
string ROOMS_PATH = "rooms.txt";
string RESULT_PATH = "results.txt";

string FILE_LOG = "log_13.txt";

enum ResponseCode {
	SUCCESS = 0,
	ERROR_CODE = 1,

	ACCOUNT_EXISTED = 201,

	ACCOUNT_LOGGED = 304,
	INCORRECT_ACCOUNT = 302,
	ACCOUNT_LOCKED = 303,
	LOGGED = 301,

	NO_LOGIN = 101,
	BAD_REQUEST = 102,
	// Command Error
	COMMAND_ERROR = 404,

	ROOM_STARTED = 501,
	ROOM_NO_EXIST = 502,

	ERROR_RESULT = 701,

	ERROR_SETUP_ROOM = 801
};