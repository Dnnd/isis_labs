#include "vector.h"
#include <malloc.h>
#include <memory.h>

int init_byte_vector(size_t initial_capacity, struct byte_vector *vec) {
    char *buffer = calloc(initial_capacity, sizeof(char));
    if (buffer == NULL) {
        return -1;
    }
    vec->length = 0;
    vec->capacity = initial_capacity;
    vec->data = buffer;
    return 0;
}

int reserve(struct byte_vector *vec, size_t new_capacity) {
    if (vec->capacity >= new_capacity) {
        return 0;
    }

    char *new_buffer = realloc(vec->data, new_capacity);
    if (new_buffer == NULL) {
        return -1;
    }
    bzero(new_buffer + vec->capacity, new_capacity - vec->capacity);
    vec->data = new_buffer;
    vec->capacity = new_capacity;
    return 0;
}

int push_back(struct byte_vector *vec, char elem) {
    if (vec->length <= vec->capacity) {
        reserve(vec, vec->capacity * 2);
    }
    vec->data[vec->length] = elem;
    ++vec->length;
    return 0;
}

int extend(struct byte_vector *vec, const char *to_append, size_t elems_count) {
    size_t required_capacity = vec->length + elems_count;
    if (required_capacity >= vec->capacity) {
        if (required_capacity >= vec->capacity * 2) {
            reserve(vec, required_capacity);
        } else {
            reserve(vec, vec->capacity * 2);
        }
    }
    memcpy(vec->data + vec->length, to_append, elems_count);
    vec->length = required_capacity;
    return 0;
}

int extend_v(struct byte_vector *vec, const struct byte_vector *other) {
    return extend(vec, other->data, other->length);
}

void clear_byte_vector(struct byte_vector *vec) {
    free(vec->data);
    vec->data = NULL;
    vec->capacity = 0;
    vec->length = 0;
}

void swap(struct byte_vector *src, struct byte_vector *dst) {
    struct byte_vector tmp = *dst;
    dst->length = src->length;
    dst->capacity = src->capacity;
    dst->data = src->data;
    src->length = tmp.length;
    src->capacity = tmp.capacity;
    src->data = tmp.data;
}
