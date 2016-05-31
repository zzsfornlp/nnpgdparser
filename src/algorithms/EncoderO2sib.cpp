/*
 * EncoderO2sib.cpp
 *
 *  Created on: 2015.7.7
 *      Author: zzs
 */

// --- from MaxParser->DependendcyEncoder2OSibling.cpp

#include "EisnerO2sib.h"
#include "Helper.h"

inline static int getKey(int s, int t, int dir, int comp, int length){
	int key = s;
	key = key * length + t;
	key = key * 2 + dir;
	key = key * 3 + comp;
	return key;
}

//return z
static double calc_inside(const int length, double *beta,const double *probs)
{
	int key, key1, key2;

	for(int i = 0; i < length; i++){
		key = getKey(i, i, 0, 1, length);
		beta[key] = 0.0;
		key = getKey(i, i, 1, 1, length);
		beta[key] = 0.0;
	}

	for(int j = 1; j < length; j++){
		for(int s = 0; s + j < length; s++){
			int t = s + j;
			//double prodProb_st = probs[s][t][0];
			//double prodProb_ts = probs[s][t][1];

			//init beta
			//incomplete spans
			//r == s
			int key_st_0 = getKey(s, t, 0, 0, length);
			//double prodProb_sst = probs_trips[s][s][t] + probs_sibs[s][t][0] + prodProb_st;
			double prodProb_sst = probs[get_index2_o2sib(length,s,s,t)];
			key1 = getKey(s, s, 0, 1, length);
			key2 = getKey(s + 1, t, 1, 1, length);
			beta[key_st_0] = logsumexp(beta[key_st_0], beta[key1] + beta[key2] + prodProb_sst, true);

			//r == t
			int key_ts_0 = getKey(s, t, 1, 0, length);
			//double prodProb_tts = probs_trips[t][t][s] + probs_sibs[t][s][0] + prodProb_ts;
			double prodProb_tts = probs[get_index2_o2sib(length,t,t,s)];
			key1 = getKey(s, t - 1, 0, 1, length);
			key2 = getKey(t, t, 1, 1, length);
			beta[key_ts_0] = logsumexp(beta[key_ts_0], beta[key1] + beta[key2] + prodProb_tts, true);

			//sibling spans
			int key_st_2 = getKey(s, t, 0, 2, length);
			beta[key_st_2] = 0.0;
			int key_ts_2 = getKey(s, t, 1, 2, length);
			beta[key_ts_2] = 0.0;
			bool flg_st_2 = true, flg_ts_2 = true;

			//complete spans
			int key_st_1 = getKey(s, t, 0, 1, length);
			beta[key_st_1] = 0.0;
			int key_ts_1 = getKey(s, t, 1, 1, length);
			beta[key_ts_1] = 0.0;
			bool flg_st_1 = true, flg_ts_1 = true;

			//calc sibling spans
			for(int r = s; r < t; r++){
				key1 = getKey(s, r, 0 ,1, length);
				key2 = getKey(r + 1, t, 1, 1, length);

				beta[key_st_2] = logsumexp(beta[key_st_2], beta[key1] + beta[key2], flg_st_2);
				flg_st_2 = false;

				beta[key_ts_2] = logsumexp(beta[key_ts_2], beta[key1] + beta[key2], flg_ts_2);
				flg_ts_2 = false;
			}

			//calc incomplete spans
			for(int r = s + 1; r < t; r++){
				key1 = getKey(s, r, 0, 0, length);
				key2 = getKey(r, t, 0, 2, length);
				//double prodProb_srt = probs_trips[s][r][t] + probs_sibs[r][t][1] + prodProb_st;
				double prodProb_srt = probs[get_index2_o2sib(length,s,r,t)];
				beta[key_st_0] = logsumexp(beta[key_st_0], beta[key1] + beta[key2] + prodProb_srt, false);

				key1 = getKey(s, r, 1, 2, length);
				key2 = getKey(r, t, 1, 0, length);
				//double prodProb_trs = probs_trips[t][r][s] + probs_sibs[r][s][1] + prodProb_ts;
				double prodProb_trs = probs[get_index2_o2sib(length,t,r,s)];
				beta[key_ts_0] = logsumexp(beta[key_ts_0], beta[key1] + beta[key2] + prodProb_trs, false);
			}

			//calc complete spans
			for(int r = s; r <= t; r++){
				if(r != s){
					key1 = getKey(s, r, 0, 0, length);
					key2 = getKey(r, t, 0, 1, length);
					beta[key_st_1] = logsumexp(beta[key_st_1], beta[key1] + beta[key2], flg_st_1);
					flg_st_1 = false;
				}
				if(r != t){
					key1 = getKey(s, r, 1, 1, length);
					key2 = getKey(r, t, 1, 0, length);
					beta[key_ts_1] = logsumexp(beta[key_ts_1], beta[key1] + beta[key2], flg_ts_1);
					flg_ts_1 = false;
				}
			}
		}
	}

	key1 = getKey(0, length - 1, 0, 1, length);
	key2 = getKey(0, length - 1, 1, 1, length);
	return logsumexp(beta[key1], beta[key2], false);
}

