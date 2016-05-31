/*
 * Process_scores.cpp
 *
 *  Created on: Oct 11, 2015
 *      Author: zzs
 */

#include "Process.h"
#include "../algorithms/Eisner.h"
#include "../algorithms/EisnerO2sib.h"
#include "../algorithms/EisnerO3g.h"
#include <exception>

//be careful about the magic numbers
static inline void TMP_push234(vector<int>* l,int h,int m,int c=IMPOSSIBLE_INDEX,int g=IMPOSSIBLE_INDEX)
{
	l->push_back(h);
	l->push_back(m);
	if(c > IMPOSSIBLE_INDEX){
		l->push_back(c);
		if(g > IMPOSSIBLE_INDEX)
			l->push_back(g);
	}
}
static inline void TMP_pop234(vector<int>* l,int h,int m,int c=IMPOSSIBLE_INDEX,int g=IMPOSSIBLE_INDEX)
{
	l->pop_back();
	l->pop_back();
	if(c > IMPOSSIBLE_INDEX){
		l->pop_back();
		if(g > IMPOSSIBLE_INDEX)
			l->pop_back();
	}
}
static inline bool TMP_check234(int h1,int h2,int c1=IMPOSSIBLE_INDEX,int c2=IMPOSSIBLE_INDEX,
		int g1=IMPOSSIBLE_INDEX,int g2=IMPOSSIBLE_INDEX)
{
	return (h1==h2)&&(c1==c2)&&(g1==g2);
}
static inline bool TMP_higho_sample(HypherParameters* HP,int h=IMPOSSIBLE_INDEX,int m=IMPOSSIBLE_INDEX,
		int s=IMPOSSIBLE_INDEX,int g=IMPOSSIBLE_INDEX)
{
	if(!HP) return 1;
	return drand48() < HP->CONF_higho_percent;
}

