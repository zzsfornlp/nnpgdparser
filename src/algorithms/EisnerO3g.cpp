/*
 * EisnerO3g.cpp
 *
 *  Created on: 2015.4.20
 *      Author: zzs
 */

#include "EisnerO3g.h"

/* the Eisner's algorithm extension for o2-grandchild
 *	--- only need the best tree
 */

static inline void assign_one_o3g(vector<int>* result,int index,int v)
{
	if((*result)[index]>=0){
		cerr << "Conflict in " << index << " and its value is " << (*result)[index] << ".\n";
	}
	(*result)[index] = v;
}

static void fill_result_o3g(int length,int *which,vector<int>* result,int g,int s,int t,int comp)
{
	if(s==t)
		return;
	int index_it = get_index_o3g(length,g,s,t,comp);
	int index = which[index_it];
	if(comp==E_INCOM_O3g){
		assign_one_o3g(result,t,s);
		if(index == t){	//first modifier
			if(t > s)
				fill_result_o3g(length,which,result,s,t,s+1,E_COM_O3g);
			else
				fill_result_o3g(length,which,result,s,t,s-1,E_COM_O3g);
		}
		else{
			if(t > s)
				fill_result_o3g(length,which,result,s,index,t,E_SIB_O3g);
			else
				fill_result_o3g(length,which,result,s,t,index,E_SIB_O3g);	//reamin the order
			fill_result_o3g(length,which,result,g,s,index,E_INCOM_O3g);
		}
	}
	else if(comp==E_SIB_O3g){	//here s must < t
		fill_result_o3g(length,which,result,g,s,index,E_COM_O3g);
		fill_result_o3g(length,which,result,g,t,index+1,E_COM_O3g);
	}
	else{
		fill_result_o3g(length,which,result,g,s,index,E_INCOM_O3g);
		fill_result_o3g(length,which,result,s,index,t,E_COM_O3g);
	}
}

vector<int>* decodeProjective_o3g(int length,double* scores)
{
	// the tables
	int total_size = length*length*length*3;
	double* scores_table = new double[total_size];
	int * which = new int[total_size];	//split point

	//1.initialize
	for(int i=0; i<total_size; i++)
		scores_table[i] = Negative_Infinity_O3g;
	for(int g=0;g<length;g++)
		for(int h=0;h<length;h++)
			for(int c=0;c<3;c++)
				scores_table[get_index_o3g(length,g,h,h,c)] = 0.0;
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
				//start the process
				double max_score;
				int split_point;
				int the_ind;
				//(1).sibling items
				max_score = Negative_Infinity_O3g;
				split_point = -2;
				for(int r = s; r < t; r++){
					double tmp = scores_table[get_index_o3g(length,g,s,r,E_COM_O3g)]
					                          +scores_table[get_index_o3g(length,g,t,r+1,E_COM_O3g)];
					if(tmp >= max_score){
						max_score = tmp;
						split_point = r;
					}
				}
				the_ind = get_index_o3g(length,g,s,t,E_SIB_O3g);	//third one no use
				scores_table[the_ind]=max_score;
				which[the_ind]=split_point;

				//(2).incomplete ones
				//-(2.1) s<--t
				//-- first modifier
				the_ind = get_index_o3g(length,g,t,s,E_INCOM_O3g);
				max_score = scores_table[get_index_o3g(length,t,s,t-1,E_COM_O3g)]
				                          +scores_table[get_index_o3g(length,g,t,t,E_COM_O3g)]
														+scores[get_index2_o3g(length,g,t,t,s)];
				split_point = s;
				//others
				for(int r=s+1;r<t;r++){
					double tmp = scores_table[get_index_o3g(length,t,s,r,E_SIB_O3g)]
					                          +scores_table[get_index_o3g(length,g,t,r,E_INCOM_O3g)]
															+scores[get_index2_o3g(length,g,t,r,s)];
					if(tmp >= max_score){
						max_score = tmp;
						split_point = r;
					}
				}
				scores_table[the_ind]=max_score;
				which[the_ind]=split_point;
				//-(2.2) s-->t
				//-- first modifier
				the_ind = get_index_o3g(length,g,s,t,E_INCOM_O3g);
				max_score = scores_table[get_index_o3g(length,g,s,s,E_COM_O3g)]
				                          +scores_table[get_index_o3g(length,s,t,s+1,E_COM_O3g)]
														+scores[get_index2_o3g(length,g,s,s,t)];
				split_point = t;
				//others
				for(int r=s+1;r<t;r++){
					double tmp = scores_table[get_index_o3g(length,s,r,t,E_SIB_O3g)]
					                          +scores_table[get_index_o3g(length,g,s,r,E_INCOM_O3g)]
															+scores[get_index2_o3g(length,g,s,r,t)];
					if(tmp >= max_score){
						max_score = tmp;
						split_point = r;
					}
				}
				scores_table[the_ind]=max_score;
				which[the_ind]=split_point;

				//(3).complete items
				//s<--t
				max_score = Negative_Infinity_O3g;
				split_point = -2;
				for(int r = s; r < t; r++){
					double tmp = scores_table[get_index_o3g(length,t,r,s,E_COM_O3g)]
					                          +scores_table[get_index_o3g(length,g,t,r,E_INCOM_O3g)];
					if(tmp >= max_score){
						max_score = tmp;
						split_point = r;
					}
				}
				the_ind = get_index_o3g(length,g,t,s,E_COM_O3g);	//third one no use
				scores_table[the_ind]=max_score;
				which[the_ind]=split_point;
				//t-->s
				max_score = Negative_Infinity_O3g;
				split_point = -2;
				for(int r = s+1; r <= t; r++){
					double tmp = scores_table[get_index_o3g(length,g,s,r,E_INCOM_O3g)]
					                          +scores_table[get_index_o3g(length,s,r,t,E_COM_O3g)];
					if(tmp >= max_score){
						max_score = tmp;
						split_point = r;
					}
				}
				the_ind = get_index_o3g(length,g,s,t,E_COM_O3g);	//third one no use
				scores_table[the_ind]=max_score;
				which[the_ind]=split_point;

			}
		}
	}
	//3.get results
	vector<int>* result = new vector<int>(length,-1);
	fill_result_o3g(length,which,result,0,0,length-1,E_COM_O3g);
	delete []scores_table;
	delete []which;
	return result;
}


