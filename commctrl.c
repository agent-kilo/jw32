#include "jw32.h"
#include <commctrl.h>
#include "debug.h"

#define MOD_NAME "commctrl"


static void define_consts_lim(JanetTable *env)
{
#define __def(const_name)                                  \
    janet_def(env, #const_name, jw32_wrap_int(const_name), \
              "Constant flag for LoadIconMetric.")

    __def(LIM_SMALL);
    __def(LIM_LARGE);

#undef __def
}


static Janet cfun_LoadIconMetric(int32_t argc, Janet *argv)
{
    HINSTANCE hInst;
    PCWSTR pszName;
    int lims;

    HRESULT hrRet;
    HICON hIcon = NULL;

    janet_fixarity(argc, 3);

    hInst = jw32_get_handle(argv, 0);
    /* XXX: Caller needs to make sure there's actually a unicode string inside the pointer */
    pszName = (PCWSTR)jw32_get_lpcstr(argv, 1);
    lims = jw32_get_int(argv, 2);

    hrRet = LoadIconMetric(hInst, pszName, lims, &hIcon);
    JW32_HR_RETURN_OR_PANIC(hrRet, jw32_wrap_handle(hIcon));
}


static const JanetReg cfuns[] = {
    {
        "LoadIconMetric",
        cfun_LoadIconMetric,
        "(" MOD_NAME "/LoadIconMetric hInst pszName lims)\n\n"
        "Loads a specified icon resource with a client-specified system metric.",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_consts_lim(env);

    janet_cfuns(env, MOD_NAME, cfuns);
}
