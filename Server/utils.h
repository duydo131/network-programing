#pragma once

#include "iostream"
#include "vector"
#include "time.h"

using namespace std;

/**
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

/**
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

/**	Write File
input :
char* filename
char[] data to write
*/
void log(string fileLog, char data[]) {
	ofstream ofs(fileLog, ofstream::app);

	ofs << data << endl;
}

/**	Get pre_log include clientIP and clientPort
input :
char[] clientIP
int clientPort
output : char* valid form
*/
char* get_pre_log(char clientIP[], int socket) {
	int size = 22;
	char* pre_log = (char*)malloc(size);
	sprintf_s(pre_log, size, "%s:%d", clientIP, socket);
	return pre_log;
}

/**	Change time to valid format
input : time_t
output : char* valid form [dd/mm/yyyy hh:mm:ss]
*/
char* get_time_request(time_t t) {
	int size = 22;
	char* time_request = (char*)malloc(size);
	tm ltm;
	localtime_s(&ltm, &t);
	sprintf_s(time_request, size, "[%02d/%02d/%04d %02d:%02d:%02d]",
		ltm.tm_mday, ltm.tm_mon + 1, ltm.tm_year + 1900,
		ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
	return time_request;
}

/**
* @Warning : Function no throw exception, string time input is supposed to be correct
* Convert string to time_t
* Form time string : yyyyMMddHHmmss
* @Param : string timestamp is formated
* @Return time_t
*/
time_t to_time_t(const string& timestamp) // throws on bad timestamp
{
	tm tm{};
	tm.tm_year = stoi(timestamp.substr(0, 4)) - 1900;
	tm.tm_mon = stoi(timestamp.substr(4, 2)) - 1;
	tm.tm_mday = stoi(timestamp.substr(6, 2));
	tm.tm_hour = stoi(timestamp.substr(8, 2));
	tm.tm_min = stoi(timestamp.substr(10, 2));
	tm.tm_sec = stoi(timestamp.substr(12, 2));

	return mktime(addressof(tm));
}

/**
* Compare 2 time_t
* @Param 2 time_t
* @Return 1 if time1 > time2 else -1
*/
int comparetime(time_t time1, time_t time2) {
	return difftime(time1, time2) > 0.0 ? 1 : -1;
}

/**
* Check room started
* @Param Room room : room need check
* @ Return 
*		True : not start
*		False : started
*/
bool check_status_room(Room room) {
	time_t current_time = time(0);
	if (comparetime(to_time_t(room.start_time), current_time) == -1) return true;
	else return false;
}

int inline random_index(int max) {
	return rand() % max;
}

int random_range(int min, int max) {
	if (max <= min) return max;
	return rand() % (max - min) + min;
}

/**
* convert IP to format string to generate id_room
* @Param char* IP : IP client
* @ Return char*
*/
char* format_client_IP(char* clientIP) {
	int length = strlen(clientIP);
	char* out = (char*)malloc(length + 1);
	for (int i = 0; i < length; i++) {
		if (*(clientIP + i) == '.') out[i] = random_index(9) + 48;
		else out[i] = *(clientIP + i);
	}
	out[length] = 0;
	return out;
}

/**
* Random question for room
* @Param 
*		int number_of_question
*		int size : size of all question in database
* @ Return : vector<int> : array index question
*/
vector<int> random_question(int number_of_question, int size) {
	vector<int> rs;
	list<int> temp;
	int length_of_all_question = size;
	for (int i = 0; i < length_of_all_question; i++) {
		temp.push_back(i);
	}
	int index;
	list<int>::iterator it;
	for (int i = 0; i < number_of_question; i++) {
		index = random_index(temp.size());
		it = temp.begin();
		advance(it, index);
		rs.push_back(*it);
		temp.erase(it);
	}
	return rs;
}

/**
*Function check result from user
*@Param string : result from user
*@return true if result is one of ['A', 'B', 'C', 'D']
*			  else false
*/
bool validate_result(vector<string> rs) {
	for (string r : rs) {
		if (r != "A" && r != "B" && r != "C" && r != "D") return false;
	}
	return true;
}