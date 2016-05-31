/*
 * options.h
 *
 *  Created on: Sep 25, 2015
 *      Author: zzs
 */

#ifndef CSNN_NN_OPTIONS_H_
#define CSNN_NN_OPTIONS_H_

#include <fstream>
#include "nn_math.h"
//options related to nn
class nn_options{
public:
	int NN_wnum;	//number of words
	int NN_pnum;	//number of pos
	int NN_dnum;	//number of distances
	int NN_out_prob;		//whether add softmax
	int NN_out_size;		//output size
	//------------------THOSE above need setting before training------------//

	int NN_wsize;	//word embed size
	int NN_psize;	//pos embed size
	int NN_dsize;	//distance embed size

	int NN_win;		//window size

	int NN_add_average;		//whether add average feature
	int NN_add_sent;		//whether add sentence features

	enum{
		NN_UNTIED_NOPE,NN_UNTIED_M,NN_UNTIED_H,NN_UNTIED_HM
	};
	int NN_untied_dim;		//0,1,2,3: 0 means no untied, 1 means based on m, 2 means on h, 3 h-m
	// [NOPE] REAL NN_untied_2brate;	//when dim==2, maybe need some back-off with random when training

	int NN_act;				//the activation function
	int NN_hidden_size;		//hidden size(near output)
	int NN_wrsize;			//word repr size
	int NN_srsize;			//sentence repr size

	REAL NN_dropout;		//dropout rate

	REAL NN_init_wb_faniorange;	//fanio for w range
	REAL NN_init_wb_brange;		//random init for b range
	REAL NN_init_wvrange;

	//special dropout and activation for the first layer
	REAL NN_dropout_repr;		//!!!! sepecial meaning when <0, really bad choice, but ...
	int NN_act_repr;

	//###FOR SENT-LEVEL REPR###
	// -- before: NN_add_sent; NN_srsize;
	enum{	//max-pooling or average-pooling
		NN_SL_ADDING_M,NN_SL_TANHMUL_M,NN_SL_ADDING_A,NN_SL_TANHMUL_A
	};
	int NN_sl_way;
	int NN_sdnum;	//number of sl-distance	//------------------need setting before training------------//
	int NN_sdsize;	//sl-distance embed size
	int NN_sl_filter;

	//-----------DONT'T NEED TO WRITE (ONLY INIT)-----------
	int NN_sl_dnobias;

//------------------------------------------------------------------------------------
	//---size for repr layer
	int get_NN_srsize(){
		if(NN_add_sent)
			return NN_srsize;
		else
			return 0;
	}
	int get_NN_rsize(){					//the size of representation layer = wr+sr
		return NN_wrsize+get_NN_srsize();
	}
	int get_NN_wv_wrsize(int order){	//the word vectors' size before rsize
		int basis = (NN_wsize+NN_psize)*NN_win*(order+1)+NN_dsize*(order);	//!DEBUG:order+1
		if(NN_add_average)
			basis += order*(NN_wsize+NN_psize);
		return basis;
	}

	//init and r/w
	void init_check(){
		if(NN_dropout_repr<0)
			NN_dropout_repr = NN_dropout;
		if(NN_act_repr<0)
			NN_act_repr = NN_act;
	}
	nn_options(){default_init();init_check();}
	nn_options(std::ifstream &fin){default_init();read(fin);init_check();}