static void calc_outside(const int length,const double *beta,const double *probs,double *alpha)
{
	int key;
	int end = length - 1;
	for(int d = 0; d < 2; d++){
		for(int c = 0 ; c < 3; c++){
			key = getKey(0, end, d, c, length);
			alpha[key] = 0.0;
		}
	}

	for(int j = end; j >= 1; j--){
		for(int s = 0; s + j < length; s++){
			int t = s + j;

			int key_a, key_b;

			//init alpha
			//sibling spans
			int key_st_2 = getKey(s, t, 0, 2, length);
			alpha[key_st_2] = 0.0;
			bool flg_st_2 = true;
			for(int r = 0; r < s; r++){
				//double prodProb_rst = probs_trips[r][s][t] + probs_sibs[s][t][1] + probs[r][t][0];
				double prodProb_rst = probs[get_index2_o2sib(length,r,s,t)];
				key_b = getKey(r, s, 0, 0, length);
				key_a = getKey(r, t, 0, 0, length);
				alpha[key_st_2] = logsumexp(alpha[key_st_2], beta[key_b] + alpha[key_a] + prodProb_rst, flg_st_2);
				flg_st_2 = false;
			}
			for(int r = t + 1; r < length; r++){
				//double prodProb_rts = probs_trips[r][t][s] + probs_sibs[t][s][1] + probs[s][r][1];
				double prodProb_rts = probs[get_index2_o2sib(length,r,t,s)];
				key_b = getKey(t, r, 1, 0, length);
				key_a = getKey(s, r, 1, 0, length);
				alpha[key_st_2] = logsumexp(alpha[key_st_2], beta[key_b] + alpha[key_a] + prodProb_rts, flg_st_2);
				flg_st_2 = false;
			}

			//complete spnas
			int key_st_1 = getKey(s, t, 0, 1, length);
			bool flg_st_1 = true;
			alpha[key_st_1] = 0.0;
			if(t + 1 < length){
				key_a = getKey(s, t + 1, 1, 0, length);
				//double prodProb = probs_trips[t + 1][t + 1][s] + probs_sibs[t + 1][s][0] + probs[s][t + 1][1];
				double prodProb = probs[get_index2_o2sib(length,t+1,t+1,s)];
				alpha[key_st_1] = logsumexp(alpha[key_st_1], alpha[key_a] + prodProb, flg_st_1);
				flg_st_1 = false;
			}

			int key_ts_1 = getKey(s, t, 1, 1, length);
			bool flg_ts_1 = true;
			alpha[key_ts_1] = 0.0;
			if(s != 0){
				key_a = getKey(s - 1, t, 0, 0, length);
				//double prodProb = probs_trips[s - 1][s - 1][t] + probs_sibs[s - 1][t][0] + probs[s - 1][t][0];
				double prodProb = probs[get_index2_o2sib(length,s-1,s-1,t)];
				alpha[key_ts_1] = logsumexp(alpha[key_ts_1], alpha[key_a] + prodProb, flg_ts_1);
				flg_ts_1 = false;
			}

			for(int r = 0; r < s; r++){
				key_b = getKey(r, s, 0, 0, length);
				key_a = getKey(r, t, 0, 1, length);
				alpha[key_st_1] = logsumexp(alpha[key_st_1], beta[key_b] + alpha[key_a], flg_st_1);
				flg_st_1 = false;

				if(!((r == 0) && (t == length -1))){
					key_b = getKey(r, s - 1, 0 ,1, length);
					key_a = getKey(r, t, 0, 2, length);
					alpha[key_ts_1] = logsumexp(alpha[key_ts_1], beta[key_b] + alpha[key_a], flg_ts_1);
					flg_ts_1 = false;
				}
			}
			for(int r = t + 1; r < length; r++){
				if(!((s == 0) && (r == length -1))){
					key_b = getKey(t + 1, r, 1, 1, length);
					key_a = getKey(s, r, 0 ,2, length);
					alpha[key_st_1] = logsumexp(alpha[key_st_1], beta[key_b] + alpha[key_a], flg_st_1);
					flg_st_1 = false;
				}

				key_b = getKey(t, r, 1, 0, length);
				key_a = getKey(s, r, 1, 1, length);
				alpha[key_ts_1] = logsumexp(alpha[key_ts_1], beta[key_b] + alpha[key_a], flg_ts_1);
				flg_ts_1 = false;
			}

			//incomplete spans
			int key_st_0 = getKey(s, t, 0, 0, length);
			alpha[key_st_0] = 0.0;
			bool flg_st_0 = true;

			int key_ts_0 = getKey(s, t, 1, 0, length);
			alpha[key_ts_0] = 0.0;
			bool flg_ts_0 = true;

			for(int r = t; r < length; r++){
				key_b = getKey(t, r, 0 ,1, length);
				key_a = getKey(s, r, 0 ,1, length);
				alpha[key_st_0] = logsumexp(alpha[key_st_0], beta[key_b] + alpha[key_a], flg_st_0);
				flg_st_0 = false;

				if(r != t){
					key_b = getKey(t, r, 0, 2, length);
					key_a = getKey(s, r, 0, 0, length);
					//double prodProb_str = probs_trips[s][t][r] + probs_sibs[t][r][1] + probs[s][r][0];
					double prodProb_str = probs[get_index2_o2sib(length,s,t,r)];
					alpha[key_st_0] = logsumexp(alpha[key_st_0], beta[key_b] + alpha[key_a] + prodProb_str, flg_st_0);
					flg_st_0 = false;
				}
			}

			for(int r = 0; r <= s; r++){
				key_b = getKey(r, s, 1, 1, length);
				key_a = getKey(r, t, 1, 1, length);
				alpha[key_ts_0] = logsumexp(alpha[key_ts_0], beta[key_b] + alpha[key_a], flg_ts_0);
				flg_ts_0 = false;

				if(r != s){
					key_b = getKey(r, s, 0, 2, length);
					key_a = getKey(r, t, 1, 0, length);
					//double prodProb_tsr = probs_trips[t][s][r] + probs_sibs[s][r][1] + probs[r][t][1];
					double prodProb_tsr = probs[get_index2_o2sib(length,t,s,r)];
					alpha[key_ts_0] = logsumexp(alpha[key_ts_0], beta[key_b] + alpha[key_a] + prodProb_tsr, flg_ts_0);
					flg_ts_0 = false;
				}
			}
		}
	}
}

