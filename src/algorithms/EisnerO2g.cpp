/*
 * EisnerO2g.cpp
 *
 *  Created on: 2015.4.20
 *      Author: zzs
 */

#include "EisnerO2g.h"

/* the Eisner's algorithm extension for o2-grandchild
 *	--- only need the best tree
 */

static inline void assign_one_o2g(vector<int>* result,int index,int v)
{
	if((*result)[index]>=0){
		cerr << "Conflict in " << index << " and its value is " << (*result)[index] << ".\n";
	}
	(*result)[index] = v;
}

static void fill_result_o2g(int length,int *which,vector<int>* result,int g,int s,int t,int comp)
{	//s is head
	if(s==t)
		return;
	int index_it = get_index_o2g(length,g,s,t,comp);
	int index = which[index_it];
	if(comp==E_INCOM_O2g){
		if(t>s){
			fill_result_o2g(length,which,result,g,s,index,E_COM_O2g);
			fill_result_o2g(length,which,result,s,t,index+1,E_COM_O2g);
		}
		else{
			fill_result_o2g(length,which,result,g,s,index+1,E_COM_O2g);
			fill_result_o2g(length,which,result,s,t,index,E_COM_O2g);
		}
	}
	else{
		assign_one_o2g(result,index,s);
		fill_result_o2g(length,which,result,g,s,index,E_INCOM_O2g);
		fill_result_o2g(length,which,result,s,index,t,E_COM_O2g);
	}
}

//scores: S[g][h][m] (S[0][0][m])
vector<int>* decodeProjective_o2g(int length,double* scores)
{
	// the tables
	int total_size = length*length*length*2;
	double* scores_table = new double[total_size];
	int * which = new int[total_size];	//split point

	//1.initialize
	for(int i=0; i<total_size; i++)
		scores_table[i] = Negative_Infinity_O2g;
	for(int g=0;g<length;g++)
		for(int s=0;s<length;s++)
			for(int c=0;c<2;c++)
				scores_table[get_index_o2g(length,g,s,s,c)] = 0.0;

	//2.loop
	for(int k=1; k<length; k++){
		//the distance k
		for(int s=0; s<length; s++){
			//span between s and t
			int t=s+k;
			if(t>=length)
				break;
			for(int g=0;g<length;g++){
				//grand-node g
				if(g >= s && g <= t && g!=0)	//special node when g==0 (0,0,m)
					continue;
				//loops
				double max_score;
				int u,the_ind;
				//(1).incomplete ones
				//(1.1) right->left
				if(s!=0){	//not the root
					max_score = Negative_Infinity_O2g;
					u = -2;
					for(int r = s; r < t; r++){
						double tmp = scores_table[get_index_o2g(length,t,s,r,E_COM_O2g)]
						                          +scores_table[get_index_o2g(length,g,t,r+1,E_COM_O2g)]
						                                        +scores[get_index2_o2g(length,g,t,s)];
						if(tmp >= max_score){
							max_score = tmp;
							u = r;
						}
					}
					the_ind = get_index_o2g(length,g,t,s,E_INCOM_O2g);
					scores_table[the_ind]=max_score;
					which[the_ind]=u;
				}
				//(1.2) left->right
				max_score = Negative_Infinity_O2g;
				u = -2;
				for(int r = s; r < t; r++){
					double tmp = scores_table[get_index_o2g(length,g,s,r,E_COM_O2g)]
					                          +scores_table[get_index_o2g(length,s,t,r+1,E_COM_O2g)]
															+scores[get_index2_o2g(length,g,s,t)];
					if(tmp >= max_score){
						max_score = tmp;
						u = r;
					}

				}
				the_ind = get_index_o2g(length,g,s,t,E_INCOM_O2g);
				scores_table[the_ind]=max_score;
				which[the_ind]=u;
				//(2).complete ones
				//(2.1) right->left
				if(s!=0){
					max_score = Negative_Infinity_O2g;
					u = -2;
					for(int r = s; r < t; r++){
						double tmp = scores_table[get_index_o2g(length,t,r,s,E_COM_O2g)]
					                          +scores_table[get_index_o2g(length,g,t,r,E_INCOM_O2g)];
						if(tmp >= max_score){
							max_score = tmp;
							u = r;
						}
					}
					the_ind = get_index_o2g(length,g,t,s,E_COM_O2g);
					scores_table[the_ind]=max_score;
					which[the_ind]=u;
				}
				//(2.2) left->right
				max_score = Negative_Infinity_O2g;
				u = -2;
				for(int r = s+1; r <= t; r++){
					double tmp = scores_table[get_index_o2g(length,g,s,r,E_INCOM_O2g)]
					                          +scores_table[get_index_o2g(length,s,r,t,E_COM_O2g)];
					if(tmp >= max_score){
						max_score = tmp;
						u = r;
					}
				}
				the_ind = get_index_o2g(length,g,s,t,E_COM_O2g);
				scores_table[the_ind]=max_score;
				which[the_ind]=u;
			}
		}
	}

	//3.get results
	vector<int>* result = new vector<int>(length,-1);
	fill_result_o2g(length,which,result,0,0,length-1,E_COM_O2g);
	delete []scores_table;
	delete []which;
	return result;
}

