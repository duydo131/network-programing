#pragma once

#include "stdio.h"
#include "iostream"
#include "fstream"
#include "string"
#include "vector"
#include "time.h"
using namespace std;

string Q_DELIMITER = "#%#";
string A_DELIMITER = "$%$";

struct Message {
	int opcode;
	int length;
	string payload;
};

struct Question {
	string question;
	string options[4];
	string answer;
};

/*
* Function to get all questions in database
* @returns list of questions in database
*/
vector<Question> getAllQuestions() {
	vector<Question> questions;
	string line;
	ifstream file("questions.txt");
	while (getline(file, line))
	{
		Question question;
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
	}
	file.close();
	return questions;
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
* Function to encode message
* @param message: [IN] message for encode
* @param buff: [OUT] encode of message
*/
void encodeMessage(Message message, char *buff) {
	string str = "";
	str += message.opcode;
	str += message.length / 256;
	str += message.length % 256;
	str.append(message.payload);
	memcpy(buff, &str[0], str.size());
}

/*
* Function to decode message
* @param buff: [IN] data for decode
* @returns message
*/
Message decodeMessage(char *buff) {
	Message message;
	message.opcode = buff[0];
	message.length = buff[1] * 256 + buff[2];
	message.payload = buff + 3;
	return message;
}

/**
* Write log of server
* @param clientIP: ip of client send request
* @param clientPort: port of client send request
* @param option: option of client with program
* @param message: message of client
* @param resCode: response code for client
*/
void log(char *clientIP, int clientPort, int option, char *message, char *resCode) {
	// time setup
	time_t t = time(NULL);
	tm tm;
	localtime_s(&tm, &t);

	// write file
	FILE *file;
	fopen_s(&file, "server_log.txt", "a");
	fprintf_s(file, "%s:%d [%d/%d/%d %d:%d:%d] $ %s $ %s\n", clientIP, clientPort, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, message, resCode);
	fclose(file);
}