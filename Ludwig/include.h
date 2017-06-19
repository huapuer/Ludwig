typedef void(*fp_integrate)(gen_t*, int);
typedef void(*fp_mute)(gen_w*, const unsigned long long);
typedef void(*fp_clear_push)(const gen_t *, gen_t *, int, gen_w*, const int, const unsigned long long);
typedef void(*fp_push)(const gen_t*, gen_t*, int, gen_w*, const int, const unsigned long long);

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
	fp_integrate integrate_fn;
	fp_clear_push clear_push_fn;
	fp_push push_fn;

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
	const float* dev_atte;
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