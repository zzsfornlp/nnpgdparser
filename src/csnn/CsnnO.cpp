/*
 * CsnnO.cpp
 *
 *  Created on: Oct 6, 2015
 *      Author: zzs
 */

#include "Csnn.h"

//Csnn
void Csnn::f_inputs(){
	//common

	//1.wrepr
	REAL* to_assign = c_wv->get_values();
	int order = get_order();
	//CHECK_EQUAL(order,this_input->get_numw(),"!!Mach and Input order NO match");
	const int* this_hasedge = HAS_HEAD[order-1];	//!!DEBUG:order-1
	//------fill the inputs-------------
	for(int i=0;i<this_bsize;i++){	//this_bsize instances
		for(int t=0;t<=order;t++){	//order+1 nodes
			//1.prepare the two nodes
			int cur_mod = this_input->inputs->at((order+1)*i+t);	//cur_mod means cur-node, not necessarily m-node
			int cur_head = this_hasedge[t];
			if(cur_head >= 0){
				cur_head = this_input->inputs->at((order+1)*i+cur_head);
				if(cur_head < 0)	//special node only for g in o3g
					cur_head = 0;
			}
			//2.prepares
			int the_win = the_option->NN_win;	//window size(should be odd)
			//2.special: s or g is -1, fill 0
			if(cur_mod < 0){
				//special word and pos
				bool hm_direction = (this_input->inputs->at((order+1)*i) > this_input->inputs->at((order+1)*i+1));
				int tmp_word_index, tmp_pos_index;
				if(cur_head >= 0){	//only for s, and don't care about which one
					tmp_word_index = hm_direction ? this_input->helper->dl_word : this_input->helper->dr_word;
					tmp_pos_index = hm_direction ? this_input->helper->dl_pos : this_input->helper->dr_pos;
				}
				else{
					tmp_word_index = this_input->helper->rg_word;
					tmp_pos_index = this_input->helper->rg_pos;
				}
				//assign
				for(int w=0;w<the_win;w++){	//!!DEBUG not <= but <
					p_word->forward(tmp_word_index,to_assign);
					to_assign += p_word->getd();
				}
				for(int w=0;w<the_win;w++){
					p_pos->forward(tmp_pos_index,to_assign);
					to_assign += p_pos->getd();
				}
				//for s, these set to 0
				if(cur_head >= 0){
					p_distance->forward(-1,to_assign);
					to_assign += p_distance->getd();
					if(the_option->NN_add_average){
						p_word->forward(-1,to_assign);
						to_assign += p_word->getd();
						p_pos->forward(-1,to_assign);
						to_assign += p_pos->getd();
					}
				}
			}
			else{
			//2.1:words
			for(int w=cur_mod-the_win/2;w<=cur_mod+the_win/2;w++){
				int single_index;
				if(w<0)
					single_index = this_input->helper->start_word;
				else if(w>=(int)this_input->wordl->size())
					single_index = this_input->helper->end_word;
				else
					single_index = this_input->wordl->at(w);
				p_word->forward(single_index,to_assign);
				to_assign += p_word->getd();
			}
			//2.2:pos
			for(int w=cur_mod-the_win/2;w<=cur_mod+the_win/2;w++){
				int single_index;
				if(w<0)
					single_index = this_input->helper->start_pos;
				else if(w>=(int)this_input->posl->size())
					single_index = this_input->helper->end_pos;
				else
					single_index = this_input->posl->at(w);
				p_pos->forward(single_index,to_assign);
				to_assign += p_pos->getd();
			}
			if(cur_head >= 0){
				//2.3:possible distance
				p_distance->forward(this_input->helper->get_distance_index(cur_head-cur_mod),to_assign);
				to_assign += p_distance->getd();
				//2.4:possible average word and pos
				if(the_option->NN_add_average){
					//between features -> (h,m] or [m,h)
					//2.4.1: word
					p_word->forward(-1,to_assign);	//set 0
					if(cur_mod != cur_head){	//!!DEBUG: only happens when cur_mod==cur_head==0
						p_word->forward(this_input->wordl->at(cur_mod),to_assign,1);// !! DEBUG : maybe miss cur_mod
						for(int ptr=cur_head+1;ptr<cur_mod;ptr++)
							p_word->forward(this_input->wordl->at(ptr),to_assign,1);
						for(int ptr=cur_mod+1;ptr<cur_head;ptr++)
							p_word->forward(this_input->wordl->at(ptr),to_assign,1);
						nn_math::op_y_mult_a(p_word->getd(),to_assign,1.0/(abs(cur_head-cur_mod)));	//average
					}
					to_assign += p_word->getd();
					//2.4.2: pos
					p_pos->forward(-1,to_assign);	//set 0
					if(cur_mod != cur_head){
						p_pos->forward(this_input->posl->at(cur_mod),to_assign,1);// !! DEBUG : maybe miss cur_mod
						for(int ptr=cur_head+1;ptr<cur_mod;ptr++)
							p_pos->forward(this_input->posl->at(ptr),to_assign,1);
						for(int ptr=cur_mod+1;ptr<cur_head;ptr++)
							p_pos->forward(this_input->posl->at(ptr),to_assign,1);
						nn_math::op_y_mult_a(p_pos->getd(),to_assign,1.0/(abs(cur_head-cur_mod)));	//average
					}
					to_assign += p_pos->getd();
				}
			}
			}
		}
	}
	//-----------fill the index for untied-------------
	switch(the_option->NN_untied_dim){
	case nn_options::NN_UNTIED_NOPE: break;
	case nn_options::NN_UNTIED_M:
		for(int i=0;i<this_bsize;i++){	//this_bsize instances
			int m = this_input->inputs->at((order+1)*i+1);
			this_untied_index.push_back(1+this_input->posl->at(m));
		}
		break;
	case nn_options::NN_UNTIED_H:
		for(int i=0;i<this_bsize;i++){	//this_bsize instances
			int h = this_input->inputs->at((order+1)*i+0);
			this_untied_index.push_back(1+this_input->posl->at(h));
		}
		break;
	case nn_options::NN_UNTIED_HM:
		for(int i=0;i<this_bsize;i++){	//this_bsize instances
			int h = this_input->inputs->at((order+1)*i);
			int m = this_input->inputs->at((order+1)*i+1);
			this_untied_index.push_back(1+this_input->posl->at(h)*the_option->NN_pnum
					+this_input->posl->at(m));
		}
		break;
	}

	//
	return;
}

void Csnn::b_inputs(){
	//common
	//1.wrepr
	REAL* to_grad = c_wv->get_gradients();
	int order = get_order();
	const int* this_hasedge = HAS_HEAD[order-1];	//!!DEBUG:order-1
	for(int i=0;i<this_bsize;i++){	//this_bsize instances
		for(int t=0;t<=order;t++){	//order+1 nodes
			//1.prepare the two nodes
			int cur_mod = this_input->inputs->at((order+1)*i+t);
			int cur_head = this_hasedge[t];
			if(cur_head >= 0){
				cur_head = this_input->inputs->at((order+1)*i+cur_head);
				if(cur_head < 0)	//special node only for g in o3g
					cur_head = 0;
			}
			//2.prepares
			int the_win = the_option->NN_win;	//window size(should be odd)
			//2.special: s or g is -1, fill 0
			if(cur_mod < 0){		//no update //!!!!DEBUG:but need to add, right...
				//special word and pos
				bool hm_direction = (this_input->inputs->at((order+1)*i) > this_input->inputs->at((order+1)*i+1));
				int tmp_word_index, tmp_pos_index;
				if(cur_head >= 0){	//only for s, and don't care about which one
					tmp_word_index = hm_direction ? this_input->helper->dl_word : this_input->helper->dr_word;
					tmp_pos_index = hm_direction ? this_input->helper->dl_pos : this_input->helper->dr_pos;
				}
				else{
					tmp_word_index = this_input->helper->rg_word;
					tmp_pos_index = this_input->helper->rg_pos;
				}
				//assign
				for(int w=0;w<the_win;w++){
					p_word->backward(tmp_word_index,to_grad);
					to_grad += p_word->getd();
				}
				for(int w=0;w<the_win;w++){
					p_pos->backward(tmp_pos_index,to_grad);
					to_grad += p_pos->getd();
				}
				if(cur_head >= 0){
					to_grad += p_distance->getd();
					if(the_option->NN_add_average){
						to_grad += p_word->getd();
						to_grad += p_pos->getd();
					}
				}
			}
			else{
				//2.1:words
				for(int w=cur_mod-the_win/2;w<=cur_mod+the_win/2;w++){
					int single_index;
					if(w<0)
						single_index = this_input->helper->start_word;
					else if(w>=(int)this_input->wordl->size())
						single_index = this_input->helper->end_word;
					else
						single_index = this_input->wordl->at(w);
					p_word->backward(single_index,to_grad);
					to_grad += p_word->getd();
				}
				//2.2:pos
				for(int w=cur_mod-the_win/2;w<=cur_mod+the_win/2;w++){
					int single_index;
					if(w<0)
						single_index = this_input->helper->start_pos;
					else if(w>=(int)this_input->posl->size())
						single_index = this_input->helper->end_pos;
					else
						single_index = this_input->posl->at(w);
					p_pos->backward(single_index,to_grad);
					to_grad += p_pos->getd();
				}
				if(cur_head >= 0){
					//2.3:possible distance
					p_distance->backward(this_input->helper->get_distance_index(cur_head-cur_mod),to_grad);
					to_grad += p_distance->getd();
					//2.4:possible average word and pos
					if(the_option->NN_add_average){
						//between features -> (h,m] or [m,h)
						//2.4.1: word
						if(cur_mod != cur_head){
							nn_math::op_y_mult_a(p_word->getd(),to_grad,1.0/(abs(cur_head-cur_mod)));	//average
							p_pos->backward(this_input->posl->at(cur_mod),to_grad);// !! DEBUG : maybe miss cur_mod
							for(int ptr=cur_head+1;ptr<cur_mod;ptr++)
								p_word->backward(this_input->wordl->at(ptr),to_grad);
							for(int ptr=cur_mod+1;ptr<cur_head;ptr++)
								p_word->backward(this_input->wordl->at(ptr),to_grad);
						}
						to_grad += p_word->getd();
						//2.4.2: pos
						if(cur_mod != cur_head){
							nn_math::op_y_mult_a(p_pos->getd(),to_grad,1.0/(abs(cur_head-cur_mod)));	//average
							p_pos->backward(this_input->posl->at(cur_mod),to_grad);// !! DEBUG : maybe miss cur_mod
							for(int ptr=cur_head+1;ptr<cur_mod;ptr++)
								p_pos->backward(this_input->posl->at(ptr),to_grad);
							for(int ptr=cur_mod+1;ptr<cur_head;ptr++)
								p_pos->backward(this_input->posl->at(ptr),to_grad);
						}
						to_grad += p_pos->getd();
					}
				}
			}
		}
	}

	//
	return;
}



