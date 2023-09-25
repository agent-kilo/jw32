#include "winuser.h"

#define MOD_NAME "winuser"


static Janet cfun_GetDesktopWindow(int32_t argc, Janet *argv)
{
    janet_fixarity(argc, 0);

    return jw32_wrap_handle(GetDesktopWindow());
}

static Janet cfun_PostThreadMessage(int32_t argc, Janet *argv)
{
    DWORD idThread;
    UINT Msg;
    WPARAM wParam;
    LPARAM lParam;

    janet_fixarity(argc, 4);

    idThread = jw32_get_dword(argv, 0);
    Msg = jw32_get_uint(argv, 1);
    wParam = jw32_get_wparam(argv, 2);
    lParam = jw32_get_lparam(argv, 3);

    return jw32_wrap_bool(PostThreadMessage(idThread, Msg, wParam, lParam));
}

static void table_to_msg(JanetTable *msg_table, MSG *msg)
{
    Janet hwnd = janet_table_get(msg_table, jw32_cstr_to_keyword("hwnd")),
        pt = janet_table_get(msg_table, jw32_cstr_to_keyword("pt"));

    memset(msg, 0, sizeof(*msg));

    table_val_to_struct_member(msg_table, msg, hwnd, handle);
    table_val_to_struct_member(msg_table, msg, message, uint);
    table_val_to_struct_member(msg_table, msg, wParam, wparam);
    table_val_to_struct_member(msg_table, msg, lParam, lparam);
    table_val_to_struct_member(msg_table, msg, time, dword);

    if (!janet_checktype(pt, JANET_NIL)) {
        if (janet_checktype(pt, JANET_TUPLE)) {
            const Janet *pt_tuple = janet_unwrap_tuple(pt);
            if (janet_tuple_length(pt_tuple) != 2) {
                janet_panicf("expected tuple of length 2 for pt field, got %v", pt);
            }
            msg->pt.x = jw32_unwrap_long(pt_tuple[0]);
            msg->pt.y = jw32_unwrap_long(pt_tuple[1]);
        } else {
            janet_panicf("expected tuple of length 2 for pt field, got %v", pt);
        }
    }

#ifdef _MAC
    table_val_to_struct_member(msg_table, msg, lPrivate, dword);
#endif
}

static JanetTable *msg_to_table(MSG *msg)
{
    Janet msg_point[2];
    JanetTable *msg_table = janet_table(7); /* the field cound in MSG struct */

    janet_table_put(msg_table, jw32_cstr_to_keyword("hwnd"), jw32_wrap_handle(msg->hwnd));
    janet_table_put(msg_table, jw32_cstr_to_keyword("message"), jw32_wrap_uint(msg->message));
    janet_table_put(msg_table, jw32_cstr_to_keyword("wParam"), jw32_wrap_wparam(msg->wParam));
    janet_table_put(msg_table, jw32_cstr_to_keyword("lParam"), jw32_wrap_lparam(msg->lParam));
    janet_table_put(msg_table, jw32_cstr_to_keyword("time"), jw32_wrap_dword(msg->time));

    msg_point[0] = jw32_wrap_long(msg->pt.x);
    msg_point[1] = jw32_wrap_long(msg->pt.y);
    janet_table_put(msg_table, jw32_cstr_to_keyword("pt"),
                    janet_wrap_tuple(janet_tuple_n(msg_point, 2)));

    /* TIL, WTF? */
#ifdef _MAC
    janet_table_put(msg_table, jw32_cstr_to_keyword("lPrivate"),
                    jw32_wrap_dword(msg->lPrivate));
#endif

    return msg_table;
}

static Janet cfun_GetMessage(int32_t argc, Janet *argv)
{
    HWND hWnd;
    UINT wMsgFilterMin, wMsgFilterMax;

    BOOL bRet;
    MSG msg;
    Janet ret_tuple[2];

    janet_fixarity(argc, 3);

    hWnd = jw32_get_handle(argv, 0);
    wMsgFilterMin = jw32_get_uint(argv, 1);
    wMsgFilterMax = jw32_get_uint(argv, 2);

    bRet = GetMessage(&msg, hWnd, wMsgFilterMin, wMsgFilterMax);

    ret_tuple[0] = jw32_wrap_bool(bRet);

    /* yeah that's right, a BOOL can be -1 */
    if (bRet != -1) {
        /* in case of WM_QUIT, we return msg too */
        ret_tuple[1] = janet_wrap_table(msg_to_table(&msg));
    } else {
        /* error */
        ret_tuple[1] = janet_wrap_nil();
    }

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static Janet cfun_TranslateMessage(int32_t argc, Janet *argv)
{
    MSG msg;

    BOOL bRet;

    JanetTable *msg_table;

    janet_fixarity(argc, 1);

    msg_table = janet_gettable(argv, 0);

    table_to_msg(msg_table, &msg);
    return jw32_wrap_bool(TranslateMessage(&msg));
}

static Janet cfun_DispatchMessage(int32_t argc, Janet *argv)
{
    MSG msg;

    LRESULT lRet;

    JanetTable *msg_table;

    janet_fixarity(argc, 1);


    msg_table = janet_gettable(argv, 0);
    
    table_to_msg(msg_table, &msg);
    return jw32_wrap_lresult(DispatchMessage(&msg));
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
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
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
    {
        "GetDesktopWindow",
        cfun_GetDesktopWindow,
        "(" MOD_NAME "/GetDesktopWindow)\n\n"
        "Win32 function wrapper.",
    },
    {
        "PostThreadMessage",
        cfun_PostThreadMessage,
        "(" MOD_NAME "/PostThreadMessage idThread uMsg wParam lParam)\n\n"
        "Returns non-zero if succeeded, zero otherwise.",
    },
    {
        "GetMessage",
        cfun_GetMessage,
        "(" MOD_NAME "/GetMessage hWnd wMsgFilterMin wMsgFilterMax)\n\n"
        "Returns a tuple (bRet, msg).",
    },
    {
        "TranslateMessage",
        cfun_TranslateMessage,
        "(" MOD_NAME "/TranslateMessage msg)\n\n"
        "Translate key messages.",
    },
    {
        "DispatchMessage",
        cfun_DispatchMessage,
        "(" MOD_NAME "/DispatchMessage msg)\n\n"
        "Returns the value returned by the window procedure.",
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
        "DefWindowProc",
        cfun_DefWindowProc,
        "(" MOD_NAME "/DefWindowProc hWnd uMsg wParam lParam)\n\n"
        "Default window procedure.",
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
}
