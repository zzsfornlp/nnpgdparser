/*
 * Helper.h
 *
 *  Created on:
 *      Author: zzs
 */

#ifndef ALGORITHMS_HELPER_H_
#define ALGORITHMS_HELPER_H_

#include <cmath>
const double MINUS_LOG_EPSILON=50;

// log(exp(x) + exp(y));
inline double logsumexp(double x, double y, bool flg) {
	if (flg) return y;  // init mode
	const double vmin = (x>y)?y:x;
	const double vmax = (x<y)?y:x;
	if (vmax > vmin + MINUS_LOG_EPSILON) {
		return vmax;
	}else{
		return vmax + log(exp(vmin - vmax) + 1.0);
	}
}

#define DOUBLE_LARGENEG_P1 (-10000000.0+1)		//maybe it is enough	/******!! ../Process.h ******/

static double* TMP_get_sumlabel(long all,int ln,const double* s)
{
	//for all, sum-score = logsum(scores-of-labels); s==all*ln
	double* ret = new double[all];
	double* to_assign = ret;
	const double* from_assign = s;
	for(long i=0;i<all;i++){
		if(*from_assign <= DOUBLE_LARGENEG_P1){	//----------------- to avoid exp for pruned edges, must be careful !!!!
			*to_assign = *from_assign;
			from_assign += ln;
			to_assign ++;
			continue;
		}
		bool flg = true;
		for(int ll=0;ll<ln;ll++){
			*to_assign = logsumexp(*to_assign,*from_assign,flg);
			from_assign++;
			flg = false;
		}
		to_assign++;
	}
	return ret;
}

#endif /* ALGORITHMS_HELPER_H_ */
