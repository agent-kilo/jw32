#include "jw32.h"
#include "debug.h"

#include <shellscalingapi.h>

#define MOD_NAME "winuser"


/* extra space for our janet objects, to deal with gc and stuff */
typedef struct {
    JanetFunction *wnd_proc;
    JanetString menu_name;
    JanetString class_name;
    WNDCLASSEX wc;
} jw32_wc_t;

typedef struct {
    DWORD ri_size;
    RAWINPUT ri;
} jw32_rawinput_t;

#define JW32_RAWINPUT_T_HEADER_SIZE (sizeof(((jw32_rawinput_t *)NULL)->ri_size))


JANET_THREAD_LOCAL JanetArray *local_class_wnd_proc_registry;
JANET_THREAD_LOCAL JanetArray *global_class_wnd_proc_registry;
JANET_THREAD_LOCAL JanetTable *atom_class_name_map;

JANET_THREAD_LOCAL JanetTable *win_event_hook_registry;
JANET_THREAD_LOCAL JanetTable *win_hook_registry;
JANET_THREAD_LOCAL JanetTable *hhook_type_map;


/* pre-defined window properties we used */
#define JW32_WND_PROC_FN_PROP_NAME "jw32_wnd_proc_fn"
#define JW32_DLG_PROC_FN_PROP_NAME "jw32_dlg_proc_fn"
#define JW32_MAX_PROP_LEN (sizeof(JW32_WND_PROC_FN_PROP_NAME)) /* includes the trailing null byte */


/*******************************************************************
 *
 * CONSTANT DEFINITIONS
 *
 *******************************************************************/

static void define_consts_wm(JanetTable *env)
{
#define __def(const_name)                                  \
    janet_def(env, #const_name, jw32_wrap_uint(const_name), \
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

static void define_consts_pm(JanetTable *env)
{
#define __def(const_name)                                  \
    janet_def(env, #const_name, jw32_wrap_uint(const_name), \
              "Constant for PeekMessage behavior.")

    __def(PM_NOREMOVE);
    __def(PM_REMOVE);
    __def(PM_NOYIELD);
#if(WINVER >= 0x0500)
    __def(PM_QS_INPUT);
    __def(PM_QS_POSTMESSAGE);
    __def(PM_QS_PAINT);
    __def(PM_QS_SENDMESSAGE);
#endif /* WINVER >= 0x0500 */

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

static void define_consts_cs(JanetTable *env)
{
#define __def(const_name)                                               \
    janet_def(env, #const_name, jw32_wrap_uint(const_name),             \
              "Constant for class styles.")

#ifndef NOWINSTYLES
    __def(CS_VREDRAW);
    __def(CS_HREDRAW);
    __def(CS_DBLCLKS);
    __def(CS_OWNDC);
    __def(CS_CLASSDC);
    __def(CS_PARENTDC);
    __def(CS_NOCLOSE);
    __def(CS_SAVEBITS);
    __def(CS_BYTEALIGNCLIENT);
    __def(CS_BYTEALIGNWINDOW);
    __def(CS_GLOBALCLASS);
    __def(CS_IME);
#if(_WIN32_WINNT >= 0x0501)
    __def(CS_DROPSHADOW);
#endif /* _WIN32_WINNT >= 0x0501 */
#endif /* !NOWINSTYLES */

#undef __def
}

static void define_consts_ws(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),    \
              "Constant for window styles.")

#ifndef NOWINSTYLES
    __def(WS_OVERLAPPED);
    __def(WS_POPUP);
    __def(WS_CHILD);
    __def(WS_MINIMIZE);
    __def(WS_VISIBLE);
    __def(WS_DISABLED);
    __def(WS_CLIPSIBLINGS);
    __def(WS_CLIPCHILDREN);
    __def(WS_MAXIMIZE);
    __def(WS_CAPTION);     /* WS_BORDER | WS_DLGFRAME  */
    __def(WS_BORDER);
    __def(WS_DLGFRAME);
    __def(WS_VSCROLL);
    __def(WS_HSCROLL);
    __def(WS_SYSMENU);
    __def(WS_THICKFRAME);
    __def(WS_GROUP);
    __def(WS_TABSTOP);
    __def(WS_MINIMIZEBOX);
    __def(WS_MAXIMIZEBOX);
    __def(WS_TILED);
    __def(WS_ICONIC);
    __def(WS_SIZEBOX);
    __def(WS_TILEDWINDOW);
    __def(WS_OVERLAPPEDWINDOW);
    __def(WS_POPUPWINDOW);
    __def(WS_CHILDWINDOW);
#endif /* !NOWINSTYLES */

#undef __def
}

static void define_consts_ws_ex(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),    \
              "Constant for extended window styles.")

#ifndef NOWINSTYLES

    __def(WS_EX_DLGMODALFRAME);
    __def(WS_EX_NOPARENTNOTIFY);
    __def(WS_EX_TOPMOST);
    __def(WS_EX_ACCEPTFILES);
    __def(WS_EX_TRANSPARENT);
#if(WINVER >= 0x0400)
    __def(WS_EX_MDICHILD);
    __def(WS_EX_TOOLWINDOW);
    __def(WS_EX_WINDOWEDGE);
    __def(WS_EX_CLIENTEDGE);
    __def(WS_EX_CONTEXTHELP);
#endif /* WINVER >= 0x0400 */
#if(WINVER >= 0x0400)
    __def(WS_EX_RIGHT);
    __def(WS_EX_LEFT);
    __def(WS_EX_RTLREADING);
    __def(WS_EX_LTRREADING);
    __def(WS_EX_LEFTSCROLLBAR);
    __def(WS_EX_RIGHTSCROLLBAR);
    __def(WS_EX_CONTROLPARENT);
    __def(WS_EX_STATICEDGE);
    __def(WS_EX_APPWINDOW);
    __def(WS_EX_OVERLAPPEDWINDOW);
    __def(WS_EX_PALETTEWINDOW);
#endif /* WINVER >= 0x0400 */
#if(_WIN32_WINNT >= 0x0500)
    __def(WS_EX_LAYERED);
#endif /* _WIN32_WINNT >= 0x0500 */
#if(WINVER >= 0x0500)
    __def(WS_EX_NOINHERITLAYOUT);
#endif /* WINVER >= 0x0500 */
#if(WINVER >= 0x0602)
    __def(WS_EX_NOREDIRECTIONBITMAP);
#endif /* WINVER >= 0x0602 */
#if(WINVER >= 0x0500)
    __def(WS_EX_LAYOUTRTL);
#endif /* WINVER >= 0x0500 */
#if(_WIN32_WINNT >= 0x0501)
    __def(WS_EX_COMPOSITED);
#endif /* _WIN32_WINNT >= 0x0501 */
#if(_WIN32_WINNT >= 0x0500)
    __def(WS_EX_NOACTIVATE);
#endif /* _WIN32_WINNT >= 0x0500 */

#endif /* !NOWINSTYLES */

#undef __def
}

static void define_consts_cw(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_int(const_name),      \
              "Constant for window position.")
    __def(CW_USEDEFAULT);
#undef __def
}

static void define_consts_sw(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_int(const_name),      \
              "Constant for ShowWindow() commands.")
    __def(SW_HIDE);
    __def(SW_SHOWNORMAL);
    __def(SW_NORMAL);
    __def(SW_SHOWMINIMIZED);
    __def(SW_SHOWMAXIMIZED);
    __def(SW_MAXIMIZE);
    __def(SW_SHOWNOACTIVATE);
    __def(SW_SHOW);
    __def(SW_MINIMIZE);
    __def(SW_SHOWMINNOACTIVE);
    __def(SW_SHOWNA);
    __def(SW_RESTORE);
    __def(SW_SHOWDEFAULT);
    __def(SW_FORCEMINIMIZE);
    __def(SW_MAX);
#undef __def
}

static void define_consts_image(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_uint(const_name),     \
              "Constant for image resource types.")
    __def(IMAGE_BITMAP);
    __def(IMAGE_CURSOR);
    __def(IMAGE_ICON);
#undef __def
}

static void define_consts_lr(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_uint(const_name),     \
              "Constant for image loading flags.")
    __def(LR_CREATEDIBSECTION);
    __def(LR_DEFAULTCOLOR);
    __def(LR_DEFAULTSIZE);
    __def(LR_LOADFROMFILE);
    __def(LR_LOADMAP3DCOLORS);
    __def(LR_LOADTRANSPARENT);
    __def(LR_MONOCHROME);
    __def(LR_SHARED);
    __def(LR_VGACOLOR);
#undef __def
}

static void define_consts_sm(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_int(const_name),     \
              "Constant for system metrics().")

#ifndef NOSYSMETRICS

    __def(SM_CXSCREEN);
    __def(SM_CYSCREEN);
    __def(SM_CXVSCROLL);
    __def(SM_CYHSCROLL);
    __def(SM_CYCAPTION);
    __def(SM_CXBORDER);
    __def(SM_CYBORDER);
    __def(SM_CXDLGFRAME);
    __def(SM_CYDLGFRAME);
    __def(SM_CYVTHUMB);
    __def(SM_CXHTHUMB);
    __def(SM_CXICON);
    __def(SM_CYICON);
    __def(SM_CXCURSOR);
    __def(SM_CYCURSOR);
    __def(SM_CYMENU);
    __def(SM_CXFULLSCREEN);
    __def(SM_CYFULLSCREEN);
    __def(SM_CYKANJIWINDOW);
    __def(SM_MOUSEPRESENT);
    __def(SM_CYVSCROLL);
    __def(SM_CXHSCROLL);
    __def(SM_DEBUG);
    __def(SM_SWAPBUTTON);
    __def(SM_RESERVED1);
    __def(SM_RESERVED2);
    __def(SM_RESERVED3);
    __def(SM_RESERVED4);
    __def(SM_CXMIN);
    __def(SM_CYMIN);
    __def(SM_CXSIZE);
    __def(SM_CYSIZE);
    __def(SM_CXFRAME);
    __def(SM_CYFRAME);
    __def(SM_CXMINTRACK);
    __def(SM_CYMINTRACK);
    __def(SM_CXDOUBLECLK);
    __def(SM_CYDOUBLECLK);
    __def(SM_CXICONSPACING);
    __def(SM_CYICONSPACING);
    __def(SM_MENUDROPALIGNMENT);
    __def(SM_PENWINDOWS);
    __def(SM_DBCSENABLED);
    __def(SM_CMOUSEBUTTONS);
#if(WINVER >= 0x0400)
    __def(SM_CXFIXEDFRAME);
    __def(SM_CYFIXEDFRAME);
    __def(SM_CXSIZEFRAME);
    __def(SM_CYSIZEFRAME);
    __def(SM_SECURE);
    __def(SM_CXEDGE);
    __def(SM_CYEDGE);
    __def(SM_CXMINSPACING);
    __def(SM_CYMINSPACING);
    __def(SM_CXSMICON);
    __def(SM_CYSMICON);
    __def(SM_CYSMCAPTION);
    __def(SM_CXSMSIZE);
    __def(SM_CYSMSIZE);
    __def(SM_CXMENUSIZE);
    __def(SM_CYMENUSIZE);
    __def(SM_ARRANGE);
    __def(SM_CXMINIMIZED);
    __def(SM_CYMINIMIZED);
    __def(SM_CXMAXTRACK);
    __def(SM_CYMAXTRACK);
    __def(SM_CXMAXIMIZED);
    __def(SM_CYMAXIMIZED);
    __def(SM_NETWORK);
    __def(SM_CLEANBOOT);
    __def(SM_CXDRAG);
    __def(SM_CYDRAG);
#endif /* WINVER >= 0x0400 */
    __def(SM_SHOWSOUNDS);
#if(WINVER >= 0x0400)
    __def(SM_CXMENUCHECK);
    __def(SM_CYMENUCHECK);
    __def(SM_SLOWMACHINE);
    __def(SM_MIDEASTENABLED);
#endif /* WINVER >= 0x0400 */
#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
    __def(SM_MOUSEWHEELPRESENT);
#endif
#if(WINVER >= 0x0500)
    __def(SM_XVIRTUALSCREEN);
    __def(SM_YVIRTUALSCREEN);
    __def(SM_CXVIRTUALSCREEN);
    __def(SM_CYVIRTUALSCREEN);
    __def(SM_CMONITORS);
    __def(SM_SAMEDISPLAYFORMAT);
#endif /* WINVER >= 0x0500 */
#if(_WIN32_WINNT >= 0x0500)
    __def(SM_IMMENABLED);
#endif /* _WIN32_WINNT >= 0x0500 */
#if(_WIN32_WINNT >= 0x0501)
    __def(SM_CXFOCUSBORDER);
    __def(SM_CYFOCUSBORDER);
#endif /* _WIN32_WINNT >= 0x0501 */
#if(_WIN32_WINNT >= 0x0501)
    __def(SM_TABLETPC);
    __def(SM_MEDIACENTER);
    __def(SM_STARTER);
    __def(SM_SERVERR2);
#endif /* _WIN32_WINNT >= 0x0501 */
#if(_WIN32_WINNT >= 0x0600)
    __def(SM_MOUSEHORIZONTALWHEELPRESENT);
    __def(SM_CXPADDEDBORDER);
#endif /* _WIN32_WINNT >= 0x0600 */
#if(WINVER >= 0x0601)
    __def(SM_DIGITIZER);
    __def(SM_MAXIMUMTOUCHES);
#endif /* WINVER >= 0x0601 */

#if (WINVER < 0x0500) && (!defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0400))
    __def(SM_CMETRICS);
#elif WINVER == 0x500
    __def(SM_CMETRICS);
#elif WINVER == 0x501
    __def(SM_CMETRICS);
#elif WINVER == 0x600
    __def(SM_CMETRICS);
#else
    __def(SM_CMETRICS);
#endif

#if(WINVER >= 0x0500)
    __def(SM_REMOTESESSION);
#if(_WIN32_WINNT >= 0x0501)
    __def(SM_SHUTTINGDOWN);
#endif /* _WIN32_WINNT >= 0x0501 */
#if(WINVER >= 0x0501)
    __def(SM_REMOTECONTROL);
#endif /* WINVER >= 0x0501 */
#if(WINVER >= 0x0501)
    __def(SM_CARETBLINKINGENABLED);
#endif /* WINVER >= 0x0501 */
#if(WINVER >= 0x0602)
    __def(SM_CONVERTIBLESLATEMODE);
    __def(SM_SYSTEMDOCKED);
#endif /* WINVER >= 0x0602 */
#endif /* WINVER >= 0x0500 */

#endif /* !NOSYSMETRICS */

#undef __def
}

static void define_consts_mf(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_uint(const_name),     \
              "Constant for menu item flags.")
    __def(MF_BITMAP);
    __def(MF_CHECKED);
    __def(MF_DISABLED);
    __def(MF_ENABLED);
    __def(MF_GRAYED);
    __def(MF_MENUBARBREAK);
    __def(MF_MENUBREAK);
    __def(MF_OWNERDRAW);
    __def(MF_POPUP);
    __def(MF_SEPARATOR);
    __def(MF_STRING);
    __def(MF_UNCHECKED);
#undef __def
}

static void define_consts_hwnd(JanetTable *env)
{
    /* janet will truncate the hwnd values if we used jw32_wrap_handle() here.
       use jw32_wrap_uint() instead to preserve all the bits */
#define __def(const_name)                                         \
    janet_def(env, #const_name, jw32_wrap_uint(const_name),     \
              "Constant for special HWND handles.")

    __def(HWND_BROADCAST);
#if(WINVER >= 0x0500)
    __def(HWND_MESSAGE);
#endif /* WINVER >= 0x0500 */
    __def(HWND_DESKTOP);
    __def(HWND_TOP);
    __def(HWND_BOTTOM);
    __def(HWND_TOPMOST);
    __def(HWND_NOTOPMOST);

#undef __def
}

static void define_consts_icon(JanetTable *env)
{
#define __def(const_name)                                         \
    janet_def(env, #const_name, jw32_wrap_int(const_name),        \
              "Constant for icon types.")

    __def(ICON_BIG);
    __def(ICON_SMALL);
#if(_WIN32_WINNT >= 0x0501)
    __def(ICON_SMALL2);
#endif /* _WIN32_WINNT >= 0x0501 */

#undef __def
}

static void define_consts_winevent(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),    \
              "Constant for setting WinEvent hooks.")
    __def(WINEVENT_INCONTEXT);
    __def(WINEVENT_OUTOFCONTEXT);
    __def(WINEVENT_SKIPOWNPROCESS);
    __def(WINEVENT_SKIPOWNTHREAD);
#undef __def
}

static void define_consts_event(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),    \
              "Constant for WinEvent types.")

    __def(EVENT_MIN);
    __def(EVENT_MAX);
    __def(EVENT_SYSTEM_SOUND);
    __def(EVENT_SYSTEM_ALERT);
    __def(EVENT_SYSTEM_FOREGROUND);
    __def(EVENT_SYSTEM_MENUSTART);
    __def(EVENT_SYSTEM_MENUEND);
    __def(EVENT_SYSTEM_MENUPOPUPSTART);
    __def(EVENT_SYSTEM_MENUPOPUPEND);
    __def(EVENT_SYSTEM_CAPTURESTART);
    __def(EVENT_SYSTEM_CAPTUREEND);
    __def(EVENT_SYSTEM_MOVESIZESTART);
    __def(EVENT_SYSTEM_MOVESIZEEND);
    __def(EVENT_SYSTEM_CONTEXTHELPSTART);
    __def(EVENT_SYSTEM_CONTEXTHELPEND);
    __def(EVENT_SYSTEM_DRAGDROPSTART);
    __def(EVENT_SYSTEM_DRAGDROPEND);
    __def(EVENT_SYSTEM_DIALOGSTART);
    __def(EVENT_SYSTEM_DIALOGEND);
    __def(EVENT_SYSTEM_SCROLLINGSTART);
    __def(EVENT_SYSTEM_SCROLLINGEND);
    __def(EVENT_SYSTEM_SWITCHSTART);
    __def(EVENT_SYSTEM_SWITCHEND);
    __def(EVENT_SYSTEM_MINIMIZESTART);
    __def(EVENT_SYSTEM_MINIMIZEEND);
#if(_WIN32_WINNT >= 0x0600)
    __def(EVENT_SYSTEM_DESKTOPSWITCH);
#endif /* _WIN32_WINNT >= 0x0600 */
#if(_WIN32_WINNT >= 0x0602)
    __def(EVENT_SYSTEM_SWITCHER_APPGRABBED);
    __def(EVENT_SYSTEM_SWITCHER_APPOVERTARGET);
    __def(EVENT_SYSTEM_SWITCHER_APPDROPPED);
    __def(EVENT_SYSTEM_SWITCHER_CANCELLED);
    __def(EVENT_SYSTEM_IME_KEY_NOTIFICATION);
#endif /* _WIN32_WINNT >= 0x0602 */
#if(_WIN32_WINNT >= 0x0601)
    __def(EVENT_SYSTEM_END);
    __def(EVENT_OEM_DEFINED_START);
    __def(EVENT_OEM_DEFINED_END);
    __def(EVENT_UIA_EVENTID_START);
    __def(EVENT_UIA_EVENTID_END);
    __def(EVENT_UIA_PROPID_START);
    __def(EVENT_UIA_PROPID_END);
