#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>

#if defined(DEBUG)
#include <stdio.h>
#define debug(msg, ...) fprintf(stderr, "[DEBUG] [%15s:%3d] " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define debug(msg, ...)
#endif

#endif // __LOG_H__
