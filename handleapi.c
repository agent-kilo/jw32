#include "jw32.h"

#define MOD_NAME "handleapi"


static Janet cfun_CloseHandle(int32_t argc, Janet *argv)
{
    HANDLE hObject;

    janet_fixarity(argc, 1);

    hObject = jw32_get_handle(argv, 0);
    return jw32_wrap_bool(CloseHandle(hObject));
}


static const JanetReg cfuns[] = {
    {
        "CloseHandle",
        cfun_CloseHandle,
        "(" MOD_NAME "/CloseHandle hObject)\n\n"
        "Closes an open object handle.",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, MOD_NAME, cfuns);
}
