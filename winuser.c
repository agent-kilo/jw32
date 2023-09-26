#include <stdio.h>
#include <inttypes.h>
#include "winuser.h"

#define MOD_NAME "winuser"


/*******************************************************************
 *
 * MESSAGING
 *
 *******************************************************************/

static int MSG_get(void *p, Janet key, Janet *out)
{
    MSG *msg = (MSG *)p;

    if (!janet_checktype(key, JANET_KEYWORD)) {
        janet_panicf("expected keyword, got %v", key);
    }

    const uint8_t *kw = janet_unwrap_keyword(key);

#define __get_member(member, type)              \
    if (!janet_cstrcmp(kw, #member)) {          \
        *out = jw32_wrap_##type(msg->member);   \
        return 1;                               \
    }

    __get_member(hwnd, handle)
    __get_member(message, uint)
    __get_member(wParam, wparam)
    __get_member(lParam, lparam)
    __get_member(time, dword)
    if (!janet_cstrcmp(kw, "pt")) {
        Janet msg_pt[2];
        msg_pt[0] = jw32_wrap_long(msg->pt.x);
        msg_pt[1] = jw32_wrap_long(msg->pt.y);
        *out = janet_wrap_tuple(janet_tuple_n(msg_pt, 2));
        return 1;
    }
#ifdef _MAC
    __get_member(lPrivate, dword)
#endif

#undef __get_member

    return 0;
}

static const JanetAbstractType jw32_at_MSG = {
    .name = MOD_NAME "/MSG",
    .gc = NULL,
    .gcmark = NULL,
    .get = MSG_get,
    JANET_ATEND_GET
};

static Janet cfun_MSG(int32_t argc, Janet *argv)
{
    if ((argc & 1) != 0) {
        janet_panicf("expected even number of arguments, got %d", argc);
    }

    MSG *msg = janet_abstract(&jw32_at_MSG, sizeof(MSG));
    memset(msg, 0, sizeof(MSG));

    for (int32_t k = 0, v = 1; k < argc; k += 2, v += 2) {
        const uint8_t *kw = janet_getkeyword(argv, k);

#define __set_member(member, type)         \
        if (!janet_cstrcmp(kw, #member)) {          \
            msg->member = jw32_get_##type(argv, v); \
        } else

        __set_member(hwnd, handle)
        __set_member(message, uint)
        __set_member(wParam, wparam)
        __set_member(lParam, lparam)
        __set_member(time, dword)
        if (!janet_cstrcmp(kw, "pt")) {
            JanetView idx = janet_getindexed(argv, v);
            if (idx.len != 2) {
                janet_panicf("expected 2 values for pt, got %d", idx.len);
            }
            /* XXX: bogus error message */
            msg->pt.x = jw32_get_long(idx.items, 0);
            msg->pt.y = jw32_get_long(idx.items, 1);
        } else
#ifdef _MAC
        __set_member(lPrivate, dword)
#endif
#undef __set_member
        {
            janet_panicf("unknown key %v", argv[k]);
        }
    }

    return janet_wrap_abstract(msg);
}

static Janet cfun_GetMessage(int32_t argc, Janet *argv)
{
    MSG *lpMsg;
    HWND hWnd;
    UINT wMsgFilterMin, wMsgFilterMax;

    BOOL bRet;

    janet_fixarity(argc, 4);

    lpMsg = janet_getabstract(argv, 0, &jw32_at_MSG);
    hWnd = jw32_get_handle(argv, 1);
    wMsgFilterMin = jw32_get_uint(argv, 2);
    wMsgFilterMax = jw32_get_uint(argv, 3);

    bRet = GetMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
    return jw32_wrap_bool(bRet);
}

static Janet cfun_TranslateMessage(int32_t argc, Janet *argv)
{
    MSG *lpMsg;

    BOOL bRet;

    janet_fixarity(argc, 1);

    lpMsg = janet_getabstract(argv, 0, &jw32_at_MSG);
    bRet = TranslateMessage(lpMsg);
    return jw32_wrap_bool(bRet);
}

static Janet cfun_DispatchMessage(int32_t argc, Janet *argv)
{
    MSG *lpMsg;

    LRESULT lRet;

    janet_fixarity(argc, 1);

    lpMsg = janet_getabstract(argv, 0, &jw32_at_MSG);
    lRet = DispatchMessage(lpMsg);
    return jw32_wrap_lresult(lRet);
}

static Janet cfun_DefWindowProc(int32_t argc, Janet *argv)
{
    HWND hWnd;
    UINT uMsg;
    WPARAM wParam;
    LPARAM lParam;

    LRESULT lRet;

    janet_fixarity(argc, 4);

    hWnd = jw32_get_handle(argv, 0);
    uMsg = jw32_get_uint(argv, 1);
    wParam = jw32_get_wparam(argv, 2);
    lParam = jw32_get_lparam(argv, 3);

    lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);

    return jw32_wrap_lresult(lRet);
}

static Janet cfun_PostThreadMessage(int32_t argc, Janet *argv)
{
    DWORD idThread;
    UINT uMsg;
    WPARAM wParam;
    LPARAM lParam;

    janet_fixarity(argc, 4);

    idThread = jw32_get_dword(argv, 0);
    uMsg = jw32_get_uint(argv, 1);
    wParam = jw32_get_wparam(argv, 2);
    lParam = jw32_get_lparam(argv, 3);

    return jw32_wrap_bool(PostThreadMessage(idThread, uMsg, wParam, lParam));
}

static void register_class_wnd_proc(Janet cls_name, Janet wnd_proc)
{
    /* TODO */
}

static Janet get_class_wnd_proc(Janet cls_name)
{
    /* TODO */
    return janet_wrap_nil();
}

LRESULT jw32_wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
/* TODO */
    printf("\n---- jw32_wnd_proc ----\n");
    printf("hWnd = 0x%" PRIx64 "\n", (uint64_t)hWnd);
    printf("uMsg = 0x%" PRIx32 "\n", uMsg);
    printf("wParam = 0x%" PRIx64 "\n", wParam);
    printf("lParam = %lld\n", lParam);
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


/*******************************************************************
 *
 * WINDOW-RELATED
 *
 *******************************************************************/

static Janet cfun_GetDesktopWindow(int32_t argc, Janet *argv)
{
    janet_fixarity(argc, 0);

    return jw32_wrap_handle(GetDesktopWindow());
}

static Janet cfun_CreateWindow(int32_t argc, Janet *argv)
{
    LPCSTR lpClassName, lpWindowName;
    DWORD dwStyle;
    int x, y, nWidth, nHeight;
    HWND hWndParent;
    HMENU hMenu;
    HINSTANCE hInstance;
    LPVOID lpParam;

    HWND hWnd;

    janet_fixarity(argc, 11);

    lpClassName = jw32_get_lpcstr(argv, 0);
    lpWindowName = jw32_get_lpcstr(argv, 1);
    dwStyle = jw32_get_dword(argv, 2);
    x = jw32_get_int(argv, 3);
    y = jw32_get_int(argv, 4);
    nWidth = jw32_get_int(argv, 5);
    nHeight = jw32_get_int(argv, 6);
    hWndParent = jw32_get_handle(argv, 7);
    hMenu = jw32_get_handle(argv, 8);
    hInstance = jw32_get_handle(argv, 9);
    lpParam = jw32_get_lpvoid(argv, 10);

    hWnd = CreateWindow(lpClassName, lpWindowName, dwStyle,
                        x, y, nWidth, nHeight,
                        hWndParent, hMenu, hInstance,
                        lpParam);

    return jw32_wrap_handle(hWnd);
}

static Janet cfun_ShowWindow(int32_t argc, Janet *argv)
{
    HWND hWnd;
    int nCmdShow;

    BOOL bRet;

    janet_fixarity(argc, 2);

    hWnd = jw32_get_handle(argv, 0);
    nCmdShow = jw32_get_int(argv, 1);

    bRet = ShowWindow(hWnd, nCmdShow);

    return jw32_wrap_bool(bRet);
}

static Janet cfun_UpdateWindow(int32_t argc, Janet *argv)
{
    HWND hWnd;

    BOOL bRet;

    janet_fixarity(argc, 1);

    hWnd = jw32_get_handle(argv, 0);

    bRet = UpdateWindow(hWnd);

    return jw32_wrap_bool(bRet);
}

/* extra space for our janet objects, to deal with gc and stuff */
typedef struct {
    JanetFunction *wnd_proc;
    JanetString menu_name;
    JanetString class_name;
    WNDCLASSEX wc;
} jw32_wc_t;

static int WNDCLASSEX_gcmark(void *p, size_t len)
{
    jw32_wc_t *jwc = (jw32_wc_t *)p;

    janet_mark(janet_wrap_abstract(jwc));
    if (jwc->wnd_proc) {
        janet_mark(janet_wrap_function(jwc->wnd_proc));
    }
    if (jwc->menu_name) {
        janet_mark(janet_wrap_string(jwc->menu_name));
    }
    if (jwc->class_name) {
        janet_mark(janet_wrap_string(jwc->class_name));
    }

    return 0;
}

static int WNDCLASSEX_get(void *p, Janet key, Janet *out)
{
    jw32_wc_t *jwc = (jw32_wc_t *)p;

    if (!janet_checktype(key, JANET_KEYWORD)) {
        janet_panicf("expected keyword, got %v", key);
    }

    const uint8_t *kw = janet_unwrap_keyword(key);

#define __get_member(member, type)               \
    if (!janet_cstrcmp(kw, #member)) {           \
        *out = jw32_wrap_##type(jwc->wc.member); \
        return 1;                                \
    }

    __get_member(cbSize, uint)
    __get_member(style, uint)
    if (!janet_cstrcmp(kw, "lpfnWndProc")) {
        *out = janet_wrap_function(jwc->wnd_proc);
        return 1;
    }
    __get_member(cbClsExtra, int)
    __get_member(cbWndExtra, int)
    __get_member(hInstance, handle)
    __get_member(hIcon, handle)
    __get_member(hCursor, handle)
    __get_member(hbrBackground, handle)
    if (!janet_cstrcmp(kw, "lpszMenuName")) {
        if (jwc->menu_name) {
            *out = janet_wrap_string(jwc->menu_name);
        } else {
            *out = janet_wrap_pointer((void *)jwc->wc.lpszMenuName);
        }
        return 1;
    }
    if (!janet_cstrcmp(kw, "lpszClassName")) {
        if (jwc->class_name) {
            *out = janet_wrap_string(jwc->class_name);
        } else {
            *out = janet_wrap_pointer((void *)jwc->wc.lpszClassName);
        }
        return 1;
    }
    __get_member(hIconSm, handle)

#undef __get_member

    return 0;
}

static const JanetAbstractType jw32_at_WNDCLASSEX = {
    .name = MOD_NAME "/WNDCLASSEX",
    .gc = NULL,
    .gcmark = WNDCLASSEX_gcmark,
    .get = WNDCLASSEX_get,
    JANET_ATEND_GET
};

static Janet cfun_WNDCLASSEX(int32_t argc, Janet *argv)
{
    if ((argc & 1) != 0) {
        janet_panicf("expected even number of arguments, got %d", argc);
    }

    jw32_wc_t *jwc = janet_abstract(&jw32_at_WNDCLASSEX, sizeof(jw32_wc_t));
    memset(jwc, 0, sizeof(jw32_wc_t));

    for (int32_t k = 0, v = 1; k < argc; k += 2, v += 2) {
        const uint8_t *kw = janet_getkeyword(argv, k);

#define __set_member(member, type)                  \
        if (!janet_cstrcmp(kw, #member)) {          \
            jwc->wc.member = jw32_get_##type(argv, v);  \
        } else

        __set_member(cbSize, uint)
        __set_member(style, uint)
        if (!janet_cstrcmp(kw, "lpfnWndProc")) {
            jwc->wnd_proc = janet_getfunction(argv, v);
            /* XXX: the wnd_proc should live beyond the wc struct's life span,
               maybe we should do this in RegisterClassEx() after a successful
               registration? */
            janet_gcroot(argv[v]);
            jwc->wc.lpfnWndProc = jw32_wnd_proc;
        } else
        __set_member(cbClsExtra, int)
        __set_member(cbWndExtra, int)
        __set_member(hInstance, handle)
        __set_member(hIcon, handle)
        __set_member(hCursor, handle)
        __set_member(hbrBackground, handle)
        if (!janet_cstrcmp(kw, "lpszMenuName")) {
            jwc->wc.lpszMenuName = jw32_get_lpcstr(argv, v);
            if (janet_checktype(argv[v], JANET_STRING)) {
                /* we need to at least keep these strings around until RegisterClassEx() is called */
                /* see WNDCLASSEX_gcmark */
                jwc->menu_name = janet_unwrap_string(argv[v]);
            }
        } else
        if (!janet_cstrcmp(kw, "lpszClassName")) {
            jwc->wc.lpszClassName = jw32_get_lpcstr(argv, v);
            if (janet_checktype(argv[v], JANET_STRING)) {
                jwc->class_name = janet_unwrap_string(argv[v]);
            }
        } else
        __set_member(hIconSm, handle)
#undef __set_member
        {
            janet_panicf("unknown key %v", argv[k]);
        }
    }

    return janet_wrap_abstract(jwc);
}

static void table_to_wndclass(JanetTable *wc_table, WNDCLASS *wc)
{
    Janet lpfnWndProc = janet_table_get(wc_table, jw32_cstr_to_keyword("lpfnWndProc"));

    memset(wc, 0, sizeof(*wc));

    table_val_to_struct_member(wc_table, wc, style, uint);

    if (janet_checktype(lpfnWndProc, JANET_FUNCTION)) {
        wc->lpfnWndProc = jw32_wnd_proc;
    }

    table_val_to_struct_member(wc_table, wc, cbClsExtra, int);
    table_val_to_struct_member(wc_table, wc, cbWndExtra, int);
    table_val_to_struct_member(wc_table, wc, hInstance, handle);
    table_val_to_struct_member(wc_table, wc, hIcon, handle);
    table_val_to_struct_member(wc_table, wc, hCursor, handle);
    table_val_to_struct_member(wc_table, wc, hbrBackground, handle);
    table_val_to_struct_member(wc_table, wc, lpszMenuName, lpcstr);
    table_val_to_struct_member(wc_table, wc, lpszClassName, lpcstr);
}

static Janet cfun_RegisterClass(int32_t argc, Janet *argv)
{
    WNDCLASS wndClass;

    ATOM aRet;

    JanetTable *wc_table;

    janet_fixarity(argc, 1);

    wc_table = janet_gettable(argv, 0);
    table_to_wndclass(wc_table, &wndClass);
    if (!(wndClass.lpfnWndProc)) {
        janet_panicf("no suitable lpfnWndProc set");
    }
    aRet = RegisterClass(&wndClass);
    if (aRet) {
        /* RegisterClass succeeded, save our real function for jw32_wnd_proc() */
        Janet wnd_proc = janet_table_get(wc_table, jw32_cstr_to_keyword("lpfnWndProc")),
            cls_name = jw32_cstr_to_keyword(wndClass.lpszClassName);
        register_class_wnd_proc(cls_name, wnd_proc);
    }

    return jw32_wrap_atom(aRet);
}


static const JanetReg cfuns[] = {

    /************************* MESSAGING ***************************/
    {
        "MSG",
        cfun_MSG,
        "(" MOD_NAME "/MSG ...)\n\n"
        "Builds a MSG struct.",
    },
    {
        "GetMessage",
        cfun_GetMessage,
        "(" MOD_NAME "/GetMessage lpMsg hWnd wMsgFilterMin wMsgFilterMax)\n\n"
        "Returns non-zero if the operation succeeds.",
    },
    {
        "TranslateMessage",
        cfun_TranslateMessage,
        "(" MOD_NAME "/TranslateMessage lpMsg)\n\n"
        "Translate key messages.",
    },
    {
        "DispatchMessage",
        cfun_DispatchMessage,
        "(" MOD_NAME "/DispatchMessage lpMsg)\n\n"
        "Returns the value returned by the window procedure.",
    },
    {
        "DefWindowProc",
        cfun_DefWindowProc,
        "(" MOD_NAME "/DefWindowProc hWnd uMsg wParam lParam)\n\n"
        "Default window procedure.",
    },
    {
        "PostThreadMessage",
        cfun_PostThreadMessage,
        "(" MOD_NAME "/PostThreadMessage idThread uMsg wParam lParam)\n\n"
        "Returns non-zero if succeeded, zero otherwise.",
    },

    /*********************** WINDOW-RELATED ************************/
    {
        "GetDesktopWindow",
        cfun_GetDesktopWindow,
        "(" MOD_NAME "/GetDesktopWindow)\n\n"
        "Win32 function wrapper.",
    },
    {
        "CreateWindow",
        cfun_CreateWindow,
        "(" MOD_NAME "/CreateWindow lpClassName lpWindowName dwStyle x y nWidth nHeight hWndParent hMenu hInstance lpParam)\n\n"
        "Creates a window.",
    },
    {
        "ShowWindow",
        cfun_ShowWindow,
        "(" MOD_NAME "/ShowWindow hWnd nCmdShow)\n\n"
        "Shows a window.",
    },
    {
        "UpdateWindow",
        cfun_UpdateWindow,
        "(" MOD_NAME "/UpdateWindow hWnd)\n\n"
        "Updates a window.",
    },
    {
        "WNDCLASSEX",
        cfun_WNDCLASSEX,
        "(" MOD_NAME "/WNDCLASSEX ...)\n\n"
        "Builds a WNDCLASSEX struct.",
    },
    {
        "RegisterClass",
        cfun_RegisterClass,
        "(" MOD_NAME "/RegisterClass wndClass)\n\n"
        "Registers a window class",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, MOD_NAME, cfuns);
    janet_register_abstract_type(&jw32_at_MSG);
}
