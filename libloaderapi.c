#include "jw32.h"
#include "debug.h"

#define MOD_NAME "libloaderapi"


static Janet cfun_GetModuleHandle(int32_t argc, Janet *argv)
{
    LPCSTR lpModuleName;

    janet_fixarity(argc, 1);

    lpModuleName = jw32_get_lpcstr(argv, 0);
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

    dwFlags = jw32_get_dword(argv, 0);
    lpModuleName = jw32_get_lpcstr(argv, 1);

    bRet = GetModuleHandleEx(dwFlags, lpModuleName, &hModule);

    ret_tuple[0] = jw32_wrap_bool(bRet);
    ret_tuple[1] = jw32_wrap_handle(hModule);

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static Janet cfun_GetModuleFileName(int32_t argc, Janet *argv)
{
    HMODULE hModule;
    JanetBuffer *file_name_buf;

    DWORD dwRet;

    int32_t buf_growth = 1;

    janet_fixarity(argc, 2);

    hModule = jw32_get_handle(argv, 0);
    file_name_buf = janet_getbuffer(argv, 1);

    do {
        janet_buffer_ensure(file_name_buf, JW32_BUFFER_INIT_CAPACITY, buf_growth);
        jw32_dbg_val(file_name_buf->capacity, "%d");
        dwRet = GetModuleFileName(hModule, file_name_buf->data, file_name_buf->capacity);
        buf_growth *= 2;
    } while (dwRet >= file_name_buf->capacity);

    if (dwRet > 0) {
        file_name_buf->count = dwRet;
    }

    return jw32_wrap_dword(dwRet);
}

static Janet cfun_FreeLibrary(int32_t argc, Janet *argv)
{
    HMODULE hLibModule;

    BOOL bRet;

    janet_fixarity(argc, 1);

    hLibModule = jw32_get_handle(argv, 0);
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
        "GetModuleFileName",
        cfun_GetModuleFileName,
        "(" MOD_NAME "/GetModuleFileName hModule lpFilename)\n\n"
        "lpFilename should be a buffer, who's content will be overridden upon a successful call",
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
