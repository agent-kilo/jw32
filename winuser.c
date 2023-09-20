#include "winuser.h"

#define MOD_NAME "winuser"


static Janet cfun_GetDesktopWindow(int32_t argc, Janet *argv)
{
    janet_fixarity(argc, 0);
    return jw32_wrap_handle(GetDesktopWindow());
}

static Janet cfun_GetMessage(int32_t argc, Janet *argv)
{
    MSG msg;
    HWND hwnd;
    UINT msg_filter_min, msg_filter_max;
    BOOL ret;

    janet_fixarity(argc, 3);

    if (janet_checktype(argv[0], JANET_POINTER)) {
        hwnd = jw32_unwrap_handle(argv[0]);
    } else if (janet_checktype(argv[0], JANET_NIL)) {
        hwnd = NULL;
    } else {
        janet_panicf("expected pointer for hwnd");
    }

    msg_filter_min = jw32_unwrap_uint(argv[1]);
    msg_filter_max = jw32_unwrap_uint(argv[2]);

    ret = GetMessage(&msg, hwnd, msg_filter_min, msg_filter_max);

    if (ret != 0) {
        JanetTable *msg_table;
        Janet msg_point[2];

        /* yeah that's right, a BOOL can be -1 */
        if (ret == -1) {
            /* error */
            janet_panicf("GetMessage failed: %" PRIu32 "\n", GetLastError());
        }

        msg_table = janet_table(7); /* the field count in MSG struct */
        janet_table_put(msg_table, janet_wrap_keyword("hwnd"), jw32_wrap_handle(msg.hwnd));
        janet_table_put(msg_table, janet_wrap_keyword("message"), jw32_wrap_uint(msg.message));
        janet_table_put(msg_table, janet_wrap_keyword("wParam"), jw32_wrap_wparam(msg.wParam));
        janet_table_put(msg_table, janet_wrap_keyword("lParam"), jw32_wrap_lparam(msg.lParam));
        janet_table_put(msg_table, janet_wrap_keyword("time"), jw32_wrap_dword(msg.time));

        msg_point[0] = jw32_wrap_long(msg.pt.x);
        msg_point[1] = jw32_wrap_long(msg.pt.y);
        janet_table_put(msg_table, janet_wrap_keyword("pt"), janet_wrap_tuple(janet_tuple_n(msg_point, 2)));
        
        janet_table_put(msg_table, janet_wrap_keyword("lPrivate"), jw32_wrap_dword(msg.lPrivate));

        return janet_wrap_table(msg_table);
    } else {
        /* WM_QUIT received */
        return janet_wrap_nil();
    }
}


static const JanetReg cfuns[] = {
    {
        "GetDesktopWindow",
        cfun_GetDesktopWindow,
        "(" MOD_NAME "/GetDesktopWindow)\n\n"
        "Win32 function wrapper.",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, MOD_NAME, cfuns);
}
