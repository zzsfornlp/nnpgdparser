/*
 * Dictionary.cpp
 *
 *  Created on: Oct 8, 2015
 *      Author: zzs
 */

#include "Dictionary.h"

string Dictionary::POS_START = "<pos-s>";
string Dictionary::POS_END = "<pos-e>";
string Dictionary::POS_UNK = "<pos-unk>";
string Dictionary::WORD_START = "<w-s>";
string Dictionary::WORD_END = "<w-e>";
string Dictionary::WORD_UNK = "<w-unk>";

string Dictionary::WORD_DUMMY_L = "<w-dl>";
string Dictionary::WORD_DUMMY_R = "<w-dr>";
string Dictionary::WORD_ROOTG = "<w-rg>";
string Dictionary::POS_DUMMY_L = "<pos-dl>";
string Dictionary::POS_DUMMY_R = "<pos-dr>";
string Dictionary::POS_ROOTG = "<pos-rg>";

//helper for special info
nn_input_helper* Dictionary::common_helper = 0;
nn_input_helper* Dictionary::get_helper(){
	if(common_helper == 0){
		common_helper = new nn_input_helper();
		common_helper->start_word = get_index_word(&WORD_START);
		common_helper->end_word = get_index_word(&WORD_END);
		common_helper->start_pos = get_index_pos(&POS_START);
		common_helper->end_pos = get_index_pos(&POS_END);

		common_helper->dl_word = get_index_word(&WORD_DUMMY_L);
		common_helper->dr_word = get_index_word(&WORD_DUMMY_R);
		common_helper->rg_word = get_index_word(&WORD_ROOTG);
		common_helper->dl_pos = get_index_pos(&POS_DUMMY_L);
		common_helper->dr_pos = get_index_pos(&POS_DUMMY_R);
		common_helper->rg_pos = get_index_pos(&POS_ROOTG);
	}
	return common_helper;
}

//main construction process
void Dictionary::construct_dictionary(vector<DependencyInstance*>* corpus,void* construct_info)
{
	printf("-Start to build dictionary\n");
	int count = 0;
	//1.pos
	count = 0;
	map_pos->insert(pair<string*, int>(&POS_START,count++));
	map_pos->insert(pair<string*, int>(&POS_END,count++));
	map_pos->insert(pair<string*, int>(&POS_UNK,count++));
	map_pos->insert(pair<string*, int>(&POS_DUMMY_L,count++));
	map_pos->insert(pair<string*, int>(&POS_DUMMY_R,count++));
	map_pos->insert(pair<string*, int>(&POS_ROOTG,count++));
	for(int i=0;i<corpus->size();i++){
		DependencyInstance* one = corpus->at(i);
		vector<string*>* one_pos = one->postags;
		int sen_length = one_pos->size();
		for(int j=0;j<sen_length;j++){
			string* to_find = one_pos->at(j);
			HashMap::iterator iter = map_pos->find(to_find);
			if(iter == map_pos->end())
				map_pos->insert(pair<string*, int>(to_find,count++));
		}
	}
	//2.deprel
	count = 0;
	for(int i=0;i<corpus->size();i++){
		DependencyInstance* one = corpus->at(i);
		vector<string*>* one_rel = one->deprels;
		int sen_length = one_rel->size();
		for(int j=1;j<sen_length;j++){	//!!here no root
			string* to_find = one_rel->at(j);
			HashMap::iterator iter = map_deprel->find(to_find);
			if(iter == map_deprel->end()){
				map_deprel->insert(pair<string*, int>(to_find,count++));
				list_deprel->push_back(to_find);
			}
		}
	}
	//3.words
	//3.1: the frequency
	HashMap word_freq(CONS_size_word);
	for(int i=0;i<corpus->size();i++){
		DependencyInstance* one = corpus->at(i);
		vector<string*>* one_word = one->forms;
		int sen_length = one_word->size();
		for(int j=0;j<sen_length;j++){
			string* to_find = one_word->at(j);
			HashMap::iterator iter = word_freq.find(to_find);
			if(iter == word_freq.end())
				word_freq.insert(pair<string*, int>(to_find,1));
			else
				word_freq.at(to_find) ++;
		}
	}
	//3.2:construct
	count = 0;
	map_word->insert(pair<string*, int>(&WORD_START,count++));
	map_word->insert(pair<string*, int>(&WORD_END,count++));
	map_word->insert(pair<string*, int>(&WORD_UNK,count++));
	map_word->insert(pair<string*, int>(&WORD_DUMMY_L,count++));
	map_word->insert(pair<string*, int>(&WORD_DUMMY_R,count++));
	map_word->insert(pair<string*, int>(&WORD_ROOTG,count++));
	for(HashMap::iterator iter=word_freq.begin();iter!=word_freq.end();iter++){
		if(iter->second < remove_single){}
		else
			map_word->insert(pair<string*, int>(iter->first,count++));
	}

	printf("---Cut words from %d to %d.\n",word_freq.size(),getnum_word());
	printf("--Final finish dictionary building, words %d,pos %d,deprel %d.\n",
			getnum_word(),getnum_pos(),getnum_deprel());
}

