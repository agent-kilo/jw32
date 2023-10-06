#include "jw32.h"

#define MOD_NAME "errhandlingapi"


static Janet cfun_GetLastError(int32_t argc, Janet *argv)
{
    janet_fixarity(argc, 0);

    return jw32_wrap_dword(GetLastError());
}


static const JanetReg cfuns[] = {
    {
        "GetLastError",
        cfun_GetLastError,
        "(" MOD_NAME "/GetLastError)\n\n"
        "Retrieves the calling thread's last-error code value.",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, MOD_NAME, cfuns);
}
