#include "jw32.h"
#include <commctrl.h>
#include "debug.h"

#define MOD_NAME "commctrl"


static void define_window_class_names(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, janet_cstringv(const_name),     \
              "Constant for global window class names.")

    __def(ANIMATE_CLASS);
    __def(DATETIMEPICK_CLASS);
    __def(HOTKEY_CLASS);
    //__def(LINK_CLASS);
    __def(MONTHCAL_CLASS);
    //__def(NATIVEFNTCTL_CLASS);
    __def(PROGRESS_CLASS);
    __def(REBARCLASSNAME);
    //__def(STANDARD_CLASSES);
    __def(STATUSCLASSNAME);
    __def(TOOLBARCLASSNAME);
    __def(TOOLTIPS_CLASS);
    __def(TRACKBAR_CLASS);
    __def(UPDOWN_CLASS);
    __def(WC_BUTTON);
    __def(WC_COMBOBOX);
    __def(WC_COMBOBOXEX);
    __def(WC_EDIT);
    __def(WC_HEADER);
    __def(WC_LISTBOX);
    __def(WC_IPADDRESS);
    //__def(WC_LINK);
    __def(WC_LISTVIEW);
    __def(WC_NATIVEFONTCTL);
    __def(WC_PAGESCROLLER);
    __def(WC_SCROLLBAR);
    __def(WC_STATIC);
    __def(WC_TABCONTROL);
    __def(WC_TREEVIEW);

#undef __def
}


static void define_const_lpstr_textcallback(JanetTable *env)
{
    janet_def(env, "LPSTR_TEXTCALLBACK", janet_wrap_pointer(LPSTR_TEXTCALLBACK),
              "Special value for lpszText in TTTOOLINFO.");
}


static void define_consts_lim(JanetTable *env)
{
#define __def(const_name)                                  \
    janet_def(env, #const_name, jw32_wrap_int(const_name), \
              "Constant flag for LoadIconMetric.")

    __def(LIM_SMALL);
    __def(LIM_LARGE);

#undef __def
}


static void define_consts_tts(JanetTable *env)
{
#define __def(const_name)                                  \
    janet_def(env, #const_name, jw32_wrap_int(const_name), \
              "Constant flag for ToolTip styles.")

    __def(TTS_ALWAYSTIP);
    __def(TTS_NOPREFIX);
    __def(TTS_NOANIMATE);
    __def(TTS_NOFADE);
    __def(TTS_BALLOON);
    __def(TTS_CLOSE);
#if (NTDDI_VERSION >= NTDDI_VISTA)
    __def(TTS_USEVISUALSTYLE);  // Use themed hyperlinks
#endif

#undef __def
}


static void define_consts_ttf(JanetTable *env)
{
#define __def(const_name)                                  \
    janet_def(env, #const_name, jw32_wrap_int(const_name), \
              "Constant flag for TOOLINFO.")

    __def(TTF_IDISHWND);
    __def(TTF_CENTERTIP);
    __def(TTF_RTLREADING);
    __def(TTF_SUBCLASS);
    __def(TTF_TRACK);
    __def(TTF_ABSOLUTE);
    __def(TTF_TRANSPARENT);
    __def(TTF_PARSELINKS);
    __def(TTF_DI_SETITEM);

#undef __def
}


static Janet cfun_LoadIconMetric(int32_t argc, Janet *argv)
{
    HINSTANCE hInst;
    PCWSTR pszName;
    int lims;

    HRESULT hrRet;
    HICON hIcon = NULL;

    janet_fixarity(argc, 3);

    hInst = jw32_get_handle(argv, 0);
    /* XXX: Caller needs to make sure there's actually a unicode string inside the pointer */
    pszName = (PCWSTR)jw32_get_lpcstr(argv, 1);
    lims = jw32_get_int(argv, 2);

    hrRet = LoadIconMetric(hInst, pszName, lims, &hIcon);
    JW32_HR_RETURN_OR_PANIC(hrRet, jw32_wrap_handle(hIcon));
}


typedef struct {
    TTTOOLINFO ti;
    JanetBuffer *text;
} jw32_tttoolinfo_t;


static int TTTOOLINFO_gcmark(void *p, size_t len)
{
    jw32_tttoolinfo_t *jti = (jw32_tttoolinfo_t *)p;

    (void)len;

    janet_mark(janet_wrap_abstract(jti));
    if (jti->text) {
        janet_mark(janet_wrap_buffer(jti->text));
    }

    return 0;
}


