#pragma once
#include "iostream"
#include "fstream"
#include "string"
#include "vector"
#include "time.h"
#include "ws2tcpip.h"
#include "winsock2.h"

using namespace std;

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

time_t to_time_t(const string& timestamp){
	tm tm{};
	tm.tm_year = stoi(timestamp.substr(0, 4)) - 1900;
	tm.tm_mon = stoi(timestamp.substr(4, 2)) - 1;
	tm.tm_mday = stoi(timestamp.substr(6, 2));
	tm.tm_hour = stoi(timestamp.substr(8, 2));
	tm.tm_min = stoi(timestamp.substr(10, 2));
	tm.tm_sec = stoi(timestamp.substr(12, 2));

	return mktime(addressof(tm));
}

int comparetime(time_t time1, time_t time2) {
	return difftime(time1, time2) > 0.0 ? 1 : -1;
}

bool check_start_time(string time) {
	string const timestamp = time;
	time_t start_time = to_time_t(timestamp);
	time_t now = time_t(0);
	int i = comparetime(now, start_time);
	return i < 0;
}

bool is_leap(int year)
{
	return (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0));
}

bool is_valid_date(int d, int m, int y)
{
	if (y > 9999 || y < 1800) return false;
	if (m < 1 || m > 12) return false;
	if (d < 1 || d > 31) return false;
	if (m == 2) return (is_leap(y)) ? (d <= 29) : (d <= 28);
	if (m == 4 || m == 6 || m == 9 || m == 11) return d <= 30;
	return true;
}

int get_line(string description, int min = 0, int max = INT_MAX) {
	string in;
	int out;
	while (true) {
		try {
			cout << description + " : ";
			getline(cin, in);
			out = stoi(in);
			if (out < min || out > max) throw 1;
			return out;
		}
		catch (exception &err) {
			cout << description + " not valid!!!\n";
			cout << "Press key to continue!!\n";
			_getch();
			system("CLS");
		}
	}
}

string format_form_date(int i) {
	string out = "00" + to_string(i);
	return out.substr(out.length() - 2, 2);
}

string get_start_time() {
	int year, month, day, hour, minute, second;
	while (true) {
		try {

			year = get_line("Year", 1000, 9999);
			month = get_line("Month", 1, 12);
			day = get_line("Day", 1, 31);
			if (!is_valid_date(day, month, year)) throw 1;
			hour = get_line("Hour", 0, 23);
			minute = get_line("Minute", 0, 59);
			second = get_line("Second", 0, 59);
			string out = to_string(year) + format_form_date(month) + format_form_date(day)
				+ format_form_date(hour) + format_form_date(minute) + format_form_date(second);
			return out;
		}
		catch (exception &ex) {
			cout << "Invalid Date!!!\n";
			cout << "Press key to continue!!\n";
			_getch();
			system("CLS");
		}
	}
	return "";
}

/*
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
*/


// duy
/*
void generate_response(char* out, char opcode, int length, const char* payload) {
	*(out) = opcode;
	add_length(length, out);
	if (payload != NULL) memcpy(out + 3, payload, length);
}


change int to number base 256
@Param Input
length : number
buff[] : char array storage length
@Param Output : no return value
void add_length(int length, char* buff) {
	*(buff + 1) = length / 256;
	*(buff + 2) = length % 256;
}
*/

/**
* Receive message of SOCKET
* @param s: SOCKET receive message
* @param buff: [OUT] char pointer to contain message receive
* @param flags: flags for recv() function
* @param session: Session pointer, session of client connect to server
* @retruns bytes received of last message
*/
int Receive(SOCKET s, char *buff, int flags) {
	char recvBuff[BUFF_SIZE], tmpBuff[BUFF_SIZE] = "";
	char clientIP[INET_ADDRSTRLEN];
	int res, clientPort, i = 0, n = 0;

	Message message;

	// receive message
	while (true)
	{
		res = recv(s, recvBuff, BUFF_SIZE, flags);
		if (res <= 0) {
			printf("Error %d: Cannot receive data from server\n", WSAGetLastError());
			break;
		}
		else {
			recvBuff[res] = 0;
			//printf("Receive from server: %s\n", recvBuff);
			strcat_s(tmpBuff, recvBuff);

			// get message to get length of message
			if (i == 0) {
				message = decodeMessage(recvBuff);
				n += message.payload.length();
			}
			else {
				n += res;
			}

			// check bytes received
			if (n >= message.length) {
				memcpy(buff, tmpBuff, BUFF_SIZE);
				break;
			}
		}
		i++;
	}

	return res;
}

/**
* Send message of SOCKET
* @param s: SOCKET send message
* @param buff: char pointer to contain message send
* @param flags: flags for send() function
* @retruns bytes sent of last message
*/
int Send(SOCKET s, char *buff, int flags) {
	int msglen, nLeft, idx;
	int res;

	msglen = strlen(buff);
	nLeft = msglen;
	idx = 0;

	while (nLeft > 0)
	{
		res = send(s, buff + idx, nLeft, 0);
		if (res == SOCKET_ERROR) {
			printf("Error %d: Cannot send data.", WSAGetLastError());
			res = false;
			break;
		}
		nLeft -= res;
		idx += res;
	}

	return res;
}

