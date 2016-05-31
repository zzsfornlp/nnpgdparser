/*
 * Csnn.cpp
 *
 *  Created on: Sep 25, 2015
 *      Author: zzs
 */

#include "Csnn.h"

const int Csnn::HAS_HEAD[3][4] = {
		{-1,0,-1,-1},	//h,m
		{-1,0,0,-1},	//h,m,s
		{3,0,0,-1}		//h,m,s,g
};
Csnn* Csnn::read(string fname){
	std::ifstream fin;
	fin.open(fname.c_str(),ifstream::binary);
	int order;
	fin.read((char*)&order,sizeof(order));
	fin.close();
	Csnn* ret = 0;
	switch(order){
		case 1:	ret = new CsnnO1(); break;
		case 2:	ret = new CsnnO2(); break;
		case 3:	ret = new CsnnO3(); break;
		default: cerr << "!!! Unknown csnn order..." << endl; break;
	}
	ret->read_init(fname);	//construct
	return ret;
}
Csnn* Csnn::create(int order,nn_options * o){
	Csnn* ret = 0;
	switch(order){
		case 1:	ret = new CsnnO1(); break;
		case 2:	ret = new CsnnO2(); break;
		case 3:	ret = new CsnnO3(); break;
	default: cerr << "!!! Unknown csnn order..." << endl; break;
	}
	ret->create_init(o);
	return ret;
}

//--prepares--
void Csnn::construct_caches(){
	//this is done when init and read
	c_allcaches = new vector<nn_cache*>();
	c_out = new nn_cache(0,the_option->NN_out_size);	c_allcaches->push_back(c_out);
	c_h = new nn_cache(0,the_option->NN_hidden_size);	c_allcaches->push_back(c_h);
	c_repr = new nn_cache(0,the_option->get_NN_rsize());	c_allcaches->push_back(c_repr);
	c_wrepr = new nn_cache(0,the_option->NN_wrsize);	c_allcaches->push_back(c_wrepr);
	c_srepr = new nn_cache(0,the_option->get_NN_srsize());	c_allcaches->push_back(c_srepr);
	c_wv = new nn_cache(0,the_option->get_NN_wv_wrsize(get_order()));	c_allcaches->push_back(c_wv);
}

void Csnn::prepare_caches(int bsize){
	//resize and clear gradients
	//this is done before each f/b
	for(vector<nn_cache*>::iterator i=c_allcaches->begin();i!=c_allcaches->end();i++){
		(*i)->resize(bsize);
		(*i)->clear_gradients(bsize);
	}
}

void Csnn::prepare_dropout(){
	//set possible dropout
	//this is done before each MiniBatch
	for(vector<nn_cache*>::iterator i=c_allcaches->begin();i!=c_allcaches->end();i++){
		if(the_option->NN_dropout > 0 && (*i)!=c_out)
			(*i)->gen_dropout(the_option->NN_dropout);
	}
	if(the_option->NN_dropout_repr){
		c_wrepr->gen_dropout(the_option->NN_dropout_repr);
		c_srepr->gen_dropout(the_option->NN_dropout_repr);
	}
}

void Csnn::clear_params(){
	//clear gradients --- for all params
	p_out->clear_grad();
	p_h->clear_grad();
	for(unsigned int i=0;i<p_untied->size();i++)
		if(p_untied->at(i) != 0)
			p_untied->at(i)->clear_grad();
	p_word->clear_grad();
	p_pos->clear_grad();
	p_distance->clear_grad();
	p_sd->clear_grad();
	if(the_option->NN_add_sent)
		p_sl->clear_grad();
}

