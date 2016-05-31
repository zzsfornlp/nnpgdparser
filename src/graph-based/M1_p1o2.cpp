/*
 * M1_p1o2.cpp
 *
 *  Created on: Nov 7, 2015
 *      Author: zzs
 */

#include "M1_p1.h"
#include "../algorithms/Eisner.h"

//only before training (and after building dictionary)
void M1_p1o2::each_create_machine()
{
	//several need setting places
	hp->hp_nn.NN_wnum = dict->getnum_word();
	hp->hp_nn.NN_pnum = dict->getnum_pos();
	hp->hp_nn.NN_dnum = dict->get_helper()->get_distance_num();
	hp->hp_nn.NN_sdnum = dict->get_helper()->get_sd_num();
	hp->hp_nn.NN_out_prob = 1;
	if(hp->CONF_labeled)
		hp->hp_nn.NN_out_size = dict->getnum_deprel()+1;
	else
		hp->hp_nn.NN_out_size = 2;
	//create the new mach
	mach = Csnn::create(2,&hp->hp_nn);	//order 2 mach
}

void M1_p1o2::each_test_one(DependencyInstance* x,int dev)
{
	int noc_dev = dev ? hp->CONF_score_noc_dev : 0;
	if(noc_dev)
		Process::parse_o2sib(x,mfo1,0);
	else
		Process::parse_o2sib(x,mfo1,mso1);
}

void M1_p1o2::each_train_one_iter()
{
	static bool** STA_noprobs = 0;	//static ine, init only once
	if(STA_noprobs==0 && !filter_read(STA_noprobs)){
		//init only once
		int all_tokens_train=0,all_token_filter_wrong=0;
		time_t now;
		time(&now);
		cout << "-Preparing no_probs at " << ctime(&now) << endl;
		STA_noprobs = new bool*[training_corpus->size()];
		for(unsigned int i=0;i<training_corpus->size();i++){
			DependencyInstance* x = training_corpus->at(i);
			STA_noprobs[i] = get_cut_o1(x,mfo1,dict,hp->CONF_score_o1filter_cut);
			all_tokens_train += x->length()-1;
			for(int m=1;m<x->length();m++)
				if(STA_noprobs[i][get_index2(x->length(),x->heads->at(m),m)])
					all_token_filter_wrong ++;
		}
		cout << "For o1 filter: all " << all_tokens_train << ";filter wrong " << all_token_filter_wrong << endl;
		filter_write(STA_noprobs);
	}

	//per-sentence approach
	int num_sentences = training_corpus->size();
	//statistics
	int skip_sent_num = 0;
	int all_forward_instance = 0;
	int all_inst_right = 0;
	int all_inst_wrong = 0;
	//some useful info
	int odim = mach->get_odim();
	//training
	time_t now;
	time(&now); //ctime is not rentrant ! use ctime_r() instead if needed
	cout << "##*** Start the p1o2 training for iter " << cur_iter << " at " << ctime(&now)
			<< "with lrate " << cur_lrate << endl;
	cout << "#Sentences is " << num_sentences << " and resample (about)" << num_sentences*hp->CONF_NN_resample << endl;
	for(int i=0;i<num_sentences;){
		//random skip (instead of shuffling every time)
		if(drand48() > hp->CONF_NN_resample || training_corpus->at(i)->length() >= hp->CONF_higho_toolong){
			skip_sent_num ++;
			i ++;
			continue;
		}

		mach->prepare_batch();
		//if nesterov update before each batch (pre-update)
		if(hp->CONF_NESTEROV_MOMENTUM)
			mach->nesterov_update(hp->CONF_UPDATE_WAY,hp->CONF_MOMENTUM_ALPHA);
		//main batch
		int this_sentence = 0;
		int this_instance = 0;
		for(;;){
			//forward
			DependencyInstance* x = training_corpus->at(i);
			nn_input* the_inputs;
			REAL *fscores = forward_scores_o2sib(x,mach,&the_inputs,dict->get_helper(),0,STA_noprobs[i],hp);

			this_instance += the_inputs->get_numi();
			all_forward_instance += the_inputs->get_numi();
			all_inst_right += the_inputs->inst_good;
			all_inst_wrong += the_inputs->inst_bad;
			this_sentence ++;
			i++;

			//prepare gradients --- softmax -> fscores as gradient
			REAL *to_change = fscores;
			for(int ii=0;ii<the_inputs->get_numi();ii++){
				int tmp_goal = the_inputs->goals->at(ii);
				to_change[tmp_goal] -= 1;	//-1 for the right one
				to_change += odim;
			}

			//backward
			mach->backward(fscores);

			delete the_inputs;
			delete []fscores;

			//out of the mini-batch
			if(i>=num_sentences)
				break;
			if(hp->CONF_minibatch > 0){
				if(this_sentence >= hp->CONF_minibatch)
					break;
			}
			else{
				if(this_instance >= -1*hp->CONF_minibatch)
					break;
			}
		}
		//real update
		mach->update(hp->CONF_UPDATE_WAY,cur_lrate,hp->CONF_NN_WD,hp->CONF_MOMENTUM_ALPHA,hp->CONF_RMS_SMOOTH);
	}
	cout << "Iter done, skip " << skip_sent_num << " sentences and f&b " << all_forward_instance
			<< ";good/bad: " << all_inst_right << "/" << all_inst_wrong << endl;
}

