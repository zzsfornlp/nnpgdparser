#ifndef _Util_H
#define _Util_H
#include<vector>
#include<string>
using namespace std;
class Util{
public:
	static vector<int>* stringsToints(vector<string*> &stringreps);
	static string* join(vector<string*> &a, char sep);
	static string* join(vector<int> &a, char sep);
	static string* intToString(const int n);
	static int stringToInt(string* str);
	static double stringToDouble(string *str);
	static vector<string*>* split(string* str, char c);
	static vector<string*>* split(string* str, string* ss);
	static string* trim(string* str);
};
#endif
