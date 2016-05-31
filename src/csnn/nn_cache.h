/*
 * nn_cache.h
 *
 *  Created on: Sep 17, 2015
 *      Author: zzs
 */

#ifndef CSNN_NN_CACHE_H_
#define CSNN_NN_CACHE_H_

/*
 * the so-called layers, basically just two arrays of bsize*length
 * -- values and gradients
 */

#include "nn_math.h"
#include <cstdlib>
#include <vector>
using std::vector;

class nn_cache{
private:
	//values and gradients AT LEAST have size bsize*length
	long bsize;		//mini-batch size
	long length;	//length of one instance
	REAL* values;
	REAL* gradients;
	REAL* dropout;

	REAL* values_before;	//for those activation methods which need original values
public:
	nn_cache(long b,long l):bsize(b),length(l),values(0),gradients(0),dropout(0),values_before(0){
		long all = bsize*length;
		if(length>0)
			dropout = new REAL[length];	//dropout
		if(all > 0){
			values = new REAL[all];
			gradients = new REAL[all];
			values_before = new REAL[all];
		}
	}
	~nn_cache(){
		delete []values;
		delete []gradients;
		delete []dropout;
		delete []values_before;
	}
	int get_len() {return length;}
	REAL* get_values_before(){return values_before;}
	REAL* get_values(){return values;}
	REAL* get_gradients(){return gradients;}
	REAL* get_dropout(){return dropout;}
	void gen_dropout(REAL rate){
		for(long i=0;i<length;i++)
			dropout[i] = (drand48()<rate) ? 0 : 1;	//0 means to perform dropout
	}
	//!!!!! here only clear the needed size because the bsize only increase!!!!!
	void clear_values(int need_bs){
		for(long i=0;i<need_bs*length;i++)
			values[i] = 0;
	}
	void clear_gradients(int need_bs){
		for(long i=0;i<need_bs*length;i++)
			gradients[i] = 0;
	}
	void clear_all(int need_bs){
		clear_values(need_bs);
		clear_gradients(need_bs);
	}
	//!! delete old data when expand
	void resize(long b){
		long old_all = bsize*length;
		long all = b*length;
		if(all > old_all){
			bsize = b;
			delete []values;
			delete []gradients;
			delete []dropout;
			delete []values_before;
			values = new REAL[all];
			gradients = new REAL[all];
			dropout = new REAL[length];
			values_before = new REAL[all];
		}
	}

	//combine and dispatch
	void combine_cache_value(vector<nn_cache*> list,int this_bsize){
		int offset = 0;
		for(unsigned int i=0;i<list.size();i++){
			nn_cache* tmp = list[i];
			for(int t=0;t<this_bsize;t++)
				memcpy(values+length*t+offset,tmp->values+tmp->length*t,tmp->length*sizeof(REAL));
			offset += tmp->length;
		}
	}
	void dispatch_cache_grad(vector<nn_cache*> list,int this_bsize){
		int offset = 0;
		for(unsigned int i=0;i<list.size();i++){
			nn_cache* tmp = list[i];
			for(int t=0;t<this_bsize;t++)
				memcpy(tmp->gradients+tmp->length*t,gradients+length*t+offset,tmp->length*sizeof(REAL));
			offset += tmp->length;
		}
	}
	//active or back-grad
	void activate(int which,int bs,REAL drop_rate,int testing){
		nn_math::act_f(which,values,bs*length,values_before);
		if(drop_rate > 0){
			if(testing){
				nn_math::op_y_mult_a(bs*length,values,1-drop_rate);
			}
			else{
				for(int i=0;i<bs*length;i++)
					if(dropout[i%length]==0)
						values[i] = 0;
			}
		}
	}
	void backgrad(int which,int bs,REAL drop_rate){
		nn_math::act_b(which,values,gradients,bs*length,values_before);
		if(drop_rate > 0){
			//no testing
			for(int i=0;i<bs*length;i++)
				if(dropout[i%length]==0)
					gradients[i] = 0;
		}
	}
	void calc_softmax(int bs){
		for(int b=0;b<bs;b++){
		    // Get the maximum value of data_out on row b
		    REAL max = values[b*length];
		    for (int i=1; i<length; i++) {
		      REAL x = values[b*length + i];
		      if (x > max)
		        max = x;
		    }
		    // Compute exp(x - max) inplace for each x in row b of data_out, and their sum
		    REAL sum_exp = 0.;
		    for (int i=0; i<length; i++) {
		      REAL exp_x = exp(values[b*length + i] - max);
		      sum_exp += exp_x;
		      values[b*length + i] = exp_x;
		    }
		    // Normalize the row, dividing all values by sum_exp
		    for (int i=0; i<length; i++) {
		    	values[b*length + i] /= sum_exp;
		    }
		}
	}
};

#endif /* CSNN_NN_CACHE_H_ */
