#ifndef HashMap_H
#define HashMap_H
#include<string>
#include<boost/unordered_map.hpp>
#include<boost/functional/hash.hpp>
using namespace std;
using namespace boost;

class str_hash{
public:
	size_t operator()(const string* key)const{
		boost::hash<string> hash_str;
		return hash_str(*key);
	}
};

class str_equals{
public:
	bool operator()(const string* s1, const string* s2)const{
		return (*s1) == (*s2);
	}
};

typedef boost::unordered_map<string*, int, str_hash, str_equals> HashMap;
//typedef boost::unordered_map<int, int> IntHashMap;

#endif
