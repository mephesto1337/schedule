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

#define CHK_ASYNC(fd, event, expr, is_error)                                                       \
    do {                                                                                           \
        if ((expr)is_error) {                                                                      \
            switch (errno) {                                                                       \
                case EWOULDBLOCK:                                                                  \
                    debug("[WOULDBLOCK] Schedule :%s ", #expr);                                    \
                    schedule(fd, event);                                                           \
                    break;                                                                         \
                case EINPROGRESS:                                                                  \
                    debug("[INPROGRESS] Schedule :%s ", #expr);                                    \
                    schedule(fd, event);                                                           \
                    break;                                                                         \
                case EALREADY:                                                                     \
                    debug("[ALREADY   ] Schedule :%s ", #expr);                                    \
                    schedule(fd, event);                                                           \
                    break;                                                                         \
                default:                                                                           \
                    perror(#expr);                                                                 \
                    goto fail;                                                                     \
            }                                                                                      \
        } else {                                                                                   \
            break;                                                                                 \
        }                                                                                          \
    } while (1)

#define CHK_ASYNC_NEG(fd, event, expr) CHK_ASYNC(fd, event, expr, < 0)

struct print_async_arg_s {
    const char *host;
    int port;
};

void *print_async(void *ptr) {
    struct print_async_arg_s *arg = (struct print_async_arg_s *)ptr;
    int sock = -1;
    ssize_t read_size;
    char buf[512];
    struct sockaddr_in ep;
    char local_ip[INET_ADDRSTRLEN];
    int local_port;
    struct sockaddr_in local_addr;
    socklen_t local_addr_len = sizeof(struct sockaddr_in);


    memset(&ep, 0, sizeof(struct sockaddr_in));
    ep.sin_addr.s_addr = inet_addr(arg->host);
    ep.sin_port = htons(arg->port & 0xffff);
    ep.sin_family = AF_INET;

    CHK_NEG(sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    CHK_NEG(fcntl(sock, F_SETFL, O_NONBLOCK));
    CHK_ASYNC_NEG(sock, POLLIN, connect(sock, (const struct sockaddr *)&ep, sizeof(ep)));
    getsockname(sock, (struct sockaddr *)&local_addr, &local_addr_len);
    inet_ntop(AF_INET, (void *)&local_addr.sin_addr, local_ip, sizeof(local_ip));
    local_port = (int)htons(local_addr.sin_port);

    do {
        CHK_ASYNC_NEG(sock, POLLIN, read_size = recv(sock, buf, sizeof(buf), 0));
        printf("Read %ld bytes from %s:%d\n", read_size, arg->host, arg->port);
    } while (read_size > 0);
    printf("Closing connection %s:%d <-> %s:%d\n", local_ip, local_port, arg->host, arg->port);
    shutdown(sock, SHUT_RDWR);
    close(sock);

    return (void *)arg->host;

fail:
    return (void *)"failed !";
}

int main(int argc, char *const argv[]) {
    struct print_async_arg_s args[argc - 1];
    task_id_t ids[argc - 1];
    const char *ret = NULL;

    for (int i = 1; i < argc; i++) {
        args[i - 1].host = argv[i];
        args[i - 1].port = 1337;
        ids[i - 1] = schedule_task(print_async, &args[i - 1], argv[i]);
    }
    start_runtime();

    printf("All done...\n");
    for (int i = 1; i < argc; i++) {
        if (get_return_value(ids[i - 1], (void **)&ret)) {
            printf("Task %lu returned \"%s\"\n", ids[i - 1], ret);
        } else {
            printf("get_return_value(%lu) failed\n", ids[i - 1]);
        }
    }

    return EXIT_SUCCESS;
}
