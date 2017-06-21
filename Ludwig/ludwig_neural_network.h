#pragma once
#include <stdio.h>
#include <Windows.h>

struct layer_t;
struct link;
struct gen_t;
struct gen_w;

extern layer_t* layer_list;

enum layer_type {
	LAYER_PHSICAL,
	LAYER_LOGICAL
};

struct gen_t {
	unsigned long long gen;
	float t;
};

struct gen_w {
	unsigned long long gen;
	unsigned long long working_gen;
	int stage;
	float t;
};

struct layer_t {
	int id;
	layer_type type;
	int size;
	link* pre;
	link* next;
	layer_t* follow;
	void* integrate_fn;
	void* clear_push_fn;
	void* push_fn;

	//phsical
	unsigned long long gen;
	unsigned long long working_gen;
	unsigned long long integrated_gen;
	unsigned long long swap_gen;
	unsigned long long working_batch;
	unsigned long long integrating_batch;
	gen_t *t;
	int cur_s_dev_t;
	int cur_t_dev_t;
	gen_t *dev_t[2];
	layer_t* logical_head;
	layer_t* logical_tail;

	//logical
	int offset;
	bool delegate;
	layer_t* phsical;
	layer_t* next_logical;
};

struct link {
	unsigned long long gen;
	unsigned long long mutated_gen;
	unsigned long long mutating_batch;
	layer_t* layer;
	int size;
	gen_w* t;
	gen_w *dev_t;
	link* another;
};

#ifdef _WINDLL
#define EXPORTS _declspec(dllexport)
#else
#define EXPORTS _declspec(dllimport)
#endif
extern "C" EXPORTS layer_t* pick_layer(int);
extern "C" EXPORTS layer_t* new_layer_phsical(int, int);
extern "C" EXPORTS layer_t* new_layer_logical(int, int, int, int, bool);
extern "C" EXPORTS link* new_link(layer_t*, int);
extern "C" EXPORTS layer_t* has_t(layer_t*, int, layer_t*, int);