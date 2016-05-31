/*
 * Process_train.cpp
 *
 *  Created on: Oct 10, 2015
 *      Author: zzs
 */

#include "Process.h"

//--------------------------------TRAIN-----------------------//
//init for training --- right after construction, but here use some virtual functions
void Process::nn_train_prepare()
{
	//2. get training corpus --- configured for training
	cout << "2.read-corpus:" << endl;
	training_corpus = read_corpus(hp->CONF_train_file);
	dev_test_corpus = 0;
	//3. get dictionary and write
	if(hp->CONF_traindict_file.length() > 0){
		cout << "3.get dict from " << hp->CONF_traindict_file << endl;
		dict = new Dictionary(hp->CONF_traindict_file);
		dict->write(hp->CONF_dict_file);
	}
	else{
		cout << "3.get dict from scratch:" << endl;
		dict = new Dictionary(training_corpus,hp->CONF_dict_remove);
		dict->write(hp->CONF_dict_file);
	}
	dict->prepare_corpus(training_corpus);	//get those indexes
	//4.create machine
	cout << "4.get mach from scratch:" << endl;
	each_create_machine();		/*************VIRTUAL************/
	init_embed();	//possible init
	cout << "- Prepare over..." << endl;
}

void Process::train()
{
	nn_train_prepare();
	//5. main training
	cout << "5.start training: " << endl;
	lrate_cut_times = lrate_force_cut_times = 0;
	last_cut_iter = -1;
	cur_iter = 0;
	cur_lrate = hp->CONF_NN_LRATE;
	dev_results = new double[hp->CONF_NN_ITER*2];	//maybe large than iter, but x2 is enough
	double best_result = 0;
	int best_iter = -1;
	string mach_cur_name = hp->CONF_mach_name+hp->CONF_mach_cur_suffix;
	string mach_best_name = hp->CONF_mach_name+hp->CONF_mach_best_suffix;
	for(int i=cur_iter;i<hp->CONF_NN_ITER || whether_keep_trainning();i++){	//at least NN_ITER iter
		if(cur_iter==hp->CONF_NN_FIXVEC){
			cout << "!! starting to fix tabs from now." << endl;
			mach->noupdate_tab();
		}
		if(cur_iter==hp->CONF_NN_untied_changetoiter){
			cout << "!! change untied to " << hp->CONF_NN_untied_changeto << endl;
			mach->change_untiedway(hp->CONF_NN_untied_changeto);
		}
		each_train_one_iter();
		cout << "-- Iter done, waiting for test dev:" << endl;
		//write curr mach -- before ...
		mach->write(mach_cur_name);
		double this_result = nn_dev_test(hp->CONF_dev_file,hp->CONF_output_file+".dev",hp->CONF_dev_file,1);
		dev_results[cur_iter] = this_result;
		//write curr mach
		mach->write(mach_cur_name);
		//possible write best mach --- and choose the later one
		if(this_result >= best_result){
			cout << "-- get better result, write to " << mach_best_name << endl;
			best_result = this_result;
			best_iter = cur_iter;
			mach->write(mach_best_name);
		}
		//lrate schedule
		set_lrate_one_iter();
		cur_iter++;
	}
	//6.results
	cout << "6.training finished with dev results: best " << best_result << "|" << best_iter << endl;
	cout << "zzzzz ";
	for(int i=0;i<cur_iter;i++)
		cout << dev_results[i] << " ";
	cout << endl;
}

double Process::nn_dev_test(string to_test,string output,string gold,int dev)
{
	time_t now;
	//also assuming test-file itself is gold file(this must be true with dev file)
	dev_test_corpus = read_corpus(to_test);
	dict->prepare_corpus(dev_test_corpus,1);	//get those indexes
	int token_num = 0;	//token number
	int miss_count = 0;
	time(&now);
	cout << "#--Test at " << ctime(&now) << std::flush;
	for(unsigned int i=0;i<dev_test_corpus->size();i++){
		DependencyInstance* t = dev_test_corpus->at(i);
		int length = t->forms->size();
		token_num += length - 1;
		each_test_one(t,dev);		/*************virtual****************/
		for(int i2=1;i2<length;i2++){	//ignore root
			if((*(t->predict_heads))[i2] != (*(t->heads))[i2])
				miss_count ++;
		}
	}
	time(&now);
	cout << "#--Finish at " << ctime(&now) << std::flush;
	dict->prepare_deprel_str(dev_test_corpus);	//get deprel's strings
	write_corpus(dev_test_corpus,output);
	string ttt;
	double rate = (double)(token_num-miss_count) / token_num;
	cout << "Evaluate:" << (token_num-miss_count) << "/" << token_num
			<< "(" << rate << ")" << endl;
	DependencyEvaluator::evaluate(gold,output,ttt,hp->CONF_labeled);

	//clear
	for(unsigned int i=0;i<dev_test_corpus->size();i++){
		delete dev_test_corpus->at(i);
	}
	delete dev_test_corpus;
	return rate;
}

void Process::test(string m_name)
{
	cout << "----- Testing -----" << endl;
	dict = new Dictionary(hp->CONF_dict_file);
	mach = Csnn::read(m_name);
	nn_dev_test(hp->CONF_test_file,hp->CONF_output_file,hp->CONF_gold_file,0);
}
