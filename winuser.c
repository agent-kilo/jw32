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

    if (janet_checktype(argv[0], JANET_POINTER)) {
        hWnd = jw32_unwrap_handle(argv[0]);
    } else if (janet_checktype(argv[0], JANET_NIL)) {
        hWnd = NULL;
    } else {
        janet_panicf("expected pointer or nil for hWnd");
    }

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
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, MOD_NAME, cfuns);
}
