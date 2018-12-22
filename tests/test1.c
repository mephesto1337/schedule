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
#include "log.h"
#include "schedule.h"

#define CHK_ASYNC(expr, is_error)                                                                  \
    do {                                                                                           \
        if ((expr)is_error) {                                                                      \
            switch (errno) {                                                                       \
                case EWOULDBLOCK:                                                                  \
                    debug("[WOULDBLOCK] Schedule :%s ", #expr);                                    \
                    schedule();                                                                    \
                    break;                                                                         \
                case EINPROGRESS:                                                                  \
                    debug("[INPROGRESS] Schedule :%s ", #expr);                                    \
                    schedule();                                                                    \
                    break;                                                                         \
                default:                                                                           \
                    perror(#expr);                                                                 \
                    exit(EXIT_FAILURE);                                                            \
            }                                                                                      \
        } else {                                                                                   \
            break;                                                                                 \
        }                                                                                          \
    } while (1)
#define CHK_ASYNC_NULL(expr) CHK_ASYNC(expr, == NULL)
#define CHK_ASYNC_NEG(expr) CHK_ASYNC(expr, < 0)

struct print_async_arg_s {
    const char *host;
    int port;
};

void print_async(void *ptr) {
    struct print_async_arg_s *arg = (struct print_async_arg_s *)ptr;
    int sock = -1;
    ssize_t read_size;
    char buf[512];
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
}

int main(int argc, char *const argv[]) {
    struct print_async_arg_s *args = NULL;

    for (int i = 1; i < argc; i++) {
        args = malloc(sizeof(struct print_async_arg_s));
        args->host = argv[i];
        args->port = 1337;
        schedule_task(print_async, args, argv[i]);
    }
    start_runtime();

    printf("All done...\n");

    return EXIT_SUCCESS;
}
