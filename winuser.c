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


static JanetArray *local_class_wnd_proc_registry;
static JanetArray *global_class_wnd_proc_registry;


static void define_consts_wm(JanetTable *env)
{
#define __def(const_name)                                  \
    janet_def(env, #const_name, jw32_wrap_int(const_name), \
              "Constant for window message type.")
    __def(WM_NULL);
    __def(WM_CREATE);
    __def(WM_DESTROY);
    __def(WM_MOVE);
    __def(WM_SIZE);
    __def(WM_ACTIVATE);
    __def(WM_SETFOCUS);
    __def(WM_KILLFOCUS);
    __def(WM_ENABLE);
    __def(WM_SETREDRAW);
    __def(WM_SETTEXT);
    __def(WM_GETTEXT);
    __def(WM_GETTEXTLENGTH);
    __def(WM_PAINT);
    __def(WM_CLOSE);
#ifndef _WIN32_WCE
    __def(WM_QUERYENDSESSION);
    __def(WM_QUERYOPEN);
    __def(WM_ENDSESSION);
#endif
    __def(WM_QUIT);
    __def(WM_ERASEBKGND);
    __def(WM_SYSCOLORCHANGE);
    __def(WM_SHOWWINDOW);
    __def(WM_WININICHANGE);
#if(WINVER >= 0x0400)
    __def(WM_SETTINGCHANGE);
#endif
    __def(WM_DEVMODECHANGE);
    __def(WM_ACTIVATEAPP);
    __def(WM_FONTCHANGE);
    __def(WM_TIMECHANGE);
    __def(WM_CANCELMODE);
    __def(WM_SETCURSOR);
    __def(WM_MOUSEACTIVATE);
    __def(WM_CHILDACTIVATE);
    __def(WM_QUEUESYNC);
    __def(WM_GETMINMAXINFO);
    __def(WM_PAINTICON);
    __def(WM_ICONERASEBKGND);
    __def(WM_NEXTDLGCTL);
    __def(WM_SPOOLERSTATUS);
    __def(WM_DRAWITEM);
    __def(WM_MEASUREITEM);
    __def(WM_DELETEITEM);
    __def(WM_VKEYTOITEM);
    __def(WM_CHARTOITEM);
    __def(WM_SETFONT);
    __def(WM_GETFONT);
    __def(WM_SETHOTKEY);
    __def(WM_GETHOTKEY);
    __def(WM_QUERYDRAGICON);
    __def(WM_COMPAREITEM);
#if(WINVER >= 0x0500)
#ifndef _WIN32_WCE
    __def(WM_GETOBJECT);
#endif
#endif /* WINVER >= 0x0500 */
    __def(WM_COMPACTING);
    __def(WM_COMMNOTIFY);
    __def(WM_WINDOWPOSCHANGING);
    __def(WM_WINDOWPOSCHANGED);
    __def(WM_POWER);
    __def(WM_COPYDATA);
    __def(WM_CANCELJOURNAL);
#if(WINVER >= 0x0400)
    __def(WM_NOTIFY);
    __def(WM_INPUTLANGCHANGEREQUEST);
    __def(WM_INPUTLANGCHANGE);
    __def(WM_TCARD);
    __def(WM_HELP);
    __def(WM_USERCHANGED);
    __def(WM_NOTIFYFORMAT);
    __def(WM_CONTEXTMENU);
    __def(WM_STYLECHANGING);
    __def(WM_STYLECHANGED);
    __def(WM_DISPLAYCHANGE);
    __def(WM_GETICON);
    __def(WM_SETICON);
#endif /* WINVER >= 0x0400 */
    __def(WM_NCCREATE);
    __def(WM_NCDESTROY);
    __def(WM_NCCALCSIZE);
    __def(WM_NCHITTEST);
    __def(WM_NCPAINT);
    __def(WM_NCACTIVATE);
    __def(WM_GETDLGCODE);
#ifndef _WIN32_WCE
    __def(WM_SYNCPAINT);
#endif
    __def(WM_NCMOUSEMOVE);
    __def(WM_NCLBUTTONDOWN);
    __def(WM_NCLBUTTONUP);
    __def(WM_NCLBUTTONDBLCLK);
    __def(WM_NCRBUTTONDOWN);
    __def(WM_NCRBUTTONUP);
    __def(WM_NCRBUTTONDBLCLK);
    __def(WM_NCMBUTTONDOWN);
    __def(WM_NCMBUTTONUP);
    __def(WM_NCMBUTTONDBLCLK);
#if(_WIN32_WINNT >= 0x0500)
    __def(WM_NCXBUTTONDOWN);
    __def(WM_NCXBUTTONUP);
    __def(WM_NCXBUTTONDBLCLK);
#endif /* _WIN32_WINNT >= 0x0500 */
#if(_WIN32_WINNT >= 0x0501)
    __def(WM_INPUT_DEVICE_CHANGE);
    __def(WM_INPUT);
#endif /* _WIN32_WINNT >= 0x0501 */
    __def(WM_KEYFIRST);
    __def(WM_KEYDOWN);
    __def(WM_KEYUP);
    __def(WM_CHAR);
    __def(WM_DEADCHAR);
    __def(WM_SYSKEYDOWN);
    __def(WM_SYSKEYUP);
    __def(WM_SYSCHAR);
    __def(WM_SYSDEADCHAR);
#if(_WIN32_WINNT >= 0x0501)
    __def(WM_UNICHAR);
#endif /* _WIN32_WINNT >= 0x0501 */
    __def(WM_KEYLAST);
#if(WINVER >= 0x0400)
    __def(WM_IME_STARTCOMPOSITION);
    __def(WM_IME_ENDCOMPOSITION);
    __def(WM_IME_COMPOSITION);
    __def(WM_IME_KEYLAST);
#endif /* WINVER >= 0x0400 */
    __def(WM_INITDIALOG);
    __def(WM_COMMAND);
    __def(WM_SYSCOMMAND);
    __def(WM_TIMER);
    __def(WM_HSCROLL);
    __def(WM_VSCROLL);
    __def(WM_INITMENU);
    __def(WM_INITMENUPOPUP);
#if(WINVER >= 0x0601)
    __def(WM_GESTURE);
    __def(WM_GESTURENOTIFY);
#endif /* WINVER >= 0x0601 */
    __def(WM_MENUSELECT);
    __def(WM_MENUCHAR);
    __def(WM_ENTERIDLE);
#if(WINVER >= 0x0500)
#ifndef _WIN32_WCE
    __def(WM_MENURBUTTONUP);
    __def(WM_MENUDRAG);
    __def(WM_MENUGETOBJECT);
    __def(WM_UNINITMENUPOPUP);
    __def(WM_MENUCOMMAND);
#if(_WIN32_WINNT >= 0x0500)
    __def(WM_CHANGEUISTATE);
    __def(WM_UPDATEUISTATE);
    __def(WM_QUERYUISTATE);
#endif /* _WIN32_WINNT >= 0x0500 */
#endif /* _WIN32_WCE */
#endif /* WINVER >= 0x0500 */
    __def(WM_CTLCOLORMSGBOX);
    __def(WM_CTLCOLOREDIT);
    __def(WM_CTLCOLORLISTBOX);
    __def(WM_CTLCOLORBTN);
    __def(WM_CTLCOLORDLG);
    __def(WM_CTLCOLORSCROLLBAR);
    __def(WM_CTLCOLORSTATIC);
    __def(WM_MOUSEFIRST);
    __def(WM_MOUSEMOVE);
    __def(WM_LBUTTONDOWN);
    __def(WM_LBUTTONUP);
    __def(WM_LBUTTONDBLCLK);
    __def(WM_RBUTTONDOWN);
    __def(WM_RBUTTONUP);
    __def(WM_RBUTTONDBLCLK);
    __def(WM_MBUTTONDOWN);
    __def(WM_MBUTTONUP);
    __def(WM_MBUTTONDBLCLK);
#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
    __def(WM_MOUSEWHEEL);
#endif
#if (_WIN32_WINNT >= 0x0500)
    __def(WM_XBUTTONDOWN);
    __def(WM_XBUTTONUP);
    __def(WM_XBUTTONDBLCLK);
#endif
#if (_WIN32_WINNT >= 0x0600)
    __def(WM_MOUSEHWHEEL);
#endif

#if (_WIN32_WINNT >= 0x0600)
    __def(WM_MOUSELAST);
#elif (_WIN32_WINNT >= 0x0500)
    __def(WM_MOUSELAST);
#elif (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
    __def(WM_MOUSELAST);
#else
    __def(WM_MOUSELAST);
#endif /* (_WIN32_WINNT >= 0x0600) */

    __def(WM_PARENTNOTIFY);
    __def(WM_ENTERMENULOOP);
    __def(WM_EXITMENULOOP);
#if(WINVER >= 0x0400)
    __def(WM_NEXTMENU);
    __def(WM_SIZING);
    __def(WM_CAPTURECHANGED);
    __def(WM_MOVING);
    __def(WM_POWERBROADCAST);
    __def(WM_DEVICECHANGE);
#endif /* WINVER >= 0x0400 */
    __def(WM_MDICREATE);
    __def(WM_MDIDESTROY);
    __def(WM_MDIACTIVATE);
    __def(WM_MDIRESTORE);
    __def(WM_MDINEXT);
    __def(WM_MDIMAXIMIZE);
    __def(WM_MDITILE);
    __def(WM_MDICASCADE);
    __def(WM_MDIICONARRANGE);
    __def(WM_MDIGETACTIVE);
    __def(WM_MDISETMENU);
    __def(WM_ENTERSIZEMOVE);
    __def(WM_EXITSIZEMOVE);
    __def(WM_DROPFILES);
    __def(WM_MDIREFRESHMENU);
#if(WINVER >= 0x0602)
    __def(WM_POINTERDEVICECHANGE);
    __def(WM_POINTERDEVICEINRANGE);
    __def(WM_POINTERDEVICEOUTOFRANGE);
#endif /* WINVER >= 0x0602 */
#if(WINVER >= 0x0601)
    __def(WM_TOUCH);
#endif /* WINVER >= 0x0601 */
#if(WINVER >= 0x0602)
    __def(WM_NCPOINTERUPDATE);
    __def(WM_NCPOINTERDOWN);
    __def(WM_NCPOINTERUP);
    __def(WM_POINTERUPDATE);
    __def(WM_POINTERDOWN);
    __def(WM_POINTERUP);
    __def(WM_POINTERENTER);
    __def(WM_POINTERLEAVE);
    __def(WM_POINTERACTIVATE);
    __def(WM_POINTERCAPTURECHANGED);
    __def(WM_TOUCHHITTESTING);
    __def(WM_POINTERWHEEL);
    __def(WM_POINTERHWHEEL);
    __def(DM_POINTERHITTEST); /* DM? */
    __def(WM_POINTERROUTEDTO);
    __def(WM_POINTERROUTEDAWAY);
    __def(WM_POINTERROUTEDRELEASED);
#endif /* WINVER >= 0x0602 */
#if(WINVER >= 0x0400)
    __def(WM_IME_SETCONTEXT);
    __def(WM_IME_NOTIFY);
    __def(WM_IME_CONTROL);
    __def(WM_IME_COMPOSITIONFULL);
    __def(WM_IME_SELECT);
    __def(WM_IME_CHAR);
#endif /* WINVER >= 0x0400 */
#if(WINVER >= 0x0500)
    __def(WM_IME_REQUEST);
#endif /* WINVER >= 0x0500 */
#if(WINVER >= 0x0400)
    __def(WM_IME_KEYDOWN);
    __def(WM_IME_KEYUP);
#endif /* WINVER >= 0x0400 */
#if((_WIN32_WINNT >= 0x0400) || (WINVER >= 0x0500))
    __def(WM_MOUSEHOVER);
    __def(WM_MOUSELEAVE);
#endif
#if(WINVER >= 0x0500)
    __def(WM_NCMOUSEHOVER);
    __def(WM_NCMOUSELEAVE);
#endif /* WINVER >= 0x0500 */
#if(_WIN32_WINNT >= 0x0501)
    __def(WM_WTSSESSION_CHANGE);
    __def(WM_TABLET_FIRST);
    __def(WM_TABLET_LAST);
#endif /* _WIN32_WINNT >= 0x0501 */
#if(WINVER >= 0x0601)
    __def(WM_DPICHANGED);
#endif /* WINVER >= 0x0601 */
#if(WINVER >= 0x0605)
    __def(WM_DPICHANGED_BEFOREPARENT);
    __def(WM_DPICHANGED_AFTERPARENT);
    __def(WM_GETDPISCALEDSIZE);
#endif /* WINVER >= 0x0605 */
    __def(WM_CUT);
    __def(WM_COPY);
    __def(WM_PASTE);
    __def(WM_CLEAR);
    __def(WM_UNDO);
    __def(WM_RENDERFORMAT);
    __def(WM_RENDERALLFORMATS);
    __def(WM_DESTROYCLIPBOARD);
    __def(WM_DRAWCLIPBOARD);
    __def(WM_PAINTCLIPBOARD);
    __def(WM_VSCROLLCLIPBOARD);
    __def(WM_SIZECLIPBOARD);
    __def(WM_ASKCBFORMATNAME);
    __def(WM_CHANGECBCHAIN);
    __def(WM_HSCROLLCLIPBOARD);
    __def(WM_QUERYNEWPALETTE);
    __def(WM_PALETTEISCHANGING);
    __def(WM_PALETTECHANGED);
    __def(WM_HOTKEY);
#if(WINVER >= 0x0400)
    __def(WM_PRINT);
    __def(WM_PRINTCLIENT);
#endif /* WINVER >= 0x0400 */
#if(_WIN32_WINNT >= 0x0500)
    __def(WM_APPCOMMAND);
#endif /* _WIN32_WINNT >= 0x0500 */
#if(_WIN32_WINNT >= 0x0501)
    __def(WM_THEMECHANGED);
#endif /* _WIN32_WINNT >= 0x0501 */
#if(_WIN32_WINNT >= 0x0501)
    __def(WM_CLIPBOARDUPDATE);
#endif /* _WIN32_WINNT >= 0x0501 */
#if(_WIN32_WINNT >= 0x0600)
    __def(WM_DWMCOMPOSITIONCHANGED);
    __def(WM_DWMNCRENDERINGCHANGED);
    __def(WM_DWMCOLORIZATIONCOLORCHANGED);
    __def(WM_DWMWINDOWMAXIMIZEDCHANGE);
#endif /* _WIN32_WINNT >= 0x0600 */
#if(_WIN32_WINNT >= 0x0601)
    __def(WM_DWMSENDICONICTHUMBNAIL);
    __def(WM_DWMSENDICONICLIVEPREVIEWBITMAP);
#endif /* _WIN32_WINNT >= 0x0601 */
#if(WINVER >= 0x0600)
    __def(WM_GETTITLEBARINFOEX);
#endif /* WINVER >= 0x0600 */
#if(WINVER >= 0x0400)
    __def(WM_HANDHELDFIRST);
    __def(WM_HANDHELDLAST);
    __def(WM_AFXFIRST);
    __def(WM_AFXLAST);
#endif /* WINVER >= 0x0400 */
    __def(WM_PENWINFIRST);
    __def(WM_PENWINLAST);
#if(WINVER >= 0x0400)
    __def(WM_APP);
#endif /* WINVER >= 0x0400 */
    __def(WM_USER);

#undef __def
}