void Csnn::construct_params(){
	//init all the params
	p_out = new nn_wb(the_option->NN_hidden_size,the_option->NN_out_size);
	p_out->get_init(the_option->NN_init_wb_faniorange,the_option->NN_init_wb_brange);
	p_h = new nn_wb(the_option->get_NN_rsize(),the_option->NN_hidden_size);
	p_h->get_init(the_option->NN_init_wb_faniorange,the_option->NN_init_wb_brange);
	//special untied param
	int all = the_option->NN_pnum*the_option->NN_pnum+1;
	p_untied = new vector<nn_wb*>(all,(nn_wb*)0);
	p_untied->at(0) = new nn_wb(the_option->get_NN_wv_wrsize(get_order()),the_option->NN_wrsize);
	(p_untied->at(0))->get_init(the_option->NN_init_wb_faniorange,the_option->NN_init_wb_brange);
	p_untied_touched = new vector<int>(p_untied->size(),0);
	//the embeddings
	p_word = new nn_wv(the_option->NN_wnum,the_option->NN_wsize);
	p_word->get_init(the_option->NN_init_wvrange);
	p_pos = new nn_wv(the_option->NN_pnum,the_option->NN_psize);
	p_pos->get_init(the_option->NN_init_wvrange);
	p_distance = new nn_wv(the_option->NN_dnum,the_option->NN_dsize);
	p_distance->get_init(the_option->NN_init_wvrange);
	p_sd = new nn_wv(the_option->NN_sdnum,the_option->NN_sdsize);
	p_sd->get_init(the_option->NN_init_wvrange);
	if(the_option->NN_add_sent)
		p_sl = new sl_part(the_option,get_order(),p_word,p_pos,p_sd);
	p_pr = 0;
}

void Csnn::read_params(std::ifstream &fin){
	p_out = new nn_wb(fin);
	p_h = new nn_wb(fin);
	//special untied param
	int un_num = 0;
	fin.read((char*)&un_num,sizeof(int));
	int all = the_option->NN_pnum*the_option->NN_pnum+1;
	p_untied = new vector<nn_wb*>(all,(nn_wb*)0);
	p_untied_touched = new vector<int>(p_untied->size(),0);
	for(int i=0;i<un_num;i++){
		int tmp_index = 0;
		fin.read((char*)&tmp_index,sizeof(int));
		p_untied->at(tmp_index) = new nn_wb(fin);
	}
	//embeddings
	p_word = new nn_wv(fin);
	p_pos = new nn_wv(fin);
	p_distance = new nn_wv(fin);
	p_sd = new nn_wv(fin);
	if(the_option->NN_add_sent)
		p_sl = new sl_part(fin,the_option,get_order(),p_word,p_pos,p_sd);
}

void Csnn::write_params(std::ofstream &fout){
	p_out->write_params(fout);
	p_h->write_params(fout);
	//special untied param
	int all = the_option->NN_pnum*the_option->NN_pnum+1;
	int un_num = 0;
	for(int i=0;i<all;i++){
		if(p_untied->at(i) != 0)
			un_num ++;
	}
	fout.write((char*)&un_num,sizeof(int));
	for(int i=0;i<all;i++){
		if(p_untied->at(i) != 0){
			fout.write((char*)&i,sizeof(int));
			p_untied->at(i)->write_params(fout);
		}
	}
	//embeddings
	p_word->write_params(fout);
	p_pos->write_params(fout);
	p_distance->write_params(fout);
	p_sd->write_params(fout);
	if(the_option->NN_add_sent)
		p_sl->write_params(fout);
}

//-----------main methods-------------------------------
void Csnn::prepare_batch()
{
	this_input=0;
	this_bsize=0;
	this_mbsize=0;

	//inactive
	delete p_untied_touched;
	p_untied_touched = new vector<int>(p_untied->size(),0);
	//set dropout if ...
	prepare_dropout();
}

