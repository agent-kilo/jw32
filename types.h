#ifndef __JW32_TYPES_H
#define __JW32_TYPES_H

#include <windows.h>
#include <janet.h>

#define jw32_wrap_handle(val) janet_wrap_pointer((void *)val)
#define jw32_unwrap_handle(val) ((HANDLE)janet_unwrap_pointer(val))

/* XXX: DWORD is uint32_t, but janet converts all numbers
   into double float. use u64 here to preserve precision */
#define jw32_wrap_dword(val) janet_wrap_u64((uint64_t)val)
#define jw32_unwrap_dword(val) ((DWORD)janet_unwrap_u64(val))

#endif
