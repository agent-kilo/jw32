#ifndef __JW32_TYPES_H
#define __JW32_TYPES_H

#include <windows.h>
#include <janet.h>

#define jw32_wrap_handle(val) janet_wrap_pointer((void *)val)
#define jw32_unwrap_handle(val) ((HANDLE)janet_unwrap_pointer(val))

/* XXX: below are 32 bit data types, but janet converts all numbers
   into double float, and doesn't have 32 bit integer types.
   use 64 bit types here to preserve precision */

/* DWORD: 32 bit unsigned */
#define jw32_wrap_dword(x)    janet_wrap_u64((uint64_t)(x))
#define jw32_unwrap_dword(x)  ((DWORD)janet_unwrap_u64(x))
/* UINT: 32 bit unsigned */
#define jw32_wrap_uint(x)     janet_wrap_u64((uint64_t)(x))
#define jw32_unwrap_uint(x)   ((UINT)janet_unwrap_u64(x))
/* WPARAM: 64 bit (on x64 machines) or 32 bit (on x86 machines) unsighed */
#define jw32_wrap_wparam(x)   janet_wrap_u64((uint64_t)(x))
#define jw32_unwrap_wparam(x) ((WPARAM)janet_unwrap_u64(x))
/* LPARAM: 32 bit signed */
#define jw32_wrap_lparam(x)   janet_wrap_s64((int64_t)(x))
#define jw32_unwrap_lparam(x) ((LPARAM)janet_unwrap_s64(x))

#endif