//##MAGIC NUMBERS of MAX-Encoder##
//direction: 0,st(right);1,ts(left)
//spans: 0,incomplete;1,complete;2,sibling

double* encodeMarginals_o2sib(const int length,const double* scores)
{
	double* marginals = new double[length*length*length];	//use get_index2
	double *beta = new double[length * length * 2 * 3];
	double *alpha = new double[length * length * 2 * 3];
	double z = calc_inside(length, beta,scores);
	calc_outside(length,beta,scores,alpha);

	//get them
	for(int s=0;s<length;s++){
		for(int t=s+1;t<length;t++){
			//sst
			int key_assign = get_index2_o2sib(length,s,s,t);
			marginals[key_assign] = exp(beta[getKey(s+1,t,1,1,length)]+alpha[getKey(s,t,0,0,length)]+scores[key_assign]-z);
			for(int r=s+1;r<t;r++){
				//srt
				key_assign = get_index2_o2sib(length,s,r,t);
				marginals[key_assign] = exp(beta[getKey(s,r,0,0,length)]+beta[getKey(r,t,0,2,length)]
												 +alpha[getKey(s,t,0,0,length)]+scores[key_assign]-z);
			}
			//tts
			key_assign = get_index2_o2sib(length,t,t,s);
			marginals[key_assign] = exp(beta[getKey(s,t-1,0,1,length)]+alpha[getKey(s,t,1,0,length)]+scores[key_assign]-z);
			for(int r=s+1;r<t;r++){
				//trs
				key_assign = get_index2_o2sib(length,t,r,s);
				marginals[key_assign] = exp(beta[getKey(r,t,1,0,length)]+beta[getKey(s,r,0,2,length)]
												 +alpha[getKey(s,t,1,0,length)]+scores[key_assign]-z);
			}
		}
	}

	delete []beta;
	delete []alpha;
	return marginals;
}

