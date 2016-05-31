/*
 * Process_scores2.cpp
 *
 *  Created on: Oct 13, 2015
 *      Author: zzs
 */
#include "Process.h"
#include "../algorithms/Eisner.h"
//1.for getting cut

//prepare bool for high-order filtering
//-score is the rearrange-score and here must be unlabeled prob
//-cut>0 means absolute,cut<0 means compared to max
static bool* TMP_get_cut_o1(int len,double* scores,double cut)
{
	double* scores_max = new double[len];
	for(int m=1;m<len;m++){	//each token's max-score head
		scores_max[m] = scores[get_index2(len,0,m)];
		for(int h=1;h<len;h++){
			if(h==m) continue;
			double tmp_s = scores[get_index2(len,h,m)];
			if(tmp_s > scores_max[m])
				scores_max[m] = tmp_s;
		}
	}
	bool* ret = new bool[len*len];
	for(int m=1;m<len;m++){
		for(int h=0;h<len;h++){
			if(h==m) continue;
			if(cut<0)
				ret[get_index2(len,h,m)] = (scores[get_index2(len,h,m)] < (scores_max[m]*-1*cut));
			else
				ret[get_index2(len,h,m)] = (scores[get_index2(len,h,m)] < cut);
		}
	}
	return ret;
}

#include "../algorithms/Helper.h"
//special routine
bool* Process::get_cut_o1(DependencyInstance* x,CsnnO1* o1_filter,Dictionary *dict,double cut)
{
	//CHECK_EQUAL(o1_filter->get_odim(),dict->getnum_deprel()+1,"Bad mach as filter");
	int fs_dim = o1_filter->get_odim();
	int cl_dim = dict->getnum_deprel();
	bool is_marginal = (fs_dim == cl_dim);
	//1.get o1-fscores
	nn_input* the_inputs;
	REAL* fscores = forward_scores_o1(x,o1_filter,&the_inputs,dict->get_helper(),1);	//testing-mode, forward scores
	//2.get binary scores
	int length = x->length();
	double* scores = new double[length*length];
	if(!is_marginal){
		for(int i=0;i<length*length;i++)
			scores[i] = 0;		//set to 0
		REAL *to_assign = fscores;
		for(int i=0;i<the_inputs->num_inst*2;i+=2){ //must be 2
			int tmph = the_inputs->inputs->at(i);
			int tmpm = the_inputs->inputs->at(i+1);
			scores[get_index2(length,tmph,tmpm)] = 1-to_assign[fs_dim-1];	//1 - the last one
			to_assign += fs_dim;
		}
	}
	else{
		//special one --- marginal probability
		for(int i=0;i<length*length;i++)
			scores[i] = DOUBLE_LARGENEG;		//set to 0
		REAL *to_assign = fscores;
		for(int i=0;i<the_inputs->num_inst*2;i+=2){ //must be 2
			int tmph = the_inputs->inputs->at(i);
			int tmpm = the_inputs->inputs->at(i+1);
			double tmp_sum = to_assign[0];
			for(int ii=1;ii<fs_dim;ii++)
				tmp_sum = logsumexp(tmp_sum,to_assign[ii],false);
			scores[get_index2(length,tmph,tmpm)] = tmp_sum;
			to_assign += fs_dim;
		}
		//reaplace it ...
		double* tmp_reaplace = scores;
		scores = encodeMarginals(length,scores);
		delete []tmp_reaplace;
	}
	//3.filter out
	bool* o1f_cut = TMP_get_cut_o1(x->length(),scores,cut);
	delete []fscores;
	delete []scores;
	delete the_inputs;
	return o1f_cut;
}

//2.the getting-score functions for parsing
//used when testing or possible parsing in training
double* Process::get_scores_o1(DependencyInstance* x,Csnn* m,Dictionary* dict,bool trans,HypherParameters*hh,bool add_margin)	//add_margin means not testing
{
	//1.get the numbers and informations
	int dictionary_labelnum = dict->getnum_deprel();
	//CHECK_EQUAL(dictionary_labelnum,m->get_classdim(),"Dictionary and Mach no match in class");
	bool is_prob = m->get_output_prob();
	bool is_trans = trans;
	//2.getting the scores
	nn_input* the_input;
	REAL* fscores = forward_scores_o1(x,m,&the_input,dict->get_helper(),(add_margin ? 0 : 1));	//testing-mode, forward scores
	if(add_margin)
		adjust_scores_before(the_input, fscores, m->get_odim(), hh->CONF_margin);
	double* rscores = rearrange_scores_o1(x,m,the_input,fscores,is_prob,is_trans,hh);
	delete the_input;
	delete []fscores;
	return rscores;
}

double* Process::get_scores_o2sib(DependencyInstance* x,Csnn* m,Dictionary* dict,bool trans,
		bool* cut_o1,double* rscores_o1,HypherParameters*hh,bool add_margin)
{
	//1.get the numbers and informations
	int dictionary_labelnum = dict->getnum_deprel();
	//CHECK_EQUAL(dictionary_labelnum,m->get_classdim(),"Dictionary and Mach no match in class");
	bool is_prob = m->get_output_prob();
	bool is_trans = trans;
	//2.getting the scores
	nn_input* the_input;
	REAL* fscores = forward_scores_o2sib(x,m,&the_input,dict->get_helper(),(add_margin ? 0 : 1),cut_o1);	//testing-mode, forward scores
	if(add_margin)
		adjust_scores_before(the_input, fscores, m->get_odim(), hh->CONF_margin);
	double* rscores = rearrange_scores_o2sib(x,m,the_input,fscores,is_prob,is_trans,rscores_o1,hh);
	delete the_input;
	delete []fscores;
	return rscores;
}

double* Process::get_scores_o3g(DependencyInstance* x,Csnn* m,Dictionary* dict,bool trans,
		bool* cut_o1,double* rscores_o1,double* rscores_o2sib,HypherParameters*hh,bool add_margin)
{
	//1.get the numbers and informations
	int dictionary_labelnum = dict->getnum_deprel();
	//CHECK_EQUAL(dictionary_labelnum,m->get_classdim(),"Dictionary and Mach no match in class");
	bool is_prob = m->get_output_prob();
	bool is_trans = trans;
	//2.getting the scores
	nn_input* the_input;
	REAL* fscores = forward_scores_o3g(x,m,&the_input,dict->get_helper(),(add_margin ? 0 : 1),cut_o1);	//testing-mode, forward scores
	if(add_margin)
		adjust_scores_before(the_input, fscores, m->get_odim(), hh->CONF_margin);
	double* rscores = rearrange_scores_o3g(x,m,the_input,fscores,is_prob,is_trans,rscores_o1,rscores_o2sib,hh);
	delete the_input;
	delete []fscores;
	return rscores;
}

