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
    size_t start;
};

void *secure_malloc(size_t size) {
    void *ptr = NULL;

    if ((ptr = malloc(size)) != NULL) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void vector_rebase(vector_t *v) {
    if (v->start == 0 || v->start + v->len < v->size) {
        return;
    }
    for (size_t idx = 0; idx < v->len; idx++) {
        v->buffer[idx] = v->buffer[idx + v->start];
    }
    v->start = 0UL;
    return;
}

void vector_resize(vector_t *v) {
    void **buffer = NULL;

    v->size <<= 1;
    if ((buffer = secure_malloc(sizeof(void *) * v->size)) == NULL) {
        exit(EXIT_FAILURE);
    }
    for (size_t idx = 0; idx < v->len; idx++) {
        buffer[idx] = v->buffer[v->start + idx];
    }
    free(v->buffer);
    v->buffer = buffer;
    v->start = 0UL;
    return;
}

void vector_init(vector_t **pv) {
    vector_t *v = NULL;

    CHK_NULL(v = secure_malloc(sizeof(struct vector_s)));
    v->len = 0UL;
    v->size = 32UL;
    v->start = 0UL;
    CHK_NULL(v->buffer = secure_malloc(sizeof(void *) * v->size));
    *pv = v;
}

void vector_free(vector_t *v) {
    if (v->buffer) {
        memset(v->buffer, 0, v->size);
        free(v->buffer);
    }
    memset(v, 0, sizeof(struct vector_s));
}

bool vector_set(vector_t *v, size_t idx, void *element) {
    if (idx < v->len) {
        v->buffer[v->start + idx] = element;
        return true;
    }

    return false;
}

bool vector_get(const vector_t *v, size_t idx, void **element) {
    if (idx < v->len) {
        if (element != NULL) {
            *element = v->buffer[v->start + idx];
        }
        return true;
    }
    return false;
}

void vector_push(vector_t *v, void *element) {
    if (v->len == v->size) {
        vector_resize(v);
    } else {
        vector_rebase(v);
    }
    v->buffer[v->start + v->len] = element;
    v->len++;
}

bool vector_pop(vector_t *v, void **element) {
    if (v->len > 0UL) {
        if (element != NULL) {
            *element = v->buffer[v->start];
        }
        v->start++;
        v->len--;
        return true;
    }

    return false;
}

bool vector_remove(vector_t *v, size_t idx, void **element) {
    if (idx == 0UL) {
        return vector_pop(v, element);
    } else if (idx + 1UL == v->len) {
        if (element != NULL) {
            *element = v->buffer[v->start + idx];
        }
        v->len--;
        return true;
    } else {
        if (idx < v->len) {
            if (element != NULL) {
                *element = v->buffer[v->start + idx];
            }
            v->len--;
            if ((idx << 1) >= v->len) {
                for (size_t i = idx; i < v->len; i++) {
                    v->buffer[v->start + i] = v->buffer[v->start + i + 1];
                }
            } else {
                for (size_t i = 0; i < idx; i++) {
                    v->buffer[v->start + (idx - i)] = v->buffer[v->start + (idx - i) - 1];
                }
                v->start++;
            }
            return true;
        }
    }

    return false;
}

size_t vector_len(const vector_t *v) { return v->len; }

size_t vector_size(const vector_t *v) { return v->size; }