//io
void Dictionary::write(string file)
{
	printf("-Writing dict to %s.\n",file.c_str());
	ofstream fout;
	fout.open(file.c_str(),ofstream::out);
	vector<HashMap*> the_list;
	the_list.push_back(map_pos);the_list.push_back(map_deprel);the_list.push_back(map_word);
	//write out
	for(int i=0;i<the_list.size();i++){
		HashMap* the_map = the_list[i];
		fout << the_list[i]->size() << "\n";
		for(HashMap::iterator i = the_map->begin();i!=the_map->end();i++){
			fout << *(i->first) << "\t" << i->second << "\n";
		}
	}
	fout.close();
	printf("-Writing finished.\n");
}
Dictionary::Dictionary(string file)
{
	remove_single = -1;	//no need
	map_word = new HashMap(CONS_size_word);
	map_pos = new HashMap(CONS_size_pos);
	map_deprel = new HashMap(CONS_size_rel);
	list_deprel = new vector<string*>(CONS_size_rel,(string*)0);
	printf("-Reading dict from %s.\n",file.c_str());
	ifstream fin;
	fin.open(file.c_str(),ifstream::in);
	vector<HashMap*> the_list;
	the_list.push_back(map_pos);the_list.push_back(map_deprel);the_list.push_back(map_word);
	//read in
	for(int i=0;i<the_list.size();i++){
		HashMap* the_map = the_list[i];
		int the_size;
		fin >> the_size;
		for(int i=0;i<the_size;i++){
			string tmp_str;
			int index;
			fin >> tmp_str >> index;
			the_map->insert(pair<string*, int>(new string(tmp_str),index));
		}
		//check
		if(the_map->size() != the_size){
			cerr << "!!! Error with the dictionary file." << endl;
		}
	}
	//build list_deprel
	for(HashMap::iterator i = map_deprel->begin();i!=map_deprel->end();i++){
		list_deprel->at(i->second) = i->first;
	}
	fin.close();
	printf("--Final reading dictionary, words %d,pos %d,deprel %d.\n",
			getnum_word(),getnum_pos(),getnum_deprel());
}

//indexing
int Dictionary::get_index_word(string* s){
	HashMap::iterator iter = map_word->find(s);
	if(iter == map_word->end())
		return map_word->find(&WORD_UNK)->second;
	else
		return iter->second;
}
int Dictionary::get_index_pos(string* s){
	HashMap::iterator iter = map_pos->find(s);
	if(iter == map_pos->end())
		return map_pos->find(&POS_UNK)->second;
	else
		return iter->second;
}
int Dictionary::get_index_deprel(string* s){
	HashMap::iterator iter = map_deprel->find(s);	//must be there
	return iter->second;
}
string* Dictionary::get_str_deprel(int index){
	return list_deprel->at(index);
}

//deal with corpus
//use the hash maps to get the three indexes
void Dictionary::prepare_corpus(vector<DependencyInstance*>* corpus,int testing)
{
	for(vector<DependencyInstance*>::iterator i=corpus->begin();i!=corpus->end();i++){
		DependencyInstance* inst = *i;
		int len = inst->length();
		inst->index_forms = new vector<int>(len);
		inst->index_pos = new vector<int>(len);
		inst->index_deprels = new vector<int>(len);
		for(int t=0;t<len;t++){
			inst->index_forms->at(t) = get_index_word(inst->forms->at(t));
			inst->index_pos->at(t) = get_index_pos(inst->postags->at(t));	//!!DEBUG should be index_pos
			if(t!=0 && !testing)	//no need when testing
				inst->index_deprels->at(t) = get_index_deprel(inst->deprels->at(t));
		}
	}
}
//get deprel-strs, only before output (CONLLWriter)
void Dictionary::prepare_deprel_str(vector<DependencyInstance*>* corpus)
{
	for(vector<DependencyInstance*>::iterator i=corpus->begin();i!=corpus->end();i++){
		DependencyInstance* inst = *i;
		int len = inst->length();
		inst->predict_deprels_str = new vector<string*>(len);
		for(int t=1;t<len;t++){	//!!here no root
			inst->predict_deprels_str->at(t) = get_str_deprel(inst->predict_deprels->at(t));
		}
	}
}