static void define_consts_mb(JanetTable *env)
{
#define __def(const_name)                                   \
    janet_def(env, #const_name, jw32_wrap_long(const_name), \
              "Constant for MessageBox().")
    /* buttons */
    __def(MB_ABORTRETRYIGNORE);
    __def(MB_CANCELTRYCONTINUE);
    __def(MB_HELP);
    __def(MB_OK);
    __def(MB_OKCANCEL);
    __def(MB_RETRYCANCEL);
    __def(MB_YESNO);
    __def(MB_YESNOCANCEL);
    /* icons */
    __def(MB_ICONEXCLAMATION);
    __def(MB_ICONWARNING);
    __def(MB_ICONINFORMATION);
    __def(MB_ICONASTERISK);
    __def(MB_ICONQUESTION);
    __def(MB_ICONSTOP);
    __def(MB_ICONERROR);
    __def(MB_ICONHAND);
    /* default button */
    __def(MB_DEFBUTTON1);
    __def(MB_DEFBUTTON2);
    __def(MB_DEFBUTTON3);
    __def(MB_DEFBUTTON4);
    /* modality */
    __def(MB_APPLMODAL);
    __def(MB_SYSTEMMODAL);
    __def(MB_TASKMODAL);
    /* other options */
    __def(MB_DEFAULT_DESKTOP_ONLY);
    __def(MB_RIGHT);
    __def(MB_RTLREADING);
    __def(MB_SETFOREGROUND);
    __def(MB_TOPMOST);
    __def(MB_SERVICE_NOTIFICATION);
#undef __def
}

