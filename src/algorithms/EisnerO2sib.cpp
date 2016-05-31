/*
 * EisnerO2sib.cpp
 *
 *  Created on: 2015年3月18日
 *      Author: zzs
 */

/*
 * Method6_EisnerO2sib.cpp
 *
 *  Created on: 2015年3月17日
 *      Author: zzs
 */

#include "../algorithms/EisnerO2sib.h"

//Algorithm of extended Eisner (order 2 for ../tools/Eisner.h)
// -- defined here for convenience

/* the O(n^3) Eisner's algotithms of order2-sib
 *	--- only need the best tree
 */

static inline void assign_one_o2sib(vector<int>* result,int index,int v)
{
	if((*result)[index]>=0){
		cerr << "Conflict in " << index << " and its value is " << (*result)[index] << ".\n";
	}
	(*result)[index] = v;
}
static void fill_result_o2sib(int length,int *which,vector<int>* result,int s,int t,int lr,int comp)
{
	//assign when incomplete
	if(s==t)
		return;
	int index_it = get_index_o2sib(length,s,t,lr,comp);
	int index = which[index_it];
	if(comp==E_INCOM_O2sib){
		if(lr==E_LEFT_O2sib){
			if(index==s){
				assign_one_o2sib(result,index,t);
				fill_result_o2sib(length,which,result,s,t-1,E_RIGHT_O2sib,E_COM_O2sib);
			}
			else{
				assign_one_o2sib(result,s,t);
				fill_result_o2sib(length,which,result,s,index,0,E_SIB_O2sib);
				fill_result_o2sib(length,which,result,index,t,E_LEFT_O2sib,E_INCOM_O2sib);
			}
		}
		else{
			if(index==t){
				assign_one_o2sib(result,index,s);
				fill_result_o2sib(length,which,result,s+1,t,E_LEFT_O2sib,E_COM_O2sib);
			}
			else{
				assign_one_o2sib(result,t,s);
				fill_result_o2sib(length,which,result,index,t,0,E_SIB_O2sib);
				fill_result_o2sib(length,which,result,s,index,E_RIGHT_O2sib,E_INCOM_O2sib);
			}
		}
	}
	else if(comp==E_COM_O2sib){
		if(lr==E_LEFT_O2sib){
			fill_result_o2sib(length,which,result,s,index,E_LEFT_O2sib,E_COM_O2sib);
			fill_result_o2sib(length,which,result,index,t,E_LEFT_O2sib,E_INCOM_O2sib);
		}
		else{
			fill_result_o2sib(length,which,result,s,index,E_RIGHT_O2sib,E_INCOM_O2sib);
			fill_result_o2sib(length,which,result,index,t,E_RIGHT_O2sib,E_COM_O2sib);
		}
	}
	else{
		fill_result_o2sib(length,which,result,s,index,E_RIGHT_O2sib,E_COM_O2sib);
		fill_result_o2sib(length,which,result,index+1,t,E_LEFT_O2sib,E_COM_O2sib);
	}
}


