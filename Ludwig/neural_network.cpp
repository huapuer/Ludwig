#include "stdfx.h"
#include "ludwig.h"

layer_t* layer_list = NULL;

layer_t* pick_layer(int idx) {
	if (!layer_list) {
		ERR("COMPILE ERROR: LAYER[%d] NOT EXSISTS!\n", idx);
	}
	else {
		if (layer_list->id == idx) {
			return layer_list;
		}
		else {
			layer_t* iter = layer_list;
			while (iter->follow) {
				if (iter->follow->id == idx) {
					return iter->follow;
				}
				iter = iter->follow;
			}
		}
	}
	ERR("COMPILE ERROR: LAYER[%d] NOT EXSISTS!\n", idx);
}

layer_t* new_layer_phsical(int id, int size) {
	layer_t* ret = (layer_t*)malloc(sizeof(layer_t));
	memset(ret, 0, sizeof(layer_t));
	ret->id = id;
	ret->type = LAYER_PHSICAL;
	ret->size = size;
	ret->working_gen = 0;
	ret->integrated_gen = 0;
	ret->swap_gen = 0;
	ret->working_batch = 0;
	ret->integrating_batch = 0;
	ret->offset = 0;
	ret->cur_s_dev_t = 0;
	ret->cur_t_dev_t = 1;
	ret->phsical = ret;

	if (!layer_list) {
		layer_list = ret;
	}
	else {
		layer_t* iter = layer_list;
		while (iter->follow) {
			iter = iter->follow;
		}
		iter->follow = ret;
	}
	return ret;
}

layer_t* new_layer_logical(int id, int phsical, int offset, int size, bool delegate) {
	layer_t* ret = (layer_t*)malloc(sizeof(layer_t));
	memset(ret, 0, sizeof(layer_t));
	ret->id = id;
	ret->type = LAYER_LOGICAL;
	ret->size = size;
	ret->delegate = delegate;

	layer_t* pl = pick_layer(phsical);
	ret->phsical = pl;
	ret->offset = offset;
	ret->integrate_fn = pl->integrate_fn;
	ret->clear_push_fn = pl->clear_push_fn;
	ret->push_fn = pl->push_fn;

	if (!pl->logical_head) {
		pl->logical_head = ret;
	}
	if (pl->logical_tail) {
		pl->logical_tail->next_logical = ret;
		pl->logical_tail = pl->logical_tail->next_logical;
	}
	else {
		pl->logical_tail = ret;
	}

	if (!layer_list) {
		layer_list = ret;
	}
	else {
		layer_t* iter = layer_list;
		while (iter->follow) {
			iter = iter->follow;
		}
		iter->follow = ret;
	}
	return ret;
}

link* new_link(layer_t* layer, int size) {
	link* ret = (link*)malloc(sizeof(link));
	memset(ret, 0, sizeof(link));
	ret->mutated_gen = 0;
	ret->mutating_batch = 0;
	ret->layer = layer;
	ret->size = size;
	ret->t = (gen_w*)malloc(sizeof(gen_w)*size);
	//TODO: initialize gen_t
	/*
	cudaMalloc((void**)&ret->dev_t, size * sizeof(gen_w));
	cudaMemcpy(ret->dev_t, ret->t, size * sizeof(gen_w), cudaMemcpyHostToDevice);
	*/
	return ret;
}

void add_link(link** head, link* next) {
	if (!*head) {
		*head = next;
		return;
	}
	else {
		link* tail = *head;
		while (tail->another) {
			tail = tail->another;
		}
		tail->another = next;
	}
}

layer_t* has_t(layer_t* s, int or_another_s, layer_t* next, int or_another_next) {
	if (!s) {
		s = pick_layer(or_another_s);
	}
	if (!s) {
		ERR("COMPILE ERROR: LAYER[%d] NOT EXSISTS!\n", or_another_s);
	}

	if (!next) {
		next = pick_layer(or_another_next);
	}
	if (!next) {
		ERR("COMPILE ERROR: LAYER[%d] NOT EXSISTS!\n", or_another_next);
	}
	int size = s->size*next->size;
	link* l = new_link(next, size);
	add_link(&s->next, l);
	//add_link(&next->pre, l);
	return next;
}