static void define_consts_button_id(JanetTable *env)
{
#define __def(const_name)                                  \
    janet_def(env, #const_name, jw32_wrap_int(const_name), \
              "Constant for MessageBox()'s return value.")
    __def(IDABORT);
    __def(IDCANCEL);
    __def(IDCONTINUE);
    __def(IDIGNORE);
    __def(IDNO);
    __def(IDOK);
    __def(IDRETRY);
    __def(IDTRYAGAIN);
    __def(IDYES);
#undef __def
}

static void define_consts_idi(JanetTable *env)
{
#define __def(const_name)                                        \
    janet_def(env, #const_name, jw32_wrap_ulong_ptr(const_name), \
              "Constant for system icon ID.")
    __def(IDI_APPLICATION);
    __def(IDI_ERROR);
    __def(IDI_QUESTION);
    __def(IDI_WARNING);
    __def(IDI_INFORMATION);
    __def(IDI_WINLOGO);
    __def(IDI_SHIELD);
#undef __def
}

static void define_consts_idc(JanetTable *env)
{
#define __def(const_name)                                        \
    janet_def(env, #const_name, jw32_wrap_ulong_ptr(const_name), \
              "Constant for system cursor ID.")
    __def(IDC_ARROW);
    __def(IDC_IBEAM);
    __def(IDC_WAIT);
    __def(IDC_CROSS);
    __def(IDC_UPARROW);
    /* XXX: the handwriting cursor (which looks like a pen) has no constant defined */
    __def(IDC_SIZENWSE);
    __def(IDC_SIZENESW);
    __def(IDC_SIZEWE);
    __def(IDC_SIZENS);
    __def(IDC_SIZEALL);
    __def(IDC_NO);
    __def(IDC_HAND);
    __def(IDC_APPSTARTING);
    __def(IDC_HELP);
    __def(IDC_PIN);
    __def(IDC_PERSON);
#undef __def
}

static void define_consts_color(JanetTable *env)
{
#define __def(const_name) \
    janet_def(env, #const_name, jw32_wrap_ulong_ptr(const_name), \
              "Constant for system cursor ID.")

#ifndef NOCOLOR

    __def(COLOR_SCROLLBAR);
    __def(COLOR_BACKGROUND);
    __def(COLOR_ACTIVECAPTION);
    __def(COLOR_INACTIVECAPTION);
    __def(COLOR_MENU);
    __def(COLOR_WINDOW);
    __def(COLOR_WINDOWFRAME);
    __def(COLOR_MENUTEXT);
    __def(COLOR_WINDOWTEXT);
    __def(COLOR_CAPTIONTEXT);
    __def(COLOR_ACTIVEBORDER);
    __def(COLOR_INACTIVEBORDER);
    __def(COLOR_APPWORKSPACE);
    __def(COLOR_HIGHLIGHT);
    __def(COLOR_HIGHLIGHTTEXT);
    __def(COLOR_BTNFACE);
    __def(COLOR_BTNSHADOW);
    __def(COLOR_GRAYTEXT);
    __def(COLOR_BTNTEXT);
    __def(COLOR_INACTIVECAPTIONTEXT);
    __def(COLOR_BTNHIGHLIGHT);
#if(WINVER >= 0x0400)
    __def(COLOR_3DDKSHADOW);
    __def(COLOR_3DLIGHT);
    __def(COLOR_INFOTEXT);
    __def(COLOR_INFOBK);
#endif /* WINVER >= 0x0400 */
#if(WINVER >= 0x0500)
    __def(COLOR_HOTLIGHT);
    __def(COLOR_GRADIENTACTIVECAPTION);
    __def(COLOR_GRADIENTINACTIVECAPTION);
#if(WINVER >= 0x0501)
    __def(COLOR_MENUHILIGHT);
    __def(COLOR_MENUBAR);
#endif /* WINVER >= 0x0501 */
#endif /* WINVER >= 0x0500 */
#if(WINVER >= 0x0400)
    __def(COLOR_DESKTOP);
    __def(COLOR_3DFACE);
    __def(COLOR_3DSHADOW);
    __def(COLOR_3DHIGHLIGHT);
    __def(COLOR_3DHILIGHT);
    __def(COLOR_BTNHILIGHT);
#endif /* WINVER >= 0x0400 */

#endif /* !NOCOLOR */

#undef __def
}

static inline int call_fn(JanetFunction *fn, int argc, const Janet *argv, Janet *out) {
  JanetFiber *fiber = NULL;
  int ret, lock;

  /* XXX: if i call any function (cfuns or janet functions) inside fn,
     there would be memory violations without this lock, i don't know why */
  lock = janet_gclock();
  if (janet_pcall(fn, argc, argv, out, &fiber) == JANET_SIGNAL_OK) {
      ret = 1;
  } else {
      janet_stacktrace(fiber, *out);
      ret = 0;
  }
  janet_gcunlock(lock);
  return ret;
}

static inline void remove_array_entry(JanetArray *array, int i) {
    memmove(array->data + i,
            array->data + i + 1,
            (array->count - i - 1) * sizeof(Janet));
    array->count -= 1;
}

static inline int search_local_class_wnd_proc(const uint8_t *class_name, HINSTANCE hInstance)
{
    for (int i = local_class_wnd_proc_registry->count - 1; i >= 0; i--) {
        Janet entry = local_class_wnd_proc_registry->data[i];
        const Janet *entry_tuple = janet_unwrap_tuple(entry); /* (class_name h_instance wnd_proc) */
        const uint8_t *entry_class_name = janet_unwrap_keyword(entry_tuple[0]);
        HINSTANCE ehInstance = jw32_unwrap_handle(entry_tuple[1]);
        if ((ehInstance == hInstance || hInstance == NULL)
            && !strcmp(entry_class_name, class_name)) {
            return i;
        }
    }
    return -1;
}

static inline int search_global_class_wnd_proc(const uint8_t *class_name)
{
    for (int i = global_class_wnd_proc_registry->count - 1; i >= 0; i--) {
        Janet entry = global_class_wnd_proc_registry->data[i];
        const Janet *entry_tuple = janet_unwrap_tuple(entry); /* (class_name wnd_proc) */
        const uint8_t *entry_class_name = janet_unwrap_keyword(entry_tuple[0]);
        if (!strcmp(entry_class_name, class_name)) {
            return i;
        }
    }
    return -1;
}

static Janet normalize_wnd_class_name(LPCSTR lpClassName)
{
    uint64_t maybe_atom = (uint64_t)lpClassName;
    if (maybe_atom & ~(uint64_t)0xffff) {
        /* higher bits are not zero, we have a string pointer */
        return jw32_cstr_to_keyword(lpClassName);
    } else {
        /* looks like an ATOM */
        ATOM atmClass = (ATOM)(maybe_atom & 0xffff);
#define __atom_name_buf_size 256 /* XXX: should be enough? */
        char buffer[__atom_name_buf_size];
        UINT uRet = GetAtomName(atmClass, buffer, __atom_name_buf_size);
        if (uRet) {
            return jw32_cstr_to_keyword(buffer);
        } else {
            return janet_wrap_nil();
        }
#undef __atom_name_buf_size
    }
}


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
            if (!janet_checkint(idx.items[0]) || !janet_checkint(idx.items[1])) {
                janet_panicf("expected 32 bit signed integers for pt, got [%v %v]",
                             idx.items[0], idx.items[1]);
            }
            msg->pt.x = janet_unwrap_integer(idx.items[0]);
            msg->pt.y = janet_unwrap_integer(idx.items[1]);
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

