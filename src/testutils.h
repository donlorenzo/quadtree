#ifndef __TESTUTILS_H__
#define __TESTUTILS_H__

#include <assert.h>
#include <stdio.h>

#define assertFalse(msg, value)                                         \
    do {                                                                \
        if (value) {                                                    \
            fprintf(stderr, "Assert failed in %s:%d: %s\n",             \
                    __FILE__, __LINE__, msg);                           \
            abort();                                                    \
        }                                                               \
    } while(0)

#define assertTrue(msg, value) assertFalse(msg, !(value))

#define assertEqualsInt(msg, value1, value2)                            \
    do {                                                                \
        if ((value1) != (value2)) {                                     \
            fprintf(stderr, "Assert failed in %s:%d: %s: expected %d got %d\n", \
                    __FILE__, __LINE__, msg, (value1), (value2));       \
            abort();                                                    \
        }                                                               \
    } while(0)

#define assertEqualsULong(msg, value1, value2)                            \
    do {                                                                \
        if ((value1) != (value2)) {                                     \
            fprintf(stderr, "Assert failed in %s:%d: %s: expected %lu got %lu\n", \
                    __FILE__, __LINE__, msg, (value1), (value2));       \
            abort();                                                    \
        }                                                               \
    } while(0)


#endif /* __TESTUTILS_H__ */