#endif /* _WIN32_WINNT >= 0x0601 */
#if(_WIN32_WINNT >= 0x0501)
    __def(EVENT_CONSOLE_CARET);
    __def(EVENT_CONSOLE_UPDATE_REGION);
    __def(EVENT_CONSOLE_UPDATE_SIMPLE);
    __def(EVENT_CONSOLE_UPDATE_SCROLL);
    __def(EVENT_CONSOLE_LAYOUT);
    __def(EVENT_CONSOLE_START_APPLICATION);
    __def(EVENT_CONSOLE_END_APPLICATION);
#endif /* _WIN32_WINNT >= 0x0501 */
#if(_WIN32_WINNT >= 0x0601)
    __def(EVENT_CONSOLE_END);
#endif /* _WIN32_WINNT >= 0x0601 */
    __def(EVENT_OBJECT_CREATE);
    __def(EVENT_OBJECT_DESTROY);
    __def(EVENT_OBJECT_SHOW);
    __def(EVENT_OBJECT_HIDE);
    __def(EVENT_OBJECT_REORDER);
    __def(EVENT_OBJECT_FOCUS);
    __def(EVENT_OBJECT_SELECTION);
    __def(EVENT_OBJECT_SELECTIONADD);
    __def(EVENT_OBJECT_SELECTIONREMOVE);
    __def(EVENT_OBJECT_SELECTIONWITHIN);
    __def(EVENT_OBJECT_STATECHANGE);
    __def(EVENT_OBJECT_LOCATIONCHANGE);
    __def(EVENT_OBJECT_NAMECHANGE);
    __def(EVENT_OBJECT_DESCRIPTIONCHANGE);
    __def(EVENT_OBJECT_VALUECHANGE);
    __def(EVENT_OBJECT_PARENTCHANGE);
    __def(EVENT_OBJECT_HELPCHANGE);
    __def(EVENT_OBJECT_DEFACTIONCHANGE);
    __def(EVENT_OBJECT_ACCELERATORCHANGE);
#if(_WIN32_WINNT >= 0x0600)
    __def(EVENT_OBJECT_INVOKED);
    __def(EVENT_OBJECT_TEXTSELECTIONCHANGED);
    __def(EVENT_OBJECT_CONTENTSCROLLED);
#endif /* _WIN32_WINNT >= 0x0600 */
#if(_WIN32_WINNT >= 0x0601)
    __def(EVENT_SYSTEM_ARRANGMENTPREVIEW);
#endif /* _WIN32_WINNT >= 0x0601 */
#if(_WIN32_WINNT >= 0x0602)
    __def(EVENT_OBJECT_CLOAKED);
    __def(EVENT_OBJECT_UNCLOAKED);
    __def(EVENT_OBJECT_LIVEREGIONCHANGED);
    __def(EVENT_OBJECT_HOSTEDOBJECTSINVALIDATED);
    __def(EVENT_OBJECT_DRAGSTART);
    __def(EVENT_OBJECT_DRAGCANCEL);
    __def(EVENT_OBJECT_DRAGCOMPLETE);

    __def(EVENT_OBJECT_DRAGENTER);
    __def(EVENT_OBJECT_DRAGLEAVE);
    __def(EVENT_OBJECT_DRAGDROPPED);
    __def(EVENT_OBJECT_IME_SHOW);
    __def(EVENT_OBJECT_IME_HIDE);
    __def(EVENT_OBJECT_IME_CHANGE);
    __def(EVENT_OBJECT_TEXTEDIT_CONVERSIONTARGETCHANGED);
#endif /* _WIN32_WINNT >= 0x0602 */
#if(_WIN32_WINNT >= 0x0601)
    __def(EVENT_OBJECT_END);
    __def(EVENT_AIA_START);
    __def(EVENT_AIA_END);
#endif /* _WIN32_WINNT >= 0x0601 */

#undef __def
}

static void define_consts_objid(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_long(const_name),     \
              "Constant for object IDs.")
    __def(CHILDID_SELF);
    __def(INDEXID_OBJECT);
    __def(INDEXID_CONTAINER);

    __def(OBJID_WINDOW);
    __def(OBJID_SYSMENU);
    __def(OBJID_TITLEBAR);
    __def(OBJID_MENU);
    __def(OBJID_CLIENT);
    __def(OBJID_VSCROLL);
    __def(OBJID_HSCROLL);
    __def(OBJID_SIZEGRIP);
    __def(OBJID_CARET);
    __def(OBJID_CURSOR);
    __def(OBJID_ALERT);
    __def(OBJID_SOUND);
    __def(OBJID_QUERYCLASSNAMEIDX);
    __def(OBJID_NATIVEOM);
#undef __def
}

static void define_consts_wh(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_int(const_name),     \
              "Constant for SetWindowsHookEx() event types.")

    __def(WH_MIN);
    __def(WH_MSGFILTER);
    __def(WH_JOURNALRECORD);
    __def(WH_JOURNALPLAYBACK);
    __def(WH_KEYBOARD);
    __def(WH_GETMESSAGE);
    __def(WH_CALLWNDPROC);
    __def(WH_CBT);
    __def(WH_SYSMSGFILTER);
    __def(WH_MOUSE);
#if defined(_WIN32_WINDOWS)
    __def(WH_HARDWARE);
#endif
    __def(WH_DEBUG);
    __def(WH_SHELL);
    __def(WH_FOREGROUNDIDLE);
#if(WINVER >= 0x0400)
    __def(WH_CALLWNDPROCRET);
#endif /* WINVER >= 0x0400 */
#if (_WIN32_WINNT >= 0x0400)
    __def(WH_KEYBOARD_LL);
    __def(WH_MOUSE_LL);
#endif // (_WIN32_WINNT >= 0x0400)
#if(WINVER >= 0x0400)
#if (_WIN32_WINNT >= 0x0400)
    __def(WH_MAX);
#else
    __def(WH_MAX);
#endif // (_WIN32_WINNT >= 0x0400)
#else
    __def(WH_MAX);
#endif
    __def(WH_MINHOOK);
    __def(WH_MAXHOOK);

#undef __def
}

static void define_consts_rid(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_uint(const_name),      \
              "Constant for Raw Input commands.")
    __def(RID_INPUT);
    __def(RID_HEADER);
#undef __def
}

static void define_consts_rim(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),      \
              "Constant for Raw Input data types.")
    __def(RIM_TYPEMOUSE);
    __def(RIM_TYPEKEYBOARD);
    __def(RIM_TYPEHID);
    __def(RIM_TYPEMAX);
#undef __def
}

static void define_consts_ridev(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),      \
              "Constant for Raw Input device flags.")

    __def(RIDEV_REMOVE);
    __def(RIDEV_EXCLUDE);
    __def(RIDEV_PAGEONLY);
    __def(RIDEV_NOLEGACY);
    __def(RIDEV_INPUTSINK);
    __def(RIDEV_CAPTUREMOUSE);
    __def(RIDEV_NOHOTKEYS);
    __def(RIDEV_APPKEYS);
#if(_WIN32_WINNT >= 0x0501)
    __def(RIDEV_EXINPUTSINK);
    __def(RIDEV_DEVNOTIFY);
#endif /* _WIN32_WINNT >= 0x0501 */
    __def(RIDEV_EXMODEMASK);

#undef __def
}

static void define_consts_ri(JanetTable *env)
{
#define __def(const_name)                                         \
    janet_def(env, #const_name, jw32_wrap_word(const_name),      \
              "Constant for Raw Input device state flags.")

    __def(RI_KEY_MAKE);
    __def(RI_KEY_BREAK);
    __def(RI_KEY_E0);
    __def(RI_KEY_E1);
    __def(RI_KEY_TERMSRV_SET_LED);
    __def(RI_KEY_TERMSRV_SHADOW);
    __def(RI_MOUSE_LEFT_BUTTON_DOWN);
    __def(RI_MOUSE_LEFT_BUTTON_UP);
    __def(RI_MOUSE_RIGHT_BUTTON_DOWN);
    __def(RI_MOUSE_RIGHT_BUTTON_UP);
    __def(RI_MOUSE_MIDDLE_BUTTON_DOWN);
    __def(RI_MOUSE_MIDDLE_BUTTON_UP);
    __def(RI_MOUSE_BUTTON_1_DOWN);
    __def(RI_MOUSE_BUTTON_1_UP);
    __def(RI_MOUSE_BUTTON_2_DOWN);
    __def(RI_MOUSE_BUTTON_2_UP);
    __def(RI_MOUSE_BUTTON_3_DOWN);
    __def(RI_MOUSE_BUTTON_3_UP);
    __def(RI_MOUSE_BUTTON_4_DOWN);
    __def(RI_MOUSE_BUTTON_4_UP);
    __def(RI_MOUSE_BUTTON_5_DOWN);
    __def(RI_MOUSE_BUTTON_5_UP);
    __def(RI_MOUSE_WHEEL);
#if(WINVER >= 0x0600)
    __def(RI_MOUSE_HWHEEL);
#endif /* WINVER >= 0x0600 */

#undef __def
}

static void define_consts_ridi(JanetTable *env)
{
#define __def(const_name)                                        \
    janet_def(env, #const_name, jw32_wrap_uint(const_name),      \
              "Constant for Raw Input device info commands.")
    __def(RIDI_PREPARSEDDATA);
    __def(RIDI_DEVICENAME);
    __def(RIDI_DEVICEINFO);
#undef __def
}

static void define_consts_input(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),    \
              "Constant for INPUT types.")
    __def(INPUT_MOUSE);
    __def(INPUT_KEYBOARD);
    __def(INPUT_HARDWARE);
#undef __def
}

static void define_consts_mouseeventf(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),    \
              "Constant for mouse input event types.")

    __def(MOUSEEVENTF_MOVE);
    __def(MOUSEEVENTF_LEFTDOWN);
    __def(MOUSEEVENTF_LEFTUP);
    __def(MOUSEEVENTF_RIGHTDOWN);
    __def(MOUSEEVENTF_RIGHTUP);
    __def(MOUSEEVENTF_MIDDLEDOWN);
    __def(MOUSEEVENTF_MIDDLEUP);
    __def(MOUSEEVENTF_XDOWN);
    __def(MOUSEEVENTF_XUP);
    __def(MOUSEEVENTF_WHEEL);
#if (_WIN32_WINNT >= 0x0600)
    __def(MOUSEEVENTF_HWHEEL);
#endif
#if(WINVER >= 0x0600)
    __def(MOUSEEVENTF_MOVE_NOCOALESCE);
#endif /* WINVER >= 0x0600 */
    __def(MOUSEEVENTF_VIRTUALDESK);
    __def(MOUSEEVENTF_ABSOLUTE);

#undef __def
}

static void define_consts_keyeventf(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),    \
              "Constant for keyboard input event types.")

    __def(KEYEVENTF_EXTENDEDKEY);
    __def(KEYEVENTF_KEYUP);
#if(_WIN32_WINNT >= 0x0500)
    __def(KEYEVENTF_UNICODE);
    __def(KEYEVENTF_SCANCODE);
#endif /* _WIN32_WINNT >= 0x0500 */

#undef __def
}

static void define_consts_monitorinfof(JanetTable *env)
{
    janet_def(env, "MONITORINFOF_PRIMARY", jw32_wrap_dword(MONITORINFOF_PRIMARY),
              "Constant for moniter info flags.");
}

static void define_consts_monitor(JanetTable *env)
{
    janet_def(env, "MONITOR_DEFAULTTONEAREST", jw32_wrap_dword(MONITOR_DEFAULTTONEAREST),
              "Constant for MonitorFrom* flags.");
    janet_def(env, "MONITOR_DEFAULTTONULL", jw32_wrap_dword(MONITOR_DEFAULTTONULL),
              "Constant for MonitorFrom* flags.");
    janet_def(env, "MONITOR_DEFAULTTOPRIMARY", jw32_wrap_dword(MONITOR_DEFAULTTOPRIMARY),
              "Constant for MonitorFrom* flags.");
}

static void define_consts_tpm(JanetTable *env)
{
#define __def(const_name)                                      \
    janet_def(env, #const_name, jw32_wrap_uint(const_name),    \
              "Constant for TrackPopupMenu flags.")

    __def(TPM_LEFTBUTTON);
    __def(TPM_RIGHTBUTTON);
    __def(TPM_LEFTALIGN);
    __def(TPM_CENTERALIGN);
    __def(TPM_RIGHTALIGN);
#if(WINVER >= 0x0400)
    __def(TPM_TOPALIGN);
    __def(TPM_VCENTERALIGN);
    __def(TPM_BOTTOMALIGN);
    __def(TPM_HORIZONTAL);
    __def(TPM_VERTICAL);
    __def(TPM_NONOTIFY);
    __def(TPM_RETURNCMD);
#endif /* WINVER >= 0x0400 */
#if(WINVER >= 0x0500)
    __def(TPM_RECURSE);
    __def(TPM_HORPOSANIMATION);
    __def(TPM_HORNEGANIMATION);
    __def(TPM_VERPOSANIMATION);
    __def(TPM_VERNEGANIMATION);
#if(_WIN32_WINNT >= 0x0500)
    __def(TPM_NOANIMATION);
#endif /* _WIN32_WINNT >= 0x0500 */
#if(_WIN32_WINNT >= 0x0501)
    __def(TPM_LAYOUTRTL);
#endif /* _WIN32_WINNT >= 0x0501 */
#endif /* WINVER >= 0x0500 */
#if(_WIN32_WINNT >= 0x0601)
    __def(TPM_WORKAREA);
#endif /* _WIN32_WINNT >= 0x0601 */

#undef __def
}


static void define_consts_vk(JanetTable *env)
{
#define __def(const_name)                                   \
    janet_def(env, #const_name, jw32_wrap_word(const_name), \
              "Constant for Virtual-Key Codes.")

    __def(VK_LBUTTON);
    __def(VK_RBUTTON);
    __def(VK_CANCEL);
    __def(VK_MBUTTON);
#if(_WIN32_WINNT >= 0x0500)
    __def(VK_XBUTTON1);
    __def(VK_XBUTTON2);
#endif /* _WIN32_WINNT >= 0x0500 */
/*
 * 0x07 : reserved
 */
    __def(VK_BACK);
    __def(VK_TAB);
/*
 * 0x0A - 0x0B : reserved
 */
    __def(VK_CLEAR);
    __def(VK_RETURN);
/*
 * 0x0E - 0x0F : unassigned
 */
    __def(VK_SHIFT);
    __def(VK_CONTROL);
    __def(VK_MENU);
    __def(VK_PAUSE);
    __def(VK_CAPITAL);
    __def(VK_KANA);
    __def(VK_HANGEUL);
    __def(VK_HANGUL);
    __def(VK_IME_ON);
    __def(VK_JUNJA);
    __def(VK_FINAL);
    __def(VK_HANJA);
    __def(VK_KANJI);
    __def(VK_IME_OFF);
    __def(VK_ESCAPE);
    __def(VK_CONVERT);
    __def(VK_NONCONVERT);
    __def(VK_ACCEPT);
    __def(VK_MODECHANGE);
    __def(VK_SPACE);
    __def(VK_PRIOR);
    __def(VK_NEXT);
    __def(VK_END);
    __def(VK_HOME);
    __def(VK_LEFT);
    __def(VK_UP);
    __def(VK_RIGHT);
    __def(VK_DOWN);
    __def(VK_SELECT);
    __def(VK_PRINT);
    __def(VK_EXECUTE);
    __def(VK_SNAPSHOT);
    __def(VK_INSERT);
    __def(VK_DELETE);
    __def(VK_HELP);
/*
 * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
 * 0x3A - 0x40 : unassigned
 * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
 */
    __def(VK_LWIN);
    __def(VK_RWIN);
    __def(VK_APPS);
/*
 * 0x5E : reserved
 */
    __def(VK_SLEEP);
    __def(VK_NUMPAD0);
    __def(VK_NUMPAD1);
    __def(VK_NUMPAD2);
    __def(VK_NUMPAD3);
    __def(VK_NUMPAD4);
    __def(VK_NUMPAD5);
    __def(VK_NUMPAD6);
    __def(VK_NUMPAD7);
    __def(VK_NUMPAD8);
    __def(VK_NUMPAD9);
    __def(VK_MULTIPLY);
    __def(VK_ADD);
    __def(VK_SEPARATOR);
    __def(VK_SUBTRACT);
    __def(VK_DECIMAL);
    __def(VK_DIVIDE);
    __def(VK_F1);
    __def(VK_F2);
    __def(VK_F3);
    __def(VK_F4);
    __def(VK_F5);
    __def(VK_F6);
    __def(VK_F7);
    __def(VK_F8);
    __def(VK_F9);
    __def(VK_F10);
    __def(VK_F11);
    __def(VK_F12);
    __def(VK_F13);
    __def(VK_F14);
    __def(VK_F15);
    __def(VK_F16);
    __def(VK_F17);
    __def(VK_F18);
    __def(VK_F19);
    __def(VK_F20);
    __def(VK_F21);
    __def(VK_F22);
    __def(VK_F23);
    __def(VK_F24);
#if(_WIN32_WINNT >= 0x0604)
/*
 * 0x88 - 0x8F : UI navigation
 */
    __def(VK_NAVIGATION_VIEW);
    __def(VK_NAVIGATION_MENU);
    __def(VK_NAVIGATION_UP);
    __def(VK_NAVIGATION_DOWN);
    __def(VK_NAVIGATION_LEFT);
    __def(VK_NAVIGATION_RIGHT);
    __def(VK_NAVIGATION_ACCEPT);
    __def(VK_NAVIGATION_CANCEL);
#endif /* _WIN32_WINNT >= 0x0604 */
    __def(VK_NUMLOCK);
    __def(VK_SCROLL);
    __def(VK_OEM_NEC_EQUAL);
    __def(VK_OEM_FJ_JISHO);
    __def(VK_OEM_FJ_MASSHOU);
    __def(VK_OEM_FJ_TOUROKU);
    __def(VK_OEM_FJ_LOYA);
    __def(VK_OEM_FJ_ROYA);
/*
 * 0x97 - 0x9F : unassigned
 */
    __def(VK_LSHIFT);
    __def(VK_RSHIFT);
    __def(VK_LCONTROL);
    __def(VK_RCONTROL);
    __def(VK_LMENU);
    __def(VK_RMENU);
#if(_WIN32_WINNT >= 0x0500)
    __def(VK_BROWSER_BACK);
    __def(VK_BROWSER_FORWARD);
    __def(VK_BROWSER_REFRESH);
    __def(VK_BROWSER_STOP);
    __def(VK_BROWSER_SEARCH);
    __def(VK_BROWSER_FAVORITES);
    __def(VK_BROWSER_HOME);
    __def(VK_VOLUME_MUTE);
    __def(VK_VOLUME_DOWN);
    __def(VK_VOLUME_UP);
    __def(VK_MEDIA_NEXT_TRACK);
    __def(VK_MEDIA_PREV_TRACK);
    __def(VK_MEDIA_STOP);
    __def(VK_MEDIA_PLAY_PAUSE);
    __def(VK_LAUNCH_MAIL);
    __def(VK_LAUNCH_MEDIA_SELECT);
    __def(VK_LAUNCH_APP1);
    __def(VK_LAUNCH_APP2);
#endif /* _WIN32_WINNT >= 0x0500 */
/*
 * 0xB8 - 0xB9 : reserved
 */
    __def(VK_OEM_1);
    __def(VK_OEM_PLUS);
    __def(VK_OEM_COMMA);
    __def(VK_OEM_MINUS);
    __def(VK_OEM_PERIOD);
    __def(VK_OEM_2);
    __def(VK_OEM_3);
/*
 * 0xC1 - 0xC2 : reserved
 */
#if(_WIN32_WINNT >= 0x0604)
    __def(VK_GAMEPAD_A);
    __def(VK_GAMEPAD_B);
    __def(VK_GAMEPAD_X);
    __def(VK_GAMEPAD_Y);
    __def(VK_GAMEPAD_RIGHT_SHOULDER);
    __def(VK_GAMEPAD_LEFT_SHOULDER);
    __def(VK_GAMEPAD_LEFT_TRIGGER);
    __def(VK_GAMEPAD_RIGHT_TRIGGER);
    __def(VK_GAMEPAD_DPAD_UP);
    __def(VK_GAMEPAD_DPAD_DOWN);
    __def(VK_GAMEPAD_DPAD_LEFT);
    __def(VK_GAMEPAD_DPAD_RIGHT);
    __def(VK_GAMEPAD_MENU);
    __def(VK_GAMEPAD_VIEW);
    __def(VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON);
    __def(VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON);
    __def(VK_GAMEPAD_LEFT_THUMBSTICK_UP);
    __def(VK_GAMEPAD_LEFT_THUMBSTICK_DOWN);
    __def(VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT);
    __def(VK_GAMEPAD_LEFT_THUMBSTICK_LEFT);
    __def(VK_GAMEPAD_RIGHT_THUMBSTICK_UP);
    __def(VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN);
    __def(VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT);
    __def(VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT);
#endif /* _WIN32_WINNT >= 0x0604 */
    __def(VK_OEM_4);
    __def(VK_OEM_5);
    __def(VK_OEM_6);
    __def(VK_OEM_7);
    __def(VK_OEM_8);
/*
 * 0xE0 : reserved
 */
    __def(VK_OEM_AX);
    __def(VK_OEM_102);
    __def(VK_ICO_HELP);
    __def(VK_ICO_00);
#if(WINVER >= 0x0400)
    __def(VK_PROCESSKEY);
#endif /* WINVER >= 0x0400 */
    __def(VK_ICO_CLEAR);
#if(_WIN32_WINNT >= 0x0500)
    __def(VK_PACKET);
#endif /* _WIN32_WINNT >= 0x0500 */
/*
 * 0xE8 : unassigned
 */
    __def(VK_OEM_RESET);
    __def(VK_OEM_JUMP);
    __def(VK_OEM_PA1);
    __def(VK_OEM_PA2);
    __def(VK_OEM_PA3);
    __def(VK_OEM_WSCTRL);
    __def(VK_OEM_CUSEL);
    __def(VK_OEM_ATTN);
    __def(VK_OEM_FINISH);
    __def(VK_OEM_COPY);
    __def(VK_OEM_AUTO);
    __def(VK_OEM_ENLW);
    __def(VK_OEM_BACKTAB);
    __def(VK_ATTN);
    __def(VK_CRSEL);
    __def(VK_EXSEL);
    __def(VK_EREOF);
    __def(VK_PLAY);
    __def(VK_ZOOM);
    __def(VK_NONAME);
    __def(VK_PA1);
    __def(VK_OEM_CLEAR);
/*
 * 0xFF : reserved
 */

#undef __def
}


