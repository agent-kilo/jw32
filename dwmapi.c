#include "jw32.h"
#include <dwmapi.h>

#define MOD_NAME "dwmapi"


static void define_consts_dwmwa(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),    \
              "Constant for DWMWINDOWATTRIBUTE.")

    __def(DWMWA_NCRENDERING_ENABLED);
    __def(DWMWA_NCRENDERING_POLICY);
    __def(DWMWA_TRANSITIONS_FORCEDISABLED);
    __def(DWMWA_ALLOW_NCPAINT);
    __def(DWMWA_CAPTION_BUTTON_BOUNDS);
    __def(DWMWA_NONCLIENT_RTL_LAYOUT);
    __def(DWMWA_FORCE_ICONIC_REPRESENTATION);
    __def(DWMWA_FLIP3D_POLICY);
    __def(DWMWA_EXTENDED_FRAME_BOUNDS);
    __def(DWMWA_HAS_ICONIC_BITMAP);
    __def(DWMWA_DISALLOW_PEEK);
    __def(DWMWA_EXCLUDED_FROM_PEEK);
    __def(DWMWA_CLOAK);
    __def(DWMWA_CLOAKED);
    __def(DWMWA_FREEZE_REPRESENTATION);
    __def(DWMWA_PASSIVE_UPDATE_MODE);
    __def(DWMWA_LAST);

#undef __def
}


static void define_consts_dwm_cloaked(JanetTable *env)
{
#define __def(const_name)                                     \
    janet_def(env, #const_name, jw32_wrap_int(const_name),    \
              "Constant for DWM cloak states.")
    __def(DWM_CLOAKED_APP);
    __def(DWM_CLOAKED_SHELL);
    __def(DWM_CLOAKED_INHERITED);
#undef __def
}


static Janet cfun_DwmGetWindowAttribute(int32_t argc, Janet *argv)
{
    HWND hwnd;
    DWORD dwAttribute;

    HRESULT hRes;

    janet_fixarity(argc, 2);

    hwnd = jw32_get_handle(argv, 0);
    dwAttribute = jw32_get_dword(argv, 1);

    switch (dwAttribute) {
    case DWMWA_CLOAKED: {
        int cloaked;
        hRes = DwmGetWindowAttribute(hwnd, dwAttribute, &cloaked, sizeof(cloaked));
        JW32_HR_RETURN_OR_PANIC(hRes, jw32_wrap_int(cloaked));
    }
    case DWMWA_EXTENDED_FRAME_BOUNDS: {
        RECT rect;
        hRes = DwmGetWindowAttribute(hwnd, dwAttribute, &rect, sizeof(rect));
        JW32_HR_RETURN_OR_PANIC(hRes, janet_wrap_struct(jw32_rect_to_struct(&rect)));
    }
    default:
        janet_panicf("unsupported attribute: %d", dwAttribute);
    }
}


static const JanetReg cfuns[] = {
    {
        "DwmGetWindowAttribute",
        cfun_DwmGetWindowAttribute,
        "(" MOD_NAME "/DwmGetWindowAttribute hwnd dwAttribute)\n\n"
        "Retrieves the current value of a specified DWM attribute applied to a window.",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_consts_dwmwa(env);
    define_consts_dwm_cloaked(env);

    janet_cfuns(env, MOD_NAME, cfuns);
}
