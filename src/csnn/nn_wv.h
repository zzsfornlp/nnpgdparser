/*
 * nn_wv.h
 *
 *  Created on: Sep 19, 2015
 *      Author: zzs
 */

#ifndef CSNN_NN_WV_H_
#define CSNN_NN_WV_H_

#include "nn_math.h"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include<boost/unordered_set.hpp>
typedef boost::unordered_set<int> IntSet;

//the word-vector parameter: w
// -- notice that the behavior of nn_wv and nn_wb are quite different, so don't add a base class
class nn_wv{
private:
	bool updating;
	int num;	//num of words
	int dim;	//dimension

	//special methods
	static const int HIT_INDEX_SIZE = 10000;
	IntSet* hit_index;
	REAL* w;
	REAL* w_grad;	//gradient
	REAL* w_moment;	//momentum
	REAL* w_square;	//square for AdaGrad

	void init_clear(){
		memset(w_grad,0,sizeof(REAL)*num*dim);
		memset(w_moment,0,sizeof(REAL)*num*dim);
		//memset(w_square,0,sizeof(REAL)*num*dim);
		for (int i = 0; i<num*dim; i++)	//set square-sum to 1 at start
			w_square[i] = 1;
	}

public:
	nn_wv(int n,int d):updating(true),num(n),dim(d){
		int all = n*d;	//int is enough
		hit_index = new IntSet(HIT_INDEX_SIZE);
		w = new REAL[all];
		w_grad = new REAL[all];
		w_moment = new REAL[all];
		w_square = new REAL[all];
	}
	void get_init(const REAL range){	//random initialize
		REAL c=range*2.0;
		for (int i=0; i<num*dim; i++)
			w[i]=c*(drand48()-0.5);
		init_clear();
	}
	void get_init_one(int index,REAL* value){	//init one
		if(index < 0 || index >= num)
			nn_math::CHECK_EQUAL(index,0,"Out of range init.");
		memcpy(w+index*dim,value,sizeof(REAL)*dim);
	}
	void clear_grad(){
		for(IntSet::iterator i = hit_index->begin();i!=hit_index->end();i++){
			int index = *i;
			memset(w_grad+index*dim,0,sizeof(REAL)*dim);
		}
		hit_index->clear();
	}
	~nn_wv(){
		delete []w;
		delete []w_grad;
		delete []w_moment;
		delete []w_square;
		delete hit_index;
	}
	bool need_updating(){return updating;}
	void set_updating(bool x){updating = x;}
	int getn(){return num;}
	int getd(){return dim;}

	//three important operations
	//forward and backward only consider one wv
	void forward(int index,REAL* out,int adding=0);
	void backward(int index,const REAL* grad);
	void update(int way,REAL lrate,REAL wdecay,REAL m_alpha,REAL rms_smooth,int mbsize);

	//binary r/w
	nn_wv(std::ifstream &fin):updating(true){
		fin.read((char*)&num,sizeof(int));
		fin.read((char*)&dim,sizeof(int));
		int all = num*dim;	//int is enough
		hit_index = new IntSet(HIT_INDEX_SIZE);
		w = new REAL[all];
		w_grad = new REAL[all];
		w_moment = new REAL[all];
		w_square = new REAL[all];
		fin.read((char*)w,sizeof(REAL)*all);
		init_clear();
	}
	void write_params(std::ofstream &fout){
		fout.write((char*)&num,sizeof(int));
		fout.write((char*)&dim,sizeof(int));
		fout.write((char*)w,sizeof(REAL)*num*dim);	//!!DEBUG, don't need &
	}

	//only for gradient check
	REAL* get_w(int index){return &w[index];}
	REAL get_g(int index){return w_grad[index];}
};



#endif /* CSNN_NN_WV_H_ */
