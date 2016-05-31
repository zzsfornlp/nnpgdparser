/*
 * nn_math.cpp
 *
 *  Created on: Sep 17, 2015
 *      Author: zzs
 */

#include "nn_math.h"
#include <cstring>
#include <iostream>
#include <cstdlib>

/* -- math wrapper */
namespace nn_math{
	//1.basic options --- simply better names?
	//y *= a
	void op_y_mult_a(const int n,REAL* y,const REAL a){
		REAL ww=a;
		SCAL(&n,&ww,y,&inc1);
	}
	//y += a*x
	void op_y_plus_ax(const int n,REAL* y,const REAL* x,const REAL a){
		AXPY(&n,&a,x,&inc1,y,&inc1);
	}
	//y = y^2
	void op_y_2(int n,REAL* y){
		VSQR(&n,y);
	}
	//y = y <ele>* x
	void op_y_elem_x(int n,REAL*y,const REAL*x){
		//element-wise multiply
		for(int i=0;i<n;i++,y++,x++)
			*y *= *x;
	}

	//  GEMM( transa, transb, m, n, k, alpha, a, lda, b, ldb, beta, c, ldc )
	// ** ld* means stride(property of A,B themselves); m,n,k means real dim after possible transposes
	//C = a*op(A)*op(B) + b*C
	//C ~ m x n
	//-stored in COLUM MAJOR order
	void op_A_mult_B(REAL *C,const REAL *A,const REAL *B,const int m,const int n,const int dimk,
			const bool transA,const bool transB,const REAL a,const REAL b){
		const char tA = transA ? 'T' : 'N';
		const char tB = transB ? 'T' : 'N';
		int ldc = m;
		int lda = transA ? dimk : m;
		int ldb = transB ? n : dimk;
		GEMM(&tA,&tB,&m,&n,&dimk,&a,A,&lda,B,&ldb,&b,C,&ldc);
	}

	//GEMV(trans,m,n,alpha,a,A,lda,x,incx,beta,y,incy)
	//y = alpha*A*x + beta*y
	//!! A always m*n
	void op_A_mult_x(REAL* y,const REAL* A,const REAL* x,const int m,const int n,
			const bool transA,const REAL a,const REAL b){
		const char tA = transA ? 'T' : 'N';
		int inc = 1;
		int lda = m;
		GEMV(&tA,&m,&n,&a,A,&lda,x,&inc,&b,y,&inc);
	}

	//2.opt updates --- min loss function
	/* details:	(choose some from the DLBook)
		1.SGD:					[1]g=sum(grad)/#inst; [2]w-=a1*g;
		2.SGD with momentum:	[1]g=sum(grad)/#inst; [2]v*=a2	[3]v-=a1*g	[4]w+=v
		3.AdaGard:				[1]g=sum(grad)/#inst; [2]g^2 [3]r+=g^2 [4]g'=-a1/sqrt(r)*g [5]w+=g'
		4.RMSprop:				<almost same> add another a3 for r smoothing
		5.Adam:					RMSprop+momentum
	*/
	void opt_update(int way,int n,REAL lrate,REAL wdecay,REAL m_alpha,REAL rms_smooth,
			REAL* w,REAL* grad,REAL* momentum,REAL* square,int mbsize)
	{
		//grad /= mbsize
		//--here, mbsize is all instances number which is different from the bsize of f/b
#ifndef USE_NONSQRT		//strange option
		op_y_mult_a(n, grad, sqrt(1.0 / mbsize));	//TRYING: use sqrt(bsize)
#else
		op_y_mult_a(n, grad, 1.0 / mbsize);		// -> change to non-sqrt
#endif // !USE_NONSQRT
		//wdecay --- add to grad
		// -- <old> op_y_mult_a(n,w,1-wdecay);
		if(wdecay > 0)
			op_y_plus_ax(n,grad,w,wdecay);
		//update
		switch(way){
		case OPT_SGD:			//lrate,w,grad
			op_y_plus_ax(n,w,grad,-1*lrate);
			break;
		case OPT_SGD_MOMENTUM:	//lrate,m_alpha,w,grad,momentum
			op_y_mult_a(n,momentum,m_alpha);
			op_y_plus_ax(n,momentum,grad,-1*lrate);
			op_y_plus_ax(n,w,momentum,1);
			break;
		case OPT_ADAGRAD:		//lrate,w,grad,square
		{
			REAL* tmp = new REAL[n];
			memcpy(tmp,grad,sizeof(REAL)*n);
			op_y_2(n,tmp);					//g^2
			op_y_plus_ax(n,square,tmp,1);	//r+=g^2
			//g' --- element wise
			for(int i=0;i<n;i++)
				tmp[i] = -1 * lrate * grad[i] / sqrt(square[i]);
			op_y_plus_ax(n,w,tmp,1);
			delete []tmp;
			break;
		}
		//this two methods do not perform well, ????,wrong implementation??
		//-------------------------------------------------------------------
		case OPT_RMSPROP:		//lrate,w,grad,square
		{
			REAL* tmp = new REAL[n];
			memcpy(tmp,grad,sizeof(REAL)*n);
			op_y_2(n,tmp);					//g^2
			op_y_mult_a(n,square,rms_smooth);	//r*=rms_s
			op_y_plus_ax(n,square,tmp,1-rms_smooth);	//r+=(1-rms_s)*g^2
			//g' --- element wise
			for(int i=0;i<n;i++)
				tmp[i] = -1 * lrate * grad[i] / sqrt(square[i]);
			op_y_plus_ax(n,w,tmp,1);
			delete []tmp;
			break;
		}
		case OPT_RMSPROP_MOMENTUM:			//lrate,w,grad,square,momentum
		{
			REAL* tmp = new REAL[n];
			memcpy(tmp,grad,sizeof(REAL)*n);
			op_y_2(n,tmp);					//g^2
			op_y_mult_a(n,square,rms_smooth);	//r*=rms_s
			op_y_plus_ax(n,square,tmp,1-rms_smooth);	//r+=(1-rms_s)*g^2
			//g' --- element wise
			for(int i=0;i<n;i++)
				tmp[i] = -1 * lrate * grad[i] / sqrt(square[i]);
			//use velocity
			op_y_mult_a(n,momentum,m_alpha);
			op_y_plus_ax(n,momentum,tmp,1);
			op_y_plus_ax(n,w,momentum,1);
			delete []tmp;
			break;
		}
		//-------------------------------------------------------------------
		default:
			std::cerr << "Unknown opt method." << std::endl;
			exit(1);
		}
	}

	//3.activation functions
	void act_f(int which,REAL* x,int n,REAL* xb){
		switch(which){
		case ACT_TANH:
			for(int i=0;i<n;i++,x++)
				*x = tanh(*x);
			break;
		case ACT_HTANH:
			for(int i=0;i<n;i++,x++){
				if(*x > 1)	*x = 1;
				else if(*x < -1)	*x=-1;
			}
			break;
		case ACT_LRECT:
			for(int i=0;i<n;i++,x++){
				if(*x < 0)	*x = 0;
			}
			break;
		case ACT_TANHCUBE:	//tanh(l^3+l)
			for(int i=0;i<n;i++,x++,xb++){
				REAL tmp = *x;
				*xb = tmp;
				*x += tmp*tmp*tmp;
				*x = tanh(*x);
			}
			break;
		case ACT_LIN:
			break;
		default:
			std::cerr << "Unknown opt method." << std::endl;
			exit(1);
		}
	}
	void act_b(int which,const REAL* x,REAL* g,int n,REAL* xb){
		switch(which){
		case ACT_TANH:	//d(tanh) = 1-tanh^2
			for(int i=0;i<n;i++,x++,g++)
				*g *= (1 - *x * *x);
			break;
		case ACT_HTANH:
			for(int i=0;i<n;i++,x++,g++){
				if(*x > 1 || *x < -1)
					*g = 0;
			}
			break;
		case ACT_LRECT:
			for(int i=0;i<n;i++,x++,g++){
				if(*x < 0)
					*g = 0;
			}
			break;
		case ACT_TANHCUBE:	//d = g*(1-t^2)*(3x^2+1)
			for(int i=0;i<n;i++,x++,g++,xb++){
				*g *= (1 - *x * *x);
				REAL tmp = *xb;
				*g *= (3*tmp*tmp+1);
			}
			break;
		case ACT_LIN:
			break;
		default:
			std::cerr << "Unknown opt method." << std::endl;
			exit(1);
		}
	}
};


