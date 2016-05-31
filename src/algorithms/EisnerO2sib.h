/*
 * EisnerO2sib.h
 *
 *  Created on: 2015.3.18
 *      Author: zzs
 */

#ifndef PARSING_V1_ALGORITHMS_EISNERO2SIB_H_
#define PARSING_V1_ALGORITHMS_EISNERO2SIB_H_

#include <iostream>
#include <cstdlib>
#include <vector>
using namespace std;

// ----------- for decoding o2sib ----------------
// -- just like ../tools/Eisner.h

#define Negative_Infinity_O2sib -1e100
#define E_LEFT_O2sib 0
#define E_RIGHT_O2sib 1
#define E_INCOM_O2sib 0
#define E_COM_O2sib 1
#define E_SIB_O2sib 2

//the index explanation --- C[len][len][2][3]
inline int get_index_o2sib(int len,int s,int t,int lr,int c)
{
	int key = s;
	key = key * len + t;
	key = key * 2 + lr;
	key = key * 3 + c;	//here *3
	return key;
}

//here different from Eisner.h::get_index2 (no direction -> it depends on h and m)
inline int get_index2_o2sib(int len,int h,int c,int m,int l=0,int ln=1)
{
	//get for scores: S[h][c][m] (S[h][-][m]=>S[h][h][m])
	int key = h;
	key = key * len + c;
	key = key * len + m;
	key = key * ln + l;
	return key;
}

extern vector<int>* decodeProjective_o2sib(int length,double* scores);
extern double* encodeMarginals_o2sib(const int length,const double* scores);
extern double* LencodeMarginals_o2sib(const long length,const double* scores,const int ln);	//labeled

#endif /* PARSING_V1_ALGORITHMS_EISNERO2SIB_H_ */