static void define_consts_spi(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_uint(const_name),     \
              "Constant for SystemParametersInfo actions.")

    __def(SPI_GETBEEP);
    __def(SPI_SETBEEP);
    __def(SPI_GETMOUSE);
    __def(SPI_SETMOUSE);
    __def(SPI_GETBORDER);
    __def(SPI_SETBORDER);
    __def(SPI_GETKEYBOARDSPEED);
    __def(SPI_SETKEYBOARDSPEED);
    __def(SPI_LANGDRIVER);
    __def(SPI_ICONHORIZONTALSPACING);
    __def(SPI_GETSCREENSAVETIMEOUT);
    __def(SPI_SETSCREENSAVETIMEOUT);
    __def(SPI_GETSCREENSAVEACTIVE);
    __def(SPI_SETSCREENSAVEACTIVE);
    __def(SPI_GETGRIDGRANULARITY);
    __def(SPI_SETGRIDGRANULARITY);
    __def(SPI_SETDESKWALLPAPER);
    __def(SPI_SETDESKPATTERN);
    __def(SPI_GETKEYBOARDDELAY);
    __def(SPI_SETKEYBOARDDELAY);
    __def(SPI_ICONVERTICALSPACING);
    __def(SPI_GETICONTITLEWRAP);
    __def(SPI_SETICONTITLEWRAP);
    __def(SPI_GETMENUDROPALIGNMENT);
    __def(SPI_SETMENUDROPALIGNMENT);
    __def(SPI_SETDOUBLECLKWIDTH);
    __def(SPI_SETDOUBLECLKHEIGHT);
    __def(SPI_GETICONTITLELOGFONT);
    __def(SPI_SETDOUBLECLICKTIME);
    __def(SPI_SETMOUSEBUTTONSWAP);
    __def(SPI_SETICONTITLELOGFONT);
    __def(SPI_GETFASTTASKSWITCH);
    __def(SPI_SETFASTTASKSWITCH);
#if(WINVER >= 0x0400)
    __def(SPI_SETDRAGFULLWINDOWS);
    __def(SPI_GETDRAGFULLWINDOWS);
    __def(SPI_GETNONCLIENTMETRICS);
    __def(SPI_SETNONCLIENTMETRICS);
    __def(SPI_GETMINIMIZEDMETRICS);
    __def(SPI_SETMINIMIZEDMETRICS);
    __def(SPI_GETICONMETRICS);
    __def(SPI_SETICONMETRICS);
    __def(SPI_SETWORKAREA);
    __def(SPI_GETWORKAREA);
    __def(SPI_SETPENWINDOWS);
    __def(SPI_GETHIGHCONTRAST);
    __def(SPI_SETHIGHCONTRAST);
    __def(SPI_GETKEYBOARDPREF);
    __def(SPI_SETKEYBOARDPREF);
    __def(SPI_GETSCREENREADER);
    __def(SPI_SETSCREENREADER);
    __def(SPI_GETANIMATION);
    __def(SPI_SETANIMATION);
    __def(SPI_GETFONTSMOOTHING);
    __def(SPI_SETFONTSMOOTHING);
    __def(SPI_SETDRAGWIDTH);
    __def(SPI_SETDRAGHEIGHT);
    __def(SPI_SETHANDHELD);
    __def(SPI_GETLOWPOWERTIMEOUT);
    __def(SPI_GETPOWEROFFTIMEOUT);
    __def(SPI_SETLOWPOWERTIMEOUT);
    __def(SPI_SETPOWEROFFTIMEOUT);
    __def(SPI_GETLOWPOWERACTIVE);
    __def(SPI_GETPOWEROFFACTIVE);
    __def(SPI_SETLOWPOWERACTIVE);
    __def(SPI_SETPOWEROFFACTIVE);
    __def(SPI_SETCURSORS);
    __def(SPI_SETICONS);
    __def(SPI_GETDEFAULTINPUTLANG);
    __def(SPI_SETDEFAULTINPUTLANG);
    __def(SPI_SETLANGTOGGLE);
    __def(SPI_GETWINDOWSEXTENSION);
    __def(SPI_SETMOUSETRAILS);
    __def(SPI_GETMOUSETRAILS);
    __def(SPI_SETSCREENSAVERRUNNING);
    __def(SPI_SCREENSAVERRUNNING);
#endif /* WINVER >= 0x0400 */
    __def(SPI_GETFILTERKEYS);
    __def(SPI_SETFILTERKEYS);
    __def(SPI_GETTOGGLEKEYS);
    __def(SPI_SETTOGGLEKEYS);
    __def(SPI_GETMOUSEKEYS);
    __def(SPI_SETMOUSEKEYS);
    __def(SPI_GETSHOWSOUNDS);
    __def(SPI_SETSHOWSOUNDS);
    __def(SPI_GETSTICKYKEYS);
    __def(SPI_SETSTICKYKEYS);
    __def(SPI_GETACCESSTIMEOUT);
    __def(SPI_SETACCESSTIMEOUT);
#if(WINVER >= 0x0400)
    __def(SPI_GETSERIALKEYS);
    __def(SPI_SETSERIALKEYS);
#endif /* WINVER >= 0x0400 */
    __def(SPI_GETSOUNDSENTRY);
    __def(SPI_SETSOUNDSENTRY);
#if(_WIN32_WINNT >= 0x0400)
    __def(SPI_GETSNAPTODEFBUTTON);
    __def(SPI_SETSNAPTODEFBUTTON);
#endif /* _WIN32_WINNT >= 0x0400 */
#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
    __def(SPI_GETMOUSEHOVERWIDTH);
    __def(SPI_SETMOUSEHOVERWIDTH);
    __def(SPI_GETMOUSEHOVERHEIGHT);
    __def(SPI_SETMOUSEHOVERHEIGHT);
    __def(SPI_GETMOUSEHOVERTIME);
    __def(SPI_SETMOUSEHOVERTIME);
    __def(SPI_GETWHEELSCROLLLINES);
    __def(SPI_SETWHEELSCROLLLINES);
    __def(SPI_GETMENUSHOWDELAY);
    __def(SPI_SETMENUSHOWDELAY);
#if (_WIN32_WINNT >= 0x0600)
    __def(SPI_GETWHEELSCROLLCHARS);
    __def(SPI_SETWHEELSCROLLCHARS);
#endif
    __def(SPI_GETSHOWIMEUI);
    __def(SPI_SETSHOWIMEUI);
#endif
#if(WINVER >= 0x0500)
    __def(SPI_GETMOUSESPEED);
    __def(SPI_SETMOUSESPEED);
    __def(SPI_GETSCREENSAVERRUNNING);
    __def(SPI_GETDESKWALLPAPER);
#endif /* WINVER >= 0x0500 */
#if(WINVER >= 0x0600)
    __def(SPI_GETAUDIODESCRIPTION);
    __def(SPI_SETAUDIODESCRIPTION);
    __def(SPI_GETSCREENSAVESECURE);
    __def(SPI_SETSCREENSAVESECURE);
#endif /* WINVER >= 0x0600 */
#if(_WIN32_WINNT >= 0x0601)
    __def(SPI_GETHUNGAPPTIMEOUT);
    __def(SPI_SETHUNGAPPTIMEOUT);
    __def(SPI_GETWAITTOKILLTIMEOUT);
    __def(SPI_SETWAITTOKILLTIMEOUT);
    __def(SPI_GETWAITTOKILLSERVICETIMEOUT);
    __def(SPI_SETWAITTOKILLSERVICETIMEOUT);
    __def(SPI_GETMOUSEDOCKTHRESHOLD);
    __def(SPI_SETMOUSEDOCKTHRESHOLD);
    __def(SPI_GETPENDOCKTHRESHOLD);
    __def(SPI_SETPENDOCKTHRESHOLD);
    __def(SPI_GETWINARRANGING);
    __def(SPI_SETWINARRANGING);
    __def(SPI_GETMOUSEDRAGOUTTHRESHOLD);
    __def(SPI_SETMOUSEDRAGOUTTHRESHOLD);
    __def(SPI_GETPENDRAGOUTTHRESHOLD);
    __def(SPI_SETPENDRAGOUTTHRESHOLD);
    __def(SPI_GETMOUSESIDEMOVETHRESHOLD);
    __def(SPI_SETMOUSESIDEMOVETHRESHOLD);
    __def(SPI_GETPENSIDEMOVETHRESHOLD);
    __def(SPI_SETPENSIDEMOVETHRESHOLD);
    __def(SPI_GETDRAGFROMMAXIMIZE);
    __def(SPI_SETDRAGFROMMAXIMIZE);
    __def(SPI_GETSNAPSIZING);
    __def(SPI_SETSNAPSIZING);
    __def(SPI_GETDOCKMOVING);
    __def(SPI_SETDOCKMOVING);
#endif /* _WIN32_WINNT >= 0x0601 */
#if(WINVER >= 0x0602)
    __def(SPI_GETTOUCHPREDICTIONPARAMETERS);
    __def(SPI_SETTOUCHPREDICTIONPARAMETERS);
    __def(SPI_GETLOGICALDPIOVERRIDE);
    __def(SPI_SETLOGICALDPIOVERRIDE);
    __def(SPI_GETMENURECT);
    __def(SPI_SETMENURECT);
#endif /* WINVER >= 0x0602 */
#if(WINVER >= 0x0500)
    __def(SPI_GETACTIVEWINDOWTRACKING);
    __def(SPI_SETACTIVEWINDOWTRACKING);
    __def(SPI_GETMENUANIMATION);
    __def(SPI_SETMENUANIMATION);
    __def(SPI_GETCOMBOBOXANIMATION);
    __def(SPI_SETCOMBOBOXANIMATION);
    __def(SPI_GETLISTBOXSMOOTHSCROLLING);
    __def(SPI_SETLISTBOXSMOOTHSCROLLING);
    __def(SPI_GETGRADIENTCAPTIONS);
    __def(SPI_SETGRADIENTCAPTIONS);
    __def(SPI_GETKEYBOARDCUES);
    __def(SPI_SETKEYBOARDCUES);
    __def(SPI_GETMENUUNDERLINES);
    __def(SPI_SETMENUUNDERLINES);
    __def(SPI_GETACTIVEWNDTRKZORDER);
    __def(SPI_SETACTIVEWNDTRKZORDER);
    __def(SPI_GETHOTTRACKING);
    __def(SPI_SETHOTTRACKING);
    __def(SPI_GETMENUFADE);
    __def(SPI_SETMENUFADE);
    __def(SPI_GETSELECTIONFADE);
    __def(SPI_SETSELECTIONFADE);
    __def(SPI_GETTOOLTIPANIMATION);
    __def(SPI_SETTOOLTIPANIMATION);
    __def(SPI_GETTOOLTIPFADE);
    __def(SPI_SETTOOLTIPFADE);
    __def(SPI_GETCURSORSHADOW);
    __def(SPI_SETCURSORSHADOW);
#if(_WIN32_WINNT >= 0x0501)
    __def(SPI_GETMOUSESONAR);
    __def(SPI_SETMOUSESONAR);
    __def(SPI_GETMOUSECLICKLOCK);
    __def(SPI_SETMOUSECLICKLOCK);
    __def(SPI_GETMOUSEVANISH);
    __def(SPI_SETMOUSEVANISH);
    __def(SPI_GETFLATMENU);
    __def(SPI_SETFLATMENU);
    __def(SPI_GETDROPSHADOW);
    __def(SPI_SETDROPSHADOW);
    __def(SPI_GETBLOCKSENDINPUTRESETS);
    __def(SPI_SETBLOCKSENDINPUTRESETS);
#endif /* _WIN32_WINNT >= 0x0501 */
    __def(SPI_GETUIEFFECTS);
    __def(SPI_SETUIEFFECTS);
#if(_WIN32_WINNT >= 0x0600)
    __def(SPI_GETDISABLEOVERLAPPEDCONTENT);
    __def(SPI_SETDISABLEOVERLAPPEDCONTENT);
    __def(SPI_GETCLIENTAREAANIMATION);
    __def(SPI_SETCLIENTAREAANIMATION);
    __def(SPI_GETCLEARTYPE);
    __def(SPI_SETCLEARTYPE);
    __def(SPI_GETSPEECHRECOGNITION);
    __def(SPI_SETSPEECHRECOGNITION);
#endif /* _WIN32_WINNT >= 0x0600 */
#if(WINVER >= 0x0601)
    __def(SPI_GETCARETBROWSING);
    __def(SPI_SETCARETBROWSING);
    __def(SPI_GETTHREADLOCALINPUTSETTINGS);
    __def(SPI_SETTHREADLOCALINPUTSETTINGS);
    __def(SPI_GETSYSTEMLANGUAGEBAR);
    __def(SPI_SETSYSTEMLANGUAGEBAR);
#endif /* WINVER >= 0x0601 */
    __def(SPI_GETFOREGROUNDLOCKTIMEOUT);
    __def(SPI_SETFOREGROUNDLOCKTIMEOUT);
    __def(SPI_GETACTIVEWNDTRKTIMEOUT);
    __def(SPI_SETACTIVEWNDTRKTIMEOUT);
    __def(SPI_GETFOREGROUNDFLASHCOUNT);
    __def(SPI_SETFOREGROUNDFLASHCOUNT);
    __def(SPI_GETCARETWIDTH);
    __def(SPI_SETCARETWIDTH);
#if(_WIN32_WINNT >= 0x0501)
    __def(SPI_GETMOUSECLICKLOCKTIME);
    __def(SPI_SETMOUSECLICKLOCKTIME);
    __def(SPI_GETFONTSMOOTHINGTYPE);
    __def(SPI_SETFONTSMOOTHINGTYPE);
    __def(SPI_GETFONTSMOOTHINGCONTRAST);
    __def(SPI_SETFONTSMOOTHINGCONTRAST);
    __def(SPI_GETFOCUSBORDERWIDTH);
    __def(SPI_SETFOCUSBORDERWIDTH);
    __def(SPI_GETFOCUSBORDERHEIGHT);
    __def(SPI_SETFOCUSBORDERHEIGHT);
    __def(SPI_GETFONTSMOOTHINGORIENTATION);
    __def(SPI_SETFONTSMOOTHINGORIENTATION);
#endif /* _WIN32_WINNT >= 0x0501 */
#if(_WIN32_WINNT >= 0x0600)
    __def(SPI_GETMINIMUMHITRADIUS);
    __def(SPI_SETMINIMUMHITRADIUS);
    __def(SPI_GETMESSAGEDURATION);
    __def(SPI_SETMESSAGEDURATION);
#endif /* _WIN32_WINNT >= 0x0600 */
#if(WINVER >= 0x0602)
    __def(SPI_GETCONTACTVISUALIZATION);
    __def(SPI_SETCONTACTVISUALIZATION);
    __def(SPI_GETGESTUREVISUALIZATION);
    __def(SPI_SETGESTUREVISUALIZATION);
#endif /* WINVER >= 0x0602 */
#if(WINVER >= 0x0602)
    __def(SPI_GETMOUSEWHEELROUTING);
    __def(SPI_SETMOUSEWHEELROUTING);
#endif /* WINVER >= 0x0602 */
#if(WINVER >= 0x0604)
    __def(SPI_GETPENVISUALIZATION);
    __def(SPI_SETPENVISUALIZATION);
    __def(SPI_GETPENARBITRATIONTYPE);
    __def(SPI_SETPENARBITRATIONTYPE);
#endif /* WINVER >= 0x0604 */
#if (NTDDI_VERSION >= NTDDI_WIN10_RS3)
    __def(SPI_GETCARETTIMEOUT);
    __def(SPI_SETCARETTIMEOUT);
#endif // NTDDI_VERSION >= NTDDI_WIN10_RS3
#if (NTDDI_VERSION >= NTDDI_WIN10_RS4)
    __def(SPI_GETHANDEDNESS);
    __def(SPI_SETHANDEDNESS);
#endif // NTDDI_VERSION >= NTDDI_WIN10_RS4

#endif /* WINVER >= 0x0500 */

#undef __def
}


static void define_consts_gwl(JanetTable *env)
{
#define __def(const_name)                                      \
    janet_def(env, #const_name, jw32_wrap_int(const_name),     \
              "Constant for window information offsets.")
#ifndef _WIN64
    __def(GWL_WNDPROC);
    __def(GWL_HINSTANCE);
    __def(GWL_HWNDPARENT);
    __def(GWL_USERDATA);
#endif
    __def(GWL_STYLE);
    __def(GWL_EXSTYLE);
    __def(GWL_ID);
#undef __def
}


static void define_consts_gwlp(JanetTable *env)
{
#define __def(const_name)                                      \
    janet_def(env, #const_name, jw32_wrap_int(const_name),     \
              "Constant for window information offsets.")
    __def(GWLP_WNDPROC);
    __def(GWLP_HINSTANCE);
    __def(GWLP_HWNDPARENT);
    __def(GWLP_USERDATA);
    __def(GWLP_ID);
#undef __def
}


static void define_consts_gw(JanetTable *env)
{
#define __def(const_name)                                      \
    janet_def(env, #const_name, jw32_wrap_uint(const_name),     \
              "Constant for GetWindow commands.")

    __def(GW_HWNDFIRST);
    __def(GW_HWNDLAST);
    __def(GW_HWNDNEXT);
    __def(GW_HWNDPREV);
    __def(GW_OWNER);
    __def(GW_CHILD);
#if(WINVER > 0x0400)
    __def(GW_ENABLEDPOPUP);
#endif
    __def(GW_MAX);

#undef __def
}


static void define_consts_lwa(JanetTable *env)
{
#define __def(const_name)                                      \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),   \
              "Constant for layered window attributes.")

    __def(LWA_COLORKEY);
    __def(LWA_ALPHA);

#undef __def
}


static void define_consts_ga(JanetTable *env)
{
#define __def(const_name)                                      \
    janet_def(env, #const_name, jw32_wrap_int(const_name),     \
              "Constant for GetAncestor flags.")

    __def(GA_PARENT);
    __def(GA_ROOT);
    __def(GA_ROOTOWNER);

#undef __def
}


static void define_consts_mdt(JanetTable *env)
{
#define __def(const_name)                                      \
    janet_def(env, #const_name, jw32_wrap_int(const_name),     \
              "Constant for monitor DPI types.")

    __def(MDT_EFFECTIVE_DPI);
    __def(MDT_ANGULAR_DPI);
    __def(MDT_RAW_DPI);
    __def(MDT_DEFAULT);

#undef __def
}


static void define_consts_swp(JanetTable *env)
{
#define __def(const_name)                                      \
    janet_def(env, #const_name, jw32_wrap_uint(const_name),    \
              "Constant SetWindowPos flags.")

    __def(SWP_NOSIZE);
    __def(SWP_NOMOVE);
    __def(SWP_NOZORDER);
    __def(SWP_NOREDRAW);
    __def(SWP_NOACTIVATE);
    __def(SWP_FRAMECHANGED);
    __def(SWP_SHOWWINDOW);
    __def(SWP_HIDEWINDOW);
    __def(SWP_NOCOPYBITS);
    __def(SWP_NOOWNERZORDER);
    __def(SWP_NOSENDCHANGING);
    __def(SWP_DRAWFRAME);
    __def(SWP_NOREPOSITION);
#if(WINVER >= 0x0400)
    __def(SWP_DEFERERASE);
    __def(SWP_ASYNCWINDOWPOS);
#endif /* WINVER >= 0x0400 */

#undef __def
}


/*******************************************************************
 *
 * HELPER FUNCTIONS
 *
 *******************************************************************/

static int call_wnd_proc_fn(JanetFunction *fn, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, Janet *out)
{
    Janet argv[4] = {
        jw32_wrap_handle(hWnd),
        jw32_wrap_uint(uMsg),
        jw32_wrap_wparam(wParam),
        jw32_wrap_lparam(lParam),
    };

    return jw32_pcall_fn(fn, 4, argv, out);
}

static LRESULT maybe_call_wnd_proc_fn(JanetFunction *fn, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT lFailRet, BOOL bCallDefProc)
{
#ifdef JW32_CALLBACK_DEBUG
    jw32_dbg_val((uint64_t)fn, "0x%" PRIx64);
#endif

    if (fn) {
        Janet ret;

        if (call_wnd_proc_fn(fn, hWnd, uMsg, wParam, lParam, &ret)) {
            return jw32_unwrap_lresult(ret);
        } else {
            jw32_dbg_msg("call_wnd_proc_fn() failed!");
            return lFailRet;
        }
    } else {
        jw32_dbg_msg("wnd_proc not found");
        if (bCallDefProc) {
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        } else {
            return -1;
        }
    }
}

static INT_PTR maybe_call_dlg_proc_fn(JanetFunction *fn, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, INT_PTR nFailRet)
{
#ifdef JW32_CALLBACK_DEBUG
    jw32_dbg_val((uint64_t)fn, "0x%" PRIx64);
#endif

    if (fn) {
        Janet ret;

        if (call_wnd_proc_fn(fn, hWnd, uMsg, wParam, lParam, &ret)) {
            INT_PTR nRet = jw32_unwrap_int_ptr(ret);
#ifdef JW32_CALLBACK_DEBUG
            jw32_dbg_val(nRet, "0x%" PRIx64);
#endif
            return nRet;
        } else {
            jw32_dbg_msg("call_wnd_proc_fn() failed!");
            return nFailRet;
        }
    } else {
        jw32_dbg_msg("dlg_proc not found");
        return FALSE;
    }
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
            && !strcmp((const char *)entry_class_name, (const char *)class_name)) {
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
        if (!strcmp((const char *)entry_class_name, (const char *)class_name)) {
            return i;
        }
    }
    return -1;
}

static void init_wnd_proc_registries(void)
{
    if (!local_class_wnd_proc_registry) {
        local_class_wnd_proc_registry = janet_array(0);
        janet_gcroot(janet_wrap_array(local_class_wnd_proc_registry));
    }
    if (!global_class_wnd_proc_registry) {
        global_class_wnd_proc_registry = janet_array(0);
        janet_gcroot(janet_wrap_array(global_class_wnd_proc_registry));
    }
    if (!atom_class_name_map) {
        atom_class_name_map = janet_table(0);
        janet_gcroot(janet_wrap_table(atom_class_name_map));
    }
}

static Janet normalize_wnd_class_name(LPCSTR lpClassName)
{
    init_wnd_proc_registries();

    if (!check_atom(lpClassName)) {
        /* we have a string pointer */
        return janet_ckeywordv(lpClassName);
    } else {
        /* looks like an ATOM */
        ATOM atmClass = lpcstr_to_atom(lpClassName);
        Janet class_name = janet_table_get(atom_class_name_map, jw32_wrap_atom(atmClass));

        jw32_dbg_val(atmClass, "%hu");
        jw32_dbg_jval(class_name);

        return class_name; /* may be nil */
    }
}

static void register_class_wnd_proc(jw32_wc_t *jwc, ATOM atmClass)
{
    Janet wnd_proc = janet_wrap_function(jwc->wnd_proc);
    Janet class_name = janet_ckeywordv(jwc->wc.lpszClassName);
    Janet atom = jw32_wrap_atom(atmClass);
    Janet reg_entry_tuple[3];
    Janet reg_entry;

    init_wnd_proc_registries();

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

    janet_table_put(atom_class_name_map, atom, class_name);
}

static void unregister_class_wnd_proc(LPCSTR lpClassName, HINSTANCE hInstance)
{
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

    /* here we keep the entry in atom_class_name_map, since there
       may be other classes with the same name */
}


/*******************************************************************
 *
 * CALLBACKS
 *
 *******************************************************************/

LRESULT CALLBACK jw32_wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef JW32_CALLBACK_DEBUG
    jw32_dbg_msg("============= jw32_wnd_proc ===============");
    jw32_dbg_val((uint64_t)hWnd, "0x%" PRIx64);
    jw32_dbg_val(uMsg, "0x%" PRIx32);
    jw32_dbg_val(wParam, "0x%" PRIx64);
    jw32_dbg_val(lParam, "%lld");
#endif

    switch (uMsg) {
    case WM_NCCREATE: {
        CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
        const Janet *param_tuple = (Janet *)cs->lpCreateParams;
        LPVOID lpRealParam = jw32_unwrap_lpvoid(param_tuple[1]);
        BOOL bSet;

        Janet wnd_proc = param_tuple[0];
        JanetFunction *wnd_proc_fn = janet_unwrap_function(wnd_proc);

        jw32_dbg_jval(wnd_proc);

        bSet = SetProp(hWnd, JW32_WND_PROC_FN_PROP_NAME, (HANDLE)wnd_proc_fn);
        if (!bSet) {
            jw32_dbg_msg("SetProp() failed!");
            jw32_dbg_val(GetLastError(), "0x%x");
            return FALSE; /* abort window creation */
        }

        cs->lpCreateParams = lpRealParam; /* XXX: can i really do this? */
        /* return FALSE when failed, to abort window creation */
        return maybe_call_wnd_proc_fn(wnd_proc_fn, hWnd, uMsg, wParam, lParam, FALSE, FALSE);
    }

    case WM_NCDESTROY: {
        JanetFunction *wnd_proc_fn = (JanetFunction *)RemoveProp(hWnd, JW32_WND_PROC_FN_PROP_NAME);
        /* return zero when failed, to indicate we did the clean-up */
        return maybe_call_wnd_proc_fn(wnd_proc_fn, hWnd, uMsg, wParam, lParam, 0, FALSE);
    }

    default: {
        JanetFunction *wnd_proc_fn = (JanetFunction *)GetProp(hWnd, JW32_WND_PROC_FN_PROP_NAME);
        /* XXX: -1 means "abort" in WM_CREATE, but other messages may not
           understand this value */
        /* and we call DefWindowProc() when we can't find wnd_proc */
        return maybe_call_wnd_proc_fn(wnd_proc_fn, hWnd, uMsg, wParam, lParam, -1, TRUE);
    }
    }
}

INT_PTR CALLBACK jw32_dlg_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef JW32_CALLBACK_DEBUG
    jw32_dbg_msg("============= jw32_dlg_proc ===============");
    jw32_dbg_val((uint64_t)hWnd, "0x%" PRIx64);
    jw32_dbg_val(uMsg, "0x%" PRIx32);
    jw32_dbg_val(wParam, "0x%" PRIx64);
    jw32_dbg_val(lParam, "%lld");
#endif

    switch (uMsg) {
    case WM_INITDIALOG: {
        const Janet *param_tuple = (Janet *)lParam;
        LPARAM lpRealParam = jw32_unwrap_lparam(param_tuple[1]);
        BOOL bSet;

        Janet dlg_proc = param_tuple[0];
        JanetFunction *dlg_proc_fn = janet_unwrap_function(dlg_proc);

        jw32_dbg_jval(dlg_proc);

        bSet = SetProp(hWnd, JW32_DLG_PROC_FN_PROP_NAME, (HANDLE)dlg_proc_fn);
        if (!bSet) {
            jw32_dbg_msg("SetProp() failed!");
            jw32_dbg_val(GetLastError(), "0x%x");
            if (!EndDialog(hWnd, -1)) {
                jw32_dbg_msg("EndDialog() failed!");
                jw32_dbg_val(GetLastError(), "0x%x");
            }
            return TRUE; /* set default keyboard focus */
        }

        /* set default keyboard focus even if we failed */
        return maybe_call_dlg_proc_fn(dlg_proc_fn, hWnd, uMsg, wParam, lpRealParam, TRUE);
    }

    case WM_NCDESTROY: {
        JanetFunction *dlg_proc_fn = (JanetFunction *)RemoveProp(hWnd, JW32_DLG_PROC_FN_PROP_NAME);
        return maybe_call_dlg_proc_fn(dlg_proc_fn, hWnd, uMsg, wParam, lParam, FALSE);
    }

    default: {
        JanetFunction *dlg_proc_fn = (JanetFunction *)GetProp(hWnd, JW32_DLG_PROC_FN_PROP_NAME);
        return maybe_call_dlg_proc_fn(dlg_proc_fn, hWnd, uMsg, wParam, lParam, FALSE);
    }
    }
}

void CALLBACK jw32_win_event_proc(HWINEVENTHOOK hWinEventHook, DWORD event,
                                  HWND hwnd, LONG idObject, LONG idChild,
                                  DWORD idEventThread, DWORD dwmsEventTime)
{
    Janet hook = jw32_wrap_handle(hWinEventHook),
        win_event_proc = janet_table_get(win_event_hook_registry, hook);
    Janet argv[7], ret;
    JanetFunction *win_event_proc_fn = NULL;

#ifdef JW32_CALLBACK_DEBUG
    jw32_dbg_msg("============= jw32_win_event_proc ===============");
    jw32_dbg_val((uint64_t)hWinEventHook, "0x%" PRIx64);
    jw32_dbg_val(event, "0x%" PRIx32);
    jw32_dbg_val((uint64_t)hwnd, "0x%" PRIx64);
    jw32_dbg_val(idObject, "%ld");
    jw32_dbg_val(idChild, "%ld");
    jw32_dbg_val(idEventThread, "0x%" PRIx32);
    jw32_dbg_val(dwmsEventTime, "0x%" PRIx32);
#endif

    if (janet_checktype(win_event_proc, JANET_NIL)) {
        jw32_dbg_msg("nil win_event_proc!");
        return;
    }

    win_event_proc_fn = janet_unwrap_function(win_event_proc);

    argv[0] = jw32_wrap_handle(hWinEventHook);
    argv[1] = jw32_wrap_dword(event);
    argv[2] = jw32_wrap_handle(hwnd);
    argv[3] = jw32_wrap_long(idObject);
    argv[4] = jw32_wrap_long(idChild);
    argv[5] = jw32_wrap_dword(idEventThread);
    argv[6] = jw32_wrap_dword(dwmsEventTime);

    if (!jw32_pcall_fn(win_event_proc_fn, 7, argv, &ret)) {
        jw32_dbg_jval(ret);
    }
}

BOOL CALLBACK jw32_monitor_enum_proc(HMONITOR hMonitor, HDC hdc, LPRECT lpRect, LPARAM lParam)
{
    JanetFunction *enum_fn = (JanetFunction *)lParam;
    Janet argv[3] = {
        jw32_wrap_handle(hMonitor),
        jw32_wrap_handle(hdc),
        janet_wrap_struct(jw32_rect_to_struct(lpRect)),
        /* janet has closures, doesn't need lParam */
    };
    Janet ret;

    if (jw32_pcall_fn(enum_fn, 3, argv, &ret)) {
        return jw32_unwrap_bool(ret);
    } else {
        jw32_dbg_msg("jw32_pcall_fn() failed, stop monitor enumeration");
        return FALSE;
    }
}

BOOL CALLBACK jw32_child_window_enum_proc(HWND hwnd, LPARAM lParam)
{
    JanetFunction *enum_fn = (JanetFunction *)lParam;
    Janet argv[1] = {
        jw32_wrap_handle(hwnd),
        /* janet has closures, doesn't need lParam */
    };
    Janet ret;

    if (jw32_pcall_fn(enum_fn, 1, argv, &ret)) {
        return jw32_unwrap_bool(ret);
    } else {
        jw32_dbg_msg("jw32_pcall_fn() failed, stop child window enumeration");
        return FALSE;
    }
}

static int PKBDLLHOOKSTRUCT_get(void *p, Janet key, Janet *out)
{
    PKBDLLHOOKSTRUCT *ppHookStruct = (PKBDLLHOOKSTRUCT *)p;
    PKBDLLHOOKSTRUCT pHookStruct = *ppHookStruct;

    if (!janet_checktype(key, JANET_KEYWORD)) {
        janet_panicf("expected keyword, got %v", key);
    }

    const uint8_t *kw = janet_unwrap_keyword(key);

#define __get_member(member, type)              \
    if (!janet_cstrcmp(kw, #member)) {          \
        *out = jw32_wrap_##type(pHookStruct->member);   \
        return 1;                               \
    }

    __get_member(vkCode, int)
    __get_member(scanCode, dword)
    __get_member(flags, dword)
    if (!janet_cstrcmp(kw, "flags.extended")) {
        *out = janet_wrap_boolean(pHookStruct->flags & LLKHF_EXTENDED);
        return 1;
    }
    if (!janet_cstrcmp(kw, "flags.lower_il_injected")) {
        *out = janet_wrap_boolean(pHookStruct->flags & LLKHF_LOWER_IL_INJECTED);
        return 1;
    }
    if (!janet_cstrcmp(kw, "flags.injected")) {
        *out = janet_wrap_boolean(pHookStruct->flags & LLKHF_INJECTED);
        return 1;
    }
    if (!janet_cstrcmp(kw, "flags.altdown")) {
        *out = janet_wrap_boolean(pHookStruct->flags & LLKHF_ALTDOWN);
        return 1;
    }
    if (!janet_cstrcmp(kw, "flags.up")) {
        *out = janet_wrap_boolean(pHookStruct->flags & LLKHF_UP);
        return 1;
    }
    __get_member(time, dword)
    __get_member(dwExtraInfo, ulong_ptr)
    if (!janet_cstrcmp(kw, "address")) {
        *out = jw32_wrap_lparam(pHookStruct);
        return 1;
    }

#undef __get_member

    return 0;
}

static const JanetAbstractType jw32_at_PKBDLLHOOKSTRUCT = {
    .name = MOD_NAME "/PKBDLLHOOKSTRUCT",
    .gc = NULL,
    .gcmark = NULL,
    .get = PKBDLLHOOKSTRUCT_get,
    JANET_ATEND_GET
};

static LRESULT call_win_hook_proc(int idHook, int nCode, WPARAM wParam, LPARAM lParam)
{
    Janet hook = jw32_wrap_int(idHook);
    Janet proc_n_hhook_v = janet_table_get(win_hook_registry, hook);

    if (janet_checktype(proc_n_hhook_v, JANET_NIL)) {
        jw32_dbg_msg("nil proc_n_hhook_v!");
        return CallNextHookEx(0, nCode, wParam, lParam);
    }

    JanetTuple proc_n_hhook = janet_unwrap_tuple(proc_n_hhook_v);
    JanetFunction *proc_fn = janet_unwrap_function(proc_n_hhook[0]);
    Janet argv[3] = {
        jw32_wrap_int(nCode),
        jw32_wrap_wparam(wParam),
        janet_wrap_nil(),
    };

    switch (idHook) {
    case WH_KEYBOARD_LL:
        PKBDLLHOOKSTRUCT *ppHookStruct = janet_abstract(&jw32_at_PKBDLLHOOKSTRUCT, sizeof(PKBDLLHOOKSTRUCT));
        *ppHookStruct = (PKBDLLHOOKSTRUCT)lParam;
        argv[2] = janet_wrap_abstract(ppHookStruct);
        break;
    default:
        argv[2] = jw32_wrap_lparam(lParam);
        break;
    }

    Janet ret;

    if (jw32_pcall_fn(proc_fn, 3, argv, &ret)) {
        return jw32_unwrap_lresult(ret);
    } else {
        jw32_dbg_msg("jw32_pcall_fn() failed");
        return CallNextHookEx(0, nCode, wParam, lParam);
    }
}

LRESULT CALLBACK jw32_win_hook_msgfilter_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    /* TODO */
    (void)nCode;
    (void)wParam;
    (void)lParam;
    return 0;
}

LRESULT CALLBACK jw32_win_hook_journalrecord_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    /* TODO */
    (void)nCode;
    (void)wParam;
    (void)lParam;
    return 0;
}

LRESULT CALLBACK jw32_win_hook_journalplayback_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    /* TODO */
    (void)nCode;
    (void)wParam;
    (void)lParam;
    return 0;
}

