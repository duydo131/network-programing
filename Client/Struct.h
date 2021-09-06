#pragma once
#include "iostream"

using namespace std;

struct Message {
	int opcode;
	int length;
	string payload;
};

struct Question {
	int id;
	string question;
	string options[4];
	string answer;
};

struct Room {
	string id;
	int number_of_question;
	int length_time;
	string start_time;
};
