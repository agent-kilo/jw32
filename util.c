#include "jw32.h"
#include <windowsx.h>

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


static Janet cfun_GET_X_LPARAM(int32_t argc, Janet *argv)
{
    LPARAM lParam;

    janet_fixarity(argc, 1);

    lParam = jw32_get_lparam(argv, 0);
    return jw32_wrap_int(GET_X_LPARAM(lParam));
}


static Janet cfun_GET_Y_LPARAM(int32_t argc, Janet *argv)
{
    LPARAM lParam;

    janet_fixarity(argc, 1);

    lParam = jw32_get_lparam(argv, 0);
    return jw32_wrap_int(GET_Y_LPARAM(lParam));
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
        "GET_X_LPARAM",
        cfun_GET_X_LPARAM,
        "(" MOD_NAME "/GET_X_LPARAM lparam)\n\n"
        "Retrieves the signed x-coordinate from the specified value.",
    },
    {
        "GET_Y_LPARAM",
        cfun_GET_Y_LPARAM,
        "(" MOD_NAME "/GET_Y_LPARAM lparam)\n\n"
        "Retrieves the signed y-coordinate from the specified value.",
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
