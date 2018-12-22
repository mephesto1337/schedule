# schedule

Small C library to switch between tasks (which must not block)
Go see tests/test1.c for an example.

Basicly, you :

- call ``schedule_task(void (*)(void *) func, void *arg, const char *name)`` to schedule a task for latter
- once all your tasks are schduled, call ``start_runtime``
- when it returns, all your tasks are done