// the routine for getting scores
// -- m means the current machine(might not be the same as options)
// -- t for assigning inputs, need outside to delete it
// -- testing for the forward of nn
REAL* Process::forward_scores_o1(DependencyInstance* x,Csnn* mac,nn_input** t,nn_input_helper* helper,
		int testing,HypherParameters*hh)
{
	//default order1 parsing
	int odim = mac->get_odim();	//1 or 2 for no-labeled, otherwise...
	int length = x->length();
	//- for output goals
	bool is_labeled = (mac->get_classdim()>1);
	int nope_goal = mac->get_classdim();	//for no-rel
	//prepare scores
	int num_pair_togo = 0;
	int num_good=0,num_bad=0;
	vector<int>* the_inputs = new vector<int>();
	vector<int>* the_goals = new vector<int>();
	//loop --- (h,m)
	for(int m=1;m<length;m++){
		for(int h=0;h<length;h++){
			if(m != h){
				TMP_push234(the_inputs,h,m);
				if(!testing){	//when training, prepare the goals
					if(TMP_check234(x->heads->at(m),h)){
						the_goals->push_back(is_labeled?(x->index_deprels->at(m)):0);
						num_good++;
					}
					else{
						if(TMP_higho_sample(hh)){
							the_goals->push_back(nope_goal);
							num_bad++;
						}
						else{
							TMP_pop234(the_inputs,h,m);
							num_pair_togo -= 1;	//well, maybe the same
						}
					}
				}
				num_pair_togo ++;
			}
		}
	}
	(*t) = new nn_input(num_pair_togo,2,the_inputs,the_goals,x->index_forms,x->index_pos,helper,
			num_good,num_bad);
	//--fix the 0 bug
	Process::CHECK_EQUAL(num_pair_togo,(*t)->num_inst,"Forward bsize nope.");
	if(num_pair_togo==0)
		return 0;
	//--
	REAL* tmp_scores = mac->forward(*t,testing);
	return tmp_scores;
}
REAL* Process::forward_scores_o2sib(DependencyInstance* x,Csnn* mac,nn_input** t,nn_input_helper* h,
		int testing,bool* cut_o1,HypherParameters* hh)
{
	//o2sib
	int odim = mac->get_odim();	//1 or 2 for no-labeled, otherwise...
	int length = x->length();
	//- for output goals
	bool is_labeled = (mac->get_classdim()>1);
	int nope_goal = mac->get_classdim();	//for no-rel
	//prepare scores
	int num_togo = 0;
	int num_good=0,num_bad=0;
	vector<int>* the_inputs = new vector<int>();
	vector<int>* the_goals = new vector<int>();
	bool* score_o1 = cut_o1;
	//loop --- (h,m,s)
	for(int m=1;m<length;m++){
		//put the real one first if training
		// --- maybe doubled, but maybe does not matter (when testing: same value, when training: double positive)
		int real_center=IMPOSSIBLE_INDEX;
		int real_head=IMPOSSIBLE_INDEX;
		if(!testing){	//when training, prepare the goals
			real_center = x->siblings->at(m);
			real_head = x->heads->at(m);
			//must get the real one
			TMP_push234(the_inputs,real_head,m,real_center);
			the_goals->push_back(is_labeled?(x->index_deprels->at(m)):0);
			num_togo += 1;
			num_good++;
		}
		//others
		for(int h=0;h<length;h++){
			if(h==m)
				continue;
			bool norpob_hm = score_o1[get_index2(length,h,m)];
			//h,m,-1
			if(!norpob_hm){
				TMP_push234(the_inputs,h,m,-1);
				if(!testing){
					if(TMP_check234(real_head,h,real_center,-1))
					{
						TMP_pop234(the_inputs,h,m,-1);
						num_togo -= 1;
					}//don't add again here
					else{
						if(TMP_higho_sample(hh)){
							the_goals->push_back(nope_goal);
							num_bad++;
						}
						else{
							TMP_pop234(the_inputs,h,m,-1);
							num_togo -= 1;	//well, maybe the same
						}
					}
				}
				num_togo += 1;
			}
			//h,m,c
			int small = GET_MIN_ONE(m,h);
			int large = GET_MAX_ONE(m,h);
			if(!norpob_hm){
				for(int c=small+1;c<large;c++){
					if(!score_o1[get_index2(length,h,c)]){
						TMP_push234(the_inputs,h,m,c);
						if(!testing){
							if(TMP_check234(real_head,h,real_center,c))
							{
								TMP_pop234(the_inputs,h,m,c);
								num_togo -= 1;
							}//don't add again here
							else{
								if(TMP_higho_sample(hh)){
									the_goals->push_back(nope_goal);
									num_bad++;
								}
								else{
									TMP_pop234(the_inputs,h,m,c);
									num_togo -= 1;	//well, maybe the same
								}
							}
						}
						num_togo += 1;
					}
				}
			}
		}
	}
	(*t) = new nn_input(num_togo,3,the_inputs,the_goals,x->index_forms,x->index_pos,h,
			num_good,num_bad);	//!!DEBUG, goals and inputs reversed
	//--fix the 0 bug
	Process::CHECK_EQUAL(num_togo,(*t)->num_inst,"Forward bsize nope.");
	if(num_togo==0)
		return 0;
	//--
	REAL* tmp_scores = mac->forward(*t,testing);
	return tmp_scores;
}
REAL* Process::forward_scores_o3g(DependencyInstance* x,Csnn* mac,nn_input** t,nn_input_helper* h,
		int testing,bool* cut_o1,HypherParameters*hh)
{
	//o3g
	int odim = mac->get_odim();	//1 or 2 for no-labeled, otherwise...
	int length = x->length();
	//- for output goals
	bool is_labeled = (mac->get_classdim()>1);
	int nope_goal = mac->get_classdim();	//for no-rel
	//prepare scores
	int num_togo = 0;
	int num_good=0,num_bad=0;
	vector<int>* the_inputs = new vector<int>();
	vector<int>* the_goals = new vector<int>();
	bool* score_o1 = cut_o1;
	//loop --- (h,m,s,g)
	for(int m=1;m<length;m++){
		//put the real one first if training
		int real_center=IMPOSSIBLE_INDEX;
		int real_head=IMPOSSIBLE_INDEX;
		int real_grand=IMPOSSIBLE_INDEX;
		if(!testing){	//when training, prepare the goals
			real_center = x->siblings->at(m);
			real_head = x->heads->at(m);
			real_grand = x->heads->at(real_head);
			//must get the real one
			TMP_push234(the_inputs,real_head,m,real_center,real_grand);
			the_goals->push_back(is_labeled?(x->index_deprels->at(m)):0);
			num_togo += 1;
			num_good++;
		}
		//--others
		//1.when h==0
		{
			int h=0;
			bool norpob_hm = score_o1[get_index2(length,h,m)];
			if(!norpob_hm){
				//0,0,0,m as g,h,c,m
				TMP_push234(the_inputs,h,m,-1,-1);
				if(!testing){
					if(TMP_check234(real_head,h,real_center,-1,real_grand,-1))
					{
						TMP_pop234(the_inputs,h,m,-1,-1);
						num_togo -= 1;
					}//don't add again here
					else{
						if(TMP_higho_sample(hh)){
							the_goals->push_back(nope_goal);
							num_bad++;
						}
						else{
							TMP_pop234(the_inputs,h,m,-1,-1);
							num_togo -= 1;	//well, maybe the same
						}
					}
				}
				num_togo += 1;
				//0,0,c,m
				for(int c=1;c<m;c++){
					if(!score_o1[get_index2(length,h,c)]){
						TMP_push234(the_inputs,h,m,c,-1);
						if(!testing){
							if(TMP_check234(real_head,h,real_center,c,real_grand,-1))
							{
								TMP_pop234(the_inputs,h,m,c,-1);
								num_togo -= 1;
							}
							else{
								if(TMP_higho_sample(hh)){
									the_goals->push_back(nope_goal);
									num_bad++;
								}
								else{
									TMP_pop234(the_inputs,h,m,c,-1);
									num_togo -= 1;	//well, maybe the same
								}
							}
						}
						num_togo += 1;
					}
				}
			}
		}
		//2.others h>0
		for(int h=1;h<length;h++){
			if(h==m)
				continue;
			bool norpob_hm = score_o1[get_index2(length,h,m)];
			int small = GET_MIN_ONE(h,m);
			int large = GET_MAX_ONE(h,m);
			if(!norpob_hm){
				for(int g=0;g<length;g++){
					bool norpob_gh = score_o1[get_index2(length,g,h)];
					if((g>=small && g<=large) || norpob_gh)	//non-proj not allowed //!!DEBUG: yi-jing-wu-yu-le...
						continue;
					//for those c
					//g,h,-1,m
					TMP_push234(the_inputs,h,m,-1,g);
					if(!testing){
						if(TMP_check234(real_head,h,real_center,-1,real_grand,g))
						{
							TMP_pop234(the_inputs,h,m,-1,g);
							num_togo -= 1;
						}
						else{
							if(TMP_higho_sample(hh)){
								the_goals->push_back(nope_goal);
								num_bad++;
							}
							else{
								TMP_pop234(the_inputs,h,m,-1,g);
								num_togo -= 1;	//well, maybe the same
							}
						}
					}
					num_togo += 1;
					//g,h,c,m
					for(int c=small+1;c<large;c++){
						if(!score_o1[get_index2(length,h,c)]){
							TMP_push234(the_inputs,h,m,c,g);
							if(!testing){
								if(TMP_check234(real_head,h,real_center,c,real_grand,g))
								{
									TMP_pop234(the_inputs,h,m,c,g);
									num_togo -= 1;
								}
								else{
									if(TMP_higho_sample(hh)){
										the_goals->push_back(nope_goal);
										num_bad++;
									}
									else{
										TMP_pop234(the_inputs,h,m,c,g);
										num_togo -= 1;	//well, maybe the same
									}
								}
							}
							num_togo += 1;
						}
					}
				}
			}
		}
	}
	(*t) = new nn_input(num_togo,4,the_inputs,the_goals,x->index_forms,x->index_pos,h,num_good,num_bad);
	//--fix the 0 bug
	Process::CHECK_EQUAL(num_togo,(*t)->num_inst,"Forward bsize nope.");
	if(num_togo==0)
		return 0;
	//--
	REAL* tmp_scores = mac->forward(*t,testing);
	return tmp_scores;
}

//-----------------------------------rearrange------------------------------------------
#include "Process_trans.cpp"	//special
// x for length, m for odim, t for outputs, fscores for forward-scores(no arrange)
// prob_output for output is label+1; prob_trans to trans prob only when prob_output
double* Process::rearrange_scores_o1(DependencyInstance* x,Csnn* m,nn_input* the_inputs,REAL* fscores,
		bool prob_output,bool prob_trans,HypherParameters*hh)
{
	const int THE_DIM = 2;
	int length = x->length();
	int fs_dim = m->get_odim();
	int num_label = fs_dim;
	if(prob_output)	//if prob output, there is one for no-rel
		num_label -= 1;
	//prepare
	double* rscores = new double[length*length*num_label];
	for(int i=0;i<length*length*num_label;i++)
		rscores[i] = DOUBLE_LARGENEG;
	//make sure the width is THE_DIM}
	CHECK_EQUAL(the_inputs->get_numw(),THE_DIM,"!!!Wrong nn_input");
	//get scores
	vector<int>* inputs_list = the_inputs->inputs;
	REAL *to_assign = fscores;
	for(int i=0;i<the_inputs->num_inst*THE_DIM;i+=THE_DIM){	//!!DEBUG: *THE_DIM
		int tmph = inputs_list->at(i);
		int tmpm = inputs_list->at(i+1);
		for(int curi=0;curi<num_label;curi++)
			rscores[get_index2(length,tmph,tmpm,curi,num_label)] = to_assign[curi];
		//(if prob, the no-rel must be at the end, which is different from before)
		to_assign += fs_dim;
	}
	//if prob-transfrom
	if(prob_output && prob_trans){
		//1.prepare the ignored nope prob
		double* nope_probs = new double[length*length];
		for(int i=0;i<length*length;i++)
			nope_probs[i] = 1;		//set to one firstly
		REAL *to_assign = fscores;
		for(int i=0;i<the_inputs->num_inst*THE_DIM;i+=THE_DIM){ //!!DEBUG: *THE_DIM
			int tmph = inputs_list->at(i);
			int tmpm = inputs_list->at(i+1);
			nope_probs[get_index2(length,tmph,tmpm)] = to_assign[fs_dim-1];	//the last one
			to_assign += fs_dim;
		}
		trans_o1(rscores,nope_probs,length,num_label);
		delete []nope_probs;
	}
	return rscores;
}

double* Process::rearrange_scores_o2sib(DependencyInstance* x,Csnn* m,nn_input* the_inputs,REAL* fscores,
		bool prob_output,bool prob_trans,double* rscores_o1,HypherParameters*hh)
{
	const int THE_DIM = 3;
	int length = x->length();
	int fs_dim = m->get_odim();
	int num_label = fs_dim;
	if(prob_output)	//if prob output, there is one for no-rel
		num_label -= 1;
	//prepare
	double* rscores = new double[length*length*length*num_label];
	for(int i=0;i<length*length*length*num_label;i++)
		rscores[i] = DOUBLE_LARGENEG;
	//make sure the width is THE_DIM
	CHECK_EQUAL(the_inputs->get_numw(),THE_DIM,"!!!Wrong nn_input");
	//get scores
	vector<int>* inputs_list = the_inputs->inputs;
	REAL *to_assign = fscores;
	for(int i=0;i<the_inputs->num_inst*THE_DIM;i+=THE_DIM){
		int tmph = inputs_list->at(i);
		int tmpm = inputs_list->at(i+1);
		int tmps = inputs_list->at(i+2);
		if(tmps<0)
			tmps = tmph;
		for(int curi=0;curi<num_label;curi++)
			rscores[get_index2_o2sib(length,tmph,tmps,tmpm,curi,num_label)] = to_assign[curi];
		//(if prob, the no-rel must be at the end, which is different from before)
		to_assign += fs_dim;
	}
	//if prob-transfrom
	if(prob_output && prob_trans){
		//1.prepare the ignored nope prob
		double* nope_probs = new double[length*length*length];
		for(int i=0;i<length*length*length;i++)
			nope_probs[i] = 1;		//set to one firstly
		REAL *to_assign = fscores;
		for(int i=0;i<the_inputs->num_inst*THE_DIM;i+=THE_DIM){
			int tmph = inputs_list->at(i);
			int tmpm = inputs_list->at(i+1);
			int tmps = inputs_list->at(i+2);
			if(tmps<0)
				tmps = tmph;
			nope_probs[get_index2_o2sib(length,tmph,tmps,tmpm)] = to_assign[fs_dim-1];	//the last one
			to_assign += fs_dim;
		}
		trans_o2sib(rscores,nope_probs,length,num_label);
		delete []nope_probs;
	}
	//if combining scores
	if(rscores_o1){
		//the provided score must be re-arranged and must have the same label-dim
		for(int m=1;m<length;m++){
			for(int h=0;h<length;h++){
				if(m!=h){
					for(int la=0;la<num_label;la++){
					double score_tmp = rscores_o1[get_index2(length,h,m,la,num_label)] * hh->CONF_score_o1scale;
					rscores[get_index2_o2sib(length,h,h,m,la,num_label)] += score_tmp;
					for(int c=h+1;c<m;c++)
						rscores[get_index2_o2sib(length,h,c,m,la,num_label)] += score_tmp;
					for(int c=m+1;c<h;c++)
						rscores[get_index2_o2sib(length,h,c,m,la,num_label)] += score_tmp;
					}
				}
			}
		}
	}
	return rscores;
}