REAL* Csnn::forward(nn_input* in,int testing,nn_cache** c_for_pr)
{
	//testing is set when no book-keeping for backward (when testing or other specific situations)
	//1.prepare inputs
	this_input = in;
	this_bsize = in->get_numi();
	//this_mbsize += this_bsize;	//not here, add when backward
	prepare_caches(this_bsize);		//set bsize and clear gradients
	this_untied_index.clear();
	f_inputs();		/**********VIRTUAL***********/	//now c_wv ready, and here also prepare this_untied_index(unchecked)

	//2.0: c_wv dropout, special option semantics
	if(the_option->NN_dropout_repr < 0)
		c_wv->activate(nn_math::ACT_LIN,this_bsize,-1*the_option->NN_dropout_repr,testing);

	//2.1:input->wrepr --- need take care of untied
	int input_size = p_untied->at(0)->geti();
	int output_size = p_untied->at(0)->geto();
	switch(the_option->NN_untied_dim){
		case nn_options::NN_UNTIED_NOPE:
		{
		//no untied --- matrix * matrix
		nn_wb* tmp = p_untied->at(0);
		tmp->forward(c_wv->get_values(),c_wrepr->get_values(),this_bsize);
		p_untied_touched->at(0) = 1;
		break;
		}

		case nn_options::NN_UNTIED_H:
		case nn_options::NN_UNTIED_M:
		case nn_options::NN_UNTIED_HM:
		{
		//untied --- one by one
		REAL* ptr_in = c_wv->get_values();
		REAL* ptr_out = c_wrepr->get_values();
		for(int i=0;i<this_bsize;i++){
			nn_wb* tmp = p_untied->at(this_untied_index[i]);
			if(testing && tmp == 0){
				//back to 0
				tmp = p_untied->at(0);
				this_untied_index[i] = 0;
			}
			else if(tmp==0){
				//create new one by copying [0]
				p_untied->at(this_untied_index[i]) = new nn_wb(*p_untied->at(0));
				tmp = p_untied->at(this_untied_index[i]);
			}
			tmp->forward(ptr_in,ptr_out,1);
			p_untied_touched->at(this_untied_index[i]) = 1;
			//NOT-THIS-ONE: tmp->set_updating(true);	//really update
			ptr_in += input_size;
			ptr_out += output_size;
		}
		break;
		}

		default:
		{
		cerr << "!!! Unknown untied-dim" << endl;
		break;
		}
	}

	//2.1.sl
	if(the_option->NN_add_sent)
		p_sl->forward(this_input,c_srepr->get_values());

	//2.1.x: active and drop-out
	c_wrepr->activate(the_option->NN_act_repr,this_bsize,the_option->NN_dropout_repr,testing);	//!!special NN_act_wrepr
	c_srepr->activate(the_option->NN_act_repr,this_bsize,the_option->NN_dropout_repr,testing);

	//2.x: combine and get c_repr, and then activate and drop-out
	vector<nn_cache*> tmp_list;tmp_list.push_back(c_wrepr);tmp_list.push_back(c_srepr);
	c_repr->combine_cache_value(tmp_list,this_bsize);
	//!!NO ACTHERE!! -- c_repr->activate(the_option->NN_act,this_bsize,the_option->NN_dropout,testing);

	//3: repr->hidden
	p_h->forward(c_repr->get_values(),c_h->get_values(),this_bsize);

	//3.x: activate and possible drop-out
	c_h->activate(the_option->NN_act,this_bsize,the_option->NN_dropout,testing);

	//4. hidden->out
	p_out->forward(c_h->get_values(),c_out->get_values(),this_bsize);

	//4.x: possible softmax
	if(the_option->NN_out_prob){
		c_out->calc_softmax(this_bsize);
	}

	//!!5.perceptron part
	if(!p_pr){
		REAL *ret = new REAL[this_bsize*p_out->geto()];
		memcpy(ret,c_out->get_values(),this_bsize*the_option->NN_out_size*sizeof(REAL));
		return ret;
	}
	else{
		nn_cache* c_pr = get_allrepr();
		if(c_for_pr){
			*c_for_pr = c_pr;
			return 0;	//only from Csnn::update_pr
		}
		else{
			REAL* ret = new REAL[this_bsize*p_pr->geto()];	//directly output
			p_pr->forward(c_pr->get_values(),ret,this_bsize);
			delete c_pr;
			return ret;
		}
	}
}

