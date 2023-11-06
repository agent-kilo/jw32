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

/* x should be an LPCSTR, which may store an ATOM when its higher
   bits are set to zero */
#define check_atom(x) (!((uint64_t)(x) & ~(uint64_t)0xffff))
#define lpcstr_to_atom(x) ((ATOM)((uint64_t)(x) & 0xffff))


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
#define jw32_wrap_wparam(x)   jw32_wrap_uint_ptr(x)
#define jw32_unwrap_wparam(x) ((WPARAM)jw32_unwrap_uint_ptr(x))
#define jw32_get_wparam(argv, n) ((WPARAM)jw32_get_uint_ptr(argv, n))


/* LPARAM: 64 bit (on x64 machines) or 32 bit (on x86 machines) sighed */
#define jw32_wrap_lparam(x)   jw32_wrap_long_ptr(x)
#define jw32_unwrap_lparam(x) ((LPARAM)jw32_unwrap_long_ptr(x))
#define jw32_get_lparam(argv, n) ((LPARAM)jw32_get_long_ptr(argv, n))


/* LONG: 32 bit signed */
#define jw32_wrap_long(x)   jw32_wrap_int(x)
#define jw32_unwrap_long(x) ((LONG)jw32_unwrap_int(x))
#define jw32_get_long(argv, n) ((LONG)jw32_get_int(argv, n))


/* ULONG: 32 bit unsigned */
#define jw32_wrap_ulong(x)   jw32_wrap_uint(x)
#define jw32_unwrap_ulong(x) ((ULONG)jw32_unwrap_uint(x))
#define jw32_get_ulong(argv, n) ((ULONG)jw32_get_uint(argv, n))


/* BOOL: 32 bit signed */
#define jw32_wrap_bool(x)   jw32_wrap_int(x)
#define jw32_unwrap_bool(x) ((BOOL)jw32_unwrap_int(x))
static inline BOOL jw32_get_bool(const Janet *argv, int32_t n)
{
    Janet x = argv[n];

    switch (janet_type(x)) {
    case JANET_BOOLEAN:
        return janet_unwrap_boolean(x) ? TRUE : FALSE;
    default:
        return (BOOL)jw32_get_int(argv, n);
    }
}


/* LRESULT: 64 bit (on x64 machines) or 32 bit (on x86 machines) signed */
#define jw32_wrap_lresult(x)   jw32_wrap_lparam(x)
#define jw32_unwrap_lresult(x) ((LRESULT)jw32_unwrap_lparam(x))
#define jw32_get_lresult(argv, n) ((LRESULT)jw32_get_lparam(argv, n))


/* ULONG_PTR: 64 bit (on x64 machines) or 32 bit (on x86 machines) unsigned */
#define jw32_wrap_ulong_ptr(x) janet_wrap_u64((uint64_t)(x))
#define jw32_unwrap_ulong_ptr(x) ((ULONG_PTR)janet_unwrap_u64(x))
static inline ULONG_PTR jw32_get_ulong_ptr(const Janet *argv, int32_t n)
{
    Janet x = argv[n];

    switch (janet_type(x)) {
    case JANET_POINTER:
        return (ULONG_PTR)janet_unwrap_pointer(x);
    default:
        return (ULONG_PTR)janet_getuinteger64(argv, n);
    }
}


/* LONG_PTR: 64 bit (on x64 machines) or 32 bit (on x86 machines) signed */
#define jw32_wrap_long_ptr(x) janet_wrap_s64((int64_t)(x))
#define jw32_unwrap_long_ptr(x) ((LONG_PTR)janet_unwrap_s64(x))
static inline LONG_PTR jw32_get_long_ptr(const Janet *argv, int32_t n)
{
    Janet x = argv[n];

    switch (janet_type(x)) {
    case JANET_POINTER:
        return (LONG_PTR)janet_unwrap_pointer(x);
    default:
        return (LONG_PTR)janet_getinteger64(argv, n);
    }
}


/* INT_PTR: 64 bit (on x64 machines) or 32 bit (on x86 machines) signed */
#define jw32_wrap_int_ptr(x) janet_wrap_s64((int64_t)(x))
#define jw32_unwrap_int_ptr(x) ((INT_PTR)janet_unwrap_s64(x))
#define jw32_get_int_ptr(argv, n) ((INT_PTR)jw32_get_long_ptr(argv, n))


/* UINT_PTR: 64 bit (on x64 machines) or 32 bit (on x86 machines) unsigned */
#define jw32_wrap_uint_ptr(x) janet_wrap_u64((uint64_t)(x))
#define jw32_unwrap_uint_ptr(x) ((UINT_PTR)janet_unwrap_u64(x))
#define jw32_get_uint_ptr(argv, n) ((UINT_PTR)jw32_get_ulong_ptr(argv, n))


/* DWORD_PTR: 64 bit (on x64 machines) or 32 bit (on x86 machines) unsigned */
#define jw32_wrap_dword_ptr(x) janet_wrap_u64((uint64_t)(x))
#define jw32_unwrap_dword_ptr(x) ((DWORD_PTR)janet_unwrap_u64(x))
#define jw32_get_dword_ptr(argv, n) ((DWORD_PTR)jw32_get_ulong_ptr(argv, n))


/* HRESULT: 32 bit signed */
#define jw32_wrap_hresult(x) jw32_wrap_long(x)
#define jw32_unwrap_hresult(x) ((HRESULT)jw32_unwrap_long(x))
#define jw32_get_hresult(argv, n) ((HRESULT)jw32_get_long(argv, n))


/* double: 64 bit floating point */
#define jw32_wrap_double(x) janet_wrap_number(x)
#define jw32_unwrap_double(x) (janet_unwrap_number(x))
#define jw32_get_double(argv, n) (janet_getnumber(argv, n))

static inline JanetTable *jw32_rect_to_table(const RECT *rect)
{
    JanetTable *rect_tb = janet_table(4);
    janet_table_put(rect_tb, janet_ckeywordv("left"), jw32_wrap_long(rect->left));
    janet_table_put(rect_tb, janet_ckeywordv("top"), jw32_wrap_long(rect->top));
    janet_table_put(rect_tb, janet_ckeywordv("right"), jw32_wrap_long(rect->right));
    janet_table_put(rect_tb, janet_ckeywordv("bottom"), jw32_wrap_long(rect->bottom));
    return rect_tb;
}

static inline JanetTuple jw32_point_to_tuple(const POINT *point)
{
    Janet tuple[2] = {
        jw32_wrap_long(point->x),
        jw32_wrap_long(point->y),
    };

    return janet_tuple_n(tuple, 2);
}

static inline POINT jw32_view_to_point(JanetView view)
{
    POINT pt;
    Janet xv, yv;

    if (view.len != 2) {
        janet_panicf("expected indexed view of length 2, got %v", view.len);
    }

    xv = view.items[0];
    yv = view.items[1];

    if (!janet_checkint(xv) || !janet_checkint(yv)) {
        janet_panicf("expected 32 bit integers for POINT, got %v and %v ", xv, yv);
    }

    pt.x = jw32_unwrap_long(xv);
    pt.y = jw32_unwrap_long(yv);

    return pt;
}

static inline JanetString jw32_bstr_to_string(BSTR from)
{
    int count = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, from, -1, NULL, 0, NULL, NULL);

    if (count <= 0) {
        return NULL;
    } else {
        /* janet_string_begin() adds one more byte for the trailing zero,
           and count includes the trailing zero, so */
        uint8_t *to = janet_string_begin(count - 1);
        to[0] = 0;

        int count_again = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                                              from, -1, (LPSTR)to, count,
                                              NULL, NULL);
        if (count_again != count) {
            /* wait for gc to claim it */
            janet_string_end(to);
            return NULL;
        }
        return janet_string_end(to);
    }
}

static inline BSTR jw32_string_to_bstr(JanetString from)
{
    int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (const char *)from, -1, NULL, 0);

    if (count <= 0) {
        return NULL;
    } else {
        BSTR to = SysAllocStringByteLen(NULL, count * sizeof(WCHAR));
        int count_again = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                              (const char *)from, -1, (LPWSTR)to, count);
        if (count_again != count) {
            SysFreeString(to);
            return NULL;
        }
        return to;
    }
}

static inline Janet jw32_parse_variant_safearray(SAFEARRAY *psa, VARTYPE vt, int cleanup)
{
#define __CLEANUP() do {                        \
        if (cleanup) {                          \
            SafeArrayDestroy(psa);              \
        }                                       \
    } while (0)

    if (psa->cDims != 1) {
        __CLEANUP();
        janet_panicf("SAFEARRAYs with more than one dimention are not supported");
    }

    LONG lLbound = psa->rgsabound[0].lLbound;
    ULONG cElements = psa->rgsabound[0].cElements;
    JanetArray *jarr = janet_array(cElements);

#define __CASE(t, wt, jt)                                               \
    case VT_##t##: do {                                                 \
        wt val;                                                         \
        for (LONG i = lLbound; i < (LONG)(lLbound + cElements); i++) {  \
            HRESULT hr = SafeArrayGetElement(psa, &i, &val);            \
            if (SUCCEEDED(hr)) {                                        \
                Janet jval = janet_wrap_##jt##(val);                    \
                janet_array_push(jarr, jval);                           \
            } else {                                                    \
                __CLEANUP();                                            \
                janet_panicf("SafeArrayGetElement() failed: 0x%x", hr); \
            }                                                           \
        }                                                               \
        __CLEANUP();                                                    \
        return janet_wrap_array(jarr);                                  \
    } while(0)

    /* TODO: other types */
    switch (vt & VT_TYPEMASK) {

    __CASE(I2, SHORT, integer);
    __CASE(I4, LONG, integer);
    __CASE(R4, FLOAT, number);
    __CASE(R8, DOUBLE, number);
    __CASE(DATE, DATE, number);
    __CASE(BOOL, VARIANT_BOOL, integer);
    __CASE(UNKNOWN, IUnknown *, pointer);
    __CASE(I1, CHAR, integer);
    __CASE(UI1, BYTE, integer);
    __CASE(UI2, USHORT, integer);
    __CASE(UI4, ULONG, u64);
    __CASE(INT, INT, integer);
    __CASE(UINT, UINT, u64);

    case VT_BSTR: {
        BSTR val;
        for (LONG i = lLbound; i < (LONG)(lLbound + cElements); i++) {
            HRESULT hr = SafeArrayGetElement(psa, &i, &val);
            if (SUCCEEDED(hr)) {
                JanetString jstr = jw32_bstr_to_string(val);
                if (!jstr) {
                    __CLEANUP();
                    janet_panicf("jw32_bstr_to_string() failed");
                }
                Janet jval = janet_wrap_string(jstr);
                janet_array_push(jarr, jval);
            } else {
                __CLEANUP();
                janet_panicf("SafeArrayGetElement() failed: 0x%x", hr);
            }
        }
        __CLEANUP();
        return janet_wrap_array(jarr);
    }

    default:
        __CLEANUP();
        janet_panicf("unsupported SAFEARRAY variant type: 0x%x", vt);
    }
    