double* LencodeMarginals_o2sib(const long length,const double* scores,const int ln)
{
	double* marginals = new double[length*length*length*ln];	//use get_index2
	double *beta = new double[length * length * 2 * 3];
	double *alpha = new double[length * length * 2 * 3];
	//sumlabel score
	double* sum_scores = TMP_get_sumlabel(length*length*length,ln,scores);
	double z = calc_inside(length, beta,sum_scores);
	calc_outside(length,beta,sum_scores,alpha);

	//get them
	for(int s=0;s<length;s++){
		for(int t=s+1;t<length;t++){
			//sst
			for(int zl=0;zl<ln;zl++){
				int key_assign = get_index2_o2sib(length,s,s,t,zl,ln);
				marginals[key_assign] = exp(beta[getKey(s+1,t,1,1,length)]+alpha[getKey(s,t,0,0,length)]+scores[key_assign]-z);
			}
			for(int r=s+1;r<t;r++){
				//srt
				for(int zl=0;zl<ln;zl++){
					int key_assign = get_index2_o2sib(length,s,r,t,zl,ln);
					marginals[key_assign] = exp(beta[getKey(s,r,0,0,length)]+beta[getKey(r,t,0,2,length)]
												 +alpha[getKey(s,t,0,0,length)]+scores[key_assign]-z);
				}
			}
			//tts
			for(int zl=0;zl<ln;zl++){
				int key_assign = get_index2_o2sib(length,t,t,s,zl,ln);
				marginals[key_assign] = exp(beta[getKey(s,t-1,0,1,length)]+alpha[getKey(s,t,1,0,length)]+scores[key_assign]-z);
			}
			for(int r=s+1;r<t;r++){
				//trs
				for(int zl=0;zl<ln;zl++){
					int key_assign = get_index2_o2sib(length,t,r,s,zl,ln);
					marginals[key_assign] = exp(beta[getKey(r,t,1,0,length)]+beta[getKey(s,r,0,2,length)]
												 +alpha[getKey(s,t,1,0,length)]+scores[key_assign]-z);
				}
			}
		}
	}

	delete []beta;
	delete []alpha;
	delete []sum_scores;
	return marginals;
}


