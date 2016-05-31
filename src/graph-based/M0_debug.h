/*
 * M0_debug.h
 *
 *  Created on: Oct 19, 2015
 *      Author: zzs
 */

#ifndef GRAPH_BASED_M0_DEBUG_H_
#define GRAPH_BASED_M0_DEBUG_H_

#include "Process.h"
//only for debugging
class M0_debug: public Process{
protected:
	virtual void each_create_machine();
	virtual void each_train_one_iter();
	virtual void each_test_one(DependencyInstance*,int);		//set predict_head or predict_deprels here

public:
	M0_debug(string cname):Process(cname){
	}
};



#endif /* GRAPH_BASED_M0_DEBUG_H_ */
