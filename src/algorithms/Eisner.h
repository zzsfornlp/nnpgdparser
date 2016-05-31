/*
 * Eisner.h
 *
 *  Created on: Jul 10, 2014
 *      Author: zzs
 */

#ifndef EISNER_H_
#define EISNER_H_

#include <iostream>
#include <cstdlib>
#include <vector>
using namespace std;

#define Negative_Infinity -1e100

#define E_LEFT 0
#define E_RIGHT 1
#define E_INCOM 0
#define E_COM 1

//the index explanation --- C[len][len][2][2]
inline int get_index(int len,int s,int t,int lr,int c)
{
	int key = s;
	key = key * len + t;
	key = key * 2 + lr;
	key = key * 2 + c;
	return key;
}
//the index2 -- scores
// -- unified with S[h][m][label] (default 1 label --- for compatibility)
inline int get_index2(int len,int h,int m,int l=0,int ln=1)
{
	int key = h;
	key = key * len + m;
	key = key * ln + l;
	return key;
}

#define E_ERROR(str) \
	cout << str << endl;\
	throw str;
	//exit(1)

extern vector<int>* decodeProjective(int length,double* scores);
extern double* encodeMarginals(const int length,const double* scores);
extern double* LencodeMarginals(const int length,const double* scores,const int ln);	//labeled

#endif /* EISNER_H_ */
