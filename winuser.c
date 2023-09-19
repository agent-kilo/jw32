#include "winuser.h"

#define MOD_NAME "winuser"


static Janet cfun_GetDesktopWindow(int32_t argc, Janet *argv)
{
    HWND hDesktopWindow = GetDesktopWindow();
    return jw32_wrap_handle(hDesktopWindow);
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
