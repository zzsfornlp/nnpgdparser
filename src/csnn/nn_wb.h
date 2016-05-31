/*
 * nn_param.h
 *
 *  Created on: Sep 19, 2015
 *      Author: zzs
 */

#ifndef CSNN_NN_WB_H_
#define CSNN_NN_WB_H_

#include "nn_math.h"
#include <cstdlib>
#include <cstring>
#include <fstream>

//the linear parameter: w and b (weight and bias)
// two ways of init: 1:nn_wb(i,o);get_init(range);	2.nn_wb(fin)
class nn_wb{
private:
	bool updating;
	int idim;
	int odim;
	bool nobias;

	REAL* w;	//o*i
	REAL* b;	//o
	//gradient calculated
	REAL* w_grad;
	REAL* b_grad;
	//gradient momentum
	REAL* w_moment;
	REAL* b_moment;
	//gradient accumulated square --- for AdaGrad
	REAL* w_square;
	REAL* b_square;

	//init for starting or reading
	void init_clear(){
		memset(w_grad,0,sizeof(REAL)*idim*odim);
		memset(w_moment,0,sizeof(REAL)*idim*odim);
		//memset(w_square,0,sizeof(REAL)*idim*odim);
		for(int i=0;i<idim*odim;i++)	//set square-sum to 1 at start
			w_square[i] = 1;
		memset(b_grad,0,sizeof(REAL)*odim);
		memset(b_moment,0,sizeof(REAL)*odim);
		//memset(b_square,0,sizeof(REAL)*odim);
		for(int i=0;i<odim;i++)
			b_square[i] = 1;
		if(nobias)
			memset(b,0,sizeof(REAL)*odim);
	}

public:
	nn_wb(int i,int o,bool no=false):updating(true),idim(i),odim(o),nobias(no){
		int all = i*o;	//int is enough
		w = new REAL[all];
		b = new REAL[o];
		w_grad = new REAL[all];
		b_grad = new REAL[o];
		w_moment = new REAL[all];
		b_moment = new REAL[o];
		w_square = new REAL[all];
		b_square = new REAL[o];
	}
	nn_wb(nn_wb &x):updating(x.updating),idim(x.idim),odim(x.odim),nobias(x.nobias){
		int i = x.idim;
		int o = x.odim;
		int all = i*o;	//int is enough
		w = new REAL[all];
		b = new REAL[o];
		w_grad = new REAL[all];
		b_grad = new REAL[o];
		w_moment = new REAL[all];
		b_moment = new REAL[o];
		w_square = new REAL[all];
		b_square = new REAL[o];
		nobias = x.nobias;
		init_clear();
		//only copy w and b, right ??
		memcpy(w,x.w,sizeof(REAL)*all);
		memcpy(b,x.b,sizeof(REAL)*o);
		//and copy others
		memcpy(w_moment,x.w_moment,sizeof(REAL)*all);
		memcpy(b_moment,x.b_moment,sizeof(REAL)*o);
		memcpy(w_square,x.w_square,sizeof(REAL)*all);
		memcpy(b_square,x.b_square,sizeof(REAL)*o);
		if(nobias)
			memset(b,0,sizeof(REAL)*odim);
	}
	void get_init(const REAL frange,const REAL range){
		//fanio for weight and random for bias
		REAL c=0;
		if(frange <= 0)
			c = range*2;	//!!different meaning
		else
			c = 2.0*frange/sqrt((REAL) (idim+odim));
		for (int i=0; i<idim*odim; i++)
			w[i]=c*(drand48()-0.5);
		c=range*2.0;
		for (int i=0; i<odim; i++)
			b[i]=c*(drand48()-0.5);
		init_clear();
	}
	void clear_grad(){
		memset(w_grad,0,sizeof(REAL)*idim*odim);
		memset(b_grad,0,sizeof(REAL)*odim);
	}
	~nn_wb(){
		delete []w;
		delete []b;
		delete []w_grad;
		delete []b_grad;
		delete []w_moment;
		delete []b_moment;
		delete []w_square;
		delete []b_square;
	}
	bool need_updating(){return updating;}
	void set_updating(bool x){updating = x;}
	int geti(){return idim;}
	int geto(){return odim;}

	//three important operations
	//- the bsize of f/b only means the instances for one pass (one sentences)
	void forward(/*const*/REAL* in,REAL* out,int bsize);							//forward setting
	void backward(/*const*/REAL* ograd,REAL* igrad,/*const*/REAL* in,int bsize);	//backward accumulate
	void update(int way,REAL lrate,REAL wdecay,REAL m_alpha,REAL rms_smooth,int mbsize);
	void nesterov_update(int way,REAL m_alpha);

	//binary r/w
	nn_wb(std::ifstream &fin):updating(true){
		fin.read((char*)&nobias,sizeof(bool));
		fin.read((char*)&idim,sizeof(int));
		fin.read((char*)&odim,sizeof(int));
		int all = idim*odim;	//int is enough
		w = new REAL[all];
		b = new REAL[odim];
		w_grad = new REAL[all];
		b_grad = new REAL[odim];
		w_moment = new REAL[all];
		b_moment = new REAL[odim];
		w_square = new REAL[all];
		b_square = new REAL[odim];
		fin.read((char*)w,sizeof(REAL)*all);	//!!DEBUG, don't need &
		fin.read((char*)b,sizeof(REAL)*odim);
		init_clear();
	}
	void write_params(std::ofstream &fout){
		fout.write((char*)&nobias,sizeof(bool));
		fout.write((char*)&idim,sizeof(int));
		fout.write((char*)&odim,sizeof(int));
		fout.write((char*)w,sizeof(REAL)*idim*odim);	//!!DEBUG, don't need &
		fout.write((char*)b,sizeof(REAL)*odim);
	}

	//only for gradient check
	REAL* get_wb(int index,bool isw){return isw ? &w[index] : &b[index];}
	REAL get_g(int index,bool isw){return isw ? w_grad[index] : b_grad[index];}

public:
	//for perceptron
	void update_pr(REAL* inputs,int row,REAL a);
	void add_w(nn_wb* x);
	void div_w(REAL a);

	void set_w(int column,int row,REAL x){
		w[column*odim+row] = x;	//!! COLUMN major
	}
};



#endif /* CSNN_NN_WB_H_ */
