#pragma once

#include "stdio.h"
#include "fstream"
#include "vector"
#include "map"

#include "Constant.h"
#include "Struct.h"
#include "utils.h"

using namespace std;

// get result of room
map<string, vector<Result> > get_result(string result_path) {
	map<string, vector<Result> > rs;
	string line;
	ifstream file(result_path);
	while (getline(file, line))
	{
		Result result;
		vector<string> data = split(line, SPACE_DELIMITER);
		string id = data[0];
		result.user = data[1];
		result.right = stoi(data[2]);
		result.wrong = stoi(data[3]);
		result.time = data[4];
		if (rs.find(id) != rs.end()) rs[id].push_back(result);
		else {
			vector<Result> r;
			r.push_back(result);
			rs[id] = r;
		}
	}
	file.close();
	return rs;
}

/**
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

/**
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

/**
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

/**
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

/**
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

/**
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

/**
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
	message.payload = data[2].substr(0, message.length);
	return message;
}

// decode message for multi message
vector<Message> decode_messages(char *buff) {
	string msg = convertToString(buff, strlen(buff));
	vector<string> data = split(msg, SPACE_DELIMITER);
	vector<Message> out;
	int id = 0, length = data.size();
	if (data[length - 1] == "") length--;
	while (id < length) {
		Message message;
		message.opcode = stoi(data[id++]);
		message.length = stoi(data[id++]);
		if (message.length > 0) {
			message.payload = data[id++].substr(0, message.length);
		}
		out.push_back(message);
	}
	return out;
}

// function encode message for 1 message, addition for function encode message for multi message
int encode_message(Message message, char *buff) {
	string str = "";
	str += to_string(message.opcode);
	str += SPACE_DELIMITER;
	str += to_string(message.length);
	if (message.length > 0) {
		str += SPACE_DELIMITER;
		str.append(message.payload);
	}
	memcpy(buff, &str[0], str.length());
	return str.length();
}

//function encode message for multi message
void encode_messages(vector<Message> in, char* buff) {
	int position = 0, add;
	for (Message message : in) {
		add = encode_message(message, buff + position);
		position += add;
		buff[position++] = ' ';
	}
	buff[position - 1] = 0;
}

// encode result from Result to string
string encode_result(Result r) {
	string payload = "";
	payload.append(r.user);
	payload.append(Q_DELIMITER);
	payload.append(to_string(r.right));
	payload.append(Q_DELIMITER);
	payload.append(to_string(r.wrong));
	payload.append(Q_DELIMITER);
	payload.append(r.time);
	payload.append(A_DELIMITER);
	return payload;
}

// save result of room to database
int save_result(string result_path, Result rs, string room_id) {
	ofstream outfile(result_path, ios::app);
	if (outfile.fail()) return -1;
	outfile << endl << room_id << SPACE_DELIMITER
		<< rs.user << SPACE_DELIMITER
		<< rs.right << SPACE_DELIMITER
		<< rs.wrong << SPACE_DELIMITER
		<< rs.time;
	outfile.close();
	return 1;
}

// decode question
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

// decode room
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

// save room to database
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