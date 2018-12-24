#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <ucontext.h>
#include <unistd.h>

#include "check.h"
#include "log.h"
#include "schedule.h"
#include "vector.h"

#define STACK_SIZE 0x23000UL
#define unreachable()                                                                              \
    do {                                                                                           \
        fprintf(stderr, "%s:%d -> Should not be reached\n", __FILE__, __LINE__);                   \
        exit(EXIT_FAILURE);                                                                        \
    } while (0)

struct task {
    ucontext_t context;
    char *name;
    task_id_t id;
    task_status_t status;
    int fd;
    short event;
};

struct task_return {
    task_id_t id;
    void *ret;
};

vector_t *coroutines = NULL;
vector_t *return_values = NULL;
task_id_t task_id_counter = 0UL;
struct task *current_task = NULL;
ucontext_t uctx_main;
size_t new_context_count = 0UL;

void __attribute__((constructor)) __init_scheduler__(void) {
    vector_init(&coroutines);
    vector_init(&return_values);
}

void __attribute__((destructor)) __exit_scheduler__(void) {
    vector_free(coroutines);
    vector_free(return_values);
}

void start_task_and_save_ret(task_id_t id, task_func_t func, void *arg) {
    struct task_return *ret = NULL;

    CHK_NULL(ret = malloc(sizeof(struct task_return)));
    ret->ret = func(arg);
    ret->id = id;
    vector_push(return_values, (void *)ret);
}

struct task *get_first_ready_task(void) {
    struct task *t = NULL;
    size_t nfds = vector_len(coroutines);
    struct pollfd fds[nfds];
    int ready_fds = 0;

    if (nfds == 0) {
        return NULL;
    }

    for (size_t i = 0; i < nfds; i++) {
        vector_get(coroutines, i, (void **)&t);
        if (t->status == READY) {
            debug("Task %lu (%s) is READY", t->id, t->name);
            vector_remove(coroutines, i, NULL);
            return t;
        }
        fds[i].fd = t->fd;
        fds[i].events = t->event;
        fds[i].revents = 0;
    }

    // No task was ready, let's poll !
    debug("Will poll, no task is ready");
    CHK_NEG(ready_fds = poll(fds, nfds, 500));
    for (size_t i = 0; i < nfds; i++) {
        vector_get(coroutines, i, (void **)&t);

        if (fds[i].revents & fds[i].events) {
            vector_remove(coroutines, i, NULL);
            t->status = READY;
            debug("Task %lu (%s) is READY", t->id, t->name);
            return t;
        }
    }

    // No task is ready, still return the first one
    vector_pop(coroutines, (void **)&t);
    debug("Task %lu (%s) is not *really* READY, but YOLO !", t->id, t->name);
    t->status = READY;
    return t;
}

void start_task(struct task *new_task, struct task *old_task) {
    debug("starting task %lu (%s)", new_task->id, new_task->name);
    new_task->status = RUNNING;
    new_task->fd = -1;
    swapcontext(&old_task->context, &new_task->context);
}

void schedule(int fd, short event) {
    struct task *task = current_task;

    debug("stopping task %s", task->name);
    task->fd = fd;
    task->status = (fd > 0) ? WAITING : READY;
    task->event = event;
    vector_push(coroutines, task);

    current_task = get_first_ready_task();
    start_task(current_task, task);
}

task_id_t get_self(void) { return current_task->id; }

task_id_t schedule_task(task_func_t entry, void *arg, const char *name) {
    struct task *task = NULL;

    CHK_NULL(task = malloc(sizeof(struct task)));
    CHK_NEG(getcontext(&task->context));
    if (name != NULL) {
        task->name = strdup(name);
        new_context_count++;
    } else {
        asprintf(&task->name, "task-%lu", ++new_context_count);
    }
    task->id = task_id_counter++;
    task->context.uc_stack.ss_size = STACK_SIZE;
    CHK_NULL(task->context.uc_stack.ss_sp =
                 mmap(NULL, task->context.uc_stack.ss_size, PROT_READ | PROT_WRITE,
                      MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
    task->context.uc_link = &uctx_main;
    task->fd = -1;
    task->status = READY;
    makecontext(&task->context, (void (*)(void))start_task_and_save_ret, 3, task->id, entry, arg);
    vector_push(coroutines, task);
    debug("New task created : %s (func=%p, arg=%p, stack=%p)", task->name, entry, arg,
          task->context.uc_stack.ss_sp);

    return task->id;
}

bool get_return_value(task_id_t id, void **ret) {
    struct task_return *tret = NULL;

    for (size_t idx = 0; idx < vector_len(return_values); idx++) {
        vector_get(return_values, idx, (void **)&tret);
        if (tret->id == id) {
            *ret = tret->ret;
            vector_remove(return_values, idx, NULL);
            free(tret);
            return true;
        }
    }
    return false;
}

void start_runtime(void) {
    while ((current_task = get_first_ready_task()) != NULL) {
        debug("starting task %lu (%s)", current_task->id, current_task->name);
        swapcontext(&uctx_main, &current_task->context);

        // now current task is done, we can free it !
        debug("Removing task %lu (%s) from list", current_task->id, current_task->name);
        munmap(current_task->context.uc_stack.ss_sp, current_task->context.uc_stack.ss_size);
        free(current_task);
        current_task = NULL;
    }
}
