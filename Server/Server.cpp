// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "iostream"
#include "fstream"
#include "string"
#include "vector"
#include "ws2tcpip.h"
#include "winsock2.h"
using namespace std;

string Q_DELIMITER = "#%#";
string A_DELIMITER = "$%$";

struct Question {
	string question;
	string options[4];
	string answer;
};

vector<Question> questions;

/*
* Function to get all questions in database
*/
void getAllQuestions() {
	string line;
	ifstream file("questions.txt");

	while ( getline(file, line) )
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
}

/*
* Function to encode questions
* @param questions: Type vector - list questions for encode
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

int main()
{	
	getAllQuestions();
	string encode = encodeQuestions(questions);
	cout << encode;
    return 0;
}


