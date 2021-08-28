#pragma once

#include "stdio.h"
#include "iostream"
#include "string"
#include "vector"
#define BUFF_SIZE 2048 // 2 MB
using namespace std;

string Q_DELIMITER = "#%#";
string A_DELIMITER = "$%$";
string SPACE_DELIMITER = " ";
string R_DELIMITER = "&%&";

struct Message {
	int opcode;
	int length;
	string payload;
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
	substr = msg.substr(end, msg.length() - end);
	message.payload = substr;
	return message;
}

/*
* Function to format time
* @param time: [IN] time need format
* @returns time format
*/
string formatTime(string time) {
	string res;
	res.push_back(time[8]);
	res.push_back(time[9]);
	res.push_back(':');
	res.push_back(time[10]);
	res.push_back(time[11]);
	res.push_back(':');
	res.push_back(time[12]);
	res.push_back(time[13]);
	res.push_back(' ');
	res.push_back(time[6]);
	res.push_back(time[7]);
	res.push_back('-');
	res.push_back(time[4]);
	res.push_back(time[5]);
	res.push_back('-');
	res.push_back(time[0]);
	res.push_back(time[1]);
	res.push_back(time[2]);
	res.push_back(time[3]);
	return res;
}