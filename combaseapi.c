#include "jw32.h"
#include "Objbase.h"

#define MOD_NAME "combaseapi"


static void define_consts_coinit(JanetTable *env)
{
#define __def(const_name)                                        \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),     \
              "Constant for COM concurrency models.")
    __def(COINIT_APARTMENTTHREADED);
    __def(COINIT_MULTITHREADED);
    __def(COINIT_DISABLE_OLE1DDE);
    __def(COINIT_SPEED_OVER_MEMORY);
#undef __def
}


static Janet cfun_CoInitializeEx(int32_t argc, Janet *argv)
{
    LPVOID pvReserved;
    DWORD dwCoInit;

    HRESULT hrRet;

    janet_fixarity(argc, 2);

    pvReserved = jw32_get_lpvoid(argv, 0);
    dwCoInit = jw32_get_dword(argv, 1);

    hrRet = CoInitializeEx(pvReserved, dwCoInit);
    return jw32_wrap_hresult(hrRet);
}


static const JanetReg cfuns[] = {
    {
        "CoInitializeEx",
        cfun_CoInitializeEx,
        "(" MOD_NAME "/CoInitializeEx pvReserved dwCoInit)\n\n"
        "Initializes the COM library."
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_consts_coinit(env);

    janet_cfuns(env, MOD_NAME, cfuns);
}