LRESULT CALLBACK jw32_win_hook_keyboard_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    /* TODO */
    (void)nCode;
    (void)wParam;
    (void)lParam;
    return 0;
}

LRESULT CALLBACK jw32_win_hook_getmessage_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    /* TODO */
    (void)nCode;
    (void)wParam;
    (void)lParam;
    return 0;
}

LRESULT CALLBACK jw32_win_hook_callwndproc_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    /* TODO */
    (void)nCode;
    (void)wParam;
    (void)lParam;
    return 0;
}

LRESULT CALLBACK jw32_win_hook_cbt_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    /* TODO */
    (void)nCode;
    (void)wParam;
    (void)lParam;
    return 0;
}

LRESULT CALLBACK jw32_win_hook_sysmsgfilter_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    /* TODO */
    (void)nCode;
    (void)wParam;
    (void)lParam;
    return 0;
}

LRESULT CALLBACK jw32_win_hook_mouse_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    /* TODO */
    (void)nCode;
    (void)wParam;
    (void)lParam;
    return 0;
}

LRESULT CALLBACK jw32_win_hook_debug_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    /* TODO */
    (void)nCode;
    (void)wParam;
    (void)lParam;
    return 0;
}

LRESULT CALLBACK jw32_win_hook_shell_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    /* TODO */
    (void)nCode;
    (void)wParam;
    (void)lParam;
    return 0;
}

LRESULT CALLBACK jw32_win_hook_foregroundidle_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    /* TODO */
    (void)nCode;
    (void)wParam;
    (void)lParam;
    return 0;
}

LRESULT CALLBACK jw32_win_hook_callwndprocret_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    /* TODO */
    (void)nCode;
    (void)wParam;
    (void)lParam;
    return 0;
}

LRESULT CALLBACK jw32_win_hook_keyboard_ll_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
#ifdef JW32_CALLBACK_DEBUG
    jw32_dbg_msg("============= jw32_win_hook_keyboard_ll_proc ===============");
    jw32_dbg_val(nCode, "%d");
    jw32_dbg_val(wParam, "0x%" PRIx64);
    jw32_dbg_val(lParam, "%lld");
#endif

    return call_win_hook_proc(WH_KEYBOARD_LL, nCode, wParam, lParam);
}

LRESULT CALLBACK jw32_win_hook_mouse_ll_proc(int nCode, WPARAM wParam, LPARAM lParam)
{
    /* TODO */
    (void)nCode;
    (void)wParam;
    (void)lParam;
    return 0;
}

/* the hook types start from -1, thus the "+1" */
static const HOOKPROC win_hook_proc_map[] = {
    [WH_MSGFILTER + 1] = jw32_win_hook_msgfilter_proc,
    [WH_JOURNALRECORD + 1] = jw32_win_hook_journalrecord_proc,
    [WH_JOURNALPLAYBACK + 1] = jw32_win_hook_journalplayback_proc,
    [WH_KEYBOARD + 1] = jw32_win_hook_keyboard_proc,
    [WH_GETMESSAGE + 1] = jw32_win_hook_getmessage_proc,
    [WH_CALLWNDPROC + 1] = jw32_win_hook_callwndproc_proc,
    [WH_CBT + 1] = jw32_win_hook_cbt_proc,
    [WH_SYSMSGFILTER + 1] = jw32_win_hook_sysmsgfilter_proc,
    [WH_MOUSE + 1] = jw32_win_hook_mouse_proc,
    [8 + 1] = NULL, /* WH_HARDWARE */
    [WH_DEBUG + 1] = jw32_win_hook_debug_proc,
    [WH_SHELL + 1] = jw32_win_hook_shell_proc,
    [WH_FOREGROUNDIDLE + 1] = jw32_win_hook_foregroundidle_proc,
    [WH_CALLWNDPROCRET + 1] = jw32_win_hook_callwndprocret_proc,
    [WH_KEYBOARD_LL + 1] = jw32_win_hook_keyboard_ll_proc,
    [WH_MOUSE_LL + 1] = jw32_win_hook_mouse_ll_proc,
};


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

static void init_win_event_hook_registry(void)
{
    if (!win_event_hook_registry) {
        win_event_hook_registry = janet_table(0);
        janet_gcroot(janet_wrap_table(win_event_hook_registry));
    }
}

static Janet cfun_SetWinEventHook(int32_t argc, Janet *argv)
{
    DWORD eventMin, eventMax;
    HMODULE hmodWinEventProc;
    JanetFunction *win_event_proc_fn;
    DWORD idProcess, idThread, dwFlags;

    HWINEVENTHOOK hHook;

    janet_fixarity(argc, 7);

    eventMin = jw32_get_dword(argv, 0);
    eventMax = jw32_get_dword(argv, 1);
    hmodWinEventProc = jw32_get_handle(argv, 2);
    win_event_proc_fn = janet_getfunction(argv, 3);
    idProcess = jw32_get_dword(argv, 4);
    idThread = jw32_get_dword(argv, 5);
    dwFlags = jw32_get_dword(argv, 6);

    hHook = SetWinEventHook(eventMin, eventMax, hmodWinEventProc,
                           jw32_win_event_proc, idProcess, idThread, dwFlags);

    if (hHook) {
        Janet hook = jw32_wrap_handle(hHook);
        init_win_event_hook_registry();
        janet_table_put(win_event_hook_registry, hook, janet_wrap_function(win_event_proc_fn));
        jw32_dbg_jval(hook);
        jw32_dbg_jval(argv[3]);
    }

    return jw32_wrap_handle(hHook);
}

static Janet cfun_UnhookWinEvent(int32_t argc, Janet *argv)
{
    HWINEVENTHOOK hWinEventHook;

    BOOL bRet;

    janet_fixarity(argc, 1);

    hWinEventHook = jw32_get_handle(argv, 0);
    bRet = UnhookWinEvent(hWinEventHook);

    if (bRet && win_event_hook_registry) {
        Janet removed = janet_table_remove(win_event_hook_registry, jw32_wrap_handle(hWinEventHook));
        jw32_dbg_jval(removed);
    }

    return jw32_wrap_bool(bRet);
}

static void init_win_hook_registry(void)
{
    if (!win_hook_registry) {
        win_hook_registry = janet_table(0);
        janet_gcroot(janet_wrap_table(win_hook_registry));
    }
    if (!hhook_type_map) {
        hhook_type_map = janet_table(0);
        janet_gcroot(janet_wrap_table(hhook_type_map));
    }
}

static Janet cfun_SetWindowsHookEx(int32_t argc, Janet *argv)
{
    int idHook;
    JanetFunction *win_hook_proc_fn;
    //HOOKPROC lpfn;
    HINSTANCE hmod;
    DWORD dwThreadId;

    HHOOK hHook;

    janet_fixarity(argc, 4);

    idHook = jw32_get_int(argv, 0);
    if (idHook < WH_MINHOOK || idHook > WH_MAXHOOK) {
        janet_panicf("unknown hook type: %d", idHook);
    }
    win_hook_proc_fn = janet_getfunction(argv, 1);
    hmod = jw32_get_handle(argv, 2);
    dwThreadId = jw32_get_dword(argv, 3);

    /* the hook types start from -1, thus the "+1" */
    HOOKPROC hookProc = win_hook_proc_map[idHook + 1];
    if (!hookProc) {
        janet_panicf("unknown hook type: %d", idHook);
    }

    init_win_hook_registry();

    Janet hook = jw32_wrap_int(idHook);
    Janet proc_n_hhook_v = janet_table_get(win_hook_registry, hook);

    jw32_dbg_jval(hook);
    jw32_dbg_jval(argv[1]);
    jw32_dbg_jval(proc_n_hhook_v);

    if (!janet_checktype(proc_n_hhook_v, JANET_NIL)) {
        /* XXX: i failed to find a clean way to pass global states into
           hook functions, so only allow one hook function for each hook
           type here. existing hooks are removed first. */
        JanetTuple proc_n_hhook = janet_unwrap_tuple(proc_n_hhook_v);
        HHOOK hOldHook = jw32_unwrap_handle(proc_n_hhook[1]);
        BOOL bUnhook = UnhookWindowsHookEx(hOldHook);
        if (!bUnhook) {
            if (GetLastError() != ERROR_INVALID_HOOK_HANDLE) {
                janet_panicf("failed to remove old hook proc: 0x%x", GetLastError());
            }
            /* Otherwise assume the hook is already removed. */
        }
    }

    hHook = SetWindowsHookEx(idHook, hookProc, hmod, dwThreadId);

    if (hHook) {
        Janet reg_tuple[2] = {
            janet_wrap_function(win_hook_proc_fn),
            jw32_wrap_handle(hHook),
        };
        janet_table_put(win_hook_registry,
                        hook,
                        janet_wrap_tuple(janet_tuple_n(reg_tuple, 2)));
    }

    return jw32_wrap_handle(hHook);
}

static Janet cfun_UnhookWindowsHookEx(int32_t argc, Janet *argv)
{
    HHOOK hhk;

    BOOL bRet;

    janet_fixarity(argc, 1);

    hhk = jw32_get_handle(argv, 0);
    bRet = UnhookWindowsHookEx(hhk);
    if (bRet) {
        Janet typev = janet_table_remove(hhook_type_map, argv[0]);
        if (!janet_checktype(typev, JANET_NIL)) {
            janet_table_remove(win_hook_registry, typev);
        }
    }

    return jw32_wrap_bool(bRet);
}

