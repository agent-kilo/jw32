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

    idThread = jw32_unwrap_dword(argv[0]);
    Msg = jw32_unwrap_uint(argv[1]);
    wParam = jw32_unwrap_wparam(argv[2]);
    lParam = jw32_unwrap_lparam(argv[3]);

    return jw32_wrap_bool(PostThreadMessage(idThread, Msg, wParam, lParam));
}

static void table_to_msg(JanetTable *msg_table, MSG *msg)
{
    Janet hwnd = janet_table_get(msg_table, jw32_cstr_to_keyword("hwnd")),
        message = janet_table_get(msg_table, jw32_cstr_to_keyword("message")),
        wParam = janet_table_get(msg_table, jw32_cstr_to_keyword("wParam")),
        lParam = janet_table_get(msg_table, jw32_cstr_to_keyword("lParam")),
        time = janet_table_get(msg_table, jw32_cstr_to_keyword("time")),
        pt = janet_table_get(msg_table, jw32_cstr_to_keyword("pt"));
#ifdef _MAC
    Janet lPrivate = janet_table_get(msg_table, jw32_cstr_to_keyword("lPrivate"));
#endif

    memset(msg, 0, sizeof(*msg));

    msg->hwnd = jw32_unwrap_handle(hwnd);
    if (!janet_checktype(message, JANET_NIL)) {
        msg->message = jw32_unwrap_uint(message);
    }
    if (!janet_checktype(wParam, JANET_NIL)) {
        msg->wParam = jw32_unwrap_wparam(wParam);
    }
    if (!janet_checktype(lParam, JANET_NIL)) {
        msg->lParam = jw32_unwrap_lparam(lParam);
    }
    if (!janet_checktype(time, JANET_NIL)) {
        msg->time = jw32_unwrap_dword(time);
    }

    if (!janet_checktype(pt, JANET_NIL)) {
        if (janet_checktype(pt, JANET_TUPLE)) {
            const Janet *pt_tuple = janet_unwrap_tuple(pt);
            if (janet_tuple_length(pt_tuple) != 2) {
                janet_panicf("expected tuple of length 2 for pt field, got %d",
                             janet_tuple_length(pt_tuple));
            }
            msg->pt.x = jw32_unwrap_long(pt_tuple[0]);
            msg->pt.y = jw32_unwrap_long(pt_tuple[1]);
        } else {
            janet_panicf("expected tuple of length 2 for pt field");
        }
    }

#ifdef _MAC
    if (!janet_checktype(lPrivate, JANET_NIL)) {
    msg->lPrivate = jw32_unwrap_dword(lPrivate);
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

    hWnd = jw32_unwrap_handle(argv[0]);
    wMsgFilterMin = jw32_unwrap_uint(argv[1]);
    wMsgFilterMax = jw32_unwrap_uint(argv[2]);

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

    janet_fixarity(argc, 1);

    table_to_msg(janet_unwrap_table(argv[0]), &msg);
    return jw32_wrap_bool(TranslateMessage(&msg));
}

static Janet cfun_DispatchMessage(int32_t argc, Janet *argv)
{
    MSG msg;

    LRESULT lRet;

    janet_fixarity(argc, 1);

    table_to_msg(janet_unwrap_table(argv[0]), &msg);
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

    lpClassName = jw32_unwrap_lpcstr(argv[0]);
    lpWindowName = jw32_unwrap_lpcstr(argv[1]);
    dwStyle = jw32_unwrap_dword(argv[2]);

    hWnd = CreateWindow(lpClassName, lpWindowName,dwStyle,
                        x, y, nWidth, nHeight,
                        hWndParent, hMenu, hInstance,
                        lpParam);
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
        "(" MOD_NAME "/PostThreadMessage idThread Msg wParam lParam)\n\n"
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
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, MOD_NAME, cfuns);
}
