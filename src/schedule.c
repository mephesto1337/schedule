#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <ucontext.h>
#include <unistd.h>

#include "check.h"
#include "schedule.h"
#include "vector.h"

#define STACK_SIZE 0x400UL
#define unreachable()                                                                              \
    do {                                                                                           \
        fprintf(stderr, "%s:%d -> Should not be reached", __FILE__, __LINE__);                     \
        exit(EXIT_FAILURE);                                                                        \
    } while (0)

vector_t *coroutines_env = NULL;
struct task {
    ucontext context;
    const char *name;
};
struct task *current_task = NULL;

void __attribute__((constructor)) __init_scheduler__(void) { vector_init(&coroutines_env); }

void __attribute__((destructor)) __exit_scheduler__(void) { vector_free(coroutines_env); }

void start_next_task(void) {
    jmp_buf cur_env;
    struct task *task = NULL;

    // Restore a saved task
    if (vector_pop(coroutines_env, (void **)&task) == false) {
        longjmp(runtime_return, 1);
        unreachable();
    }
    memcpy(&cur_env, &task->env, sizeof(cur_env));
    free(task);
    longjmp(cur_env, 1);

    unreachable();
}

void schedule(void) {
    jmp_buf cur_env;
    struct task *task = NULL;

    // Save the current task
    if (setjmp(cur_env) == 0) {
        CHK_NULL(task = malloc(sizeof(cur_env)));
        memcpy(&task->env, &cur_env, sizeof(cur_env));
        vector_push(coroutines_env, (void *)task);
    } else {
        return;
    }

    start_next_task();
}

void task_done(void) { start_next_task(); }

void schedule_task(task_func_t entry, void *arg, const char *name) {
    struct task *task = NULL;

    CHK_NULL(task = malloc(sizeof(struct task)));
    CHK_NEG(getcontext(&task->context));
    task->name = name;
    CHK_NULL(task->context.uc_stack.ss_sp =
                 mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1 0));
    task->context.uc_stack.
    if (setjmp(cur_env) == 0) {
        CHK_NULL(task = malloc(sizeof(struct task)));
        memcpy(&task->env, &cur_env, sizeof(cur_env));
        vector_push(coroutines_env, (void *)task);
        printf("Task scheduled for latter\n");
    } else {
        printf("[AFTER ] func = %p / arg = %p\n", context->func, context->args);
        context->func(context->args);
    }
}

void start_runtime(void) {
    if (vector_len(coroutines_env) == 0) {
        perror("No task to run");
        return;
    }

    if (setjmp(runtime_return) == 0) {
        printf("Starting runtime\n");
        start_next_task();
    }
    printf("Ending runtime\n");
}
