#include"Util.h"
#include<iostream>
#include"stdio.h"
using namespace std;

vector<int>* Util::stringsToints(vector<string*> &stringreps){
	vector<int>* v=new vector<int>();
	int length = (int)(stringreps.size());
	for(int i = 0; i < length; i++){
		string* a = stringreps[i];
		int num=0;
		int len = (int)(a->length());
		for(int j = 0; j < len; j++){
			num = num * 10 + (int)((*a)[j] - '0');
		}
		v->push_back(num);
	}
	return v;
}

string* Util::intToString(const int n){
	int b;
	int a = n;
	if(a == 0){
		string tmp = "0";
		return new string(tmp);
	}

	bool negative = false;
	if(a < 0){
		negative = true;
		a = -a;
	}
	string* r = new string();
	while(a > 0){
		b = a % 10;
		a = a / 10;
		string tmp(1, (char)(b + '0'));
		r->insert(0, tmp);
	}
	if(negative){
		r->insert(0, string(1, '-'));
	}
	return r;
}

string* Util::join(vector<string*> &a, char sep){
	string* r = new string();
	r->append(*(a[0]));
	int size = (int)(a.size());
	for(int i = 1;i < size; i++){
		(*r) += sep;
		r->append(*(a[i]));
	}
	return r;
}

string* Util::join(vector<int> &a, char sep){
	string* r = new string();
	string* tmp = Util::intToString(a[0]);
	r->append(*tmp);
	delete(tmp);
	int size = (int)(a.size());
	for(int i = 1; i < size; i++){
		(*r) += sep;
		tmp = Util::intToString(a[i]);
		r->append(*tmp);
		delete(tmp);
	}
	return  r;
}

vector<string*>* Util::split(std::string* str, char c){
	vector<string*>* vstring = new vector<string*>();
	string* tmp = new string();
	int len = (int)(str->length());
	for(int i = 0; i < len; i++){
		while(i < len && (*str)[i] != c){
			(*tmp) += (*str)[i];
			i++;
		}
		vstring->push_back(tmp);
		tmp = new string();
	}
	delete(tmp);
	return vstring;
}

int Util::stringToInt(std::string *str){
	int len = (int)(str->length());
	int result = 0;
	for(int i = 0; i < len; i++){
		result = result * 10 + ((*str)[i] - '0');
	}
	return result;
}

double Util::stringToDouble(std::string *str){
	double result;
	sscanf(str->c_str(), "%lf", &result);
	return result;
}

string* Util::trim(std::string *str){
	string::iterator iter1;
	iter1 = str->begin();
	while(iter1 != str->end() && (*iter1 == ' ' || *iter1 == '\n' || *iter1 == '\t')){
		++iter1;
	}
	if(iter1 == str->end()){
		return new string();
	}
	string::iterator iter2;
	iter2 = (str->end()) - 1;
	while(iter2 != iter1 && (*iter2 == ' ' || *iter2 == '\n' || *iter2 == '\t')){
		--iter2;
	}
	string tmp = string(iter1, iter2 + 1);
	return new string(tmp);
}

vector<string*>* Util::split(std::string *str, std::string *ss){
	string::size_type begin = 0;
	string::size_type end = 0;
	string::size_type len_ss = (ss->length());
	vector<string*>* result = new vector<string*>();
	string::size_type len_str = (str->length());
	while(begin != len_str){
		end = str->find(*ss, begin);
		string tmp = string(str->substr(begin, end - begin));
		string* s = new string(tmp);
		result->push_back(s);
		begin = end + len_ss;
	}
	return result;
}


