/*
 * Dictionary.h
 *
 *  Created on: Oct 8, 2015
 *      Author: zzs
 */

#ifndef PARTS_DICTIONARY_H_
#define PARTS_DICTIONARY_H_

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cctype>
#include <cstring>
#include <string>
#include <algorithm>
#include <fstream>
#include "HashMap.h"
#include "../tools/DependencyInstance.h"
#include "../csnn/nn_input_helper.h"
using namespace std;

//the dictionary
// -- for words,pos and deprels
class Dictionary{
private:
	const static int CONS_size_word = 100000;
	const static int CONS_size_pos = 1000;
	const static int CONS_size_rel = 1000;

	HashMap* map_word;
	HashMap* map_pos;
	HashMap* map_deprel;
	vector<string*>* list_deprel;

	//used only when building --- only for words
	int remove_single;	//remove rare word; survive only if > remove_single
	void construct_dictionary(vector<DependencyInstance*>*,void* construct_info=0);

public:
	static string POS_START,POS_END,POS_UNK;
	static string WORD_START,WORD_END,WORD_UNK;
	static string WORD_DUMMY_L,WORD_DUMMY_R,WORD_ROOTG;
	static string POS_DUMMY_L,POS_DUMMY_R,POS_ROOTG;

	int getnum_word(){return map_word->size();}
	int getnum_pos(){return map_pos->size();}
	int getnum_deprel(){return map_deprel->size();}
	HashMap *get_wordmap(){return map_word;}

	int get_index_word(string*);
	int get_index_pos(string*);
	int get_index_deprel(string*);
	string* get_str_deprel(int);

	void prepare_corpus(vector<DependencyInstance*>* corpus,int testing=0);
	void prepare_deprel_str(vector<DependencyInstance*>* corpus);

	Dictionary(vector<DependencyInstance*>* corpus,int remove,void* construct_info=0){
		map_word = new HashMap(CONS_size_word);
		map_pos = new HashMap(CONS_size_pos);
		map_deprel = new HashMap(CONS_size_rel);
		list_deprel = new vector<string*>();
		remove_single = remove;
		construct_dictionary(corpus,construct_info);
	}
	//io
	Dictionary(string file);
	void write(string file);
	//the helper
	static nn_input_helper* common_helper;
	nn_input_helper* get_helper();
};


#endif /* PARTS_DICTIONARY_H_ */
