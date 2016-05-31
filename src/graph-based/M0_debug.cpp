/*
 * M0_debug.cpp
 *
 *  Created on: Oct 19, 2015
 *      Author: zzs
 */

#include "M0_debug.h"

//only before training (and after building dictionary)
void M0_debug::each_create_machine()
{
	//several need setting places
	hp->hp_nn.NN_wnum = dict->getnum_word();
	hp->hp_nn.NN_pnum = dict->getnum_pos();
	hp->hp_nn.NN_dnum = dict->get_helper()->get_distance_num();
	hp->hp_nn.NN_out_prob = 1;
	if(hp->CONF_labeled)
		hp->hp_nn.NN_out_size = dict->getnum_deprel()+1;
	else
		hp->hp_nn.NN_out_size = 2;

	//for debugging --- small one
	if(hp->CONF_NN_ITER==1){
	hp->hp_nn.NN_wsize = 5;
	hp->hp_nn.NN_psize = 5;
	hp->hp_nn.NN_dsize = 5;
	hp->hp_nn.NN_win = 3;
	hp->hp_nn.NN_add_average = 1;
	hp->hp_nn.NN_hidden_size = 3;
	hp->hp_nn.NN_wrsize = 5;

	hp->hp_nn.NN_sdsize = 5;
	hp->hp_nn.NN_srsize = 5;
	hp->hp_nn.NN_sl_filter = 3;
	}

	//create the new mach
	mach = Csnn::create(1,&hp->hp_nn);
}

void M0_debug::each_test_one(DependencyInstance* x,int t)
{
	Process::parse_o1(x);
}

void M0_debug::each_train_one_iter()
{
	int num_sentences = training_corpus->size();
	for(int i=0;i<num_sentences;i++){
		//forward
		DependencyInstance* x = training_corpus->at(i);
		nn_input* the_inputs;
		REAL *fscores = forward_scores_o1(x,mach,&the_inputs,dict->get_helper(),0,hp);

		//the_inputs->num_inst = 1;	//enough maybe

		mach->check_gradients(the_inputs);
		delete []fscores;
		delete the_inputs;
	}
}


