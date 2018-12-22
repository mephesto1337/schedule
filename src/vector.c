#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "check.h"
#include "vector.h"

struct vector_s {
    void **buffer;
    size_t len;
    size_t size;
};

void vector_resize(vector_t *v) {
    v->size <<= 1;
    if ((v->buffer = realloc(v->buffer, v->size)) == NULL) {
        exit(EXIT_FAILURE);
    }
}

void vector_init(vector_t **pv) {
    vector_t *v = NULL;

    CHK_NULL(v = malloc(sizeof(struct vector_s)));
    v->len = 0UL;
    v->size = 32UL;
    CHK_NULL(v->buffer = malloc(sizeof(void *) * v->size));
    *pv = v;
}

void vector_free(vector_t *v) {
    if (v->buffer) {
        free(v->buffer);
    }
    memset(v, 0, sizeof(struct vector_s));
}

bool vector_set(vector_t *v, size_t idx, void *element) {
    if (idx < v->len) {
        v->buffer[idx] = element;
        return true;
    }

    return false;
}

bool vector_get(const vector_t *v, size_t idx, void **ret) {
    if (idx < v->len) {
        *ret = v->buffer[idx];
        return true;
    }
    return false;
}

void vector_push(vector_t *v, void *element) {
    if (v->len == v->size) {
        vector_resize(v);
    }
    v->buffer[v->len] = element;
    v->len++;
}

bool vector_pop(vector_t *v, void **element) {
    if (v->len > 0) {
        *element = v->buffer[0];
        v->len--;
        for ( size_t i = 0; i < v->len; i++ ) {
            v->buffer[i] = v-> buffer[i + 1];
        }
        v->buffer[v->len] = NULL;
        return true;
    }
    return false;
}

size_t vector_len(const vector_t *v) { return v->len; }

size_t vector_size(const vector_t *v) { return v->size; }
