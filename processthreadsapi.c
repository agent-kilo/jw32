#include "jw32.h"

#define MOD_NAME "processthreadsapi"


static Janet cfun_GetCurrentThreadId(int32_t argc, Janet *argv)
{
    return jw32_wrap_dword(GetCurrentThreadId());
}


static const JanetReg cfuns[] = {
    {
        "GetCurrentThreadId",
        cfun_GetCurrentThreadId,
        "(" MOD_NAME "/GetCurrentThreadId)\n\n"
        "Returns the current thread id.",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, MOD_NAME, cfuns);
}
