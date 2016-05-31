/*
 * main.cpp
 *
 *  Created on: Jan 2, 2015
 *      Author: zzs
 */

//main one
#include <cstdlib>
#include "graph-based/Process.h"
#include "graph-based/M0_debug.h"
#include "graph-based/M1_p1.h"
#include "graph-based/M2_p2.h"
#include "graph-based/M3_pr.h"
#include "graph-based/M4_mm.h"

/* <Version 2.2>
 * 		-- usage:	(mode 3 and 4 are for debugging)
 * 	1.training: <exe-file> conf
 * 	2.testing: <exe-file> conf best-machine-name
 * 	3.check-o1-filter: <exe-file> conf best-machine-name cut-point check-file
 */

int main(int argc,char **argv)
{
	if(argc < 2){
		HypherParameters::Error("Not enough parameters for cmd.");
	}
	string conf(argv[1]);
	HypherParameters par(conf);
	srand(par.CONF_random_seed);
	Process *x;
	switch(par.CONF_method){
	case 0:
		x = new M0_debug(conf);
		break;
	case 1:
		x = new M1_p1o1(conf);
		break;
	case 2:
		x = new M1_p1o2(conf);
		break;
//----------------method3 is quite not stable---------
	case 3:
		x = new M1_p1o3(conf);
		break;
//----------------------------------------------------
	case 4:
		x = new M2_p2o1(conf);
		break;
	case 5:
		x = new M2_p2o2(conf);
		break;
	case 6:
		x = new M2_p2o3(conf);
		break;
	case 7:
		x = new M3_pro2(conf);
		break;
	case 8:
		x = new M3_pro3(conf);
		break;
	case 11:
		x = new M4_o1(conf);
		break;
	case 12:
		x = new M4_o2(conf);
		break;
	case 13:
		x = new M4_o3(conf);
		break;
	}
	if(argc == 2){
		//training
		x->train();
		if(par.CONF_test_file.length()>0 && par.CONF_gold_file.length()>0){
			//test
			string mach_best_name = par.CONF_mach_name+par.CONF_mach_best_suffix;
			x->test(mach_best_name);
		}
	}
	else if(argc == 3){
		//only testing
		x->test(string(argv[2]));
	}
	else if(argc == 5){
		//check o1-filter
		x->check_o1_filter(string(argv[2]),string(argv[3]),string(argv[4]));
	}
	else{
		HypherParameters::Error("Wrong parameters for cmd.");
	}
	return 0;
}


