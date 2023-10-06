#ifndef __JW32_H
#define __JW32_H

#include <windows.h>
#include <janet.h>
#include "types.h"

/* minimum buffer size for ad-hoc buffers, to store names, paths and such */
#define JW32_BUFFER_INIT_CAPACITY 128

static inline int32_t lower_power_of_two(int32_t n)
{
    if (!n) {
        return 0;
    }

    for (int32_t i = 0, mask = 1; i < 31; i++, mask = (1 << i)) {
        n &= ~mask;
        if (!n) {
            return mask;
        }
    }

    return 0;
}

#endif /* __JW32_H */
