#pragma once

#include "module.h"
#include "string"
#include "vector"
#include "server_utils.h"

using namespace std;

/*	Check account in the database
input :
string : username
vector<Account> : database account
output : return code
*/
int check_account(string username, vector<Account> account) {
	for (Account acc : account) {
		if (acc.name == username) {
			if (acc.status == 0) return HAVE_ACCOUNT;
			else return ACCOUNT_LOCKED;
		}
	}
	return HAVE_NOT_ACCOUNT;
}

void process_data(char* in, char* out, vector<Account> account, Session* session, vector<Room> rooms) {
	Message message = decodeMessage(in);
	// signin
	if (message.opcode == 2) {
		if (session->status == AUTH) {
			char* payload = int_to_char(ACCOUNT_LOGGED);
			generate_response(out, '1', strlen(payload), payload);
			free(payload);
		}
		else {
			int status = check_account(message.payload, account);
			if (status == HAVE_ACCOUNT) {
				session->status = AUTH;
				generate_response(out, '0', 0, NULL);
			}
			else {
				char* payload = int_to_char(status);
				generate_response(out, '1', strlen(payload), payload);
				free(payload);
			}
		}
	}
	else {
		if (session->status == NOT_AUTH) {
			char* payload = int_to_char(NO_LOGIN);
			generate_response(out, '1', strlen(payload), payload);
			free(payload);
		}
		else {
			// choose mode
			if (message.opcode == 3 || message.opcode == 4) {
				session->mode = message.opcode;
				generate_response(out, '0', 0, NULL);
			}
			// get data
			else if (message.opcode == 5) {
				const char* payload = encodeQuestions(getAllQuestions()).c_str();
				generate_response(out, '0', strlen(payload), payload);
			}
			// get result
			else if (message.opcode == 6) {
				char* payload; // result
				generate_response(out, '0', strlen(payload), payload);
			}
			// set room
			else if (message.opcode == 7) {
				Room r;
				bool check = decode_room(message.payload, r);
				if (check) {
					generate_response(out, '0', 0, NULL);
				}
				else {
					char* payload = int_to_char(BAD_REQUEST);
					generate_response(out, '1', strlen(payload), payload);
					free(payload);
				}
			}
			// get info room
			else if (message.opcode == 8) {
				const char* payload = get_info_room(rooms).c_str();
				generate_response(out, '0', strlen(payload), payload);
			}
			// access room
			else if (message.opcode == 9) {
				char* payload; // access room
				generate_response(out, '0', strlen(payload), payload);
			}
			// submit
			else if (message.opcode == 10) {
				int number = submit(message.payload);
				char* payload = int_to_char(number);
				generate_response(out, '0', strlen(payload), payload);
			}
		}
	}
}

void generate_response(char* out, char opcode, int length, const char* payload) {
	*(out) = opcode;
	add_length(length, out);
	if(payload != NULL) memcpy(out + 3, payload, length);
}

string get_info_room(vector<Room> rooms) {
	string out;
	int index = 0;
	for (Room room : rooms) {
		out.append(int_to_char(room.number_of_question));
		out.append(SPACE_DELIMITER);
		out.append(int_to_char(room.length_time));
		out.append(SPACE_DELIMITER);
		out.append(int_to_char(room.start_time));
		out.append(R_DELIMITER);
	}
	return out;
}

int submit(string payload) {
	//process result
	return 0;
}