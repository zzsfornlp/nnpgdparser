/*
 * HypherParameters.h
 *
 *  Created on: Oct 10, 2015
 *      Author: zzs
 */

#ifndef PARTS_HYPHERPARAMETERS_H_
#define PARTS_HYPHERPARAMETERS_H_

#include <string>
#include <fstream>
#include <cstdlib>
#include <iostream>

#include "../csnn/nn_options.h"
using namespace std;

class HypherParameters{
public:
	static void Error(string x){
		cerr << x << endl;
		cout << x << endl;
		exit(1);
	}
public:
	nn_options hp_nn;	//used by nn
public:
//1.0
int CONF_method;	//which method
//1.1-files
string CONF_train_file;	//the training file
string CONF_dev_file;	//dev files
string CONF_test_file;	//test files
string CONF_output_file;
string CONF_gold_file;	//golden files
//1.2-other files
string CONF_dict_file;		//for dictionary
string CONF_traindict_file;		//outside dictionary file for training
string CONF_mach_name;		//mach name
string CONF_mach_cur_suffix;
string CONF_mach_best_suffix;
//1.3-some training criteria
double CONF_NN_LRATE;
int CONF_NN_ITER;
int CONF_NN_ITER_decrease;		//at lease cut lrate this times when stopping(so real iters maybe more than iter)
int CONF_NN_ITER_force_half;	//force cut half if no cutting for how many iters
double CONF_NN_LMULT;			//-1~0: schedule rate
double CONF_NN_WD;				//weight decay

int CONF_UPDATE_WAY;	//see nn_math.h: OPT_*
int CONF_NESTEROV_MOMENTUM;	//set to 1 if update with it firstly
double CONF_MOMENTUM_ALPHA;
double CONF_RMS_SMOOTH;

double CONF_NN_resample;				//re-sample rate
int CONF_NN_FIXVEC;						//no update on embeddings any longer
int CONF_minibatch;		//(this is not bsize of nn) -- if>0, num of sent; if<0, at least this-num-of-inst

//1.4-others
int CONF_dict_remove;	//remove words appears only less than this times
int CONF_random_seed;
int CONF_labeled;		//labeled or not

//1.5-scores
int CONF_score_prob;	//whether give transform score, only for M1 (0,1)
double CONF_score_o1filter_cut;
string CONF_score_mach_fo1;		//o1-filter mach
string CONF_score_mach_so1;		//o1-scorer mach
string CONF_score_mach_so2sib;	//o2sib-scorer mach
string CONF_score_mach_so3g;
double CONF_higho_percent;		//sample the 'wrong' factors in 2/3 order
int CONF_score_noc_dev;	//no combine scores when dev
double CONF_score_o1scale;
double CONF_score_o2scale;

double CONF_score_p2reg;	//reg directly for scores

//1.6-others
//int CONF_p1o1_training_random;	//special random one for M1_p1o1
string CONF_embed_WL;	//word-list for embedding
string CONF_embed_EM;	//embeddings
double CONF_embed_ISCALE;
int CONF_NN_untied_changeto;	//later change to untied ones
int CONF_NN_untied_changetoiter;//when to change
string CONF_o1filter_file;		//stored file for o1filter
int CONF_higho_toolong;	//when training, skip long sentences
int CONF_higho_tooshort;	//when training,skip short sentences for po3g

//1.7-perceptron
//!!what is re-used(even with different meanings) in pr-method
//1.file names: fo1,so1,so2sib and others
//2.training: CONF_NN_ITER,CONF_NN_resample,CONF_minibatch<1 for online>,CONF_NN_WD
//3.others: o1filter_file
// mach name to write at last
string CONF_pr_macho1;
string CONF_pr_macho2;
string CONF_pr_macho3;

int CONF_pr_initway;
REAL CONF_pr_alpha;		//maybe lrate, but does not change

int CONF_pr_devavrage;	//average when dev

//1.8 - margin
REAL CONF_margin;	//margin for both prob or max-margin

//1.9 - about batch-div when updating --- assuming sqrt-yes
int CONF_mbatch_way;

//init
HypherParameters(string conf):hp_nn()
{
	//defaults
	CONF_output_file = "output.txt";
	CONF_dict_file="vocab.dict";
	CONF_mach_name="nn.mach";
	CONF_mach_cur_suffix=".curr";
	CONF_mach_best_suffix=".best";
	CONF_NN_LRATE=0.1;
	CONF_NN_ITER=12;	//maybe more iters
	CONF_NN_ITER_decrease=1;
	CONF_NN_ITER_force_half=3;	//usually cut three times
	CONF_NN_LMULT=-0.5;
	CONF_NN_WD=1e-4;
	CONF_UPDATE_WAY=1;	//opt_sgd
	CONF_NESTEROV_MOMENTUM=0;
	CONF_MOMENTUM_ALPHA=0.6;	//??
	CONF_RMS_SMOOTH=0.9;		//??
	CONF_NN_resample=0.95;
	CONF_NN_FIXVEC=100;			//100 iters later??
	CONF_minibatch=-256;		//-256: at least 256 instances
	CONF_dict_remove=3;
	CONF_random_seed=12345;
	CONF_labeled=1;
	CONF_score_prob=1;
	CONF_score_o1filter_cut=1e-4;
	CONF_score_mach_so1="";
	CONF_score_mach_so2sib="";
	CONF_score_mach_so3g="";
	CONF_higho_percent=1.0;
	CONF_score_noc_dev=1;
	CONF_score_o1scale=1;
	CONF_score_o2scale=1;
	CONF_score_p2reg=1e-3;

//	CONF_p1o1_training_random=0;
	CONF_embed_WL="";
	CONF_embed_EM="";
	CONF_embed_ISCALE=1;
	CONF_NN_untied_changeto=0;
	CONF_NN_untied_changetoiter=-1; //nope
	CONF_higho_toolong = 150;
	CONF_higho_tooshort = 1;	//--only for mp2o3

	CONF_pr_macho1 = "pro1.mach";
	CONF_pr_macho2 = "pro2.mach";
	CONF_pr_macho3 = "pro3.mach";
	CONF_pr_initway = 0;
	CONF_pr_alpha = 1;
	CONF_pr_devavrage = 1;

	CONF_traindict_file = "";	//!!new-file

	CONF_margin = 0;

	CONF_mbatch_way = 0;

	//read in conf-file
#define DATA_LINE_LEN 10000
	ifstream fin(conf.c_str());
	cout << "Dealing configure file '" << conf << "'" << endl;
	// Method config
	string temp_for_m;
	fin >> temp_for_m;
	if(temp_for_m != "M")
		Error("First of conf-file must be M.");
	fin >> CONF_method;
	//
	while(!fin.eof()){
		string buf;
		char line[DATA_LINE_LEN];
		fin >> buf;
		if (buf=="") continue; // HACK
		if (buf[0]=='#') {fin.getline(line, DATA_LINE_LEN); continue;} // skip comments
		//1.1 and 1.2: file names
		if(buf=="train")		fin >> CONF_train_file;
		else if(buf=="dev")		fin >> CONF_dev_file;
		else if(buf=="test")	fin >> CONF_test_file;
		else if(buf=="output")	fin >> CONF_output_file;
		else if(buf=="gold")	fin >> CONF_gold_file;
		else if(buf=="dict")	fin >> CONF_dict_file;
		else if(buf=="tdict")	fin >> CONF_traindict_file;		//!!new-file
		else if(buf=="mach-prefix") fin >> CONF_mach_name;
		//1.3
		else if(buf=="nn_lrate") fin >> CONF_NN_LRATE;
		else if(buf=="nn_iters") fin >> CONF_NN_ITER;
		else if(buf=="nn_iters_dec") fin >> CONF_NN_ITER_decrease;
		else if(buf=="nn_iters_force") fin >> CONF_NN_ITER_force_half;
		else if(buf=="nn_lmult") fin >> CONF_NN_LMULT;
		else if(buf=="nn_wd")	 fin >> CONF_NN_WD;

		else if(buf=="nn_way")		fin >> CONF_UPDATE_WAY;
		else if(buf=="nn_nesterov")	fin >> CONF_NESTEROV_MOMENTUM;
		else if(buf=="nn_momentum")	fin >> CONF_MOMENTUM_ALPHA;
		else if(buf=="nn_rms")		fin >> CONF_RMS_SMOOTH;

		else if(buf=="nn_resample") fin >> CONF_NN_resample;
		else if(buf=="nn_fixv")		fin >> CONF_NN_FIXVEC;
		else if(buf=="nn_mbatch") 	fin >> CONF_minibatch;
		//1.4-others
		else if(buf=="o_removes")	fin >> CONF_dict_remove;
		else if(buf=="o_srand")		fin >> CONF_random_seed;
		else if(buf=="o_labeled")	fin >> CONF_labeled;
		//1.5-scores
		else if(buf=="s_prob") 		fin >> CONF_score_prob;
		else if(buf=="s_fo1_cut") 	fin >> CONF_score_o1filter_cut;
//		else if(buf=="s_so1_combine")	fin >> CONF_score_combine_o1;
//		else if(buf=="s_so2sib_combine")	fin >> CONF_score_combine_o2sib;
		else if(buf=="s_mach_fo1")	fin >> CONF_score_mach_fo1;
		else if(buf=="s_mach_so1")	fin >> CONF_score_mach_so1;
		else if(buf=="s_mach_so2sib")	fin >> CONF_score_mach_so2sib;
		else if(buf=="s_mach_so3g")	fin >> CONF_score_mach_so3g;
		else if(buf=="s_higho_per")	fin >> CONF_higho_percent;
		else if(buf=="s_noc_dev")	fin >> CONF_score_noc_dev;
		else if(buf=="s_scale_o1") fin >> CONF_score_o1scale;
		else if(buf=="s_scale_o2")	fin >> CONF_score_o2scale;
		else if(buf=="s_p2_reg")	fin >> CONF_score_p2reg;

		//1.x -special : for nn_option
		else if(buf=="n_wsize")		fin >> hp_nn.NN_wsize;
		else if(buf=="n_psize")		fin >> hp_nn.NN_psize;
		else if(buf=="n_dsize")		fin >> hp_nn.NN_dsize;
		else if(buf=="n_win")		fin >> hp_nn.NN_win;
		else if(buf=="n_adda")		fin >> hp_nn.NN_add_average;
		else if(buf=="n_adds")		fin >> hp_nn.NN_add_sent;
		else if(buf=="n_untied")	fin >> hp_nn.NN_untied_dim;
		//[nope]else if(buf=="n_untied_2r")	fin >> hp_nn.NN_untied_2brate;
		else if(buf=="n_act")		fin >> hp_nn.NN_act;
		else if(buf=="n_hidden")	fin >> hp_nn.NN_hidden_size;
		else if(buf=="n_wr")		fin >> hp_nn.NN_wrsize;
		else if(buf=="n_sr")		fin >> hp_nn.NN_srsize;
		else if(buf=="n_drop")		fin >> hp_nn.NN_dropout;
		else if(buf=="n_init_wfio")	fin >> hp_nn.NN_init_wb_faniorange;	//if <= 0, no-fanio,just brange
		else if(buf=="n_init_wb")	fin >> hp_nn.NN_init_wb_brange;
		else if(buf=="n_init_wv")	fin >> hp_nn.NN_init_wvrange;
		else if(buf=="n_drop_r")	fin >> hp_nn.NN_dropout_repr;
		else if(buf=="n_act_r")		fin >> hp_nn.NN_act_repr;

		else if(buf=="n_sdsize")	fin >> hp_nn.NN_sdsize;
		else if(buf=="n_sl_way")	fin >> hp_nn.NN_sl_way;
		else if(buf=="n_sl_filter")	fin >> hp_nn.NN_sl_filter;
		else if(buf=="n_sl_nobias")	fin >> hp_nn.NN_sl_dnobias;

		//1.6-others
//		else if(buf=="M1_train_random")		fin >> CONF_p1o1_training_random;
		else if(buf=="embed_wl")	fin >> CONF_embed_WL;
		else if(buf=="embed_em")	fin >> CONF_embed_EM;
		else if(buf=="embed_scale")	fin >> CONF_embed_ISCALE;
		else if(buf=="untied_change_way")	fin >> CONF_NN_untied_changeto;
		else if(buf=="untied_change_iter")	fin >> CONF_NN_untied_changetoiter;
		else if(buf=="filter_file")	fin >> CONF_o1filter_file;
		else if(buf=="o3_toolong")	fin >> CONF_higho_toolong;
		else if(buf=="o3_tooshort") fin >> CONF_higho_tooshort;

		//1.7-pr
		else if(buf=="p_o1name")	fin >> CONF_pr_macho1;
		else if(buf=="p_o2name")	fin >> CONF_pr_macho2;
		else if(buf=="p_o3name")	fin >> CONF_pr_macho3;
		else if(buf=="p_init")		fin >> CONF_pr_initway;
		else if(buf=="p_alpha")		fin >> CONF_pr_alpha;
		else if(buf=="p_devavr")		fin >> CONF_pr_devavrage;

		//1.8-margin
		else if(buf=="m_margin")	fin >> CONF_margin;

		//1.9-...
		else if(buf == "mb_way")	fin >> CONF_mbatch_way;

		else
			cerr << "Unknown option " << buf << endl;
	}
}

};



#endif /* PARTS_HYPHERPARAMETERS_H_ */
