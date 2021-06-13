#pragma once
#include "iostream"
#include "fstream"
#include "string"
#include "vector"

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

typedef struct Room {
	int number_of_question;
	int length_time;
	long long start_time;
} Room;

bool decode_room(string in, Room &r) {
	int space = 0;
	int index[2], inx;
	for (int i = 0; i < in.length(); i++) {
		if (in[i] == ' ') {
			space++;
			if (space > 2) return false;
			index[inx++] = i;
		}
	}
	if (space != 2) return false;
	r.number_of_question = atoi(in.substr(0, index[0]).c_str());
	r.length_time = atoi(in.substr(index[0], index[1]).c_str());
	r.start_time = atoi(in.substr(index[1], in.length()).c_str());
	return true;
}

/*
* Function to encode message
* @param message: [IN] message for encode
* @param buff: [OUT] encode of message
*/
// đức
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


// duy
void generate_response(char* out, char opcode, int length, const char* payload) {
	*(out) = opcode;
	add_length(length, out);
	if (payload != NULL) memcpy(out + 3, payload, length);
}

/*
change int to number base 256
@Param Input
length : number
buff[] : char array storage length
@Param Output : no return value
*/
void add_length(int length, char* buff) {
	*(buff + 1) = length / 256;
	*(buff + 2) = length % 256;
}

