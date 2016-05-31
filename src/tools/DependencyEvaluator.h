#ifndef DependencyEvaluator_H
#define DependencyEvaluator_H
#include <string>
#include <vector>
#include "DependencyInstance.h"
using std::string;
using std::vector;

class DependencyEvaluator{
public:
	static double evaluate(string &act_file, string &pred_file, string &format, bool labeled=true);
};
#endif