#undef __CASE
#undef __CLEANUP
}

static inline Janet jw32_parse_variant(VARIANT *v, int cleanup)
{
    VARTYPE vt = V_VT(v);
    VARTYPE is_byref = vt & VT_BYREF;
    VARTYPE is_array = vt & VT_ARRAY;

#define __CLEANUP() do {                        \
        if (cleanup) {                          \
            VariantClear(v);                    \
        }                                       \
    } while (0)

    if (is_array) {
        SAFEARRAY *psa = is_byref ? (*V_ARRAYREF(v)) : V_ARRAY(v);
        Janet ret = jw32_parse_variant_safearray(psa, vt, FALSE);
        __CLEANUP();
        return ret;
    }

#define __maybe_ref(t) (is_byref ? (*V_##t##REF(v)) : (V_##t##(v)))
#define __CASE(t, jwt)                                  \
    case VT_##t##: do {                                 \
        Janet ret = janet_wrap_##jwt##(__maybe_ref(t)); \
        __CLEANUP();                                    \
        return ret;                                     \
    } while (0)

    switch (vt & VT_TYPEMASK) {
    case VT_EMPTY:
    case VT_NULL: {
        __CLEANUP();
        return janet_wrap_nil();
    }
    __CASE(I2, integer);
    __CASE(I4, integer);
    __CASE(R4, number);
    __CASE(R8, number);
    case VT_CY: {
        CY *cy = is_byref ? V_CYREF(v) : (&V_CY(v));
        Janet ret = janet_wrap_s64(cy->int64);
        __CLEANUP();
        return ret;
    }
    __CASE(DATE, number);
    case VT_BSTR: {
        BSTR bstr = is_byref ? (*V_BSTRREF(v)) : V_BSTR(v);
        if (bstr) {
            JanetString jstr = jw32_bstr_to_string(bstr);
            __CLEANUP();
            if (!jstr) {
                janet_panicf("jw32_bstr_to_string() failed");
            }
            return janet_wrap_string(jstr);
        } else {
            /* XXX: some ui automation element properties (e.g. UIA_HelpTextPropertyId)
               would set a BSTR VARIANT to NULL, fallback to nil here. */
            __CLEANUP();
            return janet_wrap_nil();
        }
    }
    __CASE(DISPATCH, pointer);
    case VT_ERROR: { /* ERROR is a macro defined by system, can't just say __CASE(ERROR, ...) */
        Janet ret = janet_wrap_integer(is_byref ? (*V_ERRORREF(v)) : V_ERROR(v));
        __CLEANUP();
        return ret;
    }
    __CASE(BOOL, integer);
    case VT_VARIANT:
        /* can only be a ref */
        return jw32_parse_variant(V_VARIANTREF(v), cleanup);
    __CASE(UNKNOWN, pointer);
    case VT_DECIMAL: {
        DECIMAL *dec = is_byref ? V_DECIMALREF(v) : (&V_DECIMAL(v));
        Janet ret_tuple[4] = {
            janet_wrap_integer(dec->wReserved),
            janet_wrap_integer(dec->signscale),
            janet_wrap_u64(dec->Hi32),
            janet_wrap_u64(dec->Lo64),
        };
        __CLEANUP();
        return janet_wrap_tuple(janet_tuple_n(ret_tuple, 4));
    }
    case VT_RECORD: {
        /* can only be a ref */
        Janet ret = janet_wrap_pointer(V_RECORD(v));
        __CLEANUP();
        return ret;
    }
    __CASE(I1, integer);
    __CASE(UI1, integer);
    __CASE(UI2, integer);
    __CASE(UI4, u64);
    __CASE(INT, integer);
    __CASE(UINT, u64);
    default:
        __CLEANUP();
        janet_panicf("unsupported variant type: 0x%x", vt);
    }

#undef __CLEANUP
#undef __CASE
#undef __maybe_ref
}


#endif  /* __JW32_TYPES_H */
