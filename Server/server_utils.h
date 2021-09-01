#pragma once

#include "stdio.h"
#include "iostream"
#include "fstream"
#include "string"
#include "vector"
#include "time.h"
#define BUFF_SIZE 2048 // 2 KB
using namespace std;

string Q_DELIMITER = "#%#";
string A_DELIMITER = "$%$";
string SPACE_DELIMITER = " ";
string R_DELIMITER = "&%&";

string PRACTICE = "PRACTICE";

int QUESTION_SIZE = 4;
int QUESTION_SIZE_PRACTICE = 4;

string ACCOUNTS_PATH = "accounts.txt";
string QUESTIONS_PATH = "questions.txt";
string ROOMS_PATH = "rooms.txt";
string FILE_LOG = "log_13.txt";

struct Question {
	int id;
	string question;
	string options[4];
	string answer;
};

struct Room {
	string id;
	int number_of_question;
	vector<Question> questions; //lazy
	int length_time;
	string start_time;
};

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

struct Message {
	int opcode;
	int length;
	string payload;
};

struct Account {
	string username;
	string password;
	int status;
	bool login = false;
};

/*
* Function to split string by delimiter
* @returns list of string
*/
vector<string> split(string s, string del)
{
	vector<string> rs;
	string substr;
	int start = 0;
	int end = s.find(del);
	while (end != -1) {
		substr = s.substr(start, end - start);
		rs.push_back(substr);
		start = end + del.size();
		end = s.find(del, start);
	}
	substr = s.substr(start, s.length() - start);
	rs.push_back(substr);
	return rs;
}

/*
* Function to get all accounts in database
* @returns list of account in database
*/
vector<Account> getAllAccounts(string accounts_path) {
	vector<Account> accounts;
	string line;
	ifstream file(accounts_path);
	while (getline(file, line))
	{
		Account account;
		vector<string> data = split(line, SPACE_DELIMITER);
		account.username = data[0];
		account.password = data[1];
		account.status = stoi(data[2]); // convert to int
		accounts.push_back(account);
	}
	file.close();
	return accounts;
}

/*
* Function save new account to database
*/
void saveAccount(Account account, string accounts_path) {
	string input;
	input.append(account.username);
	input.append(SPACE_DELIMITER);
	input.append(account.password);
	input.append(SPACE_DELIMITER);
	input.append(to_string(account.status));

	// write file
	ofstream file(accounts_path, ios::app);
	file << input << endl;
	file.close();
}

/*
* Function to get all questions in database
* @returns list of question in database
*/
vector<Question> getAllQuestions(string questions_path) {
	vector<Question> questions;
	int id = 1;
	string line;
	ifstream file(questions_path);
	while (getline(file, line))
	{
		Question question;
		question.id = id;
		question.question = line;
		for (int i = 0; i < 5; i++) {
			getline(file, line);
			if (i != 4) {
				question.options[i] = line;
			}
			else {
				question.answer = line;
			}
		}
		questions.push_back(question);
		id++;
	}
	file.close();
	return questions;
}

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

/*
* Function to encode questions
* @param questions: [IN] List questions for encode
* @returns output string of encode list question
*/
string encodeQuestions(vector<Question> questions) {
	string output;
	int noq = questions.size();
	for (int i = 0; i < noq; i++) {
		output.append(to_string(questions[i].id));
		output.append(A_DELIMITER);
		output.append(questions[i].question);
		output.append(A_DELIMITER);
		output.append(questions[i].options[0]);
		output.append(A_DELIMITER);
		output.append(questions[i].options[1]);
		output.append(A_DELIMITER);
		output.append(questions[i].options[2]);
		output.append(A_DELIMITER);
		output.append(questions[i].options[3]);
		output.append(Q_DELIMITER);
	}
	return output;
}

/*
* Function convert char array to string
* @param a: [IN] char array
* @param size: [IN] size of char array
* @returns string converted
*/
string convertToString(char* a, int size)
{
	int i;
	string s = "";
	for (i = 0; i < size; i++) {
		s = s + a[i];
	}
	return s;
}

/*
* Function to encode message
* @param message: [IN] message for encode
* @param buff: [OUT] encode of message
*/
void encodeMessage(Message message, char *buff) {
	string str = "";
	str += to_string(message.opcode);
	str += SPACE_DELIMITER;
	str += to_string(message.length);
	str += SPACE_DELIMITER;
	str.append(message.payload);
	memcpy(buff, &str[0], BUFF_SIZE);
}

/*
* Function to decode message
* @param buff: [IN] data for decode
* @returns message
*/
Message decodeMessage(char *buff) {
	string msg = convertToString(buff, strlen(buff));
	vector<string> data = split(msg, SPACE_DELIMITER);
	Message message;
	message.opcode = stoi(data[0]);
	message.length = stoi(data[1]);
	message.payload = data[2];
	return message;
}


/*	Write File
input :
char* filename
char[] data to write
*/
void log(string fileLog, char data[]) {
	ofstream ofs(fileLog, ofstream::app);

	ofs << data << endl;
}

/*	Get pre_log include clientIP and clientPort
input :
char[] clientIP
int clientPort
output : char* valid form
*/
char* get_pre_log(char clientIP[], int clientPort) {
	int size = 22;
	char* pre_log = (char*)malloc(size);
	sprintf_s(pre_log, size, "%s:%d", clientIP, clientPort);
	return pre_log;
}

/*	Change time to valid format
input : time_t
output : char* valid form [dd/mm/yyyy hh:mm:ss]
*/
char* get_time_request(time_t t) {
	int size = 22;
	char* time_request = (char*)malloc(size);
	tm ltm;
	localtime_s(&ltm, &t);
	sprintf_s(time_request, size, "[%02d/%02d/%04d %02d:%02d:%02d]",
		ltm.tm_mday, ltm.tm_mon + 1, ltm.tm_year + 1900,
		ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
	return time_request;
}