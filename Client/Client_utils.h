#pragma once

#include "stdio.h"
#include "iostream"
#include "string"
#include "vector"
#include <conio.h>

#include "Constant.h"
#include "utils.h"
#include "Struct.h"

using namespace std;

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
	memcpy(buff, &str[0], str.length());
	*(buff + str.length()) = 0;
}

/**
* Function to decode message
* @param buff: [IN] data for decode
* @returns message
*/
Message decodeMessage(char *buff) {
	string msg = convertToString(buff, strlen(buff));
	Message message;

	vector<string> data;
	string substr;
	int start = 0, end;

	// Get opcode
	end = msg.find(SPACE_DELIMITER);
	substr = msg.substr(start, end - start);
	start = end + SPACE_DELIMITER.size();
	message.opcode = stoi(substr);

	// Get length
	end = msg.find(SPACE_DELIMITER);
	substr = msg.substr(start, end - start);
	start = end + SPACE_DELIMITER.size();
	message.length = stoi(substr);

	// Get payload
	end = msg.find(SPACE_DELIMITER, end+1);
	substr = msg.substr(end + 1, msg.length() - end);
	message.payload = substr;
	return message;
}

/**
* show result after server check answer
* Form result : correct wrong
* @Param:  string result (is encoded)
* @No Return Value
*/
void show_result(string payload) {
	vector<string> rs = split(payload, SPACE_DELIMITER);
	cout << "Result: " << endl;
	cout << "Correct: " << rs[0] << endl;
	cout << "Wrong: " << rs[1] << endl;
}

/**
* Show result of room
* @Param :  string room_id : id of room
string data : data result of room
* @No Return Value
*/
void show_info_result(string room_id, string data) {
	cout << endl << "Room ID: " << room_id << endl;
	string tab = "    ";
	vector<string> results = split(data, A_DELIMITER);
	if (results.size() <= 1) {
		cout << "Room have not result" << endl;
		return;
	}
	for (int j = 0; j < results.size() - 1; j++) {
		vector<string> info = split(results[j], Q_DELIMITER);
		cout << tab << "User: " << info[0] << endl;
		cout << tab << tab << "Correct: " << info[1] << endl;
		cout << tab << tab << "Wrong: " << info[2] << endl;
		cout << tab << tab << "User: " << formatTime(info[3]) << endl << endl;
	}
}

/**
* Show infomation of room
* @Param :  string info : infomation of room
* @No Return Value
*/
void show_info_room(string info) {
	system("CLS");
	if (info == "") {
		cout << "No Rooms Available!!!!!\n";
		return;
	}
	vector<string> payloads = split(info, Q_DELIMITER);
	for (int j = 0; j < payloads.size() - 1; j++) {
		vector<string> room_info = split(payloads[j], A_DELIMITER);
		cout << (j + 1) << ". Room ID: " << room_info[0] << endl;
		cout << "\tNumber of question : " << room_info[1] << endl;
		cout << "\tLengh time : " << room_info[2] << endl;
		cout << "\tStart time : " << formatTime(room_info[3]) << endl << endl;
	}
}