#ifndef __JW32_H
#define __JW32_H

#include <windows.h>
#include <janet.h>
#include "types.h"

/* minimum buffer size for ad-hoc buffers, to store names, paths and such */
#define JW32_BUFFER_INIT_CAPACITY 128

#define JW32_RETURN_TUPLE_2(e0, e1)                             \
    do {                                                        \
        Janet __ret_tuple[2];                                   \
        __ret_tuple[0] = e0;                                    \
        __ret_tuple[1] = e1;                                    \
        return janet_wrap_tuple(janet_tuple_n(__ret_tuple, 2)); \
    } while (0)

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

static inline int jw32_pcall_fn(JanetFunction *fn, int argc, const Janet *argv, Janet *out)
{
  JanetFiber *fiber = NULL;
  int ret, lock;

  /* XXX: if i call any function (cfuns or janet functions) inside fn,
     there would be memory violations without this lock, i don't know why */
  lock = janet_gclock();
  if (janet_pcall(fn, argc, argv, out, &fiber) == JANET_SIGNAL_OK) {
      ret = 1;
  } else {
      janet_stacktrace(fiber, *out);
      ret = 0;
  }
  janet_gcunlock(lock);
  return ret;
}

static inline Janet jw32_call_core_fn(const char *name, int argc, const Janet *argv)
{
    Janet fn = janet_resolve_core(name);
    if (!janet_checktype(fn, JANET_FUNCTION)) {
        janet_panicf("core function %s not found", name);
    }

    JanetFunction *rfn = janet_unwrap_function(fn);
    return janet_call(rfn, argc, argv);
}

static inline Janet jw32_hresult_errorv(HRESULT hr,
                                        const char *file_name,
                                        const char *cfun_name,
                                        int32_t line_no)
{
    if (0) {
        /* for debugging */
        JanetTable *obj = janet_table(4);
        janet_table_put(obj, janet_ckeywordv("hresult"), jw32_wrap_hresult(hr));
        janet_table_put(obj, janet_ckeywordv("file"), janet_cstringv(file_name));
        janet_table_put(obj, janet_ckeywordv("cfun"), janet_cstringv(cfun_name));
        janet_table_put(obj, janet_ckeywordv("line"), janet_wrap_integer(line_no));
        return janet_wrap_table(obj);
    } else {
        /* for human "readability"" */
        return jw32_wrap_hresult(hr);
    }
}

#define JW32_HRESULT_ERRORV(hr) jw32_hresult_errorv(hr, __FILE__, __func__, __LINE__)

#define JW32_HR_RETURN_OR_PANIC(hr, ret)           \
    do {                                           \
        if (S_OK == (hr)) {                        \
            return (ret);                          \
        } else {                                   \
            janet_panicv(JW32_HRESULT_ERRORV(hr)); \
        }                                          \
    } while (0)

#endif /* __JW32_H */
