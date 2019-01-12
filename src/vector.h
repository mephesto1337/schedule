#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdbool.h>
#include <unistd.h>

typedef struct vector_s vector_t;
typedef void (*vector_callback_t)(void *element, void *user);

void vector_init(vector_t **pv);
void vector_free(vector_t *v);
bool vector_set(vector_t *v, size_t idx, void *element);
bool vector_get(const vector_t *v, size_t idx, void **element);
void vector_push(vector_t *v, void *element);
bool vector_pop(vector_t *v, void **element);
bool vector_remove(vector_t *v, size_t idx, void **element);
void vector_apply(vector_t *v, vector_callback_t cb, void *user);
size_t vector_len(const vector_t *v);
size_t vector_size(const vector_t *v);

#endif // __VECTOR_H__
