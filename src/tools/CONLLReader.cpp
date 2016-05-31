#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#include"DependencyInstance.h"
#include"CONLLReader.h"
#include"Util.h"
#include<boost/regex.hpp>
using namespace std;
using namespace boost;

CONLLReader::CONLLReader(){
	inputReader = NULL;
}
void CONLLReader::startReading(const char* file){
	inputReader = fopen(file, "r");
}
void CONLLReader::finishReading(){
	fclose(inputReader);
}

DependencyInstance* CONLLReader::getNext(){
	vector<vector<string*>*>* lineList = new vector<vector<string*>*>();
	char line[10000];
	string* str;
	fgets(line, 10000, inputReader);
	int str_len = (int)(strlen(line));
	line[str_len - 1] = '\0';
	while(!feof(inputReader) && !(str_len == 1) && !(line[0] == '*')){
		str = new string(line);
		vector<string*>* aaa = Util::split(str, '\t');
		lineList->push_back(aaa);
		delete (str);
		fgets(line, 10000, inputReader);
		str_len = (int)(strlen(line));
		line[str_len - 1] = '\0';
	}
	int length = (int)lineList->size();

	if(length == 0){
		delete(lineList);
		return NULL;
	}
	
	vector<string*>* forms = new vector<string*>(length + 1);
	vector<string*>* pos = new vector<string*>(length + 1);
	vector<int>* heads = new vector<int>(length + 1);
	vector<string*>* deprel = new vector<string*>(length + 1);

	/* add root */
	string tmp = string("<root>");
	(*forms)[0] = new string(tmp);
	tmp = string("<root-POS>");
	(*pos)[0] = new string(tmp);
	(*heads)[0] = -1;
	(*deprel)[0] = 0;	//NULL
	
	for(int i = 0; i < length; i++){
		//the new form
		vector<string*>* info = (*lineList)[i];
		(*forms)[i + 1] = normalize((*info)[1]);
		//identify pos away ...
		(*pos)[i + 1] = new string(*(*info)[4]);
		(*heads)[i + 1] = Util::stringToInt((*info)[8]);
		(*deprel)[i + 1] = new string(*(*info)[9]);
		vector<string*>::iterator iter;
		for(iter = info->begin(); iter != info->end(); iter++){
			//cout<<(*iter)->c_str()<<" ";
			delete (*iter);
		}
		//cout<<endl;
		delete(info);
	}
	delete(lineList);
	return new DependencyInstance(forms,pos,deprel,heads);
}

string* CONLLReader::normalize(string* s){
	//number or float or \/
	regex expression("[0-9]+|[0-9]+\\.[0-9]+|[0-9]+[0-9,]+|[0-9]+\\\\/[0-9]+");
	cmatch what;
	string tmp;
	if(regex_match(s->c_str(), what, expression)){
		tmp = string("<num>");
		return new string(tmp);
	}
	else{
		tmp = string(*s);
		return new string(tmp);
	}
}
