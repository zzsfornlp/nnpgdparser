/*
 * nn_w.cpp
 *
 *  Created on: Sep 20, 2015
 *      Author: zzs
 */

#include "nn_wv.h"
#include "nn_wb.h"
#include "nn_math.h"

//---------------------------------------------------------
//for nn_wb
void nn_wb::forward(/*const*/REAL* in,REAL* out,int bsize)
{
	if(odim<=0)	return;
	// classical matrix multiplication -- column major
	// out   =   w   *   in   +   b
	// o*bs		o*i		i*bs
	if(!nobias){
		for (int e=0; e<bsize; e++)
			memcpy(out+e*odim,b,odim*sizeof(REAL));	//if nobias, b should always be 0
	}
	else{
		//(bug-of-overflow)memset(out,0,odim*bsize*sizeof(REAL));
		for (int e=0; e<bsize; e++)
			memset(out+e*odim,0,odim*sizeof(REAL));
	}
	if(bsize > 1)
		nn_math::op_A_mult_B(out,w,in,odim,bsize,idim,false,false,1,1);
	else
		nn_math::op_A_mult_x(out,w,in,odim,idim,false,1,1);
}

void nn_wb::backward(/*const*/REAL* ograd,REAL* igrad,/*const*/REAL* in,int bsize)
{
	if(odim<=0)	return;
	// again matrix
    // backprop gradient:   igrad   +=        w'        *   ograd
    //                    idim x bsize = (odim x idim)'  *  odim x bsize
	// here += means accumulated gradient (the task of clear belongs to the outside)
	if(bsize > 1)
		nn_math::op_A_mult_B(igrad,w,ograd,idim,bsize,odim,true,false,1,1);
	else
		nn_math::op_A_mult_x(igrad,w,ograd,odim,idim,true,1,1);

	//accumulate gradients into tmp ones
	if(updating){
		//gradient of b
		REAL *gptr = ograd;
		if(!nobias){
			for (int e=0; e<bsize; e++, gptr+=odim)
				nn_math::op_y_plus_ax(odim,b_grad,gptr,1);	//!!DEBUG: not ograd but gptr
		}
		//gradient of w
		//gw += ograd * in'
		//o*i	o*b		b*i
		nn_math::op_A_mult_B(w_grad,ograd,in,odim,idim,bsize,false,true,1,1);
	}
}

void nn_wb::update(int way,REAL lrate,REAL wdecay,REAL m_alpha,REAL rms_smooth,int mbsize)
{
	if(odim<=0)	return;
	//update
	nn_math::opt_update(way,idim*odim,lrate,wdecay,m_alpha,rms_smooth,w,w_grad,w_moment,w_square,mbsize);
	if(!nobias){
		nn_math::opt_update(way,odim,lrate,0,m_alpha,rms_smooth,b,b_grad,b_moment,b_square,mbsize);	//no wd for b
	}
	//clear the gradient
	clear_grad();
}

void nn_wb::nesterov_update(int way,REAL m_alpha)
{
	if(odim<=0)	return;
	//Nesterov update before mini-batch (currently only for nn_wb)
	nn_math::op_y_plus_ax(idim*odim,w,w_moment,m_alpha);
	if(!nobias)
		nn_math::op_y_plus_ax(odim,b,b_moment,m_alpha);
}

//----------------------------------------------------------
//for nn_wv
void nn_wv::forward(int index,REAL* out,int adding)
{
	if(index>=num){
		nn_math::CHECK_EQUAL(index,num-1,"should be less than.");
	}
	if(!adding){
		if(index<0)
			memset(out,0,sizeof(REAL)*dim);
		else
			memcpy(out,w+index*dim,sizeof(REAL)*dim);
	}
	else if(index>=0){
		nn_math::op_y_plus_ax(dim,out,w+index*dim,1);
	}
}

void nn_wv::backward(int index,const REAL* grad)
{
	if(index<0)
		return;
	hit_index->insert(index);
	nn_math::op_y_plus_ax(dim,w_grad+index*dim,grad,1);
}

void nn_wv::update(int way,REAL lrate,REAL wdecay,REAL m_alpha,REAL rms_smooth,int mbsize)
{
	//update
	for(IntSet::iterator i = hit_index->begin();i!=hit_index->end();i++){
		int index = *i;
		if(index<0)
			continue;
		nn_math::opt_update(way,dim,lrate,0,m_alpha,rms_smooth,
				w+index*dim,w_grad+index*dim,w_moment+index*dim,w_square+index*dim,mbsize);	//no wd for wv
	}
	//clear the gradient
	clear_grad();
}

//------------------------------------------------------------
//perceptron part for nn_wb
void nn_wb::update_pr(REAL* inputs,int row,REAL a)
{
	//!! this is quite specified, for COLUMN major matrix
	// and no checks here
	REAL* start = w+row;
	for(int i=0;i<idim;i++){
		*start += a * *inputs;
		start += odim;
		inputs++;
	}
}

void nn_wb::add_w(nn_wb* x)
{
	nn_math::op_y_plus_ax(idim*odim,w,x->w,1);
}

void nn_wb::div_w(REAL a)
{
	nn_math::op_y_mult_a(idim*odim,w,a);
}
