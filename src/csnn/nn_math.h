#ifndef _NN_MATH_H_
#define _NN_MATH_H_

typedef float REAL;
#include "Blas.h"
#include <cmath>
#include <iostream>

namespace nn_math{

template<class T>
static void CHECK_EQUAL(T a,T b,const char* x){
	if(a != b)
		std::cerr << "!! warning of " << x << ":" << a << " != " << b << std::endl;
}

enum{
	OPT_SGD,OPT_SGD_MOMENTUM,
	OPT_ADAGRAD,OPT_RMSPROP,OPT_RMSPROP_MOMENTUM
};

void op_y_mult_a(const int n,REAL* y,const REAL a);
void op_y_plus_ax(const int n,REAL* y,const REAL* x,const REAL a);
void op_y_elem_x(int n,REAL*y,const REAL*x);

void op_A_mult_B(REAL *C,const REAL *A,const REAL *B,const int m,const int n,const int dimk,
			const bool transA,const bool transB,const REAL a,const REAL b);
void op_A_mult_x(REAL* y,const REAL* A,const REAL* x,const int m,const int n,
			const bool transA,const REAL a,const REAL b);
void opt_update(int way,int n,REAL lrate,REAL wdecay,REAL m_alpha,REAL rms_smooth,
			REAL* w,REAL* grad,REAL* momentum,REAL* square,int mbsize);

const bool opt_changelrate[] = {true,true,false,false,false};
const bool opt_hasmomentum[] = {false,true,false,false,true};

//3.activation functions
enum{
	ACT_TANH,ACT_HTANH,ACT_LRECT,ACT_TANHCUBE,ACT_LIN
};
void act_f(int which,REAL* x,int n,REAL* xb);
void act_b(int which,const REAL* x,REAL* g,int n,REAL* xb);


};

#endif
