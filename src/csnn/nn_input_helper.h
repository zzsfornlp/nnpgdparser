/*
 * nn_input_helper.h
 *
 *  Created on: Oct 3, 2015
 *      Author: zzs
 */

#ifndef CSNN_NN_INPUT_HELPER_H_
#define CSNN_NN_INPUT_HELPER_H_
#include <cmath>

//helper for extra special info
class nn_input_helper{
public:
	//the specified index
	int start_word;
	int end_word;
	int start_pos;
	int end_pos;

	int dl_word;
	int dr_word;
	int rg_word;
	int dl_pos;
	int dr_pos;
	int rg_pos;

	//normal distance for wrepr
	static const int DIST_MAX=10,DIST_MIN=-10;
	static int get_distance_index(int distance){
		if(distance < DIST_MIN)
			distance = DIST_MIN;
		else if(distance > DIST_MAX)
			distance = DIST_MAX;
		return distance-DIST_MIN;
	}
	static int get_distance_num(){
		return DIST_MAX-DIST_MIN+1;
	}

	//special distance for srepr --- leave one for dummy distance
	// --- special specified routine [-50,-40,-30,-20,-10,-9,...,9,10,20,30,40,50,dummy]: full of magic numbers
	static int get_sd_index(int distance){
		//out-max
		if(distance < -50)
			distance = -50;
		else if(distance > 50)
			distance = 50;

		int abs_distance = abs(distance);
		if(abs_distance<10){}
		else{
			abs_distance = abs_distance/10 + 9;
		}

		if(distance >= 0)
			return abs_distance;
		else
			return abs_distance + 15;
	}
	static int get_sd_num(){
		return 31;	//15*2+1
	}
	static int get_sd_dummy(){
		return get_sd_num()-1;
	}
};


#endif /* CSNN_NN_INPUT_HELPER_H_ */
