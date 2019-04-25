#ifndef USERSPACE
#include <linux/types.h>
#include <linux/slab.h>
#else
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#endif

struct pck_data {
    uint32_t in_addr;
    uint32_t out_addr;
    uint16_t in_port;
    uint16_t out_port;
    int proto;
};


struct pck_data_kbuff {
    struct pck_data *buff;
    size_t len;
    size_t begin;
    size_t end;
    int is_empty;
};



typedef void (pck_data_handler)(struct pck_data *);

struct pck_data_kbuff make_pck_data_kbuff(size_t kbuff_len) {
    struct pck_data_kbuff kbuff = {
#ifndef USERSPACE
	    .buff = kcalloc(kbuff_len, sizeof(struct pck_data_kbuff), GFP_KERNEL),
#else
            .buff = calloc(kbuff_len, sizeof(struct pck_data_kbuff)),
#endif
            .len = kbuff_len,
            .begin = 0,
            .end = 0,
            .is_empty = 1,
    };
    return kbuff;
}

void free_pck_data_kbuff(struct pck_data_kbuff *data_source) {
#ifndef USERSPACE
    kfree(data_source->buff);
#else
    free(data_source->buff);
#endif
    data_source->begin = 0;
    data_source->end = 0;
    data_source->is_empty = 1;
}

void reset_pck_data_kbuff(struct pck_data_kbuff *data_source) {
    data_source->begin = 0;
    data_source->end = 0;
    data_source->is_empty = 1;
}

void add_pck_data(struct pck_data_kbuff *data_source, struct pck_data *new_element) {
    struct pck_data *buff = data_source->buff;
    if (data_source->end == data_source->begin && !data_source->is_empty) {
        data_source->begin = (data_source->begin + 1) % data_source->len;
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
    size_t i;
    if (data_source->is_empty) {
        return;
    }
    i = data_source->begin;

    do {
        handler(&data_source->buff[i]);
        i = (i + 1) % data_source->len;
    } while (i != data_source->end);
}
