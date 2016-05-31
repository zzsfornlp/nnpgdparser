/*
 * M2_p2o1.cpp
 *
 *  Created on: Nov 16, 2015
 *      Author: zzs
 */

#include "M2_p2.h"
#include "../algorithms/Eisner.h"

void M2_p2o1::each_create_machine()
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
	mach = Csnn::create(1,&hp->hp_nn);
}

void M2_p2o1::each_test_one(DependencyInstance* x,int dev)
{
	Process::parse_o1(x);
}

void M2_p2o1::each_train_one_iter()
{
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
	cout << "##*** //p2o1// Start the training for iter " << cur_iter << " at " << ctime(&now)
			<< "with lrate " << cur_lrate << endl;
	cout << "#Sentences is " << num_sentences << " and resample (about)" << num_sentences*hp->CONF_NN_resample << endl;
	for(int i=0;i<num_sentences;){
		//random skip (instead of shuffling every time)
		if(drand48() > hp->CONF_NN_resample){
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
			REAL *fscores = forward_scores_o1(x,mach,&the_inputs,dict->get_helper(),0,hp);
			double* rscores = 0;
			double* tmp_marginals = 0;

			this_instance += the_inputs->get_numi();
			all_forward_instance += the_inputs->get_numi();
			all_inst_right += the_inputs->inst_good;
			all_inst_wrong += the_inputs->inst_bad;
			this_sentence ++;
			this_tokens += x->length()-1;
			i++;

			adjust_scores_before(the_inputs, fscores, odim, hp->CONF_margin);
			//two situations
			int length = x->length();
			if(!hp->CONF_labeled){
				//calculate prob
				rscores = rearrange_scores_o1(x,mach,the_inputs,fscores,0,0,hp);
				tmp_marginals = encodeMarginals(length,rscores);
			}
			else{
				//calculate prob
				rscores = rearrange_scores_o1(x,mach,the_inputs,fscores,0,0,hp);
				tmp_marginals = LencodeMarginals(length,rscores,mach->get_odim());
			}
			adjust_scores_after(the_inputs, fscores, odim, hp->CONF_margin);

			//set gradients
			int HERE_dim = the_inputs->num_width;
			REAL* to_assign = fscores;
			for(int ii=0;ii<the_inputs->num_inst*HERE_dim;ii+=HERE_dim){
				int tmph = the_inputs->inputs->at(ii);
				int tmpm = the_inputs->inputs->at(ii+1);
				int tmp_goal = the_inputs->goals->at(ii/HERE_dim);
				for(int once=0;once<odim;once++,to_assign++){
					if(tmp_goal == once)
						*to_assign = -1 * (1 - tmp_marginals[get_index2(length,tmph,tmpm,once,odim)]) + *to_assign * hp->CONF_score_p2reg;
					else
						*to_assign = tmp_marginals[get_index2(length,tmph,tmpm,once,odim)] + *to_assign * hp->CONF_score_p2reg;	//now object is maximum
				}
			}

			//backward
			mach->backward(fscores);

			//mach->check_gradients(the_inputs);

			delete the_inputs;
			delete []fscores;
			delete []rscores;
			delete []tmp_marginals;

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
		if(hp->CONF_mbatch_way == 1)
			mach->set_this_mbsize(this_tokens*this_tokens);
		else if(hp->CONF_mbatch_way == 2)
			mach->set_this_mbsize(this_sentence*this_sentence);
		mach->update(hp->CONF_UPDATE_WAY,cur_lrate,hp->CONF_NN_WD,hp->CONF_MOMENTUM_ALPHA,hp->CONF_RMS_SMOOTH);
	}
	cout << "Iter done, skip " << skip_sent_num << " sentences and f&b " << all_forward_instance
			<< ";good/bad: " << all_inst_right << "/" << all_inst_wrong << endl;
}