	void default_init(){
		//!! need setting !!
		//NN_wnum = 50000;
		//NN_pnum = 50;
		//NN_dnum = 20;
		//NN_out_prob = ?;		//whether add softmax
		//NN_out_size = ?;		//output size

		//embedding size
		NN_wsize = 50;
		NN_psize = 50;
		NN_dsize = 50;
		//nn
		NN_win = 7;

		NN_add_average = 0;
		NN_add_sent = 1;

		NN_untied_dim = 0;
		//[nope]NN_untied_2brate = 0.2;

		NN_act = 0;		//ACT_TANH
		NN_hidden_size = 100;
		NN_wrsize = 200;
		NN_srsize = 100;

		NN_dropout = 0;

		NN_init_wb_faniorange = 1;
		NN_init_wb_brange = 0.1;
		NN_init_wvrange = 0.1;

		//default as NN_act and NN_dropout
		NN_dropout_repr=0;
		NN_act_repr=0;

		//###SENTENCE-LEVEL###
		NN_sl_way = 0;
		//NN_sdnum = ?;	//!! need setting !!
		NN_sdnum = 100;	//or setting
		NN_sdsize = 50;
		NN_sl_filter = 3;

		//-----------DONT'T NEED TO WRITE (ONLY INIT)-----------
		NN_sl_dnobias = 1;
	}
	//
	void read(std::ifstream &fin){
		fin.read((char*)&NN_wnum,sizeof(int));
		fin.read((char*)&NN_pnum,sizeof(int));
		fin.read((char*)&NN_dnum,sizeof(int));
		fin.read((char*)&NN_out_prob,sizeof(int));
		fin.read((char*)&NN_out_size,sizeof(int));

		fin.read((char*)&NN_wsize,sizeof(int));
		fin.read((char*)&NN_psize,sizeof(int));
		fin.read((char*)&NN_dsize,sizeof(int));

		fin.read((char*)&NN_win,sizeof(int));

		fin.read((char*)&NN_add_average,sizeof(int));
		fin.read((char*)&NN_add_sent,sizeof(int));

		fin.read((char*)&NN_untied_dim,sizeof(int));
		//[nope]fin.read((char*)&NN_untied_2brate,sizeof(REAL));

		fin.read((char*)&NN_act,sizeof(int));
		fin.read((char*)&NN_hidden_size,sizeof(int));
		fin.read((char*)&NN_wrsize,sizeof(int));
		fin.read((char*)&NN_srsize,sizeof(int));

		fin.read((char*)&NN_dropout,sizeof(REAL));

		fin.read((char*)&NN_init_wb_faniorange,sizeof(REAL));
		fin.read((char*)&NN_init_wb_brange,sizeof(REAL));
		fin.read((char*)&NN_init_wvrange,sizeof(REAL));

		fin.read((char*)&NN_dropout_repr,sizeof(REAL));
		fin.read((char*)&NN_act_repr,sizeof(int));

		fin.read((char*)&NN_sl_way,sizeof(int));
		fin.read((char*)&NN_sdnum,sizeof(int));
		fin.read((char*)&NN_sdsize,sizeof(int));
		fin.read((char*)&NN_sl_filter,sizeof(int));
	}
	void write(std::ofstream &fout){
		fout.write((char*)&NN_wnum,sizeof(int));
		fout.write((char*)&NN_pnum,sizeof(int));
		fout.write((char*)&NN_dnum,sizeof(int));
		fout.write((char*)&NN_out_prob,sizeof(int));
		fout.write((char*)&NN_out_size,sizeof(int));

		fout.write((char*)&NN_wsize,sizeof(int));
		fout.write((char*)&NN_psize,sizeof(int));
		fout.write((char*)&NN_dsize,sizeof(int));

		fout.write((char*)&NN_win,sizeof(int));

		fout.write((char*)&NN_add_average,sizeof(int));
		fout.write((char*)&NN_add_sent,sizeof(int));

		fout.write((char*)&NN_untied_dim,sizeof(int));
		//[nope]fout.write((char*)&NN_untied_2brate,sizeof(REAL));

		fout.write((char*)&NN_act,sizeof(int));
		fout.write((char*)&NN_hidden_size,sizeof(int));
		fout.write((char*)&NN_wrsize,sizeof(int));
		fout.write((char*)&NN_srsize,sizeof(int));

		fout.write((char*)&NN_dropout,sizeof(REAL));

		fout.write((char*)&NN_init_wb_faniorange,sizeof(REAL));
		fout.write((char*)&NN_init_wb_brange,sizeof(REAL));
		fout.write((char*)&NN_init_wvrange,sizeof(REAL));

		fout.write((char*)&NN_dropout_repr,sizeof(REAL));
		fout.write((char*)&NN_act_repr,sizeof(int));

		fout.write((char*)&NN_sl_way,sizeof(int));
		fout.write((char*)&NN_sdnum,sizeof(int));
		fout.write((char*)&NN_sdsize,sizeof(int));
		fout.write((char*)&NN_sl_filter,sizeof(int));
	}
};

#endif /* CSNN_NN_OPTIONS_H_ */
