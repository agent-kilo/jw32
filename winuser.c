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
    BOOL ret = GetMessage(&msg);

    if (ret != 0) {
        JanetTable *msg_table;
        JanetTuple *msg_point;

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
        janet_table_put(msg_table, janet_wrap_keyword("lPrivate"), jw32_wrap_dword(msg.lPrivate));
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
