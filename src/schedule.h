#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <stdbool.h>

typedef void* (*task_func_t)(void *);
typedef unsigned long task_id_t;

extern void schedule(void);
extern task_id_t schedule_task(task_func_t entry, void *arg, const char *name);
extern bool get_return_value(task_id_t id, void **ret);
extern void start_runtime(void);

#endif // __SCHEDULE_H__
