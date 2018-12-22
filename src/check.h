#ifndef __CHECK_H__
#define __CHECK_H__

#include <stdio.h>

#define CHK(expr, is_error)                                                                        \
    do {                                                                                           \
        if ((expr)is_error) {                                                                      \
            perror(#expr);                                                                         \
            exit(EXIT_FAILURE);                                                                    \
        }                                                                                          \
    } while (0)
#define CHK_NULL(expr) CHK(expr, == NULL)
#define CHK_NEG(expr) CHK(expr, < 0)

#endif // __CHECK_H__