double* Process::rearrange_scores_o3g(DependencyInstance* x,Csnn* m,nn_input* the_inputs,REAL* fscores,
		bool prob_output,bool prob_trans,double* rscores_o1,double* rscores_o2sib,HypherParameters*hh)
{
	const int THE_DIM = 4;
	long length = x->length();
	int fs_dim = m->get_odim();
	int num_label = fs_dim;
	if(prob_output)	//if prob output, there is one for no-rel
		num_label -= 1;
	//prepare
	double* rscores = 0;
	try{
		rscores = new double[length*length*length*length*num_label];
	}catch(std::bad_alloc& bad_one){
		nn_math::CHECK_EQUAL(0,1,"Bad allocation for scores of o3g.");
		throw bad_one;
	}

	for(long i=0;i<length*length*length*length*num_label;i++)
		rscores[i] = DOUBLE_LARGENEG;
	//make sure the width is THE_DIM
	CHECK_EQUAL(the_inputs->get_numw(),THE_DIM,"!!!Wrong nn_input");
	//get scores
	vector<int>* inputs_list = the_inputs->inputs;
	REAL *to_assign = fscores;
	for(int i=0;i<the_inputs->num_inst*THE_DIM;i+=THE_DIM){
		int tmph = inputs_list->at(i);
		int tmpm = inputs_list->at(i+1);
		int tmps = inputs_list->at(i+2);
		int tmpg = inputs_list->at(i+3);
		if(tmps<0)
			tmps = tmph;
		if(tmpg<0)
			tmpg = 0;
		for(int curi=0;curi<num_label;curi++){
			//----------------check nan-----------------------
			REAL the_one_tocheck = to_assign[curi];
			if(the_one_tocheck != the_one_tocheck || the_one_tocheck < DOUBLE_LARGENEG){
				Process::CHECK_EQUAL(the_one_tocheck,0.0f,"Maybe Nan appears.");
				throw the_one_tocheck;
			}
			//------------------------------------------------
			rscores[get_index2_o3g(length,tmpg,tmph,tmps,tmpm,curi,num_label)] = the_one_tocheck;
		}
		//(if prob, the no-rel must be at the end, which is different from before)
		to_assign += fs_dim;
	}
	//if prob-transfrom
	if(prob_output && prob_trans){
		//1.prepare the ignored nope prob
		double* nope_probs = new double[length*length*length*length];
		for(long i=0;i<length*length*length*length;i++)
			nope_probs[i] = 1;		//set to one firstly
		REAL *to_assign = fscores;
		for(int i=0;i<the_inputs->num_inst*THE_DIM;i+=THE_DIM){
			int tmph = inputs_list->at(i);
			int tmpm = inputs_list->at(i+1);
			int tmps = inputs_list->at(i+2);
			int tmpg = inputs_list->at(i+3);
			if(tmps<0)
				tmps = tmph;
			if(tmpg<0)
				tmpg = 0;
			nope_probs[get_index2_o3g(length,tmpg,tmph,tmps,tmpm)] = to_assign[fs_dim-1];	//the last one
			to_assign += fs_dim;
		}
		trans_o3g(rscores,nope_probs,length,num_label);
		delete []nope_probs;
	}
	//if combining scores
	if(rscores_o1 || rscores_o2sib){
		//the provided score must be re-arranged and must have the same label-dim
		//1.0,0,?,m
		for(int m=1;m<length;m++){
			for(int la=0;la<num_label;la++){	//LABEL
			double s_0m = 0,s_0xm=0;
			if(rscores_o1)
				s_0m = rscores_o1[get_index2(length,0,m,la,num_label)] * hh->CONF_score_o1scale;
			if(rscores_o2sib)
				s_0xm = rscores_o2sib[get_index2_o2sib(length,0,0,m,la,num_label)] * hh->CONF_score_o2scale;
			rscores[get_index2_o3g(length,0,0,0,m,la,num_label)] += s_0m + s_0xm;
			for(int c=m-1;c>0;c--){
				if(rscores_o2sib)
					s_0xm = rscores_o2sib[get_index2_o2sib(length,0,c,m,la,num_label)] * hh->CONF_score_o2scale;
				rscores[get_index2_o3g(length,0,0,c,m,la,num_label)] += s_0m + s_0xm;
			}
			}
		}
		for(int s=1;s<length;s++){
			for(int t=s+1;t<length;t++){
				for(int la=0;la<num_label;la++){	//LABEL
				double s_st=0,s_ts=0;
				if(rscores_o1){
					s_st = rscores_o1[get_index2(length,s,t,la,num_label)] * hh->CONF_score_o1scale;
					s_ts = rscores_o1[get_index2(length,t,s,la,num_label)] * hh->CONF_score_o1scale;
				}
				for(int g=0;g<length;g++){
					if(g>=s && g<=t)	//no non-projective
						continue;
					double s_sxt=0,s_txs=0;
					if(rscores_o2sib){
						s_sxt = rscores_o2sib[get_index2_o2sib(length,s,s,t,la,num_label)] * hh->CONF_score_o2scale;
						s_txs = rscores_o2sib[get_index2_o2sib(length,t,t,s,la,num_label)] * hh->CONF_score_o2scale;
					}
					rscores[get_index2_o3g(length,g,s,s,t,la,num_label)] += s_st + s_sxt;
					rscores[get_index2_o3g(length,g,t,t,s,la,num_label)] += s_ts + s_txs;
					for(int c=s+1;c<t;c++){
						double s_sct=0,s_tcs=0;
						if(rscores_o2sib){
							s_sct = rscores_o2sib[get_index2_o2sib(length,s,c,t,la,num_label)] * hh->CONF_score_o2scale;
							s_tcs = rscores_o2sib[get_index2_o2sib(length,t,c,s,la,num_label)] * hh->CONF_score_o2scale;
						}
						rscores[get_index2_o3g(length,g,s,c,t,la,num_label)] += s_st + s_sct;
						rscores[get_index2_o3g(length,g,t,c,s,la,num_label)] += s_ts + s_tcs;
					}
				}
				}
			}
		}
	}
	return rscores;
}


