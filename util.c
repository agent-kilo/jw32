#include <windows.h>
#include <janet.h>
#include "types.h"

#define MOD_NAME "util"


static Janet cfun_IsNull(int32_t argc, Janet *argv)
{
    LPVOID p;
    janet_fixarity(argc, 1);
    p = jw32_get_lpvoid(argv, 0);
    return janet_wrap_boolean(p == NULL);
}


static const JanetReg cfuns[] = {
    {
        "null?",
        cfun_IsNull,
        "(" MOD_NAME "/null? pointer)\n\n"
        "Check if pointer is NULL.",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, MOD_NAME, cfuns);

    janet_def(env, "NULL", jw32_wrap_lpvoid(NULL),
              "The NULL pointer, for comparison with API return values.");
}