static Janet cfun_PostQuitMessage(int32_t argc, Janet *argv)
{
    int nExitCode;

    janet_fixarity(argc, 1);
    nExitCode = jw32_get_int(argv, 0);
    PostQuitMessage(nExitCode);
    return janet_wrap_nil();
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

static void register_class_wnd_proc(jw32_wc_t *jwc)
{
    Janet wnd_proc = janet_wrap_function(jwc->wnd_proc);
    Janet class_name = jw32_cstr_to_keyword(jwc->wc.lpszClassName);
    Janet reg_entry_tuple[3];
    Janet reg_entry;

    reg_entry_tuple[0] = class_name;

    /* an application can register local or global classes, and they
       are looked up in different ways */
    if (jwc->wc.style & CS_GLOBALCLASS) {
        reg_entry_tuple[1] = wnd_proc;
        /* (class_name wnd_proc) */
        reg_entry = janet_wrap_tuple(janet_tuple_n(reg_entry_tuple, 2));
        janet_array_push(global_class_wnd_proc_registry, reg_entry);
    } else {
        /* (class_name h_instance wnd_proc) */
        Janet h_instance = jw32_wrap_handle(jwc->wc.hInstance);
        reg_entry_tuple[1] = h_instance;
        reg_entry_tuple[2] = wnd_proc;;
        reg_entry = janet_wrap_tuple(janet_tuple_n(reg_entry_tuple, 3));
        janet_array_push(local_class_wnd_proc_registry, reg_entry);
    }
}

static void unregister_class_wnd_proc(LPCSTR lpClassName, HINSTANCE hInstance)
{
    uint64_t maybe_atom = (uint64_t)lpClassName;
    Janet local_key, global_key;
    Janet local_key_tuple[2];
    Janet class_name = normalize_wnd_class_name(lpClassName);

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

       only unregister ONE class at a time.

       XXX: these info is obtained by observation though, i couldn't find a precise description
       in the win32 docs. i wouldn't want class name clashes in my program.... */

    const uint8_t *class_name_str = janet_unwrap_keyword(class_name);
    int found = search_local_class_wnd_proc(class_name_str, hInstance);
    if (found >= 0) {
        remove_array_entry(local_class_wnd_proc_registry, found);
        return;
    }
    found = search_global_class_wnd_proc(class_name_str);
    if (found >= 0) {
        remove_array_entry(global_class_wnd_proc_registry, found);
        return;
    }
}

LRESULT jw32_wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    printf("\n---- jw32_wnd_proc ----\n");
    printf("hWnd = 0x%" PRIx64 "\n", (uint64_t)hWnd);
    printf("uMsg = 0x%" PRIx32 "\n", uMsg);
    printf("wParam = 0x%" PRIx64 "\n", wParam);
    printf("lParam = %lld\n", lParam);

    switch (uMsg) {
    case WM_NCCREATE: {
        CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
        const Janet *param_tuple = (Janet *)cs->lpCreateParams;
        LPVOID lpRealParam = jw32_unwrap_lpvoid(param_tuple[1]);
        Janet argv[4] = {
            jw32_wrap_handle(hWnd),
            jw32_wrap_uint(uMsg),
            jw32_wrap_wparam(wParam),
            jw32_wrap_lparam(lParam),
        };
        Janet ret;

        JanetFunction *wnd_proc_fn = (JanetFunction *)GetClassLongPtr(hWnd, 0);

        /* there will be race conditions if the windows of a same class run
           on multiple threads, but in that case the class memory slot would
           just be set more than once, to the same value. a minor issue. */

        if (!wnd_proc_fn) {
            Janet wnd_proc = param_tuple[0];
            wnd_proc_fn = janet_unwrap_function(wnd_proc);

            janet_printf("wnd_proc (from lpCreateParams) = %v\n", wnd_proc);

            /* TODO: some special handling in SetClassLongPtr() binding? */
            SetClassLongPtr(hWnd, 0, (LONG_PTR)wnd_proc_fn);
        }

        cs->lpCreateParams = lpRealParam; /* XXX: can i really do this? */
        if (call_fn(wnd_proc_fn, 4, argv, &ret)) {
            return jw32_unwrap_lresult(ret);
        } else {
            /* XXX: error handling? */
            return FALSE;
        }
    }

    default: {
        JanetFunction *wnd_proc_fn = (JanetFunction *)GetClassLongPtr(hWnd, 0);
        printf("xxxxxxxxxx wnd_proc_fn = 0x%" PRIx64 "\n", (uint64_t)wnd_proc_fn);
        if (wnd_proc_fn) {
            Janet argv[4] = {
                jw32_wrap_handle(hWnd),
                jw32_wrap_uint(uMsg),
                jw32_wrap_wparam(wParam),
                jw32_wrap_lparam(lParam),
            };
            Janet ret;

            printf("xxxxxxxxxx before call_fn xxxxxxxxxx\n");
            if (call_fn(wnd_proc_fn, 4, argv, &ret)) {
                printf("xxxxxxxxxx before return xxxxxxxxxx\n");
                return jw32_unwrap_lresult(ret);
            } else {
                /* XXX: error handling? */
                printf("xxxxxxxxxx before return: ERROR xxxxxxxxxx\n");
                return FALSE;
            }

        } else {
            /* it's before WM_NCCREATE, carry on the window creation */
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }
    }
}


/*******************************************************************
 *
 * WINDOW-RELATED
 *
 *******************************************************************/

static Janet cfun_MessageBox(int32_t argc, Janet *argv)
{
    HWND hWnd;
    LPCSTR lpText;
    LPCSTR lpCaption;
    UINT uType;

    int iRet;

    janet_fixarity(argc, 4);

    hWnd = jw32_get_handle(argv, 0);
    lpText = jw32_get_lpcstr(argv, 1);
    lpCaption = jw32_get_lpcstr(argv, 2);
    uType = jw32_get_uint(argv, 3);

    iRet = MessageBox(hWnd, lpText, lpCaption, uType);
    return jw32_wrap_int(iRet);
}

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

    Janet class_name;

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

    class_name = normalize_wnd_class_name(lpClassName);
    if (!janet_checktype(class_name, JANET_NIL)) {
        const uint8_t *class_name_str = janet_unwrap_keyword(class_name);
        int found = -1;
        Janet wnd_proc = janet_wrap_nil();

        if (hInstance) {
            found = search_local_class_wnd_proc(class_name_str, hInstance);
            if (found >= 0) {
                wnd_proc = (janet_unwrap_tuple(local_class_wnd_proc_registry->data[found]))[2];
            }
        }

        if (!hInstance || found < 0) {
            found = search_global_class_wnd_proc(class_name_str);
            if (found >= 0) {
                wnd_proc = (janet_unwrap_tuple(global_class_wnd_proc_registry->data[found]))[1];
            }
        }

        if (janet_checktype(wnd_proc, JANET_FUNCTION)) {
            /* it's a class that calls jw32_wnd_proc, prepare extra goodies for it */
            Janet param_tuple[2];
            janet_printf("wnd_proc (from registry) = %v\n", wnd_proc);
            lpParam = jw32_get_lpvoid(argv, 10);
            param_tuple[0] = wnd_proc;
            param_tuple[1] = jw32_wrap_lpvoid(lpParam);
            lpParam = (LPVOID)janet_tuple_n(param_tuple, 2);
        } else {
            lpParam = jw32_get_lpvoid(argv, 10);
        }
    }
    
    hWnd = CreateWindow(lpClassName, lpWindowName, dwStyle,
                        x, y, nWidth, nHeight,
                        hWndParent, hMenu, hInstance,
                        lpParam);

    return jw32_wrap_handle(hWnd);
}

