# schedule

Small C library to switch between tasks (which must not block)
It aims to be an alternative to the pthread library.

A task is function with the same prototype as the ``start_routine`` in 
``pthread_create`` :
```c
void *(*task_function)(void *);
```

The main difference is all the tasks are run in the current thread, so none 
should be blocking.

Go see [tests/test1.c] for an example.

Basically, you :

- call ``task_id_t schedule_task(void (*)(void *) func, void *arg, const char *name)`` to
schedule a task for latter
- once all your tasks are scheduled, call ``void start_runtime()``, the call
will block until all the tasks are done.