//scores: S[h][c][m] (S[h][-][m]=>S[h][h][m])
vector<int>* decodeProjective_o2sib(int length,double* scores)
{
	// the tables
	int total_size = length*length*2*3;
	double* scores_table = new double[total_size];
	int * which = new int[total_size];	//split point

	//1.initialize
	for(int i=0; i<total_size; i++)
		scores_table[i] = Negative_Infinity_O2sib;
	for(int s=0;s<length;s++)
		for(int lr=0;lr<2;lr++)
			for(int c=0;c<3;c++)
				scores_table[get_index_o2sib(length,s,s,lr,c)] = 0.0;

	//2.loop
	for(int k=1; k<length; k++){
		//the distance k
		for(int s=0; s<length; s++){
			//span between s and t
			int t=s+k;
			if(t>=length)
				break;
			//start the process
			double max_score;
			int split_point;
			int the_ind;
			//(1).sibling items
			max_score = Negative_Infinity_O2sib;
			split_point = -2;
			for(int r = s; r < t; r++){
				double tmp = scores_table[get_index_o2sib(length,s,r,E_RIGHT_O2sib,E_COM_O2sib)]
				                          +scores_table[get_index_o2sib(length,r+1,t,E_LEFT_O2sib,E_COM_O2sib)];
				if(tmp >= max_score){
					max_score = tmp;
					split_point = r;
				}
			}
			the_ind = get_index_o2sib(length,s,t,0,E_SIB_O2sib);	//third one no use
			scores_table[the_ind]=max_score;
			which[the_ind]=split_point;

			//(2).incomplete ones
			//-(2.1) s<--t
			//-- first modifier
			the_ind = get_index_o2sib(length,s,t,E_LEFT_O2sib,E_INCOM_O2sib);
			max_score = scores_table[get_index_o2sib(length,s,t-1,E_RIGHT_O2sib,E_COM_O2sib)]
			                          +scores_table[get_index_o2sib(length,t,t,E_LEFT_O2sib,E_COM_O2sib)]
													+scores[get_index2_o2sib(length,t,t,s)];
			split_point = s;
			//others
			for(int r=s+1;r<t;r++){
				double tmp = scores_table[get_index_o2sib(length,s,r,0,E_SIB_O2sib)]
				                          +scores_table[get_index_o2sib(length,r,t,E_LEFT_O2sib,E_INCOM_O2sib)]
														+scores[get_index2_o2sib(length,t,r,s)];
				if(tmp >= max_score){
					max_score = tmp;
					split_point = r;
				}
			}
			scores_table[the_ind]=max_score;
			which[the_ind]=split_point;
			//-(2.2) s-->t
			//-- first modifier
			the_ind = get_index_o2sib(length,s,t,E_RIGHT_O2sib,E_INCOM_O2sib);
			max_score = scores_table[get_index_o2sib(length,s,s,E_RIGHT_O2sib,E_COM_O2sib)]
			                          +scores_table[get_index_o2sib(length,s+1,t,E_LEFT_O2sib,E_COM_O2sib)]
													+scores[get_index2_o2sib(length,s,s,t)];
			split_point = t;
			//others
			for(int r=s+1;r<t;r++){
				double tmp = scores_table[get_index_o2sib(length,r,t,0,E_SIB_O2sib)]
				                          +scores_table[get_index_o2sib(length,s,r,E_RIGHT_O2sib,E_INCOM_O2sib)]
														+scores[get_index2_o2sib(length,s,r,t)];
				if(tmp >= max_score){
					max_score = tmp;
					split_point = r;
				}
			}
			scores_table[the_ind]=max_score;
			which[the_ind]=split_point;

			//(3).complete items
			//s<--t
			max_score = Negative_Infinity_O2sib;
			split_point = -2;
			for(int r = s; r < t; r++){
				double tmp = scores_table[get_index_o2sib(length,s,r,E_LEFT_O2sib,E_COM_O2sib)]
				                          +scores_table[get_index_o2sib(length,r,t,E_LEFT_O2sib,E_INCOM_O2sib)];
				if(tmp >= max_score){
					max_score = tmp;
					split_point = r;
				}
			}
			the_ind = get_index_o2sib(length,s,t,E_LEFT_O2sib,E_COM_O2sib);	//third one no use
			scores_table[the_ind]=max_score;
			which[the_ind]=split_point;
			//t-->s
			max_score = Negative_Infinity_O2sib;
			split_point = -2;
			for(int r = s+1; r <= t; r++){
				double tmp = scores_table[get_index_o2sib(length,s,r,E_RIGHT_O2sib,E_INCOM_O2sib)]
				                          +scores_table[get_index_o2sib(length,r,t,E_RIGHT_O2sib,E_COM_O2sib)];
				if(tmp >= max_score){
					max_score = tmp;
					split_point = r;
				}
			}
			the_ind = get_index_o2sib(length,s,t,E_RIGHT_O2sib,E_COM_O2sib);	//third one no use
			scores_table[the_ind]=max_score;
			which[the_ind]=split_point;
		}
	}

	//3.get results
	vector<int>* result = new vector<int>(length,-1);
	fill_result_o2sib(length,which,result,0,length-1,E_RIGHT_O2sib,E_COM_O2sib);
	delete []scores_table;
	delete []which;
	return result;
}



