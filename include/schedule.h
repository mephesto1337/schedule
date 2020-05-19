#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <poll.h>
#include <stdbool.h>

typedef enum task_status_e {
    READY,
    RUNNING,
    WAITING,
} task_status_t;

typedef void *(*task_func_t)(void *);
typedef unsigned long task_id_t;

extern void schedule(int fd, short event);
extern task_id_t get_self(void);
extern task_id_t schedule_task(task_func_t entry, void *arg, const char *name);
extern bool get_return_value(task_id_t id, void **ret);
extern void start_runtime(void);

#endif // __SCHEDULE_H__
