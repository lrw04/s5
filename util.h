#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>

#define ASSERT(x)                                                             \
    do {                                                                      \
        if (!(x)) {                                                           \
            fprintf(stderr, "%s at line %d: %s failed\n", __FILE__, __LINE__, \
                    #x);                                                      \
            abort();                                                          \
        }                                                                     \
    } while (0)

#endif