static Janet cfun_CallNextHookEx(int32_t argc, Janet *argv)
{
    HHOOK hhk;
    int nCode;
    WPARAM wParam;
    LPARAM lParam;

    LRESULT lRet;

    janet_fixarity(argc, 4);

    hhk = jw32_get_handle(argv, 0);
    nCode = jw32_get_int(argv, 1);
    wParam = jw32_get_wparam(argv, 2);
    lParam = jw32_get_lparam(argv, 3);

    lRet = CallNextHookEx(hhk, nCode, wParam, lParam);
    jw32_dbg_val(lRet, "%lld");

    return jw32_wrap_lresult(lRet);
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

static Janet cfun_PeekMessage(int32_t argc, Janet *argv)
{
    MSG *lpMsg;
    HWND hWnd;
    UINT wMsgFilterMin, wMsgFilterMax;
    UINT wRemoveMsg;

    BOOL bRet;

    janet_fixarity(argc, 5);

    lpMsg = janet_getabstract(argv, 0, &jw32_at_MSG);
    hWnd = jw32_get_handle(argv, 1);
    wMsgFilterMin = jw32_get_uint(argv, 2);
    wMsgFilterMax = jw32_get_uint(argv, 3);
    wRemoveMsg = jw32_get_uint(argv, 4);

    bRet = PeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
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

static Janet cfun_PostMessage(int32_t argc, Janet *argv)
{
    HWND hWnd;
    UINT Msg;
    WPARAM wParam;
    LPARAM lParam;

    BOOL bRet;

    janet_fixarity(argc, 4);

    hWnd = jw32_get_handle(argv, 0);
    Msg = jw32_get_uint(argv, 1);
    wParam = jw32_get_wparam(argv, 2);
    lParam = jw32_get_lparam(argv, 3);

    bRet = PostMessage(hWnd, Msg, wParam, lParam);
    return jw32_wrap_bool(bRet);
}

static Janet cfun_SendMessage(int32_t argc, Janet *argv)
{
    HWND hWnd;
    UINT Msg;
    WPARAM wParam;
    LPARAM lParam;

    LRESULT lRet;

    janet_fixarity(argc, 4);

    hWnd = jw32_get_handle(argv, 0);
    Msg = jw32_get_uint(argv, 1);
    wParam = jw32_get_wparam(argv, 2);
    lParam = jw32_get_lparam(argv, 3);

    lRet = SendMessage(hWnd, Msg, wParam, lParam);
    return jw32_wrap_lresult(lRet);
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

static Janet cfun_DialogBox(int32_t argc, Janet *argv)
{
    HINSTANCE hInstance;
    LPCSTR lpTemplate;
    HWND hWndParent;
    JanetFunction *dlg_proc_fn = NULL;

    INT_PTR nRet;

    janet_fixarity(argc, 4);

    hInstance = jw32_get_handle(argv, 0);
    lpTemplate = jw32_get_lpcstr(argv, 1);
    hWndParent = jw32_get_handle(argv, 2);
    if (!janet_checktype(argv[3], JANET_NIL)) {
        dlg_proc_fn = janet_getfunction(argv, 3);
    }

    if (dlg_proc_fn) {
        /* we don't have an extra param to pass to the dlg proc,
           but include a NULL pointer here anyway, so that jw32_dlg_proc
           won't get confused. */

        LPARAM dlgParam;
        Janet param_tuple[2];

        param_tuple[0] = janet_wrap_function(dlg_proc_fn);
        param_tuple[1] = jw32_wrap_lparam((LPARAM)NULL);
        dlgParam = (LPARAM)janet_tuple_n(param_tuple, 2);

        nRet = DialogBoxParam(hInstance, lpTemplate, hWndParent, jw32_dlg_proc, dlgParam);
    } else {
        nRet = DialogBox(hInstance, lpTemplate, hWndParent, NULL);
    }
    return jw32_wrap_int_ptr(nRet);
}

static Janet cfun_CreateDialog(int32_t argc, Janet *argv)
{
    HINSTANCE hInstance;
    LPCSTR lpName;
    HWND hWndParent;
    JanetFunction *dlg_proc_fn = NULL;

    HWND hRet;

    janet_fixarity(argc, 4);

    hInstance = jw32_get_handle(argv, 0);
    lpName = jw32_get_lpcstr(argv, 1);
    hWndParent = jw32_get_handle(argv, 2);
    if (!janet_checktype(argv[3], JANET_NIL)) {
        dlg_proc_fn = janet_getfunction(argv, 3);
    }

    if (dlg_proc_fn) {
        LPARAM dlgParam;
        Janet param_tuple[2];

        param_tuple[0] = janet_wrap_function(dlg_proc_fn);
        param_tuple[1] = jw32_wrap_lparam((LPARAM)NULL);
        dlgParam = (LPARAM)janet_tuple_n(param_tuple, 2);

        hRet = CreateDialogParam(hInstance, lpName, hWndParent, jw32_dlg_proc, dlgParam);
    } else {
        hRet = CreateDialog(hInstance, lpName, hWndParent, NULL);
    }

    return jw32_wrap_handle(hRet);
}

static Janet cfun_IsDialogMessage(int32_t argc, Janet *argv)
{
    HWND hDlg;
    MSG *lpMsg;

    BOOL bRet;

    janet_fixarity(argc, 2);

    hDlg = jw32_get_handle(argv, 0);
    lpMsg = janet_getabstract(argv, 1, &jw32_at_MSG);

    bRet = IsDialogMessage(hDlg, lpMsg);
    return jw32_wrap_bool(bRet);
}

static Janet cfun_EndDialog(int32_t argc, Janet *argv)
{
    HWND hDlg;
    INT_PTR nResult;

    BOOL bRet;

    janet_fixarity(argc, 2);

    hDlg = jw32_get_handle(argv, 0);
    nResult = jw32_get_int_ptr(argv, 1);

    bRet = EndDialog(hDlg, nResult);
    return jw32_wrap_bool(bRet);
}

static Janet cfun_GetDesktopWindow(int32_t argc, Janet *argv)
{
    (void)argv;
    janet_fixarity(argc, 0);

    return jw32_wrap_handle(GetDesktopWindow());
}

static Janet cfun_GetAncestor(int32_t argc, Janet *argv)
{
    HWND hwnd;
    UINT gaFlags;

    janet_fixarity(argc, 2);

    hwnd = jw32_get_handle(argv, 0);
    gaFlags = jw32_get_uint(argv, 1);
    return jw32_wrap_handle(GetAncestor(hwnd, gaFlags));
}

static Janet cfun_GetParent(int32_t argc, Janet *argv)
{
    HWND hwnd;

    janet_fixarity(argc, 1);

    hwnd = jw32_get_handle(argv, 0);
    return jw32_wrap_handle(GetParent(hwnd));
}

static Janet cfun_CreateWindowEx(int32_t argc, Janet *argv)
{
    DWORD dwExStyle;
    LPCSTR lpClassName, lpWindowName;
    DWORD dwStyle;
    int x, y, nWidth, nHeight;
    HWND hWndParent;
    HMENU hMenu;
    HINSTANCE hInstance;
    LPVOID lpParam;

    HWND hWnd;

    Janet class_name;

    janet_fixarity(argc, 12);

    dwExStyle = jw32_get_dword(argv, 0);
    lpClassName = jw32_get_lpcstr(argv, 1);
    lpWindowName = jw32_get_lpcstr(argv, 2);
    dwStyle = jw32_get_dword(argv, 3);
    x = jw32_get_int(argv, 4);
    y = jw32_get_int(argv, 5);
    nWidth = jw32_get_int(argv, 6);
    nHeight = jw32_get_int(argv, 7);
    hWndParent = jw32_get_handle(argv, 8);
    hMenu = jw32_get_handle(argv, 9);
    hInstance = jw32_get_handle(argv, 10);

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
            jw32_dbg_jval(wnd_proc);
            lpParam = jw32_get_lpvoid(argv, 11);
            param_tuple[0] = wnd_proc;
            param_tuple[1] = jw32_wrap_lpvoid(lpParam);
            lpParam = (LPVOID)janet_tuple_n(param_tuple, 2);
        } else {
            lpParam = jw32_get_lpvoid(argv, 11);
        }
    } else {
        lpParam = jw32_get_lpvoid(argv, 11);
    }

    hWnd = CreateWindowEx(dwExStyle,
                          lpClassName, lpWindowName,
                          dwStyle,
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

    (void)len;

    jw32_dbg_msg("gcmark begin");

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

    jw32_dbg_msg("gcmark end");

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

static Janet cfun_GetProp(int32_t argc, Janet *argv)
{
    HWND hWnd;
    LPCSTR lpString;

    HANDLE hData;

    char buf[JW32_MAX_PROP_LEN];

    janet_fixarity(argc, 2);

    hWnd = jw32_get_handle(argv, 0);
    lpString = jw32_get_lpcstr(argv, 1); /* may be an atom */

    hData = GetProp(hWnd, lpString);

    if (hData) {
        if (check_atom(lpString)) {
            ATOM atom = lpcstr_to_atom(lpString);
            if (!GlobalGetAtomName(atom, buf, JW32_MAX_PROP_LEN)) {
                janet_panicf("inconsistent stat: atom 0x%hx not found", atom);
            }
            lpString = buf;
        }

        if (!strcmp(lpString, JW32_WND_PROC_FN_PROP_NAME)) {
            JanetFunction *wnd_proc_fn = (JanetFunction *)hData;
            return janet_wrap_function(wnd_proc_fn);
        } else {
            return jw32_wrap_handle(hData);
        }
    } else {
        return jw32_wrap_handle(NULL);
    }
}

static Janet cfun_SetProp(int32_t argc, Janet *argv)
{
    HWND hWnd;
    LPCSTR lpMaybeAtom, lpString;
    HANDLE hData;

    BOOL bRet;

    char buf[JW32_MAX_PROP_LEN];

    janet_fixarity(argc, 3);

    hWnd = jw32_get_handle(argv, 0);
    lpMaybeAtom = jw32_get_lpcstr(argv, 1);

    if (check_atom(lpMaybeAtom)) {
        ATOM atom = lpcstr_to_atom(lpMaybeAtom);
        if (!GlobalGetAtomName(atom, buf, JW32_MAX_PROP_LEN)) {
            lpString = NULL; /* can't find the atom string, don't bother checking it */
        } else {
            lpString = buf;
        }
    } else {
        lpString = lpMaybeAtom;
    }

    if (lpString) {
        if (!strcmp(lpString, JW32_WND_PROC_FN_PROP_NAME)) {
            JanetFunction *wnd_proc_fn = janet_getfunction(argv, 2);
            hData = (HANDLE)wnd_proc_fn;
        } else {
            hData = jw32_get_handle(argv, 2);
        }
    } else {
        hData = jw32_get_handle(argv, 2);
    }

    bRet = SetProp(hWnd, lpMaybeAtom, hData);
    return jw32_wrap_bool(bRet);
}

static Janet cfun_GetWindow(int32_t argc, Janet *argv)
{
    HWND hWnd;
    UINT uCmd;

    janet_fixarity(argc, 2);

    hWnd = jw32_get_handle(argv, 0);
    uCmd = jw32_get_uint(argv, 1);

    return jw32_wrap_handle(GetWindow(hWnd, uCmd));
}

static Janet cfun_GetWindowLong(int32_t argc, Janet *argv)
{
    HWND hWnd;
    int nIndex;

    janet_fixarity(argc, 2);

    hWnd = jw32_get_handle(argv, 0);
    nIndex = jw32_get_int(argv, 1);

    return jw32_wrap_long(GetWindowLong(hWnd, nIndex));
}

static Janet cfun_SetWindowLong(int32_t argc, Janet *argv)
{
    HWND hWnd;
    int nIndex;
    LONG dwNewLong;

    janet_fixarity(argc, 3);

    hWnd = jw32_get_handle(argv, 0);
    nIndex = jw32_get_int(argv, 1);
    dwNewLong = jw32_get_long(argv, 2);

    return jw32_wrap_long(SetWindowLong(hWnd, nIndex, dwNewLong));
}

static Janet cfun_GetWindowLongPtr(int32_t argc, Janet *argv)
{
    HWND hWnd;
    int nIndex;

    janet_fixarity(argc, 2);

    hWnd = jw32_get_handle(argv, 0);
    nIndex = jw32_get_int(argv, 1);

    return jw32_wrap_long_ptr(GetWindowLongPtr(hWnd, nIndex));
}

static Janet cfun_SetWindowLongPtr(int32_t argc, Janet *argv)
{
    HWND hWnd;
    int nIndex;
    LONG_PTR dwNewLong;

    janet_fixarity(argc, 3);

    hWnd = jw32_get_handle(argv, 0);
    nIndex = jw32_get_int(argv, 1);
    dwNewLong = jw32_get_long_ptr(argv, 2);

    return jw32_wrap_long_ptr(SetWindowLongPtr(hWnd, nIndex, dwNewLong));
}

static Janet cfun_GetWindowRect(int32_t argc, Janet *argv)
{
    HWND hWnd;

    RECT rect = {0, 0, 0, 0};
    Janet ret_tuple[2];

    janet_fixarity(argc, 1);

    hWnd = jw32_get_handle(argv, 0);

    ret_tuple[0] = jw32_wrap_bool(GetWindowRect(hWnd, &rect));
    ret_tuple[1] = janet_wrap_struct(jw32_rect_to_struct(&rect));

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}


static Janet cfun_SetForegroundWindow(int32_t argc, Janet *argv)
{
    HWND hWnd;

    janet_fixarity(argc, 1);

    hWnd = jw32_get_handle(argv, 0);
    return jw32_wrap_bool(SetForegroundWindow(hWnd));
}


static Janet cfun_GetForegroundWindow(int32_t argc, Janet *argv)
{
    (void)argv;
    janet_fixarity(argc, 0);
    return jw32_wrap_handle(GetForegroundWindow());
}


static Janet cfun_IsWindow(int32_t argc, Janet *argv)
{
    HWND hWnd;
    
    janet_fixarity(argc, 1);

    hWnd = jw32_get_handle(argv, 0);
    return jw32_wrap_bool(IsWindow(hWnd));
}


static Janet cfun_IsWindowVisible(int32_t argc, Janet *argv)
{
    HWND hWnd;
    
    janet_fixarity(argc, 1);

    hWnd = jw32_get_handle(argv, 0);
    return jw32_wrap_bool(IsWindowVisible(hWnd));
}


static Janet cfun_IsIconic(int32_t argc, Janet *argv)
{
    HWND hWnd;
    
    janet_fixarity(argc, 1);

    hWnd = jw32_get_handle(argv, 0);
    return jw32_wrap_bool(IsIconic(hWnd));
}


static Janet cfun_IsZoomed(int32_t argc, Janet *argv)
{
    HWND hWnd;
    
    janet_fixarity(argc, 1);

    hWnd = jw32_get_handle(argv, 0);
    return jw32_wrap_bool(IsZoomed(hWnd));
}


static Janet cfun_SetWindowPos(int32_t argc, Janet *argv)
{
    HWND hWnd;
    HWND hWndInsertAfter;
    int X, Y, cx, cy;
    UINT uFlags;

    janet_fixarity(argc, 7);

    hWnd = jw32_get_handle(argv, 0);
    hWndInsertAfter = jw32_get_handle(argv, 1);
    X = jw32_get_int(argv, 2);
    Y = jw32_get_int(argv, 3);
    cx = jw32_get_int(argv, 4);
    cy = jw32_get_int(argv, 5);
    uFlags = jw32_get_uint(argv, 6);

    return jw32_wrap_bool(SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags));
}


static Janet cfun_GetWindowThreadProcessId(int32_t argc, Janet *argv)
{
    HWND hWnd;

    DWORD threadId = 0;
    DWORD processId = 0;
    Janet ret_tuple[2];
    
    janet_fixarity(argc, 1);

    hWnd = jw32_get_handle(argv, 0);

    threadId = GetWindowThreadProcessId(hWnd, &processId);
    ret_tuple[0] = jw32_wrap_dword(threadId);
    ret_tuple[1] = jw32_wrap_dword(processId);

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}


static Janet cfun_EnumChildWindows(int32_t argc, Janet *argv)
{
    HWND hWndParent;
    JanetFunction *enum_fn;
    /* janet has closures, we don't need this */
    //LPARAM lParam;

    BOOL bRet;

    janet_fixarity(argc, 2);

    hWndParent = jw32_get_handle(argv, 0);
    enum_fn = janet_getfunction(argv, 1);

    bRet = EnumChildWindows(hWndParent, jw32_child_window_enum_proc, (LPARAM)enum_fn);

    return jw32_wrap_bool(bRet);
}


static Janet cfun_SetLayeredWindowAttributes(int32_t argc, Janet *argv)
{
    HWND hwnd;
    COLORREF crKey;
    BYTE bAlpha;
    DWORD dwFlags;

    int32_t alpha_val;

    BOOL bRet;

    janet_fixarity(argc, 4);

    hwnd = jw32_get_handle(argv, 0);
    crKey = jw32_get_dword(argv, 1);
    alpha_val = jw32_get_int(argv, 2);
    if (alpha_val < 0 || alpha_val > 255) {
        janet_panicf("bad slot #2: expected an unsigned 8-bit integer, got %v", argv[2]);
    }
    bAlpha = (BYTE)alpha_val;
    dwFlags = jw32_get_dword(argv, 3);

    bRet = SetLayeredWindowAttributes(hwnd, crKey, bAlpha, dwFlags);

    return jw32_wrap_bool(bRet);
}


static Janet cfun_GetLayeredWindowAttributes(int32_t argc, Janet *argv)
{
    HWND hwnd;

    BOOL bRet;

    COLORREF crKey;
    BYTE bAlpha;
    DWORD dwFlags;

    janet_fixarity(argc, 1);

    hwnd = jw32_get_handle(argv, 0);
    bRet = GetLayeredWindowAttributes(hwnd, &crKey, &bAlpha, &dwFlags);

    if (bRet) {
        Janet ret_tuple[3] = {
            jw32_wrap_dword(crKey),
            jw32_wrap_int(bAlpha),
            jw32_wrap_dword(dwFlags),
        };
        return janet_wrap_tuple(janet_tuple_n(ret_tuple, 3));
    }
    return janet_wrap_nil();
}


/*******************************************************************
 *
 * INPUT
 *
 *******************************************************************/

static int RAWINPUT_get(void *p, Janet key, Janet *out)
{
    jw32_rawinput_t *jpri = (jw32_rawinput_t *)p;
    RAWINPUT *pri = &(jpri->ri);

    if (!janet_checktype(key, JANET_KEYWORD)) {
        janet_panicf("expected keyword, got %v", key);
    }

    const uint8_t *kw = janet_unwrap_keyword(key);

#define __get_member(member, type) do {                 \
        if (!janet_cstrcmp(kw, #member)) {              \
            *out = jw32_wrap_##type(pri->member);       \
            return 1;                                   \
        }                                               \
    } while (0)

    if (!janet_cstrcmp(kw, "size")) {
        *out = jw32_wrap_dword(jpri->ri_size);
        return 1;
    }

    /* header */
    __get_member(header.dwType, dword);
    __get_member(header.dwSize, dword);
    __get_member(header.hDevice, handle);
    __get_member(header.wParam, wparam);

    /* mouse */
    __get_member(data.mouse.usFlags, word);
    __get_member(data.mouse.ulButtons, ulong);
    __get_member(data.mouse.usButtonFlags, word);
    __get_member(data.mouse.usButtonData, word);
    __get_member(data.mouse.ulRawButtons, ulong);
    __get_member(data.mouse.lLastX, long);
    __get_member(data.mouse.lLastY, long);
    __get_member(data.mouse.ulExtraInformation, ulong);

    /* keyboard */
    __get_member(data.keyboard.MakeCode, word);
    __get_member(data.keyboard.Flags, word);
    __get_member(data.keyboard.Reserved, word);
    __get_member(data.keyboard.VKey, word);
    __get_member(data.keyboard.Message, uint);
    __get_member(data.keyboard.ExtraInformation, ulong);

    /* hid */
    __get_member(data.hid.dwSizeHid, dword);
    __get_member(data.hid.dwCount, dword);
    /* TODO */
    //__get_member(data.hid.bRawData, ...);

#undef __get_member

    return 0;
}

static const JanetAbstractType jw32_at_RAWINPUT = {
    .name = MOD_NAME "/RAWINPUT",
    .gc = NULL,
    .gcmark = NULL,
    .get = RAWINPUT_get,
    JANET_ATEND_GET
};

static Janet cfun_RAWINPUT(int32_t argc, Janet *argv)
{
    if ((argc & 1) != 0) {
        janet_panicf("expected even number of arguments, got %d", argc);
    }

    DWORD dwSize = sizeof(RAWINPUT);

    for (int32_t k = 0, v = 1; k < argc; k += 2, v += 2) {
        const uint8_t *kw = janet_getkeyword(argv, k);
        if (!janet_cstrcmp(kw, "header.dwSize") || !janet_cstrcmp(kw, "size")) {
            dwSize = jw32_get_dword(argv, v);
            break;
        }
    }

    jw32_rawinput_t *jpri = janet_abstract(&jw32_at_RAWINPUT, dwSize + JW32_RAWINPUT_T_HEADER_SIZE);
    memset(jpri, 0, dwSize + JW32_RAWINPUT_T_HEADER_SIZE);
    jpri->ri_size = dwSize;

    RAWINPUT *pri = &(jpri->ri);
    pri->header.dwSize = dwSize;

    for (int32_t k = 0, v = 1; k < argc; k += 2, v += 2) {
        const uint8_t *kw = janet_getkeyword(argv, k);

#define __set_member(member, type)                      \
        if (!janet_cstrcmp(kw, #member)) {              \
            pri->member = jw32_get_##type(argv, v);     \
            continue;                                   \
        }

        if (!janet_cstrcmp(kw, "size")) {
            /* already set */
            continue;
        }

        /* header */
        __set_member(header.dwType, dword)
        if (!janet_cstrcmp(kw, "header.dwSize")) {
            /* already set */
            continue;
        }
        __set_member(header.hDevice, handle)
        __set_member(header.wParam, wparam)

        /* mouse */
        __set_member(data.mouse.usFlags, word)
        __set_member(data.mouse.ulButtons, ulong)
        __set_member(data.mouse.usButtonFlags, word)
        __set_member(data.mouse.usButtonData, word)
        __set_member(data.mouse.ulRawButtons, ulong)
        __set_member(data.mouse.lLastX, long)
        __set_member(data.mouse.lLastY, long)
        __set_member(data.mouse.ulExtraInformation, ulong)

        /* keyboard */
        __set_member(data.keyboard.MakeCode, word)
        __set_member(data.keyboard.Flags, word)
        __set_member(data.keyboard.Reserved, word)
        __set_member(data.keyboard.VKey, word)
        __set_member(data.keyboard.Message, uint)
        __set_member(data.keyboard.ExtraInformation, ulong)

        /* hid */
        __set_member(data.hid.dwSizeHid, dword)
        __set_member(data.hid.dwCount, dword)
        /* TODO */
        //__set_member(data.hid.bRawData, ...)

#undef __set_member

        janet_panicf("unknown key %v", argv[k]);
    }

    return janet_wrap_abstract(jpri);
}

static int RAWINPUTDEVICE_get(void *p, Janet key, Janet *out)
{
    RAWINPUTDEVICE *prid = (RAWINPUTDEVICE *)p;

    if (!janet_checktype(key, JANET_KEYWORD)) {
        janet_panicf("expected keyword, got %v", key);
    }

    const uint8_t *kw = janet_unwrap_keyword(key);

#define __get_member(member, type) do {                 \
        if (!janet_cstrcmp(kw, #member)) {              \
            *out = jw32_wrap_##type(prid->member);      \
            return 1;                                   \
        }                                               \
    } while (0)

    __get_member(usUsagePage, word);
    __get_member(usUsage, word);
    __get_member(dwFlags, dword);
    __get_member(hwndTarget, handle);

#undef __get_member

    return 0;
}

static const JanetAbstractType jw32_at_RAWINPUTDEVICE = {
    .name = MOD_NAME "/RAWINPUTDEVICE",
    .gc = NULL,
    .gcmark = NULL,
    .get = RAWINPUTDEVICE_get,
    JANET_ATEND_GET
};

static Janet cfun_RAWINPUTDEVICE(int32_t argc, Janet *argv)
{
    if ((argc & 1) != 0) {
        janet_panicf("expected even number of arguments, got %d", argc);
    }

    RAWINPUTDEVICE *prid = janet_abstract(&jw32_at_RAWINPUTDEVICE, sizeof(RAWINPUTDEVICE));
    memset(prid, 0, sizeof(RAWINPUTDEVICE));

    for (int32_t k = 0, v = 1; k < argc; k += 2, v += 2) {
        const uint8_t *kw = janet_getkeyword(argv, k);

#define __set_member(member, type)                       \
        if (!janet_cstrcmp(kw, #member)) {               \
            prid->member = jw32_get_##type(argv, v);     \
            continue;                                    \
        }

        __set_member(usUsagePage, word)
        __set_member(usUsage, word)
        __set_member(dwFlags, dword)
        __set_member(hwndTarget, handle)

#undef __set_member

        janet_panicf("unknown key %v", argv[k]);
    }

    return janet_wrap_abstract(prid);
}

static Janet cfun_RegisterRawInputDevices(int32_t argc, Janet *argv)
{
    JanetView raw_input_devices;

    BOOL bRet;

    RAWINPUTDEVICE *arrRid;

    janet_fixarity(argc, 1);

    raw_input_devices = janet_getindexed(argv, 0);
    arrRid = GlobalAlloc(GPTR, sizeof(RAWINPUTDEVICE) * raw_input_devices.len);
    if (!arrRid) {
        JANET_OUT_OF_MEMORY;
    }

    for (int32_t i = 0; i < raw_input_devices.len; i++) {
        Janet item = raw_input_devices.items[i];
        if (!janet_checkabstract(item, &jw32_at_RAWINPUTDEVICE)) {
            GlobalFree(arrRid);
            janet_panicf("expected a RAWINPUTDEVICE, got %v", item);
        }
        RAWINPUTDEVICE *prid = janet_unwrap_abstract(item);
        memcpy(&(arrRid[i]), prid, sizeof(RAWINPUTDEVICE));
    }

    bRet = RegisterRawInputDevices(arrRid, raw_input_devices.len, sizeof(RAWINPUTDEVICE));

    GlobalFree(arrRid);

    return jw32_wrap_bool(bRet);
}

static Janet cfun_GetRegisteredRawInputDevices(int32_t argc, Janet *argv)
{
    JanetArray *rid_arr;

    UINT uiRet = 0;

    RAWINPUTDEVICE *prid = NULL;
    UINT uiNumDevices = 0;

    janet_fixarity(argc, 1);

    rid_arr = janet_getarray(argv, 0);

    uiRet = GetRegisteredRawInputDevices(NULL, &uiNumDevices, sizeof(RAWINPUTDEVICE));
    jw32_dbg_val(uiNumDevices, "%u");

    if (uiNumDevices > 0) {
        prid = GlobalAlloc(GPTR, sizeof(RAWINPUTDEVICE) * uiNumDevices);
        uiRet = GetRegisteredRawInputDevices(prid, &uiNumDevices, sizeof(RAWINPUTDEVICE));
        jw32_dbg_val(uiRet, "%u");
    }

    if (uiRet != (UINT)(-1)) {
        janet_array_setcount(rid_arr, 0);
        for (UINT i = 0; i < uiRet; i++) {
            RAWINPUTDEVICE *item = janet_abstract(&jw32_at_RAWINPUTDEVICE, sizeof(RAWINPUTDEVICE));
            memcpy(item, &(prid[i]), sizeof(RAWINPUTDEVICE));
            janet_array_push(rid_arr, janet_wrap_abstract(item));
        }
    }

    if (prid) {
        GlobalFree(prid);
    }

    return jw32_wrap_uint(uiRet);
}

static Janet cfun_GetRawInputData(int32_t argc, Janet *argv)
{
    HRAWINPUT hRawInput;
    UINT uiCommand;
    jw32_rawinput_t *jpri;

    UINT uiRet;
    UINT cbSize = 0;
    Janet ret_tuple[2];

    RAWINPUT *pri;

    janet_fixarity(argc, 3);

    hRawInput = jw32_get_handle(argv, 0);
    uiCommand = jw32_get_uint(argv, 1);
    if (janet_checktype(argv[2], JANET_NIL)) {
        jpri = NULL;
        pri = NULL;
    } else {
        jpri = janet_getabstract(argv, 2, &jw32_at_RAWINPUT);
        pri = &(jpri->ri);
        cbSize = jpri->ri_size;
    }
    jw32_dbg_val((uint64_t)pri, "0x%llx");
    jw32_dbg_val(cbSize, "%u");

    uiRet = GetRawInputData(hRawInput, uiCommand, pri, &cbSize, sizeof(RAWINPUTHEADER));
    jw32_dbg_val(uiRet, "%u");
    jw32_dbg_val(cbSize, "%u");

    ret_tuple[0] = jw32_wrap_uint(uiRet);
    ret_tuple[1] = jw32_wrap_uint(cbSize);

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static Janet cfun_GetRawInputDeviceInfo(int32_t argc, Janet *argv)
{
    HANDLE hDevice;
    UINT uiCommand;

    UINT uiRet = 0;

    janet_fixarity(argc, 3);

    hDevice = jw32_get_handle(argv, 0);
    uiCommand = jw32_get_uint(argv, 1);

    switch (uiCommand) {
    case RIDI_DEVICENAME: {
        JanetBuffer *buf = janet_getbuffer(argv, 2);
        UINT cbSize = 0;
        uiRet = GetRawInputDeviceInfo(hDevice, uiCommand, NULL, &cbSize);
        if (0 == uiRet) {
            jw32_dbg_val(cbSize, "%u");
            janet_buffer_ensure(buf, cbSize * sizeof(TCHAR), 1);
            uiRet = GetRawInputDeviceInfo(hDevice, uiCommand, buf->data, &cbSize);
            if (uiRet != (UINT)(-1)) {
                /* exclude trailing null byte */
                buf->count = (uiRet - 1) * sizeof(TCHAR);
            }
        }
        break;
    }
    default:
        janet_panicf("unsupported command: 0x%x", uiCommand);
    }

    return jw32_wrap_uint(uiRet);
}

static int INPUT_get(void *p, Janet key, Janet *out)
{
    INPUT *input = (INPUT *)p;

    if (!janet_checktype(key, JANET_KEYWORD)) {
        janet_panicf("expected keyword, got %v", key);
    }

    const uint8_t *kw = janet_unwrap_keyword(key);

#define __get_member(member, type) do {                 \
        if (!janet_cstrcmp(kw, #member)) {              \
            *out = jw32_wrap_##type(input->member);     \
            return 1;                                   \
        }                                               \
    } while (0)

    __get_member(type, dword);

    /* mouse input */
    __get_member(mi.dx, long);
    __get_member(mi.dy, long);
    __get_member(mi.mouseData, dword);
    __get_member(mi.dwFlags, dword);
    __get_member(mi.time, dword);
    __get_member(mi.dwExtraInfo, ulong_ptr);

    /* keyboard input */
    __get_member(ki.wVk, word);
    __get_member(ki.wScan, word);
    __get_member(ki.dwFlags, dword);
    __get_member(ki.time, dword);
    __get_member(ki.dwExtraInfo, ulong_ptr);

    /* hardware input */
    __get_member(hi.uMsg, dword);
    __get_member(hi.wParamL, word);
    __get_member(hi.wParamH, word);

#undef __get_member

    return 0;
}

static const JanetAbstractType jw32_at_INPUT = {
    .name = MOD_NAME "/INPUT",
    .gc = NULL,
    .gcmark = NULL,
    .get = INPUT_get,
    JANET_ATEND_GET
};

static Janet cfun_INPUT(int32_t argc, Janet *argv)
{
    if ((argc & 1) != 0) {
        janet_panicf("expected even number of arguments, got %d", argc);
    }

    INPUT *input = janet_abstract(&jw32_at_INPUT, sizeof(INPUT));
    memset(input, 0, sizeof(INPUT));

    for (int32_t k = 0, v = 1; k < argc; k += 2, v += 2) {
        const uint8_t *kw = janet_getkeyword(argv, k);

#define __set_member(member, type)                    \
        if (!janet_cstrcmp(kw, #member)) {            \
            input->member = jw32_get_##type(argv, v); \
            continue;                                 \
        }

        __set_member(type, dword)

        /* mouse input */
        __set_member(mi.dx, long)
        __set_member(mi.dy, long)
        __set_member(mi.mouseData, dword)
        __set_member(mi.dwFlags, dword)
        __set_member(mi.time, dword)
        __set_member(mi.dwExtraInfo, ulong_ptr)

        /* keyboard input */
        __set_member(ki.wVk, word)
        __set_member(ki.wScan, word)
        __set_member(ki.dwFlags, dword)
        __set_member(ki.time, dword)
        __set_member(ki.dwExtraInfo, ulong_ptr)

        /* hardware input */
        __set_member(hi.uMsg, dword)
        __set_member(hi.wParamL, word)
        __set_member(hi.wParamH, word)

#undef __set_member

        janet_panicf("unknown key %v", argv[k]);
    }

    return janet_wrap_abstract(input);
}

static Janet cfun_SendInput(int32_t argc, Janet *argv)
{
    JanetView input_list;

    UINT uiRet;

    INPUT *pInputs;

    janet_fixarity(argc, 1);

    input_list = janet_getindexed(argv, 0);
    pInputs = GlobalAlloc(GPTR, input_list.len * sizeof(INPUT));
    for (int i = 0; i < input_list.len; i++) {
        Janet item = input_list.items[i];
        if (!janet_checkabstract(item, &jw32_at_INPUT)) {
            GlobalFree(pInputs);
            janet_panicf("expected an INPUT, got %v", item);
        }
        memcpy(&(pInputs[i]), janet_unwrap_abstract(item), sizeof(INPUT));
    }

    uiRet = SendInput(input_list.len, pInputs, sizeof(INPUT));
    GlobalFree(pInputs);
    return jw32_wrap_uint(uiRet);
}

static Janet cfun_GetPhysicalCursorPos(int32_t argc, Janet *argv)
{
    (void)argv;

    BOOL bRet;
    POINT pt = {0, 0};
    Janet ret_tuple[2];

    janet_fixarity(argc, 0);

    bRet = GetPhysicalCursorPos(&pt);
    ret_tuple[0] = jw32_wrap_bool(bRet);
    ret_tuple[1] = janet_wrap_tuple(jw32_point_to_tuple(&pt));

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static Janet cfun_GetCursorPos(int32_t argc, Janet *argv)
{
    (void)argv;

    BOOL bRet;
    POINT pt = {0, 0};
    Janet ret_tuple[2];

    janet_fixarity(argc, 0);

    bRet = GetCursorPos(&pt);
    ret_tuple[0] = jw32_wrap_bool(bRet);
    ret_tuple[1] = janet_wrap_tuple(jw32_point_to_tuple(&pt));

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static Janet cfun_SetCursorPos(int32_t argc, Janet *argv)
{
    int x, y;

    BOOL bRet;

    janet_fixarity(argc, 2);

    x = jw32_get_int(argv, 0);
    y = jw32_get_int(argv, 1);

    bRet = SetCursorPos(x, y);
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

static Janet cfun_LoadImage(int32_t argc, Janet *argv)
{
    HINSTANCE hInst;
    LPCSTR name;
    UINT type;
    int cx, cy;
    UINT fuLoad;

    HANDLE hRet;

    janet_fixarity(argc, 6);

    hInst = jw32_get_handle(argv, 0);
    name = jw32_get_lpcstr(argv, 1);
    type = jw32_get_uint(argv, 2);
    cx = jw32_get_int(argv, 3);
    cy = jw32_get_int(argv, 4);
    fuLoad = jw32_get_uint(argv, 5);

    hRet = LoadImage(hInst, name, type, cx, cy, fuLoad);
    return jw32_wrap_handle(hRet);
}

static Janet cfun_CreateMenu(int32_t argc, Janet *argv)
{
    (void)argv;
    janet_fixarity(argc, 0);

    return jw32_wrap_handle(CreateMenu());
}

static Janet cfun_CreatePopupMenu(int32_t argc, Janet *argv)
{
    (void)argv;
    janet_fixarity(argc, 0);

    return jw32_wrap_handle(CreatePopupMenu());
}

static Janet cfun_DestroyMenu(int32_t argc, Janet *argv)
{
    HMENU hMenu;

    BOOL bRet;

    janet_fixarity(argc, 1);

    hMenu = jw32_get_handle(argv, 0);
    bRet = DestroyMenu(hMenu);
    return jw32_wrap_bool(bRet);
}

static Janet cfun_AppendMenu(int32_t argc, Janet *argv)
{
    HMENU hMenu;
    UINT uFlags;
    UINT_PTR uIDNewItem;
    LPCSTR lpNewItem;

    BOOL bRet;

    janet_fixarity(argc, 4);

    hMenu = jw32_get_handle(argv, 0);
    uFlags = jw32_get_uint(argv, 1);
    uIDNewItem = jw32_get_uint_ptr(argv, 2);
    lpNewItem = jw32_get_lpcstr(argv, 3);

    bRet = AppendMenu(hMenu, uFlags, uIDNewItem, lpNewItem);
    return jw32_wrap_bool(bRet);
}

static Janet cfun_SetMenu(int32_t argc, Janet *argv)
{
    HWND hWnd;
    HMENU hMenu;

    BOOL bRet;

    janet_fixarity(argc, 2);

    hWnd = jw32_get_handle(argv, 0);
    hMenu = jw32_get_handle(argv, 1);

    bRet = SetMenu(hWnd, hMenu);
    return jw32_wrap_bool(bRet);
}


static Janet cfun_TrackPopupMenuEx(int32_t argc, Janet *argv)
{
    HMENU hMenu;
    UINT uFlags;
    int x, y;
    HWND hwnd;
    LPTPMPARAMS lptpm;

    BOOL bRet;

    TPMPARAMS tpm;

    janet_fixarity(argc, 6);

    hMenu = jw32_get_handle(argv, 0);
    uFlags = jw32_get_uint(argv, 1);
    x = jw32_get_int(argv, 2);
    y = jw32_get_int(argv, 3);
    hwnd = jw32_get_handle(argv, 4);

    memset(&tpm, 0, sizeof(tpm));
    tpm.cbSize = sizeof(tpm);
    if (janet_checktype(argv[5], JANET_NIL)) {
        lptpm = NULL;
    } else {
        if (janet_checktype(argv[5], JANET_TABLE)) {
            JanetTable *tbl_rect = janet_unwrap_table(argv[5]);
            Janet val;

#define __extract_rect_field(name)                                      \
            val = janet_table_get(tbl_rect, janet_ckeywordv(#name));    \
            if (janet_checkint(val)) {                                  \
                tpm.rcExclude.##name## = janet_unwrap_integer(val);     \
            } else {                                                    \
                tpm.rcExclude.##name## = 0;                             \
            }

            __extract_rect_field(left);
            __extract_rect_field(top);
            __extract_rect_field(right);
            __extract_rect_field(bottom);

#undef __extract_rect_field

            lptpm = &tpm;
        } else {
            janet_panicf("bad slot #%d: expected a table or nil, got %v", 5, argv[5]);
        }
    }

    bRet = TrackPopupMenuEx(hMenu, uFlags, x, y, hwnd, lptpm);
    return jw32_wrap_bool(bRet);
}


/*******************************************************************
 *
 * MISC.
 *
 *******************************************************************/

static Janet cfun_SetProcessDPIAware(int32_t argc, Janet *argv)
{
    (void)argv;
    janet_fixarity(argc, 0);
    return jw32_wrap_bool(SetProcessDPIAware());
}

static Janet cfun_EnumDisplayMonitors(int32_t argc, Janet *argv)
{
    HDC hdc;
    LPCRECT lprcClip;
    JanetFunction *enum_fn;
    /* janet has closures, we don't need this */
    //LPARAM dwData;

    BOOL bRet;

    RECT rcClip;

    janet_fixarity(argc, 3);

    hdc = jw32_get_handle(argv, 0);

    switch (janet_type(argv[1])) {

    case JANET_POINTER: {
        lprcClip = (LPCRECT)janet_unwrap_pointer(argv[1]);
        if (lprcClip != NULL) {
            janet_panicf("bad slot #1: only accept NULL for pointers, got %v", argv[1]);
        }
        break;
    }

    case JANET_NIL:
        lprcClip = NULL;
        break;

    default:
        rcClip = jw32_get_rect(argv, 1);
        lprcClip = &rcClip;
        break;
    }

    enum_fn = janet_getfunction(argv, 2);

    bRet = EnumDisplayMonitors(hdc, lprcClip, jw32_monitor_enum_proc, (LPARAM)enum_fn);

    return jw32_wrap_bool(bRet);
}

static int MONITORINFOEX_get(void *p, Janet key, Janet *out)
{
    MONITORINFOEX *lpmi = (MONITORINFOEX *)p;

    if (!janet_checktype(key, JANET_KEYWORD)) {
        janet_panicf("expected keyword, got %v", key);
    }

    const uint8_t *kw = janet_unwrap_keyword(key);

#define __get_member(name, exp)                 \
    if (!janet_cstrcmp(kw, name)) {             \
        *out = (exp);                           \
        return 1;                               \
    }

    __get_member("cbSize", jw32_wrap_dword(lpmi->cbSize));
    __get_member("rcMonitor", janet_wrap_struct(jw32_rect_to_struct(&(lpmi->rcMonitor))));
    __get_member("rcWork", janet_wrap_struct(jw32_rect_to_struct(&(lpmi->rcWork))));
    __get_member("dwFlags", jw32_wrap_dword(lpmi->dwFlags));
    __get_member("szDevice", janet_cstringv(lpmi->szDevice));

#undef __get_member

    return 0;
}

static const JanetAbstractType jw32_at_MONITORINFOEX = {
    .name = MOD_NAME "/MONITORINFOEX",
    .gc = NULL,
    .gcmark = NULL,
    .get = MONITORINFOEX_get,
    JANET_ATEND_GET
};

static Janet cfun_MONITORINFOEX(int32_t argc, Janet *argv)
{
    if ((argc & 1) != 0) {
        janet_panicf("expected even number of arguments, got %d", argc);
    }
    
    MONITORINFOEX *lpmi = janet_abstract(&jw32_at_MONITORINFOEX, sizeof(MONITORINFOEX));
    memset(lpmi, 0, sizeof(MONITORINFOEX));

    BOOL bSizeSet = FALSE;

    for (int32_t k = 0, v = 1; k < argc; k += 2, v += 2) {
        const uint8_t *kw = janet_getkeyword(argv, k);

#define __set_member(member, type)                      \
        if (!janet_cstrcmp(kw, #member)) {              \
            lpmi->member = jw32_get_##type(argv, v);    \
            continue;                                   \
        }

        if (!janet_cstrcmp(kw, "cbSize")) {
            lpmi->cbSize = jw32_get_dword(argv, v);
            bSizeSet = TRUE;
            continue;
        }
        __set_member(rcMonitor, rect)
        __set_member(rcWork, rect)
        __set_member(dwFlags, dword)
        if (!janet_cstrcmp(kw, "szDevice")) {
            const uint8_t *name = NULL;
            uint32_t name_len = 0;
            switch (janet_type(argv[v])) {
            case JANET_STRING: {
                name = janet_unwrap_string(argv[v]);
                name_len = janet_string_length(name);
                break;
            }
            case JANET_BUFFER: {
                JanetBuffer *buf = janet_unwrap_buffer(argv[v]);
                name = buf->data;
                name_len = buf->count;
                break;
            }
            default:
                janet_panicf("invalid value for szDevice: %v", argv[v]);
            }
            name_len = name_len < (CCHDEVICENAME - 1) ? name_len : (CCHDEVICENAME - 1);
            memcpy(lpmi->szDevice, name, name_len);
            lpmi->szDevice[name_len] = 0;
        }

#undef __set_member
    }

    if (!bSizeSet) {
        lpmi->cbSize = sizeof(MONITORINFOEX);
    }

    return janet_wrap_abstract(lpmi);
}

static Janet cfun_GetMonitorInfo(int32_t argc, Janet *argv)
{
    HMONITOR hMonitor;
    LPMONITORINFO lpmi;

    BOOL bRet;

    janet_fixarity(argc, 2);

    hMonitor = jw32_get_handle(argv, 0);
    lpmi = janet_getabstract(argv, 1, &jw32_at_MONITORINFOEX);

    bRet = GetMonitorInfo(hMonitor, lpmi);

    return jw32_wrap_bool(bRet);
}


static Janet cfun_MonitorFromPoint(int32_t argc, Janet *argv)
{
    JanetView pt_view;
    DWORD dwFlags;

    HMONITOR hMonitor;

    POINT pt;

    janet_fixarity(argc, 2);

    pt_view = janet_getindexed(argv, 0);
    dwFlags = jw32_get_dword(argv, 1);

    pt = jw32_view_to_point(pt_view);

    hMonitor = MonitorFromPoint(pt, dwFlags);
    return jw32_wrap_handle(hMonitor);
}


static Janet cfun_MonitorFromRect(int32_t argc, Janet *argv)
{
    RECT rect;
    DWORD dwFlags;

    HMONITOR hMonitor;

    janet_fixarity(argc, 2);

    rect = jw32_get_rect(argv, 0);
    dwFlags = jw32_get_dword(argv, 1);

    hMonitor = MonitorFromRect(&rect, dwFlags);
    return jw32_wrap_handle(hMonitor);
}


static Janet cfun_MonitorFromWindow(int32_t argc, Janet *argv)
{
    HWND hwnd;
    DWORD dwFlags;

    HMONITOR hMonitor;

    janet_fixarity(argc, 2);

    hwnd = jw32_get_handle(argv, 0);
    dwFlags = jw32_get_dword(argv, 1);

    hMonitor = MonitorFromWindow(hwnd, dwFlags);
    return jw32_wrap_handle(hMonitor);
}


static Janet cfun_GetSystemMetrics(int32_t argc, Janet *argv)
{
    int nIndex;
    janet_fixarity(argc, 1);
    nIndex = jw32_get_int(argv, 0);
    return jw32_wrap_int(GetSystemMetrics(nIndex));
}


static Janet cfun_SystemParametersInfo(int32_t argc, Janet *argv)
{
    UINT uiAction;
    UINT uiParam;
    UINT fWinIni;

    BOOL bSpiRet;

    janet_fixarity(argc, 4);

    uiAction = jw32_get_uint(argv, 0);
    uiParam = jw32_get_uint(argv, 1);
    /* The third argument is parsed according to uiAction below */;
    fWinIni = jw32_get_uint(argv, 3);

    switch (uiAction) {
    case SPI_GETFOREGROUNDLOCKTIMEOUT: {
        DWORD dwRet = 0;
        Janet ret[2];
        bSpiRet = SystemParametersInfo(uiAction, uiParam, &dwRet, fWinIni);
        ret[0] = jw32_wrap_bool(bSpiRet);
        ret[1] = jw32_wrap_dword(dwRet);
        return janet_wrap_tuple(janet_tuple_n(ret, 2));
    }
    case SPI_SETFOREGROUNDLOCKTIMEOUT: {
        UINT_PTR param = jw32_get_dword(argv, 2);
        bSpiRet = SystemParametersInfo(uiAction, uiParam, (PVOID)param, fWinIni);
        return jw32_wrap_bool(bSpiRet);
    }
    default:
        janet_panicf("unsupported action: %x", uiAction);
    }
}


static Janet cfun_SetTimer(int32_t argc, Janet *argv)
{
    HWND hWnd;
    UINT_PTR nIDEvent;
    UINT uElapse;
    TIMERPROC lpTimerFunc;

    janet_fixarity(argc, 4);

    hWnd = jw32_get_handle(argv, 0);
    nIDEvent = jw32_get_uint_ptr(argv, 1);
    uElapse = jw32_get_uint(argv, 2);
    /* TODO: lpTimerFunc */
    lpTimerFunc = (TIMERPROC)jw32_get_handle(argv, 3);

    return jw32_wrap_uint_ptr(SetTimer(hWnd, nIDEvent, uElapse, NULL));
}


static Janet cfun_KillTimer(int32_t argc, Janet *argv)
{
    HWND hWnd;
    UINT_PTR nIDEvent;

    janet_fixarity(argc, 2);

    hWnd = jw32_get_handle(argv, 0);
    nIDEvent = jw32_get_uint_ptr(argv, 1);

    return jw32_wrap_bool(KillTimer(hWnd, nIDEvent));
}


static Janet cfun_GetAsyncKeyState(int32_t argc, Janet *argv)
{
    int vKey;

    janet_fixarity(argc, 1);

    vKey = jw32_get_int(argv, 0);
    /* The return value is actually a SHORT */
    return jw32_wrap_int(GetAsyncKeyState(vKey));
}


static Janet cfun_GetDpiForSystem(int32_t argc, Janet *argv)
{
    (void)argv;

    janet_fixarity(argc, 0);

    return jw32_wrap_uint(GetDpiForSystem());
}


static Janet cfun_GetDpiForMonitor(int32_t argc, Janet *argv)
{
    HMONITOR hmonitor;
    MONITOR_DPI_TYPE dpiType;

    HRESULT hrRet;
    UINT dpiX, dpiY;

    Janet ret_tuple[2];

    janet_fixarity(argc, 2);

    hmonitor = jw32_get_handle(argv, 0);
    dpiType = jw32_get_int(argv, 1);

    hrRet = GetDpiForMonitor(hmonitor, dpiType, &dpiX, &dpiY);
    if (S_OK == hrRet) {
        ret_tuple[0] = jw32_wrap_uint(dpiX);
        ret_tuple[1] = jw32_wrap_uint(dpiY);
    }
    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_tuple(janet_tuple_n(ret_tuple, 2)));
}


static Janet cfun_GetDpiForWindow(int32_t argc, Janet *argv)
{
    HWND hwnd;

    janet_fixarity(argc, 1);

    hwnd = jw32_get_handle(argv, 0);
    return jw32_wrap_uint(GetDpiForWindow(hwnd));
}


static Janet cfun_GetSystemDpiForProcess(int32_t argc, Janet *argv)
{
    HANDLE hProcess;

    janet_fixarity(argc, 1);

    hProcess = jw32_get_handle(argv, 0);
    return jw32_wrap_uint(GetSystemDpiForProcess(hProcess));
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
        "SetWinEventHook",
        cfun_SetWinEventHook,
        "(" MOD_NAME "/SetWinEventHook eventMin eventMax hmodWinEventProc pfnWinEventProc idProcess idThread dwFlags)\n\n"
        "Sets an event hook function for a range of events.",
    },
    {
        "UnhookWinEvent",
        cfun_UnhookWinEvent,
        "(" MOD_NAME "/UnhookWinEvent hWinEventHook)\n\n"
        "Removes an event hook function.",
    },
    {
        "SetWindowsHookEx",
        cfun_SetWindowsHookEx,
        "(" MOD_NAME "/SetWindowsHookEx idHook lpfn hmod dwThreadId)\n\n"
        "Installs an application-defined hook procedure into a hook chain.",
    },
    {
        "UnhookWindowsHookEx",
        cfun_UnhookWindowsHookEx,
        "(" MOD_NAME "/UnhookWindowsHookEx hhk)\n\n"
        "Removes a hook procedure installed in a hook chain by SetWindowsHookEx().",
    },
    {
        "CallNextHookEx",
        cfun_CallNextHookEx,
        "(" MOD_NAME "/CallNextHookEx hhk nCode wParam lParam)\n\n"
        "Passes the hook information to the next hook procedure in the current hook chain.",
    },
    {
        "GetMessage",
        cfun_GetMessage,
        "(" MOD_NAME "/GetMessage lpMsg hWnd wMsgFilterMin wMsgFilterMax)\n\n"
        "Returns non-zero if the operation succeeds.",
    },
    {
        "PeekMessage",
        cfun_PeekMessage,
        "(" MOD_NAME "/PeekMessage lpMsg hWnd wMsgFilterMin wMsgFilterMax wRemoveMsg)\n\n"
        "Checks the thread message queue for a posted message.",
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
    {
        "PostMessage",
        cfun_PostMessage,
        "(" MOD_NAME "/PostMessage hWnd uMsg wParam lParam)\n\n"
        "Post the specified message to a window message queue.",
    },
    {
        "SendMessage",
        cfun_SendMessage,
        "(" MOD_NAME "/SendMessage hWnd uMsg wParam lParam)\n\n"
        "Calls the window procedure with the specified message.",
    },
    {
        "IsDialogMessage",
        cfun_IsDialogMessage,
        "(" MOD_NAME "/IsDialogMessage hDlg lpMsg)\n\n"
        "Process possible dialog messages.",
    },

    /*********************** WINDOW-RELATED ************************/
    {
        "MessageBox",
        cfun_MessageBox,
        "(" MOD_NAME "/MessageBox hWnd lpText lpCaption uType)\n\n"
        "Shows a message box.",
    },
    {
        "DialogBox",
        cfun_DialogBox,
        "(" MOD_NAME "/DialogBox hInstance lpTemplate hWndParent lpDialogFunc)\n\n"
        "Shows a dialog box.",
    },
    {
        "CreateDialog",
        cfun_CreateDialog,
        "(" MOD_NAME "/CreateDialog hInstance lpName hWndParent lpDialogFunc)\n\n"
        "Creates a dialog box.",
    },
    {
        "EndDialog",
        cfun_EndDialog,
        "(" MOD_NAME "/EndDialog hDlg nResult)\n\n"
        "Ends a dialog box.",
    },
    {
        "GetDesktopWindow",
        cfun_GetDesktopWindow,
        "(" MOD_NAME "/GetDesktopWindow)\n\n"
        "Win32 function wrapper.",
    },
    {
        "GetAncestor",
        cfun_GetAncestor,
        "(" MOD_NAME "/GetAncestor hwnd gaFlags)\n\n"
        "Retrieves the handle to the ancestor of the specified window.",
    },
    {
        "GetParent",
        cfun_GetParent,
        "(" MOD_NAME "/GetParent hwnd)\n\n"
        "Retrieves the handle to the specified window's parent or owner.",
    },
    {
        "CreateWindowEx",
        cfun_CreateWindowEx,
        "(" MOD_NAME "/CreateWindowEx dwExStyle lpClassName lpWindowName dwStyle x y nWidth nHeight hWndParent hMenu hInstance lpParam)\n\n"
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
    {
        "GetProp",
        cfun_GetProp,
        "(" MOD_NAME "/GetProp hWnd lpString)\n\n"
        "Gets a window property",
    },
    {
        "SetProp",
        cfun_SetProp,
        "(" MOD_NAME "/SetProp hWnd lpString hData)\n\n"
        "Sets a window property",
    },
    {
        "GetWindow",
        cfun_GetWindow,
        "(" MOD_NAME "/GetWindow hWnd uCmd)\n\n"
        "Retrieves a handle to a window that has the specified relationship to the specified window.",
    },
    {
        "GetWindowLong",
        cfun_GetWindowLong,
        "(" MOD_NAME "/GetWindowLong hWnd nIndex)\n\n"
        "Retrieves information about the specified window.",
    },
    {
        "SetWindowLong",
        cfun_SetWindowLong,
        "(" MOD_NAME "/SetWindowLong hWnd nIndex dwNewLong)\n\n"
        "Changes an attribute of the specified window.",
    },
    {
        "GetWindowLongPtr",
        cfun_GetWindowLongPtr,
        "(" MOD_NAME "/GetWindowLongPtr hWnd nIndex)\n\n"
        "Retrieves information about the specified window.",
    },
    {
        "SetWindowLongPtr",
        cfun_SetWindowLongPtr,
        "(" MOD_NAME "/SetWindowLongPtr hWnd nIndex dwNewLong)\n\n"
        "Changes an attribute of the specified window.",
    },
    {
        "GetWindowRect",
        cfun_GetWindowRect,
        "(" MOD_NAME "/GetWindowRect hWnd)\n\n"
        "Returns the dimensions of a window's bounding rectangle.",
    },
    {
        "SetForegroundWindow",
        cfun_SetForegroundWindow,
        "(" MOD_NAME "/SetForegroundWindow hWnd)\n\n"
        "Brings the thread that created the specified window into the foreground and activates the window.",
    },
    {
        "GetForegroundWindow",
        cfun_GetForegroundWindow,
        "(" MOD_NAME "/GetForegroundWindow)\n\n"
        "Retrieves a handle to the foreground window.",
    },
    {
        "IsWindow",
        cfun_IsWindow,
        "(" MOD_NAME "/IsWindow hWnd)\n\n"
        "Checks whether hWnd identifies an existing window.",
    },
    {
        "IsWindowVisible",
        cfun_IsWindowVisible,
        "(" MOD_NAME "/IsWindowVisible hWnd)\n\n"
        "Checks whether the window specified by hWnd has the WS_VISIBLE flag.",
    },
    {
        "IsIconic",
        cfun_IsIconic,
        "(" MOD_NAME "/IsIconic hWnd)\n\n"
        "Checks whether the window specified by hWnd is minimized.",
    },
    {
        "IsZoomed",
        cfun_IsZoomed,
        "(" MOD_NAME "/IsZoomed hWnd)\n\n"
        "Checks whether the window specified by hWnd is maximized.",
    },
    {
        "SetWindowPos",
        cfun_SetWindowPos,
        "(" MOD_NAME "/SetWindowPos hWnd hWndInsertAfter X Y cx cy uFlags)\n\n"
        "Changes the size, position, and z-order of a window.",
    },
    {
        "GetWindowThreadProcessId",
        cfun_GetWindowThreadProcessId,
        "(" MOD_NAME "/GetWindowThreadProcessId hWnd)\n\n"
        "Retrieves the identifier of the thread and the process that created the specified window.",
    },
    {
        "EnumChildWindows",
        cfun_EnumChildWindows,
        "(" MOD_NAME "/EnumChildWindows hWndParent lpEnumFunc)\n\n"
        "Enumerates child windows.",
    },
    {
        "SetLayeredWindowAttributes",
        cfun_SetLayeredWindowAttributes,
        "(" MOD_NAME "/SetLayeredWindowAttributes hwnd crKey bAlpha dwFlags)\n\n"
        "Sets the opacity and transparency color key of a layered window.",
    },
    {
        "GetLayeredWindowAttributes",
        cfun_GetLayeredWindowAttributes,
        "(" MOD_NAME "/GetLayeredWindowAttributes hwnd)\n\n"
        "Retrieves the opacity and transparency color key of a layered window.",
    },

    /************************** INPUT **************************/
    {
        "RAWINPUT",
        cfun_RAWINPUT,
        "(" MOD_NAME "/RAWINPUT ...)\n\n"
        "Builds a RAWINPUT struct.",
    },
    {
        "RAWINPUTDEVICE",
        cfun_RAWINPUTDEVICE,
        "(" MOD_NAME "/RAWINPUTDEVICE ...)\n\n"
        "Builds a RAWINPUTDEVICE struct.",
    },
    {
        "RegisterRawInputDevices",
        cfun_RegisterRawInputDevices,
        "(" MOD_NAME "/RegisterRawInputDevices pRawInputDevices)\n\n"
        "Registers raw input devices.",
    },
    {
        "GetRegisteredRawInputDevices",
        cfun_GetRegisteredRawInputDevices,
        "(" MOD_NAME "/GetRegisteredRawInputDevices pRawInputDevices)\n\n"
        "Retrieves registered raw input devices.",
    },
    {
        "GetRawInputData",
        cfun_GetRawInputData,
        "(" MOD_NAME "/GetRawInputData hRawInput uiCommand pRawInput)\n\n"
        "Retrieves raw input data.",
    },
    {
        "GetRawInputDeviceInfo",
        cfun_GetRawInputDeviceInfo,
        "(" MOD_NAME "/GetRawInputDeviceInfo hDevice uiCommand pData)\n\n"
        "Retrieves raw input device info.",
    },
    {
        "INPUT",
        cfun_INPUT,
        "(" MOD_NAME "/INPUT ...)\n\n"
        "Builds an INPUT struct.",
    },
    {
        "SendInput",
        cfun_SendInput,
        "(" MOD_NAME "/SendInput pInputs)\n\n"
        "Send simulated inputs.",
    },
    {
        "GetPhysicalCursorPos",
        cfun_GetPhysicalCursorPos,
        "(" MOD_NAME "/GetPhysicalCursorPos)\n\n"
        "Retrieves the position of the cursor in physical coordinates.",
    },
    {
        "GetCursorPos",
        cfun_GetCursorPos,
        "(" MOD_NAME "/GetCursorPos)\n\n"
        "Retrieves the position of the cursor in screen coordinates.",
    },
    {
        "SetCursorPos",
        cfun_SetCursorPos,
        "(" MOD_NAME "/SetCursorPos x y)\n\n"
        "Moves the cursor to the specified screen coordinates.",
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
        "Loads a cursor.",
    },
    {
        "LoadImage",
        cfun_LoadImage,
        "(" MOD_NAME "/LoadImage hInst name type cx cy fuLoad)\n\n"
        "Loads an image.",
    },
    {
        "CreateMenu",
        cfun_CreateMenu,
        "(" MOD_NAME "/CreateMenu)\n\n"
        "Creates a menu.",
    },
    {
        "CreatePopupMenu",
        cfun_CreatePopupMenu,
        "(" MOD_NAME "/CreatePopupMenu)\n\n"
        "Creates a popup menu.",
    },
    {
        "DestroyMenu",
        cfun_DestroyMenu,
        "(" MOD_NAME "/DestroyMenu hMenu)\n\n"
        "Creates a menu.",
    },
    {
        "AppendMenu",
        cfun_AppendMenu,
        "(" MOD_NAME "/AppendMenu hMenu uFlags uIDNewItem lpNewItem)\n\n"
        "Appends a new item to a menu.",
    },
    {
        "SetMenu",
        cfun_SetMenu,
        "(" MOD_NAME "/SetMenu hWnd hMenu)\n\n"
        "Assigns a menu to a window.",
    },
    {
        "TrackPopupMenuEx",
        cfun_TrackPopupMenuEx,
        "(" MOD_NAME "/TrackPopupMenuEx hMenu uFlags x y hwnd rect)\n\n"
        "Displays a shortcut menu at the specified location and tracks the selection of items.",
    },

    /************************** MISC. **************************/
    {
        "SetProcessDPIAware",
        cfun_SetProcessDPIAware,
        "(" MOD_NAME "/SetProcessDPIAware)\n\n"
        "Sets the process-default DPI awareness to system-DPI awareness.",
    },
    {
        "EnumDisplayMonitors",
        cfun_EnumDisplayMonitors,
        "(" MOD_NAME "/EnumDisplayMonitors hdc rcClip lpfnEnum)\n\n"
        "Enumerates display monitors.",
    },
    {
        "MONITORINFOEX",
        cfun_MONITORINFOEX,
        "(" MOD_NAME "/MONITORINFOEX ...)\n\n"
        "Creates a MONITORINFOEX struct.",
    },
    {
        "GetMonitorInfo",
        cfun_GetMonitorInfo,
        "(" MOD_NAME "/GetMonitorInfo hMonitor lpmi)\n\n"
        "Retrieves information about a display monitor.",
    },
    {
        "MonitorFromPoint",
        cfun_MonitorFromPoint,
        "(" MOD_NAME "/MonitorFromPoint pt dwFlags)\n\n"
        "Retrieves a handle to the monitor that contains the specified point.",
    },
    {
        "MonitorFromRect",
        cfun_MonitorFromRect,
        "(" MOD_NAME "/MonitorFromRect lprc dwFlags)\n\n"
        "Retrieves a handle to the monitor that has the largest area of intersection with the specified rectangle.",
    },
    {
        "MonitorFromWindow",
        cfun_MonitorFromWindow,
        "(" MOD_NAME "/MonitorFromWindow hwnd dwFlags)\n\n"
        "Retrieves a handle to the monitor that has the largest area of intersection with the specified window.",
    },
    {
        "GetSystemMetrics",
        cfun_GetSystemMetrics,
        "(" MOD_NAME "/GetSystemMetrics nIndex)\n\n"
        "Retrieves information about system settings.",
    },
    {
        "SystemParametersInfo",
        cfun_SystemParametersInfo,
        "(" MOD_NAME "/SystemParametersInfo uiAction uiParam pvParam fWinIni)\n\n"
        "Retrieves or sets the value of one of the system-wide parameters.",
    },
    {
        "SetTimer",
        cfun_SetTimer,
        "(" MOD_NAME "/SetTimer hWnd nIDEvent uElapse lpTimerFunc)\n\n"
        "Creates a timer.",
    },
    {
        "KillTimer",
        cfun_KillTimer,
        "(" MOD_NAME "/KillTimer hWnd nIDEvent)\n\n"
        "Destroys a timer.",
    },
    {
        "GetAsyncKeyState",
        cfun_GetAsyncKeyState,
        "(" MOD_NAME "/GetAsyncKeyState vKey)\n\n"
        "Gets the current key state.",
    },
    {
        "GetDpiForSystem",
        cfun_GetDpiForSystem,
        "(" MOD_NAME "/GetDpiForSystem)\n\n"
        "Gets the system DPI.",
    },
    {
        "GetDpiForMonitor",
        cfun_GetDpiForMonitor,
        "(" MOD_NAME "/GetDpiForMonitor hMonitor dpiType)\n\n"
        "Gets the DPI value for a display monitor.",
    },
    {
        "GetDpiForWindow",
        cfun_GetDpiForWindow,
        "(" MOD_NAME "/GetDpiForWindow hwnd)\n\n"
        "Gets the DPI value for a window.",
    },
    {
        "GetSystemDpiForProcess",
        cfun_GetSystemDpiForProcess,
        "(" MOD_NAME "/GetSystemDpiForProcess hProcess)\n\n"
        "Gets the system DPI value for a process.",
    },

    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_consts_wm(env);
    define_consts_pm(env);
    define_consts_mb(env);
    define_consts_button_id(env);
    define_consts_idi(env);
    define_consts_idc(env);
    define_consts_color(env);
    define_consts_cs(env);
    define_consts_ws(env);
    define_consts_ws_ex(env);
    define_consts_cw(env);
    define_consts_sw(env);
    define_consts_image(env);
    define_consts_lr(env);
    define_consts_sm(env);
    define_consts_mf(env);
    define_consts_hwnd(env);
    define_consts_icon(env);
    define_consts_winevent(env);
    define_consts_event(env);
    define_consts_objid(env);
    define_consts_wh(env);
    define_consts_rid(env);
    define_consts_rim(env);
    define_consts_ridev(env);
    define_consts_ri(env);
    define_consts_ridi(env);
    define_consts_input(env);
    define_consts_mouseeventf(env);
    define_consts_keyeventf(env);
    define_consts_monitorinfof(env);
    define_consts_monitor(env);
    define_consts_tpm(env);
    define_consts_vk(env);
    define_consts_spi(env);
    define_consts_gwl(env);
    define_consts_gwlp(env);
    define_consts_gw(env);
    define_consts_lwa(env);
    define_consts_ga(env);
    define_consts_mdt(env);
    define_consts_swp(env);

    janet_register_abstract_type(&jw32_at_MSG);
    janet_register_abstract_type(&jw32_at_WNDCLASSEX);
    janet_register_abstract_type(&jw32_at_RAWINPUT);
    janet_register_abstract_type(&jw32_at_RAWINPUTDEVICE);
    janet_register_abstract_type(&jw32_at_INPUT);
    janet_register_abstract_type(&jw32_at_MONITORINFOEX);
    janet_register_abstract_type(&jw32_at_PKBDLLHOOKSTRUCT);

    janet_cfuns(env, MOD_NAME, cfuns);
}
