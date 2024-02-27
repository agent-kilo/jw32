#include "jw32.h"
#include <strsafe.h>
#include "debug.h"

#define MOD_NAME "shellapi"


static void define_consts_nim(JanetTable *env)
{
#define __def(const_name)                                      \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),   \
              "Constant for Shell_NotifyIcon dwMessage.")

    __def(NIM_ADD);
    __def(NIM_MODIFY);
    __def(NIM_DELETE);
    __def(NIM_SETFOCUS);
    __def(NIM_SETVERSION);

#undef __def
}


static void define_consts_nif(JanetTable *env)
{
#define __def(const_name)                                      \
    janet_def(env, #const_name, jw32_wrap_uint(const_name),    \
              "Constant for uFlags in NOTIFYICONDATA.")

    __def(NIF_MESSAGE);
    __def(NIF_ICON);
    __def(NIF_TIP);
    __def(NIF_STATE);
    __def(NIF_INFO);
#if (_WIN32_IE >= 0x600)
    __def(NIF_GUID);
#endif
#if (NTDDI_VERSION >= NTDDI_VISTA)
    __def(NIF_REALTIME);
    __def(NIF_SHOWTIP);
#endif // (NTDDI_VERSION >= NTDDI_VISTA)

#undef __def
}


static void define_consts_notifyicon_version(JanetTable *env)
{
#define __def(const_name)                                      \
    janet_def(env, #const_name, jw32_wrap_uint(const_name),   \
              "Constant for NOTIFYICONDATA version.")

    __def(    NOTIFYICON_VERSION);
#if (NTDDI_VERSION >= NTDDI_VISTA)
    __def(    NOTIFYICON_VERSION_4);
#endif // (NTDDI_VERSION >= NTDDI_VISTA)

#undef __def
}


static int NOTIFYICONDATA_get(void *p, Janet key, Janet *out)
{
    NOTIFYICONDATA *nid = (NOTIFYICONDATA *)p;

    if (!janet_checktype(key, JANET_KEYWORD)) {
        janet_panicf("expected keyword, got %v", key);
    }

    const uint8_t *kw = janet_unwrap_keyword(key);

#define __get_member(member, type) do {                 \
        if (!janet_cstrcmp(kw, #member)) {              \
            *out = jw32_wrap_##type(nid->member);      \
            return 1;                                   \
        }                                               \
    } while (0)

    __get_member(cbSize, dword);
    __get_member(hWnd, handle);
    __get_member(uID, uint);
    __get_member(uFlags, uint);
    __get_member(uCallbackMessage, uint);
    __get_member(hIcon, handle);
    if (!janet_cstrcmp(kw, "szTip")) {
        *out = janet_cstringv(nid->szTip);
        return 1;
    }
    __get_member(dwState, dword);
    __get_member(dwStateMask, dword);
    if (!janet_cstrcmp(kw, "szInfo")) {
        *out = janet_cstringv(nid->szInfo);
        return 1;
    }
    __get_member(uTimeout, uint);
    __get_member(uVersion, uint);
    if (!janet_cstrcmp(kw, "szInfoTitle")) {
        *out = janet_cstringv(nid->szInfoTitle);
        return 1;
    }
    __get_member(dwInfoFlags, dword);
    if (!janet_cstrcmp(kw, "guidItem")) {
        /* TODO */
        *out = janet_wrap_nil();
        return 1;
    }
    __get_member(hBalloonIcon, handle);

#undef __get_member

    return 0;
}


static const JanetAbstractType jw32_at_NOTIFYICONDATA = {
    .name = MOD_NAME "/NOTIFYICONDATA",
    .gc = NULL,
    .gcmark = NULL,
    .get = NOTIFYICONDATA_get,
    JANET_ATEND_GET
};


static Janet cfun_NOTIFYICONDATA(int32_t argc, Janet *argv)
{
    if ((argc & 1) != 0) {
        janet_panicf("expected even number of arguments, got %d", argc);
    }

    DWORD cbSize = sizeof(NOTIFYICONDATA);
    for (int32_t k = 0, v = 1; k < argc; k += 2, v += 2) {
        const uint8_t *kw = janet_getkeyword(argv, k);
        if (!janet_cstrcmp(kw, "cbSize")) {
            cbSize = jw32_get_dword(argv, v);
            break;
        }
    }

    /* XXX: Do we need to check the struct size according to DLL version? */
    NOTIFYICONDATA *nid = janet_abstract(&jw32_at_NOTIFYICONDATA, cbSize);
    memset(nid, 0, cbSize);
    nid->cbSize = cbSize;

    for (int32_t k = 0, v = 1; k < argc; k += 2, v += 2) {
        const uint8_t *kw = janet_getkeyword(argv, k);

#define __set_member(member, type)                  \
        if (!janet_cstrcmp(kw, #member)) {          \
            nid->member = jw32_get_##type(argv, v); \
            continue;                               \
        }

        if (!janet_cstrcmp(kw, "cbSize")) {
            /* already set */
            continue;
        }

        __set_member(hWnd, handle)
        __set_member(uID, uint)
        __set_member(uFlags, uint)
        __set_member(uCallbackMessage, uint)
        __set_member(hIcon, handle)
        if (!janet_cstrcmp(kw, "szTip")) {
            LPCSTR szTip = jw32_get_lpcstr(argv, v);
            StringCchCopy(nid->szTip, ARRAYSIZE(nid->szTip), szTip);
            continue;
        }
        __set_member(dwState, dword)
        __set_member(dwStateMask, dword)
        if (!janet_cstrcmp(kw, "szInfo")) {
            LPCSTR szInfo = jw32_get_lpcstr(argv, v);
            StringCchCopy(nid->szInfo, ARRAYSIZE(nid->szInfo), szInfo);
            continue;
        }
        __set_member(uTimeout, uint)
        __set_member(uVersion, uint)
        if (!janet_cstrcmp(kw, "szInfoTitle")) {
            LPCSTR szInfoTitle = jw32_get_lpcstr(argv, v);
            StringCchCopy(nid->szInfoTitle, ARRAYSIZE(nid->szInfoTitle), szInfoTitle);
            continue;
        }
        __set_member(dwInfoFlags, dword)
        if (!janet_cstrcmp(kw, "guidItem")) {
            /* TODO */
            continue;
        }
        __set_member(hBalloonIcon, handle)

#undef __set_member

        janet_panicf("unknown key %v", argv[k]);
    }

    return janet_wrap_abstract(nid);
}


static Janet cfun_Shell_NotifyIcon(int32_t argc, Janet *argv)
{
    DWORD dwMessage;
    PNOTIFYICONDATA lpData;

    BOOL bRet;

    janet_fixarity(argc, 2);

    dwMessage = jw32_get_dword(argv, 0);
    lpData = janet_getabstract(argv, 1, &jw32_at_NOTIFYICONDATA);

    bRet = Shell_NotifyIcon(dwMessage, lpData);
    return jw32_wrap_bool(bRet);
}


static const JanetReg cfuns[] = {
    {
        "NOTIFYICONDATA",
        cfun_NOTIFYICONDATA,
        "(" MOD_NAME "/NOTIFYICONDATA ...)\n\n"
        "Builds a NOTIFYICONDATA struct.",
    },
    {
        "Shell_NotifyIcon",
        cfun_Shell_NotifyIcon,
        "(" MOD_NAME "/Shell_NotifyIcon dwMessage NOTIFYICONDATA)\n\n"
        "Sends a message to the taskbar's status area.",
    },

    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_consts_nim(env);
    define_consts_nif(env);
    define_consts_notifyicon_version(env);

    janet_register_abstract_type(&jw32_at_NOTIFYICONDATA);

    janet_cfuns(env, MOD_NAME, cfuns);
}
