#include <setjmp.h>
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
        fprintf(stderr, "%s:%d -> Should not be reached", __FILE__, __LINE__);                     \
        exit(EXIT_FAILURE);                                                                        \
    } while (0)

vector_t *coroutines = NULL;
struct task {
    ucontext_t context;
    char *name;
};
struct task *current_task = NULL;
ucontext_t uctx_main;
size_t new_context_count = 0UL;

void __attribute__((constructor)) __init_scheduler__(void) { vector_init(&coroutines); }

void __attribute__((destructor)) __exit_scheduler__(void) { vector_free(coroutines); }

void schedule(void) {
    struct task *task = current_task;

    debug("stopping task %s", current_task->name);
    vector_push(coroutines, current_task);
    vector_pop(coroutines, (void **)&current_task);
    debug("restarting task %s", current_task->name);
    swapcontext(&task->context, &current_task->context);
}

void schedule_task(task_func_t entry, void *arg, const char *name) {
    struct task *task = NULL;

    CHK_NULL(task = malloc(sizeof(struct task)));
    CHK_NEG(getcontext(&task->context));
    if (name != NULL) {
        task->name = strdup(name);
        new_context_count++;
    } else {
        asprintf(&task->name, "task-%lu", ++new_context_count);
    }
    task->context.uc_stack.ss_size = STACK_SIZE;
    CHK_NULL(task->context.uc_stack.ss_sp =
                 mmap(NULL, task->context.uc_stack.ss_size, PROT_READ | PROT_WRITE,
                      MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
    task->context.uc_link = &uctx_main;
    makecontext(&task->context, (void (*)(void))entry, 1, arg);
    vector_push(coroutines, task);
    debug("New task created : %s (func=%p, arg=%p, stack=%p)", task->name, entry, arg, task->context.uc_stack.ss_sp);

    return;
}

void start_runtime(void) {
    while (vector_pop(coroutines, (void **)&current_task)) {
        debug("starting task %s", current_task->name);
        swapcontext(&uctx_main, &current_task->context);

        // now current task is done, we can free it !
        munmap(current_task->context.uc_stack.ss_sp, current_task->context.uc_stack.ss_size);
        free(current_task);
        current_task = NULL;
    }
}
