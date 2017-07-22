#pragma once
#include <stdio.h>
#include <Windows.h>

struct layer_t;
struct link;
struct gen_t;
struct gen_w;

extern layer_t* layer_list;
extern link* link_list;

enum layer_type {
	LAYER_PHSICAL,
	LAYER_LOGICAL
};

enum link_type {
	LINK_FORWARD,
	LINK_FULL
};

struct gen_t {
	unsigned long long gen;
	float* t;
};

struct gen_w {
	unsigned long long gen;
	unsigned long long working_gen;
	int stage;
	float* t;
	float* r;
	float* r2;
	float* pr;
	float* po;
};

struct layer_t {
	int id;
	layer_type type;
	int size;
	link* pre;
	link* next;
	layer_t* follow;

	//phsical
	unsigned long long gen;
	unsigned long long pulling_gen;
	unsigned long long integrated_gen;
	unsigned long long swap_gen;
	unsigned long long pulling_batch;
	unsigned long long integrating_batch;
	gen_t host_t;
	int cur_s_dev_t;
	int cur_t_dev_t;
	gen_t dev_t[2];
	layer_t* logical_head;
	layer_t* logical_tail;
	layer_t* updated_pre;
	layer_t* updated_next;

	//logical
	int offset;
	bool delegate;
	layer_t* phsical;
	layer_t* next_logical;
};

struct link {
	int id;
	link_type type;
	unsigned long long gen;
	unsigned long long mutated_gen;
	unsigned long long pushed_gen;
	unsigned long long mutating_batch;
	unsigned long long counting_batch;
	layer_t* t_layer;
	layer_t* s_layer;
	int size;
	gen_w host_t;
	gen_w dev_t;
	link* another_next;
	link* another_pre;
	link* follow;
};

#ifdef _WINDLL
#define EXPORTS _declspec(dllexport)
#else
#define EXPORTS _declspec(dllimport)
#endif
extern "C" EXPORTS layer_t* pick_layer(int);
extern "C" EXPORTS link* pick_link(int);
extern "C" EXPORTS layer_t* has_layer_phsical(int, int);
extern "C" EXPORTS layer_t* has_layer_logical(int, int, int, int, bool);
extern "C" EXPORTS layer_t* has_link(int, link_type, layer_t*, int, layer_t*, int);