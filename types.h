#ifndef __JW32_TYPES_H
#define __JW32_TYPES_H

#include <windows.h>
#include <janet.h>


#define jw32_cstr_to_keyword(x) janet_wrap_keyword(janet_ckeyword(x))


static inline Janet jw32_wrap_handle(void *x)
{
    return janet_wrap_pointer(x);
}

static inline HANDLE jw32_unwrap_handle(Janet x)
{
    if (janet_checktype(x, JANET_POINTER) || janet_checktype(x, JANET_NUMBER)) {
        return ((HANDLE)janet_unwrap_pointer(x));
    } else if (janet_checktype(x, JANET_NIL)) {
        return NULL;
    } else {
        janet_panicf("expected pointer or nil for HANDLE or pointer types");
    }
}


static inline Janet jw32_wrap_lpcstr(LPCSTR x)
{
    const uint8_t *s = janet_string(x, strlen(x));
    return janet_wrap_string(s);
}

/* XXX: doesn't increase ref count, don't use in async stuff */
static inline LPCSTR jw32_unwrap_lpcstr(Janet x)
{
    if (janet_checktype(x, JANET_STRING)) {
        return ((LPCSTR)janet_unwrap_string(x));
    } else if (janet_checktype(x, JANET_POINTER) || janet_checktype(x, JANET_NUMBER)) {
        return ((LPCSTR)janet_unwrap_pointer(x));
    } else if (janet_checktype(x, JANET_NIL)) {
        return NULL;
    } else {
        janet_panicf("expected string or nil for LPCSTR types");
    }
}


/* LPVOID: 64 bit (on x64 machines) or 32 bit (on x86 machines) unsigned pointer */
#define jw32_wrap_lpvoid(x) jw32_wrap_handle(x)
#define jw32_unwrap_lpvoid(x) ((LPVOID)jw32_unwrap_handle(x))
/* WORD: 16 bit unsigned */
#define jw32_wrap_word(x) janet_wrap_integer(x)
#define jw32_unwrap_word(x) ((WORD)janet_unwrap_integer(x))
/* ATOM: 16 bit unsigned */
#define jw32_wrap_atom(x) janet_wrap_integer(x)
#define jw32_unwrap_atom(x) ((ATOM)janet_unwrap_integer(x))


/* XXX: below are some number types, but janet only has
   int32_t <-> double float conversions.
   use 64 bit types here to avoid undefined sign-ness conversions
   and preserve precision */

/* DWORD: 32 bit unsigned */
#define jw32_wrap_dword(x)    janet_wrap_u64((uint64_t)(x))
#define jw32_unwrap_dword(x)  ((DWORD)janet_unwrap_u64(x))
/* int: 32 bit signed */
#define jw32_wrap_int(x)     janet_wrap_integer((int32_t)(x))
#define jw32_unwrap_int(x)   ((int)janet_unwrap_integer(x))
/* UINT: 32 bit unsigned */
#define jw32_wrap_uint(x)     janet_wrap_u64((uint64_t)(x))
#define jw32_unwrap_uint(x)   ((UINT)janet_unwrap_u64(x))
/* WPARAM: 64 bit (on x64 machines) or 32 bit (on x86 machines) unsighed */
#define jw32_wrap_wparam(x)   janet_wrap_u64((uint64_t)(x))
#define jw32_unwrap_wparam(x) ((WPARAM)janet_unwrap_u64(x))
/* LPARAM: 32 bit signed */
#define jw32_wrap_lparam(x)   janet_wrap_s64((int64_t)(x))
#define jw32_unwrap_lparam(x) ((LPARAM)janet_unwrap_s64(x))
/* LONG: 32 bit signed */
#define jw32_wrap_long(x)   janet_wrap_s64((int64_t)(x))
#define jw32_unwrap_long(x) ((LONG)janet_unwrap_s64(x))
/* BOOL: 32 bit signed */
#define jw32_wrap_bool(x)   janet_wrap_integer((int32_t)(x))
#define jw32_unwrap_bool(x) ((BOOL)janet_unwrap_integer(x))
/* LRESULT: 64 bit (on x64 machines) or 32 bit (on x86 machines) signed */
#define jw32_wrap_lresult(x)   janet_wrap_s64((int64_t)(x))
#define jw32_unwrap_lresult(x) ((LRESULT)janet_unwrap_s64(x))

#endif
