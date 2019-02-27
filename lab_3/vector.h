#ifndef LAB_3_VECTOR_H
#define LAB_3_VECTOR_H


#include <stddef.h>


struct byte_vector {
    char *data;
    size_t length;
    size_t capacity;
};


int init_byte_vector(size_t initial_capacity, struct byte_vector *vec);

int reserve(struct byte_vector *vec, size_t new_capacity);

int push_back(struct byte_vector *vec, char elem);

int extend(struct byte_vector *vec, const char *to_append, size_t elems_count);

int extend_v(struct byte_vector *vec, const struct byte_vector *other);

void clear_byte_vector(struct byte_vector *vec);

void swap(struct byte_vector *src, struct byte_vector *dst);

#endif //LAB_3_VECTOR_H
