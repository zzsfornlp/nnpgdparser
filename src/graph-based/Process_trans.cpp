/*
 * Process_helper.cpp
 *
 *  Created on:
 *      Author: zzs
 */
#include "../algorithms/Eisner.h"
#include "../algorithms/EisnerO2sib.h"
#include "../algorithms/EisnerO3g.h"
#include "Process.h"

//--------------------transfrom scores only for (0,1)--------------------------
#include <cmath>
static inline void SET_LOG_HERE(double* tmp_yes,double* tmp_nope,int ln){
	for(int i=0;i<ln;i++){
		if(tmp_yes[i]<=0)	//!!DEBUG, tmp_nope should be 1
			//no scores here
			tmp_yes[i] = DOUBLE_LARGENEG;
		else
			tmp_yes[i] = log(tmp_yes[i]);
	}
	if(*tmp_nope <= 0)//!! ==0, it can happen
		*tmp_nope = DOUBLE_LARGENEG;
	else
		*tmp_nope = log(*tmp_nope);
}

//score[h,m,l] = log(prob(h,m==l)*products(prob(h,m'==0)))
//s[len,len,ln]; nope[len,len]
static void trans_o1(double* s,double* nope,int len,int ln)
{
	//to log number
	{
	double* tmp_yes = s;
	double* tmp_nope = nope;
	for(int i=0;i<len*len;i++){
		SET_LOG_HERE(tmp_yes,tmp_nope,ln);
		tmp_yes += ln;
		tmp_nope += 1;
	}
	}
	//sum
	for(int m=1;m<len;m++){
		double all_nope = 0;
		for(int h=0;h<len;h++){
			if(h==m)
				continue;
			all_nope += nope[get_index2(len,h,m)];
		}
		for(int h=0;h<len;h++){
			if(h==m)
				continue;
			int ind = get_index2(len,h,m);
			for(int la=0;la<ln;la++){
				int ind_la = get_index2(len,h,m,la,ln);
				s[ind_la] += all_nope-nope[ind];
			}
		}
	}
}

static void trans_o2sib(double* s,double* nope,int len,int ln)
{
	{
	//to log number
	double* tmp_yes = s;
	double* tmp_nope = nope;
	for(int i=0;i<len*len*len;i++){
		SET_LOG_HERE(tmp_yes,tmp_nope,ln);
		tmp_yes += ln;
		tmp_nope += 1;
	}
	}
	//sum
	for(int m=1;m<len;m++){
		double all_nope = 0;
		for(int h=0;h<len;h++){
			if(h==m)
				continue;
			all_nope += nope[get_index2_o2sib(len,h,h,m)];
			for(int c=h+1;c<m;c++)
				all_nope += nope[get_index2_o2sib(len,h,c,m)];
			for(int c=m+1;c<h;c++)
				all_nope += nope[get_index2_o2sib(len,h,c,m)];
		}
		for(int h=0;h<len;h++){
			if(h==m)
				continue;
			int ind = get_index2_o2sib(len,h,h,m);
			for(int la=0;la<ln;la++){
				int ind_la = get_index2_o2sib(len,h,h,m,la,ln);
				s[ind_la] += all_nope-nope[ind];
			}
			for(int c=h+1;c<m;c++){
				int ind = get_index2_o2sib(len,h,c,m);
				for(int la=0;la<ln;la++){
					int ind_la = get_index2_o2sib(len,h,c,m,la,ln);
					s[ind_la] += all_nope-nope[ind];
				}
			}
			for(int c=m+1;c<h;c++){
				int ind = get_index2_o2sib(len,h,c,m);
				for(int la=0;la<ln;la++){
					int ind_la = get_index2_o2sib(len,h,c,m,la,ln);
					s[ind_la] += all_nope-nope[ind];
				}
			}
		}
	}
}

static void trans_o3g(double* s,double* nope,long len,int ln)
{
	{
	//to log number
	double* tmp_yes = s;
	double* tmp_nope = nope;
	for(long i=0;i<len*len*len*len;i++){
		SET_LOG_HERE(tmp_yes,tmp_nope,ln);
		tmp_yes += ln;
		tmp_nope += 1;
	}
	}
	//sum
	for(int m=1;m<len;m++){
		double all_nope = 0;
		//1.calculate nope
		{//h=0
			all_nope += nope[get_index2_o3g(len,0,0,0,m)];	//special one
			for(int c=m-1;c>0;c--)
				all_nope += nope[get_index2_o3g(len,0,0,c,m)];	//0,0,c,m
		}
		for(int h=1;h<len;h++){
			if(h==m)
				continue;
			int small = GET_MIN_ONE(h,m);
			int large = GET_MAX_ONE(h,m);
			for(int g=0;g<small;g++){
				all_nope += nope[get_index2_o3g(len,g,h,h,m)];	//g,h,h,m
				for(int c=small+1;c<large;c++)
					all_nope += nope[get_index2_o3g(len,g,h,c,m)];	//g,h,c,m
			}
			for(int g=large+1;g<len;g++){
				all_nope += nope[get_index2_o3g(len,g,h,h,m)];	//g,h,h,m
				for(int c=small+1;c<large;c++)
					all_nope += nope[get_index2_o3g(len,g,h,c,m)];	//g,h,c,m
			}
		}
		//2.then ...
		{
			long ind = 0;
			ind = get_index2_o3g(len,0,0,0,m);	//special one
			for(int la=0;la<ln;la++){
				long ind_la = get_index2_o3g(len,0,0,0,m,la,ln);
				s[ind_la] += all_nope-nope[ind];
			}
			for(int c=m-1;c>0;c--){
				ind = get_index2_o3g(len,0,0,c,m);	//0,0,c,m
				for(int la=0;la<ln;la++){
					long ind_la = get_index2_o3g(len,0,0,c,m,la,ln);
					s[ind_la] += all_nope-nope[ind];
				}
			}
		}
		for(int h=1;h<len;h++){
			long ind = 0;
			if(h==m)
				continue;
			int small = GET_MIN_ONE(h,m);
			int large = GET_MAX_ONE(h,m);
			for(int g=0;g<small;g++){
				ind = get_index2_o3g(len,g,h,h,m);	//g,h,h,m
				for(int la=0;la<ln;la++){
					long ind_la = get_index2_o3g(len,g,h,h,m,la,ln);
					s[ind_la] += all_nope-nope[ind];
				}
				for(int c=small+1;c<large;c++){
					ind = get_index2_o3g(len,g,h,c,m);	//g,h,c,m
					for(int la=0;la<ln;la++){
						long ind_la = get_index2_o3g(len,g,h,c,m,la,ln);
						s[ind_la] += all_nope-nope[ind];
					}
				}
			}
			for(int g=large+1;g<len;g++){
				ind = get_index2_o3g(len,g,h,h,m);	//g,h,h,m
				for(int la=0;la<ln;la++){
					long ind_la = get_index2_o3g(len,g,h,h,m,la,ln);
					s[ind_la] += all_nope-nope[ind];
				}
				for(int c=small+1;c<large;c++){
					ind = get_index2_o3g(len,g,h,c,m);	//g,h,c,m
					for(int la=0;la<ln;la++){
						long ind_la = get_index2_o3g(len,g,h,c,m,la,ln);
						s[ind_la] += all_nope-nope[ind];
					}
				}
			}
		}
	}
}
