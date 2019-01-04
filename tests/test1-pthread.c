#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "check.h"
#include "log.h"

#define CHK_SYNC(expr, is_error)                                                                   \
    do {                                                                                           \
        if ((expr)is_error) {                                                                      \
            perror(#expr);                                                                         \
            goto fail;                                                                             \
        }                                                                                          \
    } while (0)
#define CHK_SYNC_NEG(expr) CHK_SYNC(expr, < 0)

struct print_async_arg_s {
    const char *host;
    int port;
};

void *print_async(void *ptr) {
    struct print_async_arg_s *arg = (struct print_async_arg_s *)ptr;
    int sock = -1;
    ssize_t read_size;
    char buf[512];
    char local_ip[INET_ADDRSTRLEN];
    int local_port;
    struct sockaddr_in local_addr;
    socklen_t local_addr_len = sizeof(struct sockaddr_in);

    struct sockaddr_in ep;

    memset(&ep, 0, sizeof(struct sockaddr_in));
    ep.sin_addr.s_addr = inet_addr(arg->host);
    ep.sin_port = htons(arg->port & 0xffff);
    ep.sin_family = AF_INET;

    CHK_NEG(sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    CHK_SYNC_NEG(connect(sock, (const struct sockaddr *)&ep, sizeof(ep)));
    getsockname(sock, (struct sockaddr *)&local_addr, &local_addr_len);
    inet_ntop(AF_INET, (void *)&local_addr.sin_addr, local_ip, sizeof(local_ip));
    local_port = (int)htons(local_addr.sin_port);

    do {
        CHK_SYNC_NEG(read_size = recv(sock, buf, sizeof(buf), 0));
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
    pthread_t ids[argc - 1];
    const char *ret = NULL;

    for (int i = 1; i < argc; i++) {
        args[i - 1].host = argv[i];
        args[i - 1].port = 1337;
        CHK_NEG(pthread_create(&ids[i - 1], NULL, print_async, (void *)&args[i - 1]));
    }

    printf("Will wait for tasks...\n");
    for (int i = 1; i < argc; i++) {
        CHK_NEG(pthread_join(ids[i - 1], (void **)&ret));
        printf("Task %lu returned \"%s\"\n", ids[i - 1], ret);
    }

    return EXIT_SUCCESS;
}
