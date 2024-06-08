#include "jw32.h"
#include "debug.h"

#define MOD_NAME "consoleapi"


static Janet cfun_FreeConsole(int32_t argc, Janet *argv)
{
    janet_fixarity(argc, 0);
    (void)argv;

    return jw32_wrap_bool(FreeConsole());
}


static Janet cfun_AllocConsole(int32_t argc, Janet *argv)
{
    janet_fixarity(argc, 0);
    (void)argv;

    return jw32_wrap_bool(AllocConsole());
}


static const JanetReg cfuns[] = {
    {
        "FreeConsole",
        cfun_FreeConsole,
        "(" MOD_NAME "/FreeConsole)\n\n"
        "Detaches the calling process from its console.",
    },
    {
        "AllocConsole",
        cfun_AllocConsole,
        "(" MOD_NAME "/AllocConsole)\n\n"
        "Allocates a new console for the calling process.",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, MOD_NAME, cfuns);
}
