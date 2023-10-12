#include "jw32.h"

#define MOD_NAME "util"


static Janet cfun_IsNull(int32_t argc, Janet *argv)
{
    LPVOID p;
    janet_fixarity(argc, 1);
    p = jw32_get_lpvoid(argv, 0);
    return janet_wrap_boolean(p == NULL);
}

static Janet cfun_LOWORD(int32_t argc, Janet *argv)
{
    DWORD_PTR dwordptr;

    janet_fixarity(argc, 1);

    dwordptr = jw32_get_dword_ptr(argv, 0);
    return jw32_wrap_word(LOWORD(dwordptr));
}

static Janet cfun_HIWORD(int32_t argc, Janet *argv)
{
    DWORD_PTR dwordptr;

    janet_fixarity(argc, 1);

    dwordptr = jw32_get_dword_ptr(argv, 0);
    return jw32_wrap_word(HIWORD(dwordptr));
}

static Janet cfun_SUCCEEDED(int32_t argc, Janet *argv)
{
    HRESULT code;

    janet_fixarity(argc, 1);
    code = jw32_get_hresult(argv, 0); /* may also be an SCODE */
    return janet_wrap_boolean(SUCCEEDED(code));
}

static Janet cfun_FAILED(int32_t argc, Janet *argv)
{
    HRESULT code;

    janet_fixarity(argc, 1);
    code = jw32_get_hresult(argv, 0); /* may also be an SCODE */
    return janet_wrap_boolean(FAILED(code));
}


static const JanetReg cfuns[] = {
    {
        "null?",
        cfun_IsNull,
        "(" MOD_NAME "/null? pointer)\n\n"
        "Check if pointer is NULL.",
    },
    {
        "LOWORD",
        cfun_LOWORD,
        "(" MOD_NAME "/LOWORD dwordptr)\n\n"
        "Retrieves LOWORD.",
    },
    {
        "HIWORD",
        cfun_HIWORD,
        "(" MOD_NAME "/HIWORD dwordptr)\n\n"
        "Retrieves HIWORD.",
    },
    {
        "SUCCEEDED",
        cfun_SUCCEEDED,
        "(" MOD_NAME "/SUCCEEDED hr)\n\n"
        "Check if an HRESULT or SCODE represents success.",
    },
    {
        "FAILED",
        cfun_FAILED,
        "(" MOD_NAME "/FAILED hr)\n\n"
        "Check if an HRESULT or SCODE represents failure.",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, MOD_NAME, cfuns);

    janet_def(env, "NULL", jw32_wrap_lpvoid(NULL),
              "The NULL pointer, for comparison with API return values.");
    janet_def(env, "TRUE", jw32_wrap_bool(TRUE),
              "TRUE value, for comparison with API return values.");
    janet_def(env, "FALSE", jw32_wrap_bool(FALSE),
              "FALSE value, for comparison with API return values.");
}
