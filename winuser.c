#include <stdio.h>
#include <inttypes.h>
#include "winuser.h"

#define MOD_NAME "winuser"


/* extra space for our janet objects, to deal with gc and stuff */
typedef struct {
    JanetFunction *wnd_proc;
    JanetString menu_name;
    JanetString class_name;
    WNDCLASSEX wc;
} jw32_wc_t;


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

static JanetTable *class_wnd_proc_registry;
static JanetTable *atom_class_map;

static void register_class_wnd_proc(jw32_wc_t *jwc, ATOM atmClass)
{
    Janet wnd_proc = janet_wrap_function(jwc->wnd_proc);
    Janet atom = jw32_wrap_atom(atmClass);
    Janet class_name = jw32_cstr_to_keyword(jwc->wc.lpszClassName);
    Janet key;

    /* an application can register local or global classes, and they
       are looked up in different ways */
    if (jwc->wc.style & CS_GLOBALCLASS) {
        key = class_name;
    } else {
        Janet h_instance = jw32_wrap_handle(jwc->wc.hInstance);
        Janet key_tuple[2] = { class_name, h_instance };
        key = janet_wrap_tuple(janet_tuple_n(key_tuple, 2));
    }

    janet_table_put(class_wnd_proc_registry, key, wnd_proc);
    janet_table_put(atom_class_map, atom, class_name);
}

static void unregister_class_wnd_proc(LPCSTR lpClassName, HINSTANCE hInstance)
{
    uint64_t maybe_atom = (uint64_t)lpClassName;
    Janet class_name;
    Janet local_key, global_key;
    Janet local_key_tuple[2];

    if (maybe_atom & ~(uint64_t)0xffff) {
        /* higher bits are not zero, we have a string pointer */
        class_name = jw32_cstr_to_keyword(lpClassName);
    } else {
        /* looks like an ATOM */
        ATOM atmClass = (ATOM)(maybe_atom & 0xffff);
#define __atom_name_buf_size 256 /* XXX: should be enough? */
        char buffer[__atom_name_buf_size];
        UINT uRet = GetAtomName(atmClass, buffer, __atom_name_buf_size);
        if (uRet) {
            class_name = jw32_cstr_to_keyword(buffer);
        } else {
            class_name = janet_wrap_nil();
        }
#undef __atom_name_buf_size
    }

    if (janet_checktype(class_name, JANET_NIL)) {
        return;
    }

    /* the logic inside UnregisterClass():

       hInstance == NULL means wildcard, search ALL local classes first; when there were
       multiple local classes with the same name, the one registered last gets
       unregistered first.

       hInstance != NULL means to search the local classes registered by this module first.

       if the class was not found among local classes, try to unregister global classes with
       that name.

       only unregister ONE class at a time. */

    local_key_tuple[0] = class_name;
    local_key_tuple[1] = jw32_wrap_handle(hInstance);
    local_key = janet_wrap_tuple(janet_tuple_n(local_key_tuple, 2));
    janet_table_put(class_wnd_proc_registry, local_key, janet_wrap_nil());

    global_key = class_name;
    janet_table_put(class_wnd_proc_registry, global_key, janet_wrap_nil());
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
        if (jwc->wnd_proc) {
            *out = janet_wrap_function(jwc->wnd_proc);
            return 1;
        } else {
            return 0;
        }
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

    int size_set = 0;

    for (int32_t k = 0, v = 1; k < argc; k += 2, v += 2) {
        const uint8_t *kw = janet_getkeyword(argv, k);

#define __set_member(member, type)                  \
        if (!janet_cstrcmp(kw, #member)) {          \
            jwc->wc.member = jw32_get_##type(argv, v);  \
        } else

        if (!janet_cstrcmp(kw, "cbSize")) {
            jwc->wc.cbSize = jw32_get_uint(argv, v);
            size_set = 1;
        } else
        __set_member(style, uint)
        if (!janet_cstrcmp(kw, "lpfnWndProc")) {
            jwc->wnd_proc = janet_getfunction(argv, v);
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

    if (!size_set) {
        jwc->wc.cbSize = sizeof(WNDCLASSEX);
    }

    return janet_wrap_abstract(jwc);
}

static Janet cfun_RegisterClassEx(int32_t argc, Janet *argv)
{
    jw32_wc_t *jwc;

    ATOM aRet;

    janet_fixarity(argc, 1);

    jwc = janet_getabstract(argv, 0, &jw32_at_WNDCLASSEX);
    if (!(jwc->wnd_proc)) {
        janet_panicf("no suitable lpfnWndProc set");
    }
    aRet = RegisterClassEx(&(jwc->wc));
    if (aRet) {
        /* RegisterClass succeeded, save our real function for jw32_wnd_proc() */
        register_class_wnd_proc(jwc, aRet);
    }

    return jw32_wrap_atom(aRet);
}

static Janet cfun_GetClassInfoEx(int32_t argc, Janet *argv)
{
    HINSTANCE hInstance;
    LPCSTR lpszClass;
    jw32_wc_t *jwc;

    BOOL bRet;

    janet_fixarity(argc, 3);

    hInstance = jw32_get_handle(argv, 0);
    lpszClass = jw32_get_lpcstr(argv, 1);
    jwc = janet_getabstract(argv, 2, &jw32_at_WNDCLASSEX);

    bRet = GetClassInfoEx(hInstance, lpszClass, &(jwc->wc));
    if (bRet) {
        if (jwc->wc.lpszClassName) {
            jwc->class_name = janet_cstring(jwc->wc.lpszClassName);
        }
        if (jwc->wc.lpszMenuName) {
            jwc->menu_name = janet_cstring(jwc->wc.lpszMenuName);
        }
        /* TODO: jwc->wnd_proc */
    }

    return jw32_wrap_bool(bRet);
}

static Janet cfun_UnregisterClass(int32_t argc, Janet *argv)
{
    LPCSTR lpClassName;
    HINSTANCE hInstance;

    BOOL bRet;

    janet_fixarity(argc, 2);

    lpClassName = jw32_get_lpcstr(argv, 0);
    hInstance = jw32_get_handle(argv, 1);

    bRet = UnregisterClass(lpClassName, hInstance);
    if(bRet) {
        unregister_class_wnd_proc(lpClassName, hInstance);
    }
    return jw32_wrap_bool(bRet);
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
        "RegisterClassEx",
        cfun_RegisterClassEx,
        "(" MOD_NAME "/RegisterClassEx lpWndClassEx)\n\n"
        "Registers a window class",
    },
    {
        "GetClassInfoEx",
        cfun_GetClassInfoEx,
        "(" MOD_NAME "/GetClassInfoEx hInstance lpszClass lpwcx)\n\n"
        "Get info for a window class",
    },
    {
        "UnregisterClass",
        cfun_UnregisterClass,
        "(" MOD_NAME "/UnregisterClass lpClassName hInstance)\n\n"
        "Unregisters a window class",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, MOD_NAME, cfuns);
    janet_register_abstract_type(&jw32_at_MSG);

    class_wnd_proc_registry = janet_table(0);
    atom_class_map = janet_table(0);

    janet_def(env, "class_wnd_proc_registry", janet_wrap_table(class_wnd_proc_registry),
              "Where all the WndProcs reside.");
    janet_def(env, "atom_class_map", janet_wrap_table(atom_class_map),
              "ATOM -> lpszClassName map.");
}
