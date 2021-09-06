#pragma once

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

struct Result {
	string user;
	int right;
	int wrong;
	string time;
};