/*
 * sl_part.cpp
 *
 *  Created on: Dec 4, 2015
 *      Author: zzs
 */

#include "sl_part.h"
#include <exception>

void sl_part::forward(nn_input* inputs,REAL* out)
{
	this_input = inputs;
	const long this_size = this_input->num_inst;
	const long this_len = this_input->wordl->size();
	const int the_win = op->NN_sl_filter;	//should be odd-number

	const int idim_wp = p_main->geti();
	const int idim_dist = p_dist->geti();
	const int odim = p_main->geto();

	//1.allocate memory
	delete []c_input_wp;
	delete []c_input_dist;
	delete []c_output_wp;
	delete []c_output_dist;
	delete []c_output_tmp;
	delete []c_which;
	try{
		c_input_wp = new REAL[idim_wp*this_len];
		c_input_dist = new REAL[idim_dist*this_len*this_size];
		c_output_wp = new REAL[odim*this_len];
		c_output_dist = new REAL[odim*this_len*this_size];
		c_output_tmp = new REAL[odim*this_len*this_size];
		c_which = new int[odim*this_size];
	}catch(std::bad_alloc& bad_one){
		nn_math::CHECK_EQUAL(0,1,"Bad allocation for sl_part-forward.");
		throw bad_one;
	}

	//2.fill inputs
	//2.1 sentence
	REAL* to_assign = c_input_wp;
	for(int i=0;i<this_len;i++){	//for each center word
		for(int w=i-the_win/2;w<=i+the_win/2;w++){
			int single_index,pos_index;
			if(w<0){
				single_index = this_input->helper->start_word;
				pos_index = this_input->helper->start_pos;
			}
			else if(w>=this_len){
				single_index = this_input->helper->end_word;
				pos_index = this_input->helper->end_pos;
			}
			else{
				single_index = this_input->wordl->at(w);
				pos_index = this_input->posl->at(w);
			}
			d_word->forward(single_index,to_assign);
			to_assign += d_word->getd();
			d_pos->forward(pos_index,to_assign);
			to_assign += d_pos->getd();
		}
	}
	nn_math::CHECK_EQUAL(to_assign,c_input_wp+idim_wp*this_len,"Forward error of c_input_wp.");
	//2.2 sl distances
	to_assign = c_input_dist;
	for(vector<int>::const_iterator iter=this_input->inputs->begin();iter!=this_input->inputs->end();iter+=this_input->num_width){
		for(int i=0;i<this_len;i++){
			for(int ord=0;ord<this_input->num_width;ord++){
				int location = *(iter+ord);
				if(location < 0)
					d_ds->forward(this_input->helper->get_sd_dummy(),to_assign);
				else{
					location = this_input->helper->get_sd_index(location - i);
					d_ds->forward(location,to_assign);
				}
				to_assign += d_ds->getd();
			}
		}
	}
	nn_math::CHECK_EQUAL(to_assign,c_input_dist+idim_dist*this_len*this_size,"Forward error of c_input_dist.");

	//3.forward
	p_main->forward(c_input_wp,c_output_wp,this_len);
	p_dist->forward(c_input_dist,c_output_dist,this_len*this_size);

	//4.attach distance
	for(int i=0;i<this_size;i++)
		memcpy(c_output_tmp+i*odim*this_len,c_output_wp,sizeof(REAL)*odim*this_len);
	switch(op->NN_sl_way){
	case nn_options::NN_SL_ADDING_M:
	case nn_options::NN_SL_ADDING_A:	//directly adding
		nn_math::op_y_plus_ax(odim*this_len*this_size,c_output_tmp,c_output_dist,1);
		break;
	case nn_options::NN_SL_TANHMUL_M:
	case nn_options::NN_SL_TANHMUL_A:	//element-wise multiply tanh(dist)
		nn_math::act_f(nn_math::ACT_TANH,c_output_dist,odim*this_len*this_size,0);
		nn_math::op_y_elem_x(odim*this_len*this_size,c_output_tmp,c_output_dist);
		break;
	default:
		nn_math::CHECK_EQUAL(op->NN_sl_way,(int)nn_options::NN_SL_ADDING_M,"Forward error of unknown attach-method.");
	}

	//5.max-pooling or average-pooling
	REAL* mto_assign = out;
	int* mto_index = c_which;
	const REAL* mfrom_assign = c_output_tmp;
	for(int inst=0;inst<this_size;inst++){
		switch(op->NN_sl_way){
		case nn_options::NN_SL_ADDING_M:
		case nn_options::NN_SL_TANHMUL_M:
			//(1).all set to 0
			memcpy(mto_assign,mfrom_assign,sizeof(REAL)*odim);
			for(int tmpi=0;tmpi<odim;tmpi++)
				mto_index[tmpi] = 0;
			mfrom_assign += odim;
			//(2).update max one
			for(int i=1;i<this_len;i++){	//from next one
				for(int elem=0;elem<odim;elem++){
					REAL TMP_cur = *mfrom_assign++;	//add here
					if(TMP_cur > mto_assign[elem]){
						mto_assign[elem] = TMP_cur;
						mto_index[elem] = i;
					}
				}
			}
			break;
		case nn_options::NN_SL_ADDING_A:	//adding-average maybe is bad for it can not distinguish
		case nn_options::NN_SL_TANHMUL_A:
			//set to 0 firstly
			for(int tmpi=0;tmpi<odim;tmpi++)
				mto_assign[tmpi] = 0;
			//adding all
			for(int i=0;i<this_len;i++){	//from 0
				nn_math::op_y_plus_ax(odim,mto_assign,mfrom_assign,1);
				mfrom_assign += odim;
			}
			//average
			nn_math::op_y_mult_a(odim,mto_assign,1.0/this_len);
			break;
		default:
			nn_math::CHECK_EQUAL(op->NN_sl_way,(int)nn_options::NN_SL_ADDING_M,"Forward error of unknown attach-method.");
		}
		mto_assign += odim;
		mto_index += odim;
	}
	nn_math::CHECK_EQUAL(mto_assign,out+odim*this_size,"Forward error of mto_assign.");
	nn_math::CHECK_EQUAL(mto_index,c_which+odim*this_size,"Forward error of mto_index.");
	nn_math::CHECK_EQUAL(mfrom_assign,(const REAL*)c_output_tmp+odim*this_len*this_size,"Forward error of mfrom_assign.");

	return;
}


void sl_part::backward(/*const*/REAL* ograd)
{
	const long this_size = this_input->num_inst;
	const long this_len = this_input->wordl->size();
	const int the_win = op->NN_sl_filter;	//should be odd-number

	const int idim_wp = p_main->geti();
	const int idim_dist = p_dist->geti();
	const int odim = p_main->geto();

	//1.allocate memory
	REAL* g_input_wp;
	REAL* g_input_dist;
	REAL* g_output_wp;
	REAL* g_output_dist;
	try{
		g_input_wp = new REAL[idim_wp*this_len];
		g_input_dist = new REAL[idim_dist*this_len*this_size];
		g_output_wp = new REAL[odim*this_len];
		g_output_dist = new REAL[odim*this_len*this_size];
	}catch(std::bad_alloc& bad_one){
		nn_math::CHECK_EQUAL(0,1,"Bad allocation for sl_part-backward.");
		throw bad_one;
	}
	memset(g_input_wp,0,sizeof(REAL)*idim_wp*this_len);
	memset(g_input_dist,0,sizeof(REAL)*idim_dist*this_len*this_size);
	memset(g_output_wp,0,sizeof(REAL)*odim*this_len);
	memset(g_output_dist,0,sizeof(REAL)*odim*this_len*this_size);

	//5.max-pooling && 4.attach distance
	// ograd: srsize * num_inst (odim*this_size)
	REAL* grad_assign = ograd;
	REAL* grad_dist = g_output_dist;
	REAL* grad_wp = g_output_wp;
	REAL* val_dist = c_output_dist;
	REAL* val_wp = c_output_wp;
	int* max_index = c_which;
	for(int inst=0;inst<this_size;inst++){
		for(int elem=0;elem<odim;elem++){
			REAL tmp_g = grad_assign[elem];
			int tmp_index = max_index[elem];
			int tmp_one = odim*tmp_index+elem;
			switch(op->NN_sl_way){
			case nn_options::NN_SL_ADDING_M:	//directly adding
				grad_wp[tmp_one] += tmp_g;
				grad_dist[tmp_one] += tmp_g;
				break;
			case nn_options::NN_SL_TANHMUL_M:	//element-wise multiply tanh(dist)
				grad_wp[tmp_one] += tmp_g*val_dist[tmp_one];
				grad_dist[tmp_one] += tmp_g*val_wp[tmp_one] * (1 - val_dist[tmp_one] * val_dist[tmp_one]);
				break;
			case nn_options::NN_SL_ADDING_A:
				tmp_g /= this_len;
				for(int tmpl=0;tmpl<this_len;tmpl++){
					int tmp_one = odim*tmpl+elem;
					grad_wp[tmp_one] += tmp_g;
					grad_dist[tmp_one] += tmp_g;
				}
				break;
			case nn_options::NN_SL_TANHMUL_A:
				tmp_g /= this_len;
				for(int tmpl=0;tmpl<this_len;tmpl++){
					int tmp_one = odim*tmpl+elem;
					grad_wp[tmp_one] += tmp_g*val_dist[tmp_one];
					grad_dist[tmp_one] += tmp_g*val_wp[tmp_one] * (1 - val_dist[tmp_one] * val_dist[tmp_one]);
				}
				break;
			default:
				nn_math::CHECK_EQUAL(op->NN_sl_way,(int)nn_options::NN_SL_ADDING_M,"Backward error of unknown attach-method.");
			}
		}
		grad_assign += odim;
		grad_dist += odim*this_len;
		//grad_wp += 0;
		val_dist += odim*this_len;
		//val_wp += 0;
		max_index += odim;
	}
	nn_math::CHECK_EQUAL(grad_assign,ograd+odim*this_size,"Backward error of grad_assign.");
	nn_math::CHECK_EQUAL(grad_dist,g_output_dist+odim*this_len*this_size,"Backward error of grad_dist.");
	//nn_math::CHECK_EQUAL(grad_wp,g_output_wp+odim*this_len,"Backward error of grad_wp.");
	nn_math::CHECK_EQUAL(val_dist,c_output_dist+odim*this_len*this_size,"Backward error of val_dist.");
	//nn_math::CHECK_EQUAL(val_wp,c_output_wp+odim*this_len,"Backward error of val_wp.");
	nn_math::CHECK_EQUAL(max_index,c_which+odim*this_size,"Backward error of max_index.");

	//3.backward
	// --- notice this may be some kind of wasteful because many entries are 0, but maybe this is the most convenient way
	p_main->backward(g_output_wp,g_input_wp,c_input_wp,this_len);
	p_dist->backward(g_output_dist,g_input_dist,c_input_dist,this_len*this_size);

	//2.back to embedding
	//2.1 sentence
	REAL* to_grad = g_input_wp;
	for(int i=0;i<this_len;i++){	//for each center word
		for(int w=i-the_win/2;w<=i+the_win/2;w++){
			int single_index,pos_index;
			if(w<0){
				single_index = this_input->helper->start_word;
				pos_index = this_input->helper->start_pos;
			}
			else if(w>=this_len){
				single_index = this_input->helper->end_word;
				pos_index = this_input->helper->end_pos;
			}
			else{
				single_index = this_input->wordl->at(w);
				pos_index = this_input->posl->at(w);
			}
			d_word->backward(single_index,to_grad);
			to_grad += d_word->getd();
			d_pos->backward(pos_index,to_grad);
			to_grad += d_pos->getd();
		}
	}
	nn_math::CHECK_EQUAL(to_grad,g_input_wp+idim_wp*this_len,"Backward error of g_input_wp.");
	//2.2 sl distances
	to_grad = g_input_dist;
	for(vector<int>::const_iterator iter=this_input->inputs->begin();iter!=this_input->inputs->end();iter+=this_input->num_width){
		for(int i=0;i<this_len;i++){
			for(int ord=0;ord<this_input->num_width;ord++){
				int location = *(iter+ord);
				if(location < 0)
					d_ds->backward(this_input->helper->get_sd_dummy(),to_grad);
				else{
					location = this_input->helper->get_sd_index(location - i);
					d_ds->backward(location,to_grad);
				}
				to_grad += d_ds->getd();
			}
		}
	}
	nn_math::CHECK_EQUAL(to_grad,g_input_dist+idim_dist*this_len*this_size,"Backward error of g_input_dist.");

	//clear
	delete[] g_input_wp;
	delete[] g_input_dist;
	delete[] g_output_wp;
	delete[] g_output_dist;

	return;
}