static Janet cfun_DestroyWindow(int32_t argc, Janet *argv)
{
    HWND hWnd;
    BOOL bRet;

    janet_fixarity(argc, 1);
    hWnd = jw32_get_handle(argv, 0);
    bRet = DestroyWindow(hWnd);
    return jw32_wrap_bool(bRet);
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

    printf("!!!!!!!!!! WNDCLASSEX_gcmark !!!!!!!!!!\n");

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

    printf("!!!!!!!!!! WNDCLASSEX_gcmark: done !!!!!!!!!!\n");

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

    /* extra space for our janet wndproc */
    jwc->wc.cbClsExtra += sizeof(JanetFunction *); 

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
        register_class_wnd_proc(jwc);
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


/*******************************************************************
 *
 * RESOURCES
 *
 *******************************************************************/

static Janet cfun_LoadIcon(int32_t argc, Janet *argv)
{
    HINSTANCE hInstance;
    LPCSTR lpIconName;

    HICON hRet;

    janet_fixarity(argc, 2);

    hInstance = jw32_get_handle(argv, 0);
    lpIconName = jw32_get_lpcstr(argv, 1);

    hRet = LoadIcon(hInstance, lpIconName);
    return jw32_wrap_handle(hRet);
}

static Janet cfun_LoadCursor(int32_t argc, Janet *argv)
{
    HINSTANCE hInstance;
    LPCSTR lpCursorName;

    HCURSOR hRet;

    janet_fixarity(argc, 2);

    hInstance = jw32_get_handle(argv, 0);
    lpCursorName = jw32_get_lpcstr(argv, 1);

    hRet = LoadCursor(hInstance, lpCursorName);
    return jw32_wrap_handle(hRet);
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
        "PostQuitMessage",
        cfun_PostQuitMessage,
        "(" MOD_NAME "/PostQuitMessage nExitCode)\n\n"
        "Tells the current thread to quit.",
    },
    {
        "PostThreadMessage",
        cfun_PostThreadMessage,
        "(" MOD_NAME "/PostThreadMessage idThread uMsg wParam lParam)\n\n"
        "Returns non-zero if succeeded, zero otherwise.",
    },

    /*********************** WINDOW-RELATED ************************/
    {
        "MessageBox",
        cfun_MessageBox,
        "(" MOD_NAME "/MessageBox hWnd lpText lpCaption uType)\n\n"
        "Shows a message box.",
    },
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
        "DestroyWindow",
        cfun_DestroyWindow,
        "(" MOD_NAME "/DestroyWindow hWnd)\n\n"
        "Destroys a window.",
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

    /************************** RESOURCES **************************/
    {
        "LoadIcon",
        cfun_LoadIcon,
        "(" MOD_NAME "/LoadIcon hInstance lpIconName)\n\n"
        "Loads an icon.",
    },
    {
        "LoadCursor",
        cfun_LoadCursor,
        "(" MOD_NAME "/LoadCursor hInstance lpCursorName)\n\n"
        "Loads an cursor.",
    },

    {NULL, NULL, NULL},
};


static void init_global_states(JanetTable *env)
{
    local_class_wnd_proc_registry = janet_array(0);
    global_class_wnd_proc_registry = janet_array(0);

    janet_def(env, "local_class_wnd_proc_registry", janet_wrap_array(local_class_wnd_proc_registry),
              "Where all the local WndProcs reside.");
    janet_def(env, "global_class_wnd_proc_registry", janet_wrap_array(global_class_wnd_proc_registry),
              "Where all the global WndProcs reside.");
}


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_consts_wm(env);
    define_consts_mb(env);
    define_consts_button_id(env);
    define_consts_idi(env);
    define_consts_idc(env);
    define_consts_color(env);

    janet_register_abstract_type(&jw32_at_MSG);

    janet_cfuns(env, MOD_NAME, cfuns);

    init_global_states(env);
}
