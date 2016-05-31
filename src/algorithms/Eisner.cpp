/*
 * Eisner.cpp
 *
 *  Created on: Jul 10, 2014
 *      Author: zzs
 */

#include "../algorithms/Eisner.h"
/* the O(n^3) Eisner's algotithms
 *	--- only need the best tree
 */
static void fill_result(int length,int *which,vector<int>* result,int s,int t,int lr,int comp)
{
	if(s==t)
		return;
	int index_it = get_index(length,s,t,lr,comp);
	int index = which[index_it];
	if(comp==E_INCOM){
		if(lr==E_LEFT){
			fill_result(length,which,result,s,index,E_RIGHT,E_COM);
			fill_result(length,which,result,index+1,t,E_LEFT,E_COM);
		}
		else{
			fill_result(length,which,result,s,index,E_RIGHT,E_COM);
			fill_result(length,which,result,index+1,t,E_LEFT,E_COM);
		}
	}
	else{
		if(lr==E_LEFT){
			if((*result)[index]>=0){
				E_ERROR("Comflict...");
			}
			(*result)[index] = t;
			fill_result(length,which,result,s,index,E_LEFT,E_COM);
			fill_result(length,which,result,index,t,E_LEFT,E_INCOM);
		}
		else{
			if((*result)[index]>=0){
				E_ERROR("Comflict...");
			}
			(*result)[index] = s;
			fill_result(length,which,result,s,index,E_RIGHT,E_INCOM);
			fill_result(length,which,result,index,t,E_RIGHT,E_COM);
		}

	}
}

//#define double int
vector<int>* decodeProjective(int length,double* scores)
{
	// the tables
	int total_size = length*length*2*2;
	double* scores_table = new double[total_size];
	int * which = new int[total_size];

	//1.initialize
	for(int i=0; i<total_size; i++)
		scores_table[i] = Negative_Infinity;
	for(int s=0;s<length;s++)
		for(int lr=0;lr<2;lr++)
			for(int c=0;c<2;c++)
				scores_table[get_index(length,s,s,lr,c)] = 0.0;
	//2.loop
	for(int k=1; k<length; k++){
		//the distance k
		for(int s=0; s<length; s++){
			//s -> t
			int t=s+k;
			if(t>=length)
				break;
			//loops
			double max_score;
			int u,the_ind;
			//(1).incomplete ones
			//(1.1) right->left
			if(s!=0){	//not the root
			max_score = Negative_Infinity;
			u = -2;
			for(int r = s; r < t; r++){
				double tmp = scores_table[get_index(length,s,r,E_RIGHT,E_COM)]
				                          +scores_table[get_index(length,r+1,t,E_LEFT,E_COM)]
				                                        +scores[get_index2(length,t,s)];
				if(tmp >= max_score){
					max_score = tmp;
					u = r;
				}
			}
			the_ind = get_index(length,s,t,E_LEFT,E_INCOM);
			scores_table[the_ind]=max_score;
			which[the_ind]=u;
			}
			//(1.2) left->right
			max_score = Negative_Infinity;
			u = -2;
			for(int r = s; r < t; r++){
				double tmp = scores_table[get_index(length,s,r,E_RIGHT,E_COM)]
				                          +scores_table[get_index(length,r+1,t,E_LEFT,E_COM)]
				                                        +scores[get_index2(length,s,t)];
				if(tmp >= max_score){
					max_score = tmp;
					u = r;
				}

			}
			the_ind = get_index(length,s,t,E_RIGHT,E_INCOM);
			scores_table[the_ind]=max_score;
			which[the_ind]=u;
			//(2).complete ones
			//(2.1) right->left
			if(s!=0){
			max_score = Negative_Infinity;
			u = -2;
			for(int r = s; r < t; r++){
				double tmp = scores_table[get_index(length,s,r,E_LEFT,E_COM)]
				                          +scores_table[get_index(length,r,t,E_LEFT,E_INCOM)];
				if(tmp >= max_score){
					max_score = tmp;
					u = r;
				}
			}
			the_ind = get_index(length,s,t,E_LEFT,E_COM);
			scores_table[the_ind]=max_score;
			which[the_ind]=u;
			}
			//(2.2) left->right
			max_score = Negative_Infinity;
			u = -2;
			for(int r = s+1; r <= t; r++){
				double tmp = scores_table[get_index(length,s,r,E_RIGHT,E_INCOM)]
				                          +scores_table[get_index(length,r,t,E_RIGHT,E_COM)];
				if(tmp >= max_score){
					max_score = tmp;
					u = r;
				}
			}
			the_ind = get_index(length,s,t,E_RIGHT,E_COM);
			scores_table[the_ind]=max_score;
			which[the_ind]=u;
		}
	}

	//3.get results
	vector<int>* result = new vector<int>(length,-1);
	fill_result(length,which,result,0,length-1,E_RIGHT,E_COM);
	delete []scores_table;
	delete []which;
	return result;
}


