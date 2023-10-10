#ifndef __JW32_DEBUG_H
#define __JW32_DEBUG_H

#include <stdio.h>
#include <inttypes.h>
#include <janet.h>

#ifdef JW32_DEBUG

#define jw32_dbg(fmt, ...)                                              \
    do {                                                                \
        fprintf(stderr, "-- %s:%s:%d: " fmt "\n", __FILE__, __func__, __LINE__, __VA_ARGS__); \
        fflush(stderr);                                                 \
    } while (0)

#define jw32_dbg_msg(msg)                                               \
    do {                                                                \
        fprintf(stderr, "-- %s:%s:%d: " msg "\n", __FILE__, __func__, __LINE__); \
        fflush(stderr);                                                 \
    } while (0)

#define jw32_dbg_val(val, fmt)                  \
    jw32_dbg(#val " = " fmt, (val))

#define jw32_dbg_jval(val)                                              \
    do {                                                                \
        janet_eprintf("-- %s:%s:%d: " #val " = %v\n", __FILE__, __func__, __LINE__, (val)); \
        fflush(stderr);                                                 \
    } while (0)

#else /* JW32_DEBUG */

#define jw32_dbg(fmt, ...)                      \
    do {} while (0)

#define jw32_dbg_msg(msg)                       \
    do {} while (0)

#define jw32_dbg_val(val, fmt)                  \
    do { (void)(val); } while (0)

#define jw32_dbg_jval(val)                      \
    do { (void)(val); } while (0)

#endif /* JW32_DEBUG */

#endif /* __JW32_DEBUG_H */
