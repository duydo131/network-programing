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
string SPACE_DELIMITER = " ";

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
	int start = 0;
	int end = s.find(del);
	while (end != -1) {
		string substr = s.substr(start, end - start);
		rs.push_back(substr);
		start = end + del.size();
		end = s.find(del, start);
	}
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
	string line;
	ifstream file(questions_path);
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