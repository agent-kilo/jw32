#include "libloaderapi.h"

#define MOD_NAME "libloaderapi"


static Janet cfun_GetModuleHandle(int32_t argc, Janet *argv)
{
    LPCSTR lpModuleName;

    janet_fixarity(argc, 1);

    lpModuleName = jw32_unwrap_lpcstr(argv[0]);
    return jw32_wrap_handle(GetModuleHandle(lpModuleName));
}

static Janet cfun_GetModuleHandleEx(int32_t argc, Janet *argv)
{
    DWORD dwFlags;
    LPCSTR lpModuleName;

    BOOL bRet;
    HMODULE hModule = NULL;
    Janet ret_tuple[2];

    janet_fixarity(argc, 2);

    dwFlags = jw32_unwrap_dword(argv[0]);
    lpModuleName = jw32_unwrap_lpcstr(argv[1]);

    bRet = GetModuleHandleEx(dwFlags, lpModuleName, &hModule);

    ret_tuple[0] = jw32_wrap_bool(bRet);
    ret_tuple[1] = jw32_wrap_handle(hModule);

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static Janet cfun_FreeLibrary(int32_t argc, Janet *argv)
{
    HMODULE hLibModule;

    BOOL bRet;

    janet_fixarity(argc, 1);

    hLibModule = jw32_unwrap_handle(argv[0]);
    bRet = FreeLibrary(hLibModule);
    return jw32_wrap_bool(bRet);
}


static const JanetReg cfuns[] = {
    {
        "GetModuleHandle",
        cfun_GetModuleHandle,
        "(" MOD_NAME "/GetModuleHandle lpModuleName)\n\n"
        "Returns the module handle specified by lpModuleName.",
    },
    {
        "GetModuleHandleEx",
        cfun_GetModuleHandleEx,
        "(" MOD_NAME "/GetModuleHandleEx dwFlags lpModuleName)\n\n"
        "Returns the module handle specified by lpModuleName.",
    },
    {
        "FreeLibrary",
        cfun_FreeLibrary,
        "(" MOD_NAME "/FreeLibrary hLibModule)\n\n"
        "Frees the loaded dynamic-link library module.",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, MOD_NAME, cfuns);
}
