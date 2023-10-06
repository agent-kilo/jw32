#include "jw32.h"

#define MOD_NAME "errhandlingapi"


static void define_consts_sem(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_uint(const_name),     \
              "Constant for process error modes.")
    __def(SEM_FAILCRITICALERRORS);
    __def(SEM_NOALIGNMENTFAULTEXCEPT);
    __def(SEM_NOGPFAULTERRORBOX);
    __def(SEM_NOOPENFILEERRORBOX);
#undef __def
}


static Janet cfun_GetLastError(int32_t argc, Janet *argv)
{
    janet_fixarity(argc, 0);

    return jw32_wrap_dword(GetLastError());
}

static Janet cfun_SetErrorMode(int32_t argc, Janet *argv)
{
    UINT uMode, uRet;

    janet_fixarity(argc, 1);

    uMode = jw32_get_uint(argv, 0);

    uRet = SetErrorMode(uMode);
    return jw32_wrap_uint(uRet);
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
    define_consts_sem(env);

    janet_cfuns(env, MOD_NAME, cfuns);
}
