/*
 * sl_part.h
 *
 *  Created on: Dec 4, 2015
 *      Author: zzs
 */

#ifndef CSNN_SL_PART_H_
#define CSNN_SL_PART_H_

#include "nn_cache.h"
#include "nn_input.h"
#include "nn_math.h"
#include "nn_wb.h"
#include "nn_wv.h"
#include <vector>
#include <string>
#include <fstream>

#include "nn_options.h"
using namespace std;

//Sentence-Level part

class sl_part{
	const static int SL_VALID = 67890;
protected:
	nn_options* op;
	int order;
	bool updating;
	nn_wb* p_main;	//main parameters
	nn_wb* p_dist;	//about distance info

	//from outside
	const nn_input* this_input;
	nn_wv* d_word;
	nn_wv* d_pos;
	nn_wv* d_ds;	//remember this is sl-distance

	//caches
	REAL* c_input_wp;	//for input words and pos	(filter_num*(w+p)*len)
	REAL* c_input_dist;	//for input distances		((order+1)*sd*len*num_inst)
	REAL* c_output_wp;	//srsize*len
	REAL* c_output_dist;//srsize*len*num_inst
	//[nope, directly use outside one]REAL* c_max;		//srsize*num_inst --- after max-pooling
	REAL* c_output_tmp;	//before max-pooling
	int * c_which;		//which one is max (srsize*num_inst, [0,len))

	void the_init(nn_options* o,int the_order,nn_wv* dw,nn_wv* dp,nn_wv* dd){
		op = o;
		order = the_order;
		this_input = 0;
		c_input_wp = c_input_dist = 0;
		c_output_wp = c_output_dist = 0;
		c_output_tmp = 0;
		c_which = 0;
		d_word=dw;d_pos=dp;d_ds=dd;
	}

public:
	bool need_updating(){return updating;}
	void set_updating(bool x){updating = x;}

	//--------------------------------init-------------------------------------
	sl_part(nn_options* o,int the_order,nn_wv* dw,nn_wv* dp,nn_wv* dd):updating(true){
		the_init(o,the_order,dw,dp,dd);
		//init parameters
		int tmp_idim = op->NN_sl_filter * (op->NN_wsize+op->NN_psize);
		p_main = new nn_wb(tmp_idim,op->NN_srsize);
		p_main->get_init(op->NN_init_wb_faniorange,op->NN_init_wb_brange);
		tmp_idim = (order+1) * op->NN_sdsize;
		bool TMP_nobias = op->NN_sl_dnobias;
		p_dist = new nn_wb(tmp_idim,op->NN_srsize,TMP_nobias);	//nobias for p_dist
		p_dist->get_init(op->NN_init_wb_faniorange,op->NN_init_wb_brange);
	}
	sl_part(std::ifstream &fin,nn_options* o,int the_order,nn_wv* dw,nn_wv* dp,nn_wv* dd):updating(true){
		the_init(o,the_order,dw,dp,dd);
		//init parameters
		int TMP_valid = 0;
		fin.read((char*)&TMP_valid,sizeof(int));
		nn_math::CHECK_EQUAL(SL_VALID,TMP_valid,"Wrong when reading sl_part");
		p_main = new nn_wb(fin);
		p_dist = new nn_wb(fin);
	}
	void write_params(std::ofstream &fout){
		int TMP_valid = SL_VALID;
		fout.write((char*)&TMP_valid,sizeof(int));
		p_main->write_params(fout);
		p_dist->write_params(fout);
	}

	//---------------------------------routines------------------------------------
	//- the bsize of f/b only means the instances for one pass (one sentences)
	void forward(nn_input* inputs,REAL* out);							//forward setting
	void backward(/*const*/REAL* ograd);	//backward accumulate

	void update(int way,REAL lrate,REAL wdecay,REAL m_alpha,REAL rms_smooth,int mbsize){
		p_main->update(way,lrate,wdecay,m_alpha,rms_smooth,mbsize);
		p_dist->update(way,lrate,wdecay,m_alpha,rms_smooth,mbsize);
	}
	void nesterov_update(int way,REAL m_alpha){}//nope

	void clear_grad(){
		p_main->clear_grad();
		p_dist->clear_grad();
	}

	//--------------------for gradient check---------------------------------------
	nn_wb* get_pmain(){return p_main;}
	nn_wb* get_pdist(){return p_dist;}
};


#endif /* CSNN_SL_PART_H_ */