static int TTTOOLINFO_get(void *p, Janet key, Janet *out)
{
    jw32_tttoolinfo_t *jti = (jw32_tttoolinfo_t *)p;
    TTTOOLINFO *ti = &(jti->ti);

    if (!janet_checktype(key, JANET_KEYWORD)) {
        janet_panicf("expected keyword, got %v", key);
    }

    const uint8_t *kw = janet_unwrap_keyword(key);

#define __get_member(member, type) do {                 \
        if (!janet_cstrcmp(kw, #member)) {              \
            *out = jw32_wrap_##type(ti->member);        \
            return 1;                                   \
        }                                               \
    } while (0)

    __get_member(cbSize, uint);
    __get_member(uFlags, uint);
    __get_member(hwnd, handle);
    __get_member(uId, uint_ptr);
    if (!janet_cstrcmp(kw, "rect")) {
        *out = janet_wrap_table(jw32_rect_to_table(&(ti->rect)));
        return 1;
    }
    __get_member(hinst, handle);
    if (!janet_cstrcmp(kw, "lpszText")) {
        if (jti->text) {
            *out = janet_wrap_buffer(jti->text);
        } else {
            *out = janet_wrap_pointer(ti->lpszText);
        }
        return 1;
    }
    __get_member(lParam, lparam);
    __get_member(lpReserved, handle);
    if (!janet_cstrcmp(kw, "address")) {
        *out = jw32_wrap_lparam(ti);
        return 1;
    }

#undef __get_member

    return 0;
}


static const JanetAbstractType jw32_at_TTTOOLINFO = {
    .name = MOD_NAME "/TTTOOLINFO",
    .gc = NULL,
    .gcmark = TTTOOLINFO_gcmark,
    .get = TTTOOLINFO_get,
    JANET_ATEND_GET
};


static Janet cfun_TTTOOLINFO(int32_t argc, Janet *argv)
{
    if ((argc & 1) != 0) {
        janet_panicf("expected even number of arguments, got %d", argc);
    }

    jw32_tttoolinfo_t *jti = janet_abstract(&jw32_at_TTTOOLINFO, sizeof(jw32_tttoolinfo_t));
    TTTOOLINFO *ti = &(jti->ti);
    memset(jti, 0, sizeof(*jti));

    int size_set = 0;

    for (int32_t k = 0, v = 1; k < argc; k += 2, v += 2) {
        const uint8_t *kw = janet_getkeyword(argv, k);

#define __set_member(member, type)                      \
        if (!janet_cstrcmp(kw, #member)) {              \
            ti->member = jw32_get_##type(argv, v);      \
            continue;                                   \
        }

        if (!janet_cstrcmp(kw, "cbSize")) {
            ti->cbSize = jw32_get_uint(argv, v);
            size_set = 1;
            continue;
        }
        __set_member(uFlags, uint)
        __set_member(hwnd, handle)
        if (!janet_cstrcmp(kw, "uId")) {
            if (janet_checktype(argv[v], JANET_POINTER)) {
                /* This field also accepts HWNDs */
                ti->uId = (UINT_PTR)janet_unwrap_pointer(argv[v]);
            } else {
                ti->uId = jw32_get_uint_ptr(argv, v);
            }
        }
        __set_member(rect, rect)
        __set_member(hinst, handle)
        if (!janet_cstrcmp(kw, "lpszText")) {
            ti->lpszText = jw32_get_lpstr(argv, v);
            if (janet_checktype(argv[v], JANET_BUFFER)) {
                jti->text = janet_unwrap_buffer(argv[v]);
            }
            continue;
        }
        __set_member(lParam, lparam)
        __set_member(lpReserved, handle)

#undef __set_member

        janet_panicf("unknown key %v", argv[k]);
    }

    if (!size_set) {
        ti->cbSize = sizeof(*ti);
    }

    return janet_wrap_abstract(jti);
}


static const JanetReg cfuns[] = {
    {
        "LoadIconMetric",
        cfun_LoadIconMetric,
        "(" MOD_NAME "/LoadIconMetric hInst pszName lims)\n\n"
        "Loads a specified icon resource with a client-specified system metric.",
    },
    {
        "TTTOOLINFO",
        cfun_TTTOOLINFO,
        "(" MOD_NAME "/TTTOOLINFO ...)\n\n"
        "Builds a TTTOOLINFO struct.",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_window_class_names(env);
    define_const_lpstr_textcallback(env);
    define_consts_lim(env);
    define_consts_tts(env);
    define_consts_ttf(env);

    janet_cfuns(env, MOD_NAME, cfuns);
}
