#ifndef DependencyInstance_H
#define DependencyInstance_H
#include<vector>
#include<string>
using namespace std;

/* An instance means one sentence
 * 	--- also simpilfied version
 */

class DependencyInstance{
private:
	void init();
public:
	//GET from init
	vector<string*>* forms;
	vector<string*>* postags;
	vector<int>* heads;
	vector<string*>* deprels;

	//calculated from init (-1 as nope)
	vector<int>* siblings;

	//used when decoding --- set up after construction of dictionary
	vector<int>* index_forms;
	vector<int>* index_pos;
	vector<int>* index_deprels;

	//to-predict --- GET from decoder
	vector<int>* predict_heads;
	vector<int>* predict_deprels;

	//GET before output
	vector<string*>* predict_deprels_str;

	DependencyInstance();
	DependencyInstance(vector<string*>* forms, vector<string*>* postags,
			std::vector<string*> *deprels,vector<int>* heads);
	~DependencyInstance();
	int length();
};
#endif
