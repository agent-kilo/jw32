#ifndef __JW32_TYPES_H
#define __JW32_TYPES_H

#include <windows.h>
#include <janet.h>


#define jw32_cstr_to_keyword(x) janet_wrap_keyword(janet_ckeyword(x))


/* HANDLE: defined as pointers in win32 APIs */
#define jw32_wrap_handle(x) janet_wrap_pointer(x)
#define jw32_unwrap_handle(x) ((HANDLE)janet_unwrap_pointer(x))

static inline HANDLE jw32_get_handle(const Janet *argv, int32_t n)
{
    Janet x = argv[n];

    switch (janet_type(x)) {
    case JANET_POINTER:
        return jw32_unwrap_handle(x);
    case JANET_NUMBER:
    case JANET_ABSTRACT: /* u64 types */
        /* janet_unwrap_u64() does its own type & range checks */
        return (HANDLE)janet_unwrap_u64(x);
    case JANET_NIL:
        return NULL;
    default:
        janet_panicf("bad slot #%d, expected %T or %T, got %v",
                     n, JANET_TFLAG_POINTER, JANET_TFLAG_NIL, x);
    }
}


/* LPCSTR: const string pointers */
#define jw32_wrap_lpcstr(x) janet_wrap_string(janet_string((x), strlen(x)))
#define jw32_unwrap_lpcstr(x) ((LPCSTR)janet_unwrap_string(x))

/* XXX: doesn't increase ref count, don't use in async stuff */
static inline LPCSTR jw32_get_lpcstr(const Janet *argv, int32_t n)
{
    Janet x = argv[n];
    
    switch(janet_type(x)) {
    case JANET_STRING:
        return jw32_unwrap_lpcstr(x);
    case JANET_POINTER:
        return (LPCSTR)janet_unwrap_pointer(x);
    case JANET_NUMBER:
    case JANET_ABSTRACT: /* u64 types */
        /* janet_unwrap_u64() does its own type & range checks */
        return (LPCSTR)janet_unwrap_u64(x);
    case JANET_NIL:
        return NULL;
    default:
        janet_panicf("bad slot #%d, expected %T, %T or %T, got %v",
                     n, JANET_TFLAG_STRING, JANET_TFLAG_POINTER, JANET_TFLAG_NIL, x);
    }
}


/* LPVOID: 64 bit (on x64 machines) or 32 bit (on x86 machines) unsigned pointer */
#define jw32_wrap_lpvoid(x) jw32_wrap_handle(x)
#define jw32_unwrap_lpvoid(x) ((LPVOID)jw32_unwrap_handle(x))
#define jw32_get_lpvoid(argv, n) ((LPVOID)jw32_get_handle(argv, n))


/* WORD: 16 bit unsigned */
#define jw32_wrap_word(x) janet_wrap_integer(x)
#define jw32_unwrap_word(x) ((WORD)janet_unwrap_integer(x))
#define JW32_WORD_MIN 0
#define JW32_WORD_MAX 0xFFFF

static inline WORD jw32_get_word(const Janet *argv, int32_t n)
{
    Janet x = argv[n];
    int32_t val;

    if (!janet_checktype(x, JANET_NUMBER)) {
        janet_panic_type(x, n, JANET_TFLAG_NUMBER);
    }

    val = janet_unwrap_integer(x);
    if (val >= JW32_WORD_MIN && val <= JW32_WORD_MAX && val == (WORD)val) {
        return (WORD)val;
    } else {
        janet_panicf("bad slot #%d, expected 16 bit unsigned integer, got %v", n, x);
    }
}


/* ATOM: 16 bit unsigned */
#define jw32_wrap_atom(x) jw32_wrap_word(x)
#define jw32_unwrap_atom(x) ((ATOM)jw32_unwrap_word(x))
#define jw32_get_atom(argv, n) ((ATOM)jw32_get_word(argv, n))


/* XXX: below are some number types, but janet only has
   int32_t <-> double float conversions.
   use 64 bit types here to avoid undefined sign-ness conversions
   and preserve precision */


/* DWORD: 32 bit unsigned */
#define jw32_wrap_dword(x)    janet_wrap_u64((uint64_t)(x))
#define jw32_unwrap_dword(x)  ((DWORD)janet_unwrap_u64(x))
#define JW32_DWORD_MIN 0
#define JW32_DWORD_MAX 0xFFFFFFFF

static inline DWORD jw32_get_dword(const Janet *argv, int32_t n)
{
    Janet x = argv[n];

    uint64_t val = janet_unwrap_u64(x);
    if (val >= JW32_DWORD_MIN && val <= JW32_DWORD_MAX && val == (DWORD)val) {
        return (DWORD)val;
    } else {
        janet_panicf("bad slot #%d, expected 32 bit unsigned integer, got %v", n, x);
    }
}


/* int: 32 bit signed */
#define jw32_wrap_int(x)     janet_wrap_integer((int32_t)(x))
#define jw32_unwrap_int(x)   ((int)janet_unwrap_integer(x))
#define jw32_get_int(argv, n) ((int)janet_getinteger(argv, n))


/* UINT: 32 bit unsigned */
#define jw32_wrap_uint(x)     jw32_wrap_dword(x)
#define jw32_unwrap_uint(x)   ((UINT)jw32_unwrap_dword(x))
#define jw32_get_uint(argv, n) ((UINT)jw32_get_dword(argv, n))


/* WPARAM: 64 bit (on x64 machines) or 32 bit (on x86 machines) unsighed */
#define jw32_wrap_wparam(x)   janet_wrap_u64((uint64_t)(x))
#define jw32_unwrap_wparam(x) ((WPARAM)janet_unwrap_u64(x))
#define jw32_get_wparam(argv, n) ((WPARAM)janet_getuinteger64(argv, n))


/* LPARAM: 64 bit (on x64 machines) or 32 bit (on x86 machines) sighed */
#define jw32_wrap_lparam(x)   janet_wrap_s64((int64_t)(x))
#define jw32_unwrap_lparam(x) ((LPARAM)janet_unwrap_s64(x))
#define jw32_get_lparam(argv, n) ((LPARAM)janet_getinteger64(argv, n))


/* LONG: 32 bit signed */
#define jw32_wrap_long(x)   jw32_wrap_int(x)
#define jw32_unwrap_long(x) ((LONG)jw32_unwrap_int(x))
#define jw32_get_long(argv, n) ((LONG)jw32_get_int(argv, n))

/* BOOL: 32 bit signed */
#define jw32_wrap_bool(x)   jw32_wrap_int(x)
#define jw32_unwrap_bool(x) ((BOOL)jw32_unwrap_int(x))
#define jw32_get_bool(argv, n) ((BOOL)jw32_get_int(argv, n))


/* LRESULT: 64 bit (on x64 machines) or 32 bit (on x86 machines) signed */
#define jw32_wrap_lresult(x)   jw32_wrap_lparam(x)
#define jw32_unwrap_lresult(x) ((LRESULT)jw32_unwrap_lparam(x))
#define jw32_get_lresult(argv, n) ((LRESULT)jw32_get_lparam(argv, n))


#define table_val_to_struct_member(tp, sp, member, type)                \
    do {                                                                \
        Janet j_##member = janet_table_get((tp), jw32_cstr_to_keyword(#member)); \
        if (!janet_checktype(j_##member, JANET_NIL)) {                  \
            (sp)->member = jw32_get_##type(&(j_##member), 0);           \
        }                                                               \
    } while (0)


#endif  /* __JW32_TYPES_H */
