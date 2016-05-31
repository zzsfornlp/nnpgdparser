/*
 * M4_mm.h
 *
 *  Created on: Mar 11, 2016
 *      Author: zzs
 */

#ifndef GRAPH_BASED_M4_MM_H_
#define GRAPH_BASED_M4_MM_H_

#include "Process.h"
#include "M3_pr.h"	//why ?? -> get_nninput_*

//should have the same interface as M2, and only training process is different ...

void MM_margin_backward(Csnn*, nn_input*, int, REAL);

//o1
class M4_o1: public Process{
protected:
	virtual void each_create_machine();
	virtual void each_train_one_iter();
	virtual void each_test_one(DependencyInstance*,int);		//set predict_head or predict_deprels here
public:
	M4_o1(string cname):Process(cname){
		adjust_hp_p2();
	}
};

//o2
class M4_o2: public Process{
private:
	CsnnO1 *mfo1;	//filter
	CsnnO1 *mso1;	//scorer
protected:
	virtual void each_create_machine();
	virtual void each_train_one_iter();
	virtual void each_test_one(DependencyInstance*,int);		//set predict_head or predict_deprels here
public:
	M4_o2(string cname):Process(cname){
		adjust_hp_p2();
		mfo1 = dynamic_cast<CsnnO1*>(Csnn::read(hp->CONF_score_mach_fo1));
		if(hp->CONF_score_mach_so1.size() > 0)
			mso1 = dynamic_cast<CsnnO1*>(Csnn::read(hp->CONF_score_mach_so1));
		else
			mso1 = 0;
	}
};

//o3
class M4_o3: public Process{
private:
	CsnnO1 *mfo1;	//filter
	CsnnO1 *mso1;	//scorer
	CsnnO2 *mso2;	//scorer o2sib
protected:
	virtual void each_create_machine();
	virtual void each_train_one_iter();
	virtual void each_test_one(DependencyInstance*,int);		//set predict_head or predict_deprels here
public:
	M4_o3(string cname):Process(cname){
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



#endif /* GRAPH_BASED_M4_MM_H_ */
