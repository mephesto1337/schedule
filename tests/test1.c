#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "check.h"
#include "schedule.h"

#define CHK_ASYNC(expr, is_error)                                                                  \
    do {                                                                                           \
        if ((expr)is_error) {                                                                      \
            if (errno == EWOULDBLOCK || errno == EINPROGRESS) {                                    \
                printf("Schedule (%s)\n", #expr);                                                  \
                schedule();                                                                        \
            } else {                                                                               \
                perror(#expr);                                                                     \
                exit(EXIT_FAILURE);                                                                \
            }                                                                                      \
        }                                                                                          \
    } while (0)
#define CHK_ASYNC_NULL(expr) CHK_ASYNC(expr, == NULL)
#define CHK_ASYNC_NEG(expr) CHK_ASYNC(expr, < 0)

struct print_async_arg_s {
    const char *host;
    int port;
};

void print_async(void *ptr) {
    struct print_async_arg_s *arg = (struct print_async_arg_s *)ptr;
    printf("print_async(host=\"%s\", port=%d)\n", arg->host, arg->port);
    int sock = -1;
    ssize_t read_size;
    char buf[BUFSIZ];
    struct sockaddr_in ep;

    memset(&ep, 0, sizeof(struct sockaddr_in));
    ep.sin_addr.s_addr = inet_addr(arg->host);
    ep.sin_port = htons(arg->port & 0xffff);
    ep.sin_family = AF_INET;

    CHK_NEG(sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    CHK_NEG(fcntl(sock, F_SETFL, O_NONBLOCK));
    CHK_ASYNC_NEG(connect(sock, (const struct sockaddr *)&ep, sizeof(ep)));

    do {
        CHK_ASYNC_NEG(read_size = recv(sock, buf, sizeof(buf), 0));
        printf("Read %ld bytes from %s:%d\n", read_size, arg->host, arg->port);
    } while (read_size > 0);
    shutdown(sock, SHUT_RDWR);
    close(sock);

    task_done();
}

int main(int argc, char *const argv[]) {
    struct print_async_arg_s *args = NULL;
    struct new_task_s tasks[argc - 1];

    for (int i = 1; i < argc; i++) {
        args = malloc(sizeof(struct print_async_arg_s));
        args->host = argv[i];
        args->port = 1337;
        tasks[i - 1].func = print_async;
        tasks[i - 1].args = args;
        schedule_task(0UL, &tasks[i - 1]);
    }
    start_runtime();

    printf("Will wait...\n");
    sleep(30);

    return EXIT_SUCCESS;
}
