/*
 * nn_input.h
 *
 *  Created on: Sep 17, 2015
 *      Author: zzs
 */

#ifndef CSNN_NN_INPUT_H_
#define CSNN_NN_INPUT_H_

/*
 * 	specified input for the nn
 * 	three vectors: word-form of sentence, pos of sentence, streams of inputs
 */

#include <vector>
using namespace std;
#include "nn_input_helper.h"
#include "nn_math.h"

class nn_input{
public:
	int num_inst;		//total number of instance
	int num_width;		//width of one instance
	nn_input_helper* helper;
	//these three vectors all allocated outside
	//!! must -- size(inputs) == num_inst*num_width
	vector<int>* inputs;	//num_inst*num_width
	vector<int>* goals;		//num_inst*1
	vector<int>* wordl;
	vector<int>* posl;

	//statistics
	int inst_good;
	int inst_bad;

	nn_input(int i,int w,vector<int>* il,vector<int>* gl,vector<int>* wl,vector<int>* pl,nn_input_helper* h,
			int ngood,int nbad):num_inst(i),num_width(w),helper(h),inputs(il),goals(gl),wordl(wl),posl(pl){
		nn_math::CHECK_EQUAL(i,(int)inputs->size()/w,"BAD nn_input.");
		inst_good = ngood;
		inst_bad = nbad;
	}
	~nn_input(){delete inputs;delete goals;}	//only delete this part
	int get_numi(){return num_inst;}
	int get_numw(){return num_width;}
};



#endif /* CSNN_NN_INPUT_H_ */
