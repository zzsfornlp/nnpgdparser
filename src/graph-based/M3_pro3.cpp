/*
 * M3_pro3.cpp
 *
 *  Created on: Dec 28, 2015
 *      Author: zzs
 */

#include "M3_pr.h"
#include "../algorithms/Eisner.h"

//only before training (and after building dictionary)
void M3_pro3::each_create_machine()
{
	mso1 = dynamic_cast<CsnnO1*>(Csnn::read(hp->CONF_score_mach_so1));
	mso2 = dynamic_cast<CsnnO2*>(Csnn::read(hp->CONF_score_mach_so2sib));
	mach = Csnn::read(hp->CONF_score_mach_so3g);

	//enforcing labeling
	int class_num = dict->getnum_deprel();
	Process::CHECK_EQUAL(mso1->get_odim(),class_num,"Pr. so1 bad odim.");
	Process::CHECK_EQUAL(mso2->get_odim(),class_num,"Pr. so2sib bad odim.");
	Process::CHECK_EQUAL(mach->get_odim(),class_num,"Pr. so3g bad odim.");

	mso1->start_perceptron(class_num,hp->CONF_pr_initway);
	mso2->start_perceptron(class_num,hp->CONF_pr_initway);
	mach->start_perceptron(class_num,hp->CONF_pr_initway);
	//but not for fo1 !!
}

void M3_pro3::each_test_one(DependencyInstance* x,int dev)
{
	if(x->length() >= hp->CONF_higho_toolong){
		//tricky ...
		Csnn* tmp = mach;
		mach = mso2;
		Process::parse_o2sib(x,mfo1,mso1);
		mach = tmp;
	}
	else{
		Process::parse_o3g(x,mfo1,mso1,mso2);
	}
}

double M3_pro3::nn_dev_test(string to_test,string output,string gold,int dev)	//overwrite
{
	CsnnO1* old_o1 = mso1;
	CsnnO2* old_o2 = mso2;
	Csnn* old_o3 = mach;
	if(dev && hp->CONF_pr_devavrage){
		//again really tricky
		mso1->write(hp->CONF_pr_macho1);
		mso2->write(hp->CONF_pr_macho2);
		mach->write(hp->CONF_pr_macho3);
		mso1 = dynamic_cast<CsnnO1*>(Csnn::read(hp->CONF_pr_macho1));
		mso2 = dynamic_cast<CsnnO2*>(Csnn::read(hp->CONF_pr_macho2));
		mach = Csnn::read(hp->CONF_pr_macho3);
		mso1->finish_perceptron();
		mso2->finish_perceptron();
		mach->finish_perceptron();      //only for the re-read ones
	}
	//dev-
	double ret = Process::nn_dev_test(to_test,output,gold,dev);
	if(dev && hp->CONF_pr_devavrage){
		delete mso1;
		delete mso2;
		delete mach;
		mso1 = old_o1;
		mso2 = old_o2;
		mach = old_o3;
	}
	return ret;
}

void M3_pro3::each_train_one_iter()
{
	//per-sentence approach
	int num_sentences = training_corpus->size();
	//statistics
	int skip_sent_num = 0;
	//training
	time_t now;
	time(&now); //ctime is not rentrant ! use ctime_r() instead if needed
	cout << "##*** Start the o3 Perceptron training for iter " << cur_iter << " at " << ctime(&now) << endl;
	cout << "#Sentences is " << num_sentences << " and resample (about)" << num_sentences*hp->CONF_NN_resample << endl;

	vector<DependencyInstance*> xs;
	int all_token=0,all_right=0;
	for(int i=0;i<num_sentences;){
		//random skip (instead of shuffling every time)
		if(drand48() > hp->CONF_NN_resample || training_corpus->at(i)->length() >= hp->CONF_higho_toolong){
			skip_sent_num ++;
			i ++;
			continue;
		}
		//main batch
		int this_instance_toupdate = 0;
		for(;;){
			//forward
			DependencyInstance* x = training_corpus->at(i);
			xs.push_back(x);

			Process::parse_o3g(x,mfo1,mso1,mso2, true);
			// -- statistic
			all_token += x->length()-1;
			for(int i2=1;i2<x->length();i2++){	//ignore root
				if((*(x->predict_heads))[i2] == (*(x->heads))[i2])
					all_right ++;
				else
					this_instance_toupdate++;
			}
			//
			i++;

			if(i>=num_sentences)
				break;
			//out of the mini-batch
			while(training_corpus->at(i)->length() >= hp->CONF_higho_toolong){	//HAVE to compromise, bad choice
				skip_sent_num ++;
				i ++;
			}
			if(i>=num_sentences)
				break;
			if (hp->CONF_minibatch > 0) {
				if (int(xs.size()) >= hp->CONF_minibatch)
					break;
			}
			else {
				if (this_instance_toupdate >= -1 * hp->CONF_minibatch)
					break;
			}
		}
		//update
		for(int ii=0;ii<xs.size();ii++){
			DependencyInstance* x = xs[ii];
			nn_input* good_o1,* good_o2,* good_o3;
			nn_input* bad_o1, * bad_o2,* bad_o3;
			get_nninput_o1(x,&good_o1,&bad_o1,dict);
			get_nninput_o2sib(x,&good_o2,&bad_o2,dict);
			get_nninput_o3g(x,&good_o3,&bad_o3,dict);
			mso1->update_pr(good_o1,bad_o1,hp->CONF_pr_alpha,hp->CONF_NN_WD);
			mso2->update_pr(good_o2,bad_o2,hp->CONF_pr_alpha,hp->CONF_NN_WD);
			mach->update_pr(good_o3,bad_o3,hp->CONF_pr_alpha,hp->CONF_NN_WD);
			delete good_o1;delete bad_o1;
			delete good_o2;delete bad_o2;
			delete good_o3;delete bad_o3;
		}
		xs.clear();
		mso1->update_pr_adding();
		mso2->update_pr_adding();
		mach->update_pr_adding();
	}
	cout << "Iter done, skip " << skip_sent_num << " sentences." << "AND training UAS:"
			<< all_right << "/" << all_token << "=" << all_right/(0.0+all_token) << endl;

}

void M3_pro3::train()
{
	Process::train();
	//averaging and saving
	cout << "--//don't care about the nn.mach.* files ..." << endl;
	mso1->finish_perceptron();
	mso2->finish_perceptron();
	mach->finish_perceptron();
	mso1->write(hp->CONF_pr_macho1);
	mso2->write(hp->CONF_pr_macho2);
	mach->write(hp->CONF_pr_macho3);
	cout << "--OK, final averaging and saving." << endl;
}

void M3_pro3::test(string x)
{
	cout << "!! Warning, the machine name should be specified in conf, no-use of cmd mach name." << endl;
	mso1 = dynamic_cast<CsnnO1*>(Csnn::read(hp->CONF_pr_macho1));
	mso2 = dynamic_cast<CsnnO2*>(Csnn::read(hp->CONF_pr_macho2));
	//<not here>mach = Csnn::read(hp->CONF_pr_macho2);
	Process::test(hp->CONF_pr_macho3);
}
