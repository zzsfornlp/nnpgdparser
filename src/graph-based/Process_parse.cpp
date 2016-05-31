/*
 * Process_parse.cpp
 *
 *  Created on: Oct 12, 2015
 *      Author: zzs
 */

#include "Process.h"
#include "../algorithms/Eisner.h"
#include "../algorithms/EisnerO2sib.h"
#include "../algorithms/EisnerO3g.h"

//get max label
static void TMP_get_maxlabel(long all,int ln,double* s,double** ascores_p,int** max_deperl_p)
{
	double* ascores = new double[all];
	int* max_deprel = new int[all];
	double* to_scores = s;
	//get the max one for each edge --- every dictionary_labelnum points
	for(long tmp_ind=0;tmp_ind<all;tmp_ind++){
		double tmp_max = to_scores[0];
		int tmp_max_index = 0;
		for(int tmp_index=0;tmp_index<ln;tmp_index++){	//!!DEBUG: missing init =0
			double this_s = to_scores[tmp_index];
			if(this_s > tmp_max){
				tmp_max = this_s;
				tmp_max_index = tmp_index;
			}
		}
		to_scores += ln;
		ascores[tmp_ind] = tmp_max;
		max_deprel[tmp_ind] = tmp_max_index;
	}
	(*ascores_p) = ascores;
	(*max_deperl_p) = max_deprel;
}

//the parse routines
//1.prepare possible o1-cut 2.get lower-order scores and combine 3.parse and take care of labels
//!! here only testing mode --- forward_scores_o*'s testing set to 1
void Process::parse_o1(DependencyInstance* x,bool add_margin)
{
	//1.get the numbers and informations
	int dictionary_labelnum = dict->getnum_deprel();
	int mach_outputnum = mach->get_odim();
	bool is_labeled = (mach_outputnum >= dictionary_labelnum);	//at least labnum (or +1)
	double* rscores = get_scores_o1(x,mach,dict,hp->CONF_score_prob,hp,add_margin);
	//3.graph-algorithm
	if(is_labeled){
		int len = x->length();
		double* ascores;
		int* max_deprel;
		TMP_get_maxlabel(len*len,dictionary_labelnum,rscores,&ascores,&max_deprel);
		x->predict_heads = decodeProjective(len,ascores);
		x->predict_deprels = new vector<int>(len,0);
		for(int tmp_ind=1;tmp_ind<len;tmp_ind++){	//start from 1
			int best_deprel_ind = get_index2(len,x->predict_heads->at(tmp_ind),tmp_ind);
			x->predict_deprels->at(tmp_ind) = max_deprel[best_deprel_ind];
		}
		delete []ascores;
		delete []max_deprel;
	}
	else{
		x->predict_heads = decodeProjective(x->length(),rscores);
		x->predict_deprels = new vector<int>(x->length(),0);	//nope
	}
	delete []rscores;
}

//o1_filter,o1_scorer must have same dictionary, o1_filter must be unlabel and prob, o1_scorer must be same conf
void Process::parse_o2sib(DependencyInstance* x,CsnnO1* o1_filter,CsnnO1* o1_scorer,bool add_margin)
{
	//1.get the numbers and informations
	int len = x->length();
	int dictionary_labelnum = dict->getnum_deprel();
	int mach_outputnum = mach->get_odim();
	bool is_labeled = (mach_outputnum >= dictionary_labelnum);	//at least labnum (or +1)
	bool is_prob = ((mach_outputnum==2) || (mach_outputnum > dictionary_labelnum));
	bool is_trans = (is_prob && hp->CONF_score_prob);
	//1.5 the o1 filter --- must unlabel and prob
	bool* o1f_cut = get_cut_o1(x,o1_filter,dict,hp->CONF_score_o1filter_cut);
	//1.6 the o1 scorer --- must be same labeled-conf
	double* o1s_rscores = 0;
	if(o1_scorer){
		bool o1s_is_labeled = (o1_scorer->get_odim() >= dictionary_labelnum);
		if(is_labeled != o1s_is_labeled)
			cerr << "Label-conf no match for o1_scorer" << endl;
		o1s_rscores = get_scores_o1(x,o1_scorer,dict,is_trans,hp, add_margin);
	}
	//2.get o2sib scores
	double* rscores = get_scores_o2sib(x,mach,dict,is_trans,o1f_cut,o1s_rscores,hp,add_margin);
	//3.graph-algorithm
	if(is_labeled){
		int len = x->length();
		double* ascores;
		int* max_deprel;
		TMP_get_maxlabel(len*len*len,dictionary_labelnum,rscores,&ascores,&max_deprel);
		x->predict_heads = decodeProjective_o2sib(len,ascores);
		x->predict_deprels = new vector<int>(len,0);
		for(int tmp_ind=1;tmp_ind<len;tmp_ind++){	//start from 1
			int m = tmp_ind;
			int h = x->predict_heads->at(m);
			int step = (h>m)?1:-1;//m->h
			int c = m + step;
			for(;c!=h;c+=step)	//nearest c of m with h as head
				if(h == x->predict_heads->at(c))
					break;
			int best_deprel_ind = get_index2_o2sib(len,h,c,m);
			x->predict_deprels->at(tmp_ind) = max_deprel[best_deprel_ind];
		}
		delete []ascores;
		delete []max_deprel;
	}
	else{
		x->predict_heads = decodeProjective_o2sib(x->length(),rscores);
		x->predict_deprels = new vector<int>(x->length(),0);	//nope
	}
	delete []rscores;
	delete []o1f_cut;
	delete []o1s_rscores;
}

