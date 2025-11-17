#include "jw32.h"

#define MOD_NAME "winreg"


static void define_consts_rrf(JanetTable *env)
{
#define __def(const_name)                                     \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),    \
              "Constant for RegGetValue.")

    __def(RRF_RT_REG_NONE);
    __def(RRF_RT_REG_SZ);
    __def(RRF_RT_REG_EXPAND_SZ);
    __def(RRF_RT_REG_BINARY);
    __def(RRF_RT_REG_DWORD);
    __def(RRF_RT_REG_MULTI_SZ);
    __def(RRF_RT_REG_QWORD);
    __def(RRF_RT_DWORD);
    __def(RRF_RT_QWORD);
    __def(RRF_RT_ANY);
#if (_WIN32_WINNT >= _WIN32_WINNT_WINTHRESHOLD)
    __def(RRF_SUBKEY_WOW6464KEY);
    __def(RRF_SUBKEY_WOW6432KEY);
    __def(RRF_WOW64_MASK);
#endif
    __def(RRF_NOEXPAND);
    __def(RRF_ZEROONFAILURE);

#undef __def
}


static void define_consts_hkey(JanetTable *env)
{
#define __def(const_name)                                     \
    janet_def(env, #const_name, jw32_wrap_handle(const_name), \
              "Reserved registry key handle.")

    __def(HKEY_CLASSES_ROOT);
    __def(HKEY_CURRENT_USER);
    __def(HKEY_LOCAL_MACHINE);
    __def(HKEY_USERS);
    __def(HKEY_PERFORMANCE_DATA);
    __def(HKEY_PERFORMANCE_TEXT);
    __def(HKEY_PERFORMANCE_NLSTEXT);
#if(WINVER >= 0x0400)
    __def(HKEY_CURRENT_CONFIG);
    __def(HKEY_DYN_DATA);
    __def(HKEY_CURRENT_USER_LOCAL_SETTINGS);
#endif

#undef __def
}


static Janet cfun_RegOpenCurrentUser(int32_t argc, Janet *argv)
{
    REGSAM samDesired;

    LSTATUS lRet;

    HKEY hkResult = NULL;
    Janet ret_tuple[2];

    janet_fixarity(argc, 1);

    samDesired = jw32_get_dword(argv, 0);

    lRet = RegOpenCurrentUser(samDesired, &hkResult);

    ret_tuple[0] = jw32_wrap_long(lRet);
    ret_tuple[1] = jw32_wrap_handle(hkResult);

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}


static Janet cfun_RegOpenKeyEx(int32_t argc, Janet *argv)
{
    HKEY hKey;
    LPCSTR lpSubKey;
    DWORD ulOptions;
    REGSAM samDesired;

    LSTATUS lRet;

    HKEY hkResult = NULL;
    Janet ret_tuple[2];

    janet_fixarity(argc, 4);

    hKey = jw32_get_handle(argv, 0);
    lpSubKey = jw32_get_lpcstr(argv, 1);
    ulOptions = jw32_get_dword(argv, 2);
    samDesired = jw32_get_dword(argv, 3);

    lRet = RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, &hkResult);

    ret_tuple[0] = jw32_wrap_long(lRet);
    ret_tuple[1] = jw32_wrap_handle(hkResult);

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}


static Janet cfun_RegGetValue(int32_t argc, Janet *argv)
{
    HKEY hKey;
    LPCSTR lpSubKey;
    LPCSTR lpValue;
    DWORD dwFlags;

    LSTATUS lRet;

    DWORD dwType = 0;
    JanetBuffer *data_buf = NULL;
    DWORD cbData = 0;

    Janet ret_tuple[3];

    janet_fixarity(argc, 4);

    hKey = jw32_get_handle(argv, 0);
    lpSubKey = jw32_get_lpcstr(argv, 1);
    lpValue = jw32_get_lpcstr(argv, 2);
    dwFlags = jw32_get_dword(argv, 3);

    /* Get the required buffer size first */
    lRet = RegGetValue(hKey, lpSubKey, lpValue, dwFlags, &dwType, NULL, &cbData);
    if (ERROR_SUCCESS != lRet) {
        ret_tuple[0] = jw32_wrap_long(lRet);
        ret_tuple[1] = jw32_wrap_dword(dwType);
        ret_tuple[2] = janet_wrap_nil();
        return janet_wrap_tuple(janet_tuple_n(ret_tuple, 3));
    }

    /* cbData now contains the required buffer size */
    data_buf = janet_buffer(cbData);
    lRet = RegGetValue(hKey, lpSubKey, lpValue, dwFlags, &dwType, data_buf->data, &cbData);
    if (ERROR_SUCCESS != lRet) {
        ret_tuple[0] = jw32_wrap_long(lRet);
        ret_tuple[1] = jw32_wrap_dword(dwType);
        ret_tuple[2] = janet_wrap_buffer(data_buf);
        return janet_wrap_tuple(janet_tuple_n(ret_tuple, 3));
    }

    data_buf->count = cbData;

    ret_tuple[0] = jw32_wrap_long(lRet);
    ret_tuple[1] = jw32_wrap_dword(dwType);
    ret_tuple[2] = janet_wrap_buffer(data_buf);
    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 3));
}


static Janet cfun_RegCloseKey(int32_t argc, Janet *argv)
{
    HKEY hKey;

    LSTATUS lRet;

    janet_fixarity(argc, 1);

    hKey = jw32_get_handle(argv, 0);

    lRet = RegCloseKey(hKey);

    return jw32_wrap_long(lRet);
}


static const JanetReg cfuns[] = {
    {
        "RegOpenCurrentUser",
        cfun_RegOpenCurrentUser,
        "(" MOD_NAME "/RegOpenCurrentUser samDesired)\n\n"
        "Retrieves a handle to the HKEY_CURRENT_USER key.",
    },
    {
        "RegOpenKeyEx",
        cfun_RegOpenKeyEx,
        "(" MOD_NAME "/RegOpenKeyEx hKey lpSubKey ulOptions samDesired)\n\n"
        "Opens the specified registry key.",
    },
    {
        "RegGetValue",
        cfun_RegGetValue,
        "(" MOD_NAME "/RegGetValue hkey lpSubKey lpValue dwFlags)\n\n"
        "Retrieves the type and data for the specified registry value.",
    },
    {
        "RegCloseKey",
        cfun_RegCloseKey,
        "(" MOD_NAME "/RegCloseKey hkey)\n\n"
        "Closes a handle to a registry key.",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_consts_rrf(env);
    define_consts_hkey(env);

    janet_cfuns(env, MOD_NAME, cfuns);
}
