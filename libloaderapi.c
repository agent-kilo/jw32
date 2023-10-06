#include "jw32.h"
#include "debug.h"

#define MOD_NAME "libloaderapi"


static void define_consts_load_library_ex(JanetTable *env)
{
#define __def(const_name)                                    \
    janet_def(env, #const_name, jw32_wrap_dword(const_name), \
              "Constant flag for LoadLibraryEx.")
    __def(DONT_RESOLVE_DLL_REFERENCES);
    __def(LOAD_IGNORE_CODE_AUTHZ_LEVEL);
    __def(LOAD_LIBRARY_AS_DATAFILE);
    __def(LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE);
    __def(LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    __def(LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
    __def(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    __def(LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
    __def(LOAD_LIBRARY_SEARCH_SYSTEM32);
    __def(LOAD_LIBRARY_SEARCH_USER_DIRS);
    __def(LOAD_WITH_ALTERED_SEARCH_PATH);
    __def(LOAD_LIBRARY_REQUIRE_SIGNED_TARGET);
    __def(LOAD_LIBRARY_SAFE_CURRENT_DIRS);
#undef __def
}


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

    int32_t cap = JW32_BUFFER_INIT_CAPACITY;

    janet_fixarity(argc, 2);

    hModule = jw32_get_handle(argv, 0);
    file_name_buf = janet_getbuffer(argv, 1);

    if (file_name_buf->capacity > JW32_BUFFER_INIT_CAPACITY) {
        cap = lower_power_of_two(file_name_buf->capacity);
    }

    jw32_dbg_val(cap, "%d");

    do {
        janet_buffer_ensure(file_name_buf, cap, 1);
        jw32_dbg_val(file_name_buf->capacity, "%d");
        dwRet = GetModuleFileName(hModule, file_name_buf->data, file_name_buf->capacity);
        cap *= 2;
    } while (dwRet >= file_name_buf->capacity);

    if (dwRet > 0) {
        file_name_buf->count = dwRet;
    }

    return jw32_wrap_dword(dwRet);
}

static Janet cfun_LoadLibrary(int32_t argc, Janet *argv)
{
    LPCSTR lpLibFileName;

    HMODULE hRet;

    janet_fixarity(argc, 1);

    lpLibFileName = jw32_get_lpcstr(argv, 0);
    hRet = LoadLibrary(lpLibFileName);
    return jw32_wrap_handle(hRet);
}

static Janet cfun_LoadLibraryEx(int32_t argc, Janet *argv)
{
    LPCSTR lpLibFileName;
    HANDLE hFile;
    DWORD dwFlags;

    HMODULE hRet;

    janet_fixarity(argc, 3);

    lpLibFileName = jw32_get_lpcstr(argv, 0);
    hFile = jw32_get_handle(argv, 1);
    dwFlags = jw32_get_dword(argv, 2);

    hRet = LoadLibraryEx(lpLibFileName, hFile, dwFlags);
    return jw32_wrap_handle(hRet);
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
        "LoadLibrary",
        cfun_LoadLibrary,
        "(" MOD_NAME "/LoadLibrary lpLibFileName)\n\n"
        "Loads a dynamic-link library module.",
    },
    {
        "LoadLibraryEx",
        cfun_LoadLibraryEx,
        "(" MOD_NAME "/LoadLibraryEx lpLibFileName hFile dwFlags)\n\n"
        "Loads a dynamic-link library module.",
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
    define_consts_load_library_ex(env);

    janet_cfuns(env, MOD_NAME, cfuns);
}