//bs must be this_bsize...
// -- here add gradient for possible outside gradients (for direct link)
void Csnn::backward(REAL* gradients)
{
	this_mbsize += this_bsize;
	//1:prepare the output gradient --- ignore softmax (this is done by outside)
	REAL* to_grad = c_out->get_gradients();
	for(int i=0;i<this_bsize*the_option->NN_out_size;i++)
		to_grad[i] += gradients[i];

	//2:out->hidden
	p_out->backward(c_out->get_gradients(),c_h->get_gradients(),c_h->get_values(),this_bsize);	//no active or drop

	//2.x:hidden
	c_h->backgrad(the_option->NN_act,this_bsize,the_option->NN_dropout);

	//3:hidden->repr
	p_h->backward(c_h->get_gradients(),c_repr->get_gradients(),c_repr->get_values(),this_bsize);

	//3.x:repr and split it
	//!!NO-BACKGRAD HERE!! -- c_repr->backgrad(the_option->NN_act,this_bsize,the_option->NN_dropout);
	vector<nn_cache*> tmp_list;tmp_list.push_back(c_wrepr);tmp_list.push_back(c_srepr);
	c_repr->dispatch_cache_grad(tmp_list,this_bsize);	//here gradient not adding, but really too lazy to change it

	//4.0.x: wrepr backgrad
	c_wrepr->backgrad(the_option->NN_act_repr,this_bsize,the_option->NN_dropout_repr);	//!!NN_act_wrepr
	c_srepr->backgrad(the_option->NN_act_repr,this_bsize,the_option->NN_dropout_repr);

	//4.1:wrepr->input --- the untied
	if(the_option->NN_untied_dim==nn_options::NN_UNTIED_NOPE){
		//no untied, matrix * matrix
		p_untied->at(0)->backward(c_wrepr->get_gradients(),c_wv->get_gradients(),c_wv->get_values(),this_bsize);
	}
	else{
		//untied --- one by one
		int input_size = p_untied->at(0)->geti();
		int output_size = p_untied->at(0)->geto();
		REAL* ptr_in = c_wv->get_gradients();
		REAL* ptr_in_v = c_wv->get_values();
		REAL* ptr_out = c_wrepr->get_gradients();
		for(int i=0;i<this_bsize;i++){
			nn_wb* tmp = p_untied->at(this_untied_index[i]);
			tmp->backward(ptr_out,ptr_in,ptr_in_v,1);
			ptr_in += input_size;
			ptr_out += output_size;
		}
	}

	//4.1.sl
	if(the_option->NN_add_sent)
		p_sl->backward(c_srepr->get_gradients());

	//5: the input wv
	//-c_wv dropout, special option semantics
	if(the_option->NN_dropout_repr < 0)
		c_wv->backgrad(nn_math::ACT_LIN,this_bsize,-1*the_option->NN_dropout_repr);

	// - no activation
	b_inputs();			/**********VIRTUAL***********/
}

void Csnn::update(int way,REAL lrate,REAL wdecay,REAL m_alpha,REAL rms_smooth)
{
	if(this_mbsize <= 0)	//no updates
		return;
	if(p_out->need_updating())
		p_out->update(way,lrate,wdecay,m_alpha,rms_smooth,this_mbsize);
	if(p_h->need_updating())
		p_h->update(way,lrate,wdecay,m_alpha,rms_smooth,this_mbsize);
	for(unsigned int i=0;i<p_untied->size();i++){
		nn_wb* ttt = p_untied->at(i);
		if(ttt != 0 && ttt->need_updating() && p_untied_touched->at(i))
			ttt->update(way,lrate,wdecay,m_alpha,rms_smooth,this_mbsize);
	}
	if(p_word->need_updating())
		p_word->update(way,lrate,wdecay,m_alpha,rms_smooth,this_mbsize);
	if(p_pos->need_updating())
		p_pos->update(way,lrate,wdecay,m_alpha,rms_smooth,this_mbsize);
	if(p_distance->need_updating())
		p_distance->update(way,lrate,wdecay,m_alpha,rms_smooth,this_mbsize);
	if(p_sd->need_updating())
		p_sd->update(way,lrate,wdecay,m_alpha,rms_smooth,this_mbsize);
	if(the_option->NN_add_sent && p_sl->need_updating())
		p_sl->update(way,lrate,wdecay,m_alpha,rms_smooth,this_mbsize);
	this_mbsize = 0;
}

void Csnn::nesterov_update(int way,REAL m_alpha)
{
	//if no momentum
	if(!nn_math::opt_hasmomentum[way]){
		//cerr << "!! warning this update-way has no momentum." << endl;
		return;
	}
	//currently only for nn_wb
	if(p_out->need_updating())
		p_out->nesterov_update(way,m_alpha);
	if(p_h->need_updating())
		p_h->nesterov_update(way,m_alpha);
	for(unsigned int i=0;i<p_untied->size();i++){
		nn_wb* ttt = p_untied->at(i);
		if(ttt != 0 && ttt->need_updating())
			ttt->nesterov_update(way,m_alpha);
	}
	if(the_option->NN_add_sent && p_sl->need_updating())
		p_sl->nesterov_update(way,m_alpha);
}


Csnn::~Csnn()
{
	for(int i=0;i<c_allcaches->size();i++)
		delete c_allcaches->at(i);
	delete c_allcaches;
	//remains ... (too lazy to do it ...)
}
