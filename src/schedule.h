#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

typedef void (*task_func_t)(void *);

extern void schedule(void);
extern void task_done(void);
extern void schedule_task(task_func_t entry, void *arg, const char *name);
extern void start_runtime(void);

#endif // __SCHEDULE_H__
