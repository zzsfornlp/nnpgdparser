/*
 * training_prepare.cpp
 *
 *  Created on: Dec 19, 2014
 *      Author: zzs
 */
#include <iostream>
#include <vector>
#include <string>
#include "CONLLReader.h"
#include "CONLLWriter.h"
#include "DependencyInstance.h"
using namespace std;

//input
vector<DependencyInstance*>* read_corpus(string file)
{
	vector<DependencyInstance*>* ret = new vector<DependencyInstance*>();
	CONLLReader* reader = new CONLLReader();
	reader->startReading(file.c_str());
	DependencyInstance* x = reader->getNext();
	while(x != NULL){
		ret->push_back(x);
		x = reader->getNext();
	}
	reader->finishReading();
	delete reader;
	return ret;
}

//output
void write_corpus(vector<DependencyInstance*>* instances,string file)
{
	CONLLWriter* writer = new CONLLWriter();
	writer->startWriting(file.c_str());
	for(int i=0;i<instances->size();i++)
		writer->write(instances->at(i));
	writer->finishWriting();
	delete writer;
}

