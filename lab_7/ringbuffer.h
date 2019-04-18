#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

struct pck_data {
	uint32_t in_addr;
	uint32_t out_addr;
	uint16_t in_port;
	uint16_t out_port;
	int proto;
};


struct pck_data_kbuff{
	struct pck_data *buff;
	size_t len;
	size_t begin;
	size_t end;
	int is_empty;
};

typedef void (pck_data_handler)(struct pck_data*);

struct pck_data_kbuff make_pck_data_kbuff(size_t kbuff_len) {
	struct pck_data_kbuff kbuff = {
//	.buff = kcalloc(kbuff_len, sizeof(struct pck_data_kbuff), GFP_KERNEL),
	.buff = calloc(kbuff_len, sizeof(struct pck_data_kbuff)),
		.len = kbuff_len,
		.begin = 0,
		.end = 0,
		.is_empty = 1,
	};
	return kbuff;
}

void add_pck_data(struct pck_data_kbuff *data_source, struct pck_data *new_element) {
	struct pck_data* buff = data_source->buff;
	if (data_source->end == data_source->begin && !data_source->is_empty) {
		data_source->begin = (data_source->begin + 1 ) % data_source->len;
	}
	buff[data_source->end] = *new_element;
	data_source->end = (data_source->end + 1) % data_source->len;
	data_source->is_empty = 0;
}

void pop_pck_data(struct pck_data_kbuff *data_source) {
	if (data_source->is_empty) {
		return;
	}
	data_source->begin = (data_source->begin + 1) % data_source->len;	
	if (data_source->begin == data_source->end) {
		data_source->is_empty = 1;
	}
}

void for_each_in_kbuff(struct pck_data_kbuff *data_source, pck_data_handler handler) {
	for( size_t i = data_source->begin; i != data_source->end; i = (i + 1) % data_source->len) {
		handler(&data_source->buff[i]);
	}
}	
