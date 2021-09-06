#pragma once
#include "iostream"
#include "vector"

using namespace std;

/**
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

/**
*Function check result from user
*@Param string : result from user
*@return true if result is one of ['A', 'B', 'C', 'D']
*			  else false
*/
bool validate_result(string r) {
	return r == "A" || r == "B" || r == "C" || r == "D";
}

/**
*Function check user submit
*@Param string : input from user
*@return True input == 'X'
*		 False else
*/
bool check_submit(string r) {
	return r == "X";
}

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

/**
* @Warning : Function no throw exception, string time input is supposed to be correct
* Convert string to time_t
* Form time string : yyyyMMddHHmmss
* @Param : string timestamp is formated
* @Return time_t
*/
time_t to_time_t(const string& timestamp) {
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
* Check string time input is valid and after current
* @Param string t : time string is formated
* @Return true if input is valid 
*else false
*/
bool check_start_time(string t) {
	string const timestamp = t;
	time_t start_time = to_time_t(timestamp);
	time_t now = time(0);
	int i = comparetime(now, start_time);
	return i < 0;
}
/**
* Check leap year
* @Param : int year
* @Return : True if year is leap
			False else
*/
bool is_leap(int year) {
	return (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0));
}

/**
* Check input time is valid date
* @Param 
*		d : day
*		m : month
*		y : year
* @Return 
*		True : valid date
*		False : invalid date
*/
bool is_valid_date(int d, int m, int y) {
	if (y > 9999 || y < 1800) return false;
	if (m < 1 || m > 12) return false;
	if (d < 1 || d > 31) return false;
	if (m == 2) return (is_leap(y)) ? (d <= 29) : (d <= 28);
	if (m == 4 || m == 6 || m == 9 || m == 11) return d <= 30;
	return true;
}

/**
* Get integer from user
* @Param :
*		string description : description for interger input
*		int min : minimum of interger (default 0)
*		int max : maximum of interger (default INT_MAX)
* @Return integer 
*/
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
		catch (int i) {
			cout << description + " not valid!!!\n";
			cout << "Press key to continue!!\n";
			_getch();
			system("CLS");
		}
		catch (exception ex) {
			cout << description + " not valid!!!\n";
			cout << "Press key to continue!!\n";
			_getch();
			system("CLS");
		}
	}
}

/**
* convert int to form string
* Example: 11 -> "11", 1 -> "01"
* @Param : int i : integer need convert 
* @Return string
*/
string format_form_date(int i) {
	string out = "00" + to_string(i);
	return out.substr(out.length() - 2, 2);
}

/**
* Convert time_t to string
* Format : "yyyyMMddHHmmss"
* @Param : time_t time : time need convert
* @Return : string
*/
string get_time(time_t time) {
	struct tm ltm;
	localtime_s(&ltm, &time);
	string out = "";
	out += to_string(1900 + ltm.tm_year);
	out += format_form_date(1 + ltm.tm_mon);
	out += format_form_date(ltm.tm_mday);
	out += format_form_date(ltm.tm_hour);
	out += format_form_date(ltm.tm_min);
	out += format_form_date(ltm.tm_sec);
	return out;
}

/**
* Get time from user, have check input
* @No Param
* @Return : string (time string is formated)
*/
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
			if (!check_start_time(out)) throw 1;
			return out;
		}
		catch (int ex) {
			cout << "Invalid Date!!!\n";
			cout << "Press key to continue!!\n";
			_getch();
			system("CLS");
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

void show_rules(int number_of_questions, int length_time, string room_id = "") {
	system("CLS");
	if (room_id != "") cout << "Room ID: " << room_id << endl;
	else cout << "Mode Pracrice" << endl;
	cout << "Number of question : " << number_of_questions << endl;
	cout << "Length time : " << length_time << "s" << endl;
	cout << endl << "Choose answer A, B, C, D.\nChoose X if you want submit" << endl;
	cout << "Are you ready!!\n";
	_getch();
	system("CLS");
}