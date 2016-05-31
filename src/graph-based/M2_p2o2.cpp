/*
 * M2_p2o2.cpp
 *
 *  Created on: Nov 16, 2015
 *      Author: zzs
 */

#include "M2_p2.h"
#include "../algorithms/Eisner.h"
#include "../algorithms/EisnerO2sib.h"

void M2_p2o2::each_create_machine()
{
	//several need setting places
	hp->hp_nn.NN_wnum = dict->getnum_word();
	hp->hp_nn.NN_pnum = dict->getnum_pos();
	hp->hp_nn.NN_dnum = dict->get_helper()->get_distance_num();
	hp->hp_nn.NN_sdnum = dict->get_helper()->get_sd_num();
	hp->hp_nn.NN_out_prob = 0;	//not softmax here
	if(hp->CONF_labeled)
		hp->hp_nn.NN_out_size = dict->getnum_deprel();
	else
		hp->hp_nn.NN_out_size = 1;
	//create the new mach
	mach = Csnn::create(2,&hp->hp_nn);
}

void M2_p2o2::each_test_one(DependencyInstance* x,int dev)
{
	int noc_dev = dev ? hp->CONF_score_noc_dev : 0;
	if(noc_dev)
		Process::parse_o2sib(x,mfo1,0);
	else
		Process::parse_o2sib(x,mfo1,mso1);
}

void M2_p2o2::each_train_one_iter()
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
	cout << "##*** // Start the p2o2 training for iter " << cur_iter << " at " << ctime(&now)
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
		int this_tokens = 0;
		for(;;){
			//forward
			DependencyInstance* x = training_corpus->at(i);
			nn_input* the_inputs;
			REAL *fscores = forward_scores_o2sib(x,mach,&the_inputs,dict->get_helper(),0,STA_noprobs[i],hp);
			double* rscores = 0;
			double* tmp_marginals = 0;

			this_instance += the_inputs->get_numi();
			all_forward_instance += the_inputs->get_numi();
			all_inst_right += the_inputs->inst_good;
			all_inst_wrong += the_inputs->inst_bad;
			this_sentence ++;
			this_tokens += x->length() - 1;
			i++;

			adjust_scores_before(the_inputs, fscores, odim, hp->CONF_margin);
			//two situations
			int length = x->length();
			if(!hp->CONF_labeled){
				//calculate prob
				rscores = rearrange_scores_o2sib(x,mach,the_inputs,fscores,0,0,0,hp);
				tmp_marginals = encodeMarginals_o2sib(length,rscores);
			}
			else{
				//calculate prob
				rscores = rearrange_scores_o2sib(x,mach,the_inputs,fscores,0,0,0,hp);
				tmp_marginals = LencodeMarginals_o2sib(length,rscores,mach->get_odim());
			}
			adjust_scores_after(the_inputs, fscores, odim, hp->CONF_margin);

			//set gradients
			int HERE_dim = the_inputs->num_width;
			REAL* to_assign = fscores;
			for(int ii=0;ii<the_inputs->num_inst*HERE_dim;ii+=HERE_dim){
				int tmph = the_inputs->inputs->at(ii);
				int tmpm = the_inputs->inputs->at(ii+1);
				int tmps = the_inputs->inputs->at(ii+2);
				if(tmps<0)
					tmps = tmph;
				int tmp_goal = the_inputs->goals->at(ii/HERE_dim);
				for(int once=0;once<odim;once++,to_assign++){
					if(tmp_goal == once)
						*to_assign = -1 * (1 - tmp_marginals[get_index2_o2sib(length,tmph,tmps,tmpm,once,odim)]) + *to_assign * hp->CONF_score_p2reg;
					else
						*to_assign = tmp_marginals[get_index2_o2sib(length,tmph,tmps,tmpm,once,odim)] + *to_assign * hp->CONF_score_p2reg;	//now object is maximum
				}
			}

			//backward
			mach->backward(fscores);

			delete the_inputs;
			delete []fscores;
			delete []rscores;
			delete []tmp_marginals;

			if(i>=num_sentences)
				break;
			//out of the mini-batch
			while(training_corpus->at(i)->length() >= hp->CONF_higho_toolong){	//HAVE to compromise, bad choice
				skip_sent_num ++;
				i ++;
			}
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
		if (hp->CONF_mbatch_way == 1)
			mach->set_this_mbsize(this_tokens*this_tokens);
		else if (hp->CONF_mbatch_way == 2)
			mach->set_this_mbsize(this_sentence*this_sentence);
		mach->update(hp->CONF_UPDATE_WAY,cur_lrate,hp->CONF_NN_WD,hp->CONF_MOMENTUM_ALPHA,hp->CONF_RMS_SMOOTH);
	}
	cout << "Iter done, skip " << skip_sent_num << " sentences and f&b " << all_forward_instance
			<< ";good/bad: " << all_inst_right << "/" << all_inst_wrong << endl;
}
