/*
 * M1_p1o1.h
 *
 *  Created on: Oct 14, 2015
 *      Author: zzs
 */

#ifndef GRAPH_BASED_M1_P1_H_
#define GRAPH_BASED_M1_P1_H_

#include "Process.h"
//the method 1: probability 1 order 1
class M1_p1o1: public Process{
protected:
	virtual void each_create_machine();
	virtual void each_train_one_iter();
	virtual void each_test_one(DependencyInstance*,int);		//set predict_head or predict_deprels here
public:
	M1_p1o1(string cname):Process(cname){
	}
};

//the method 2: probability 1 order 2 (o2sib)
class M1_p1o2: public Process{
private:
	CsnnO1 *mfo1;	//filter
	CsnnO1 *mso1;	//scorer
protected:
	virtual void each_create_machine();
	virtual void each_train_one_iter();
	virtual void each_test_one(DependencyInstance*,int);		//set predict_head or predict_deprels here
public:
	M1_p1o2(string cname):Process(cname){
		mfo1 = dynamic_cast<CsnnO1*>(Csnn::read(hp->CONF_score_mach_fo1));
		if(hp->CONF_score_mach_so1.size() > 0)
			mso1 = dynamic_cast<CsnnO1*>(Csnn::read(hp->CONF_score_mach_so1));
		else
			mso1 = 0;
	}
};

//the method 3: probability 1 order 3 (o3g)
class M1_p1o3: public Process{
private:
	CsnnO1 *mfo1;	//filter
	CsnnO1 *mso1;	//scorer
	CsnnO2 *mso2;	//scorer o2sib
protected:
	virtual void each_create_machine();
	virtual void each_train_one_iter();
	virtual void each_test_one(DependencyInstance*,int);		//set predict_head or predict_deprels here
public:
	M1_p1o3(string cname):Process(cname){
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


#endif /* GRAPH_BASED_M1_P1_H_ */