void Process::parse_o3g(DependencyInstance* x,CsnnO1* o1_filter,CsnnO1* o1_scorer,CsnnO2* o2_scorer,bool add_margin)
{
	//1.get the numbers and informations
	int len = x->length();
	int dictionary_labelnum = dict->getnum_deprel();
	int mach_outputnum = mach->get_odim();
	bool is_labeled = (mach_outputnum >= dictionary_labelnum);	//at least labnum (or +1)
	bool is_prob = ((mach_outputnum==2) || (mach_outputnum > dictionary_labelnum));
	bool is_trans = (is_prob && hp->CONF_score_prob);
	//1.5 the o1 filter --- must unlabel and prob
	bool* o1f_cut = get_cut_o1(x,o1_filter,dict,hp->CONF_score_o1filter_cut);
	//1.6 the o1 scorer --- must be same labeled-conf
	double* o1s_rscores = 0;
	if(o1_scorer){
		bool o1s_is_labeled = (o1_scorer->get_odim() >= dictionary_labelnum);
		if(is_labeled != o1s_is_labeled)
			cerr << "Label-conf no match for o1_scorer" << endl;
		o1s_rscores = get_scores_o1(x,o1_scorer,dict,is_trans,hp, add_margin);
	}
	//1.7. the o2 scorer --- ...
	double* o2s_rscores = 0;
	if(o2_scorer){
		bool o2s_is_labeled = (o2_scorer->get_odim() >= dictionary_labelnum);
		if(is_labeled != o2s_is_labeled)
			cerr << "Label-conf no match for o2_scorer" << endl;
		o2s_rscores = get_scores_o2sib(x,o2_scorer,dict,is_trans,o1f_cut,0,hp, add_margin);	//here no combine scores
	}
	//2.get o3g scores
	double* rscores = get_scores_o3g(x,mach,dict,is_trans,o1f_cut,o1s_rscores,o2s_rscores,hp,add_margin);
	//3.graph-algorithm
	if(is_labeled){
		long len = x->length();
		double* ascores;
		int* max_deprel;
		TMP_get_maxlabel(len*len*len*len,dictionary_labelnum,rscores,&ascores,&max_deprel);
		x->predict_heads = decodeProjective_o3g(len,ascores);
		x->predict_deprels = new vector<int>(len,0);
		for(int tmp_ind=1;tmp_ind<len;tmp_ind++){	//start from 1
			int m = tmp_ind;
			int h = x->predict_heads->at(m);
			int step = (h>m)?1:-1;//m->h
			int c = m + step;
			for(;c!=h;c+=step)	//nearest c of m with h as head
				if(h == x->predict_heads->at(c))
					break;
			int g = (h==0) ? 0 : x->predict_heads->at(h);	//specially(0,h,?,m)
			int best_deprel_ind = get_index2_o3g(len,g,h,c,m);
			x->predict_deprels->at(tmp_ind) = max_deprel[best_deprel_ind];
		}
		delete []ascores;
		delete []max_deprel;
	}
	else{
		x->predict_heads = decodeProjective_o3g(x->length(),rscores);
		x->predict_deprels = new vector<int>(x->length(),0);	//nope
	}
	delete []rscores;
	delete []o1f_cut;
	delete []o1s_rscores;
	delete []o2s_rscores;
}
