/*
 * M2_p2.h
 *
 *  Created on: Nov 16, 2015
 *      Author: zzs
 */

#ifndef GRAPH_BASED_M2_P2_H_
#define GRAPH_BASED_M2_P2_H_

#include "Process.h"
//the method 1: probability 2 order 1
class M2_p2o1: public Process{
protected:
	virtual void each_create_machine();
	virtual void each_train_one_iter();
	virtual void each_test_one(DependencyInstance*,int);		//set predict_head or predict_deprels here
public:
	M2_p2o1(string cname):Process(cname){
		adjust_hp_p2();
	}
};

//the method 2: probability 2 order 2 (o2sib)
class M2_p2o2: public Process{
private:
	CsnnO1 *mfo1;	//filter
	CsnnO1 *mso1;	//scorer
protected:
	virtual void each_create_machine();
	virtual void each_train_one_iter();
	virtual void each_test_one(DependencyInstance*,int);		//set predict_head or predict_deprels here
public:
	M2_p2o2(string cname):Process(cname){
		adjust_hp_p2();
		mfo1 = dynamic_cast<CsnnO1*>(Csnn::read(hp->CONF_score_mach_fo1));
		if(hp->CONF_score_mach_so1.size() > 0)
			mso1 = dynamic_cast<CsnnO1*>(Csnn::read(hp->CONF_score_mach_so1));
		else
			mso1 = 0;
	}
};

//the method 3: probability 2 order 3 (o3g)
class M2_p2o3: public Process{
private:
	CsnnO1 *mfo1;	//filter
	CsnnO1 *mso1;	//scorer
	CsnnO2 *mso2;	//scorer o2sib
protected:
	virtual void each_create_machine();
	virtual void each_train_one_iter();
	virtual void each_test_one(DependencyInstance*,int);		//set predict_head or predict_deprels here
public:
	M2_p2o3(string cname):Process(cname){
		adjust_hp_p2();
		mfo1 = dynamic_cast<CsnnO1*>(Csnn::read(hp->CONF_score_mach_fo1));
		if(hp->CONF_score_mach_so1.size() > 0)
			mso1 = dynamic_cast<CsnnO1*>(Csnn::read(hp->CONF_score_mach_so1));
		else
			mso1 = 0;
		if(hp->CONF_score_mach_so2sib.size() > 0)
			mso2 = dynamic_cast<CsnnO2*>(Csnn::read(hp->CONF_score_mach_so2sib));
		else
			mso2 = 0;
	}
};




#endif /* GRAPH_BASED_M2_P2_H_ */
