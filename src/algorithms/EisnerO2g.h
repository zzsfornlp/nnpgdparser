/*
 * EisnerO2g.h
 *
 *  Created on: 2015.4.20
 *      Author: zzs
 */

#ifndef ALGORITHMS_EISNERO2G_H_
#define ALGORITHMS_EISNERO2G_H_

#include <iostream>
#include <cstdlib>
#include <vector>
using namespace std;

/***************************************GIVING UP****************************************/
// ----------- for decoding o2g ----------------

#define Negative_Infinity_O2g -1e100
#define E_INCOM_O2g 0
#define E_COM_O2g 1

//the index explanation --- C[len][len][len][2]
// -- slightly different than the previous ones
inline int get_index_o2g(int len,int g,int h,int m,int c)
{
	int key = g;
	key = key * len + h;
	key = key * len + m;
	key = key * 2 + c;
	return key;
}

inline int get_index2_o2g(int len,int g,int h,int m)
{
	//get for scores: S[g][h][m] (S[0][0][m])
	int key = g;
	key = key * len + h;
	key = key * len + m;
	return key;
}

extern vector<int>* decodeProjective_o2g(int length,double* scores);
//extern double* encodeMarginals_o2g(const int length,const double* scores);

#endif /* ALGORITHMS_EISNERO2G_H_ */
