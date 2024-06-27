#include "jw32.h"
#include "jw32_com.h"
#include <shobjidl_core.h>
#include <shobjidl.h>
#include "debug.h"

#define MOD_NAME "shobjidl_core"


JanetTable *IVirtualDesktopManager_proto;
JanetTable *IDesktopWallpaper_proto;


static void define_uuids(JanetTable *env)
{
    janet_def(env, "CLSID_VirtualDesktopManager", jw32_wrap_refclsid(&CLSID_VirtualDesktopManager),
              "Class ID for VirtualDesktopManager.");
    janet_def(env, "CLSID_DesktopWallpaper", jw32_wrap_refclsid(&CLSID_DesktopWallpaper),
              "Class ID for DesktopWallpaper.");
}


static Janet IVirtualDesktopManager_GetWindowDesktopId(int32_t argc, Janet *argv)
{
    IVirtualDesktopManager *self;
    HWND toplevelWindow;

    HRESULT hrRet;
    GUID desktopId;

    janet_fixarity(argc, 2);

    self = (IVirtualDesktopManager *)jw32_com_get_obj_ref(argv, 0);
    toplevelWindow = jw32_get_handle(argv, 1);

    hrRet = self->lpVtbl->GetWindowDesktopId(self, toplevelWindow, &desktopId);
    if (S_OK == hrRet) {
        JanetString guid_str = jw32_guid_to_string(&desktopId);
        if (!guid_str) {
            /* XXX: always raise E_INVALIDARG when failed */
            janet_panicv(JW32_HRESULT_ERRORV(E_INVALIDARG));
        }
        return janet_wrap_string(guid_str);
    } else {
        janet_panicv(JW32_HRESULT_ERRORV(hrRet));
    }
}


static Janet IVirtualDesktopManager_IsWindowOnCurrentVirtualDesktop(int32_t argc, Janet *argv)
{
    IVirtualDesktopManager *self;
    HWND toplevelWindow;

    HRESULT hrRet;
    BOOL onCurrentDesktop;

    janet_fixarity(argc, 2);

    self = (IVirtualDesktopManager *)jw32_com_get_obj_ref(argv, 0);
    toplevelWindow = jw32_get_handle(argv, 1);

    hrRet = self->lpVtbl->IsWindowOnCurrentVirtualDesktop(self, toplevelWindow, &onCurrentDesktop);
    JW32_HR_RETURN_OR_PANIC(hrRet, jw32_wrap_bool(onCurrentDesktop));
}


static Janet IVirtualDesktopManager_MoveWindowToDesktop(int32_t argc, Janet *argv)
{
    IVirtualDesktopManager *self;
    HWND toplevelWindow;
    JanetString desktop_id_str;

    GUID desktopId;

    HRESULT hrRet;

    janet_fixarity(argc, 3);

    self = (IVirtualDesktopManager *)jw32_com_get_obj_ref(argv, 0);
    toplevelWindow = jw32_get_handle(argv, 1);
    desktop_id_str = janet_getstring(argv, 2);

    if (jw32_string_to_guid(desktop_id_str, &desktopId)) {
        hrRet = self->lpVtbl->MoveWindowToDesktop(self, toplevelWindow, &desktopId);
        JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_nil());
    } else {
        /* XXX: always raise E_INVALIDARG when failed */
        janet_panicv(JW32_HRESULT_ERRORV(E_INVALIDARG));
    }
}


static const JanetMethod IVirtualDesktopManager_methods[] = {
    {"GetWindowDesktopId", IVirtualDesktopManager_GetWindowDesktopId},
    {"IsWindowOnCurrentVirtualDesktop", IVirtualDesktopManager_IsWindowOnCurrentVirtualDesktop},
    {"MoveWindowToDesktop", IVirtualDesktopManager_MoveWindowToDesktop},
    {NULL, NULL},
};


static Janet IDesktopWallpaper_GetMonitorDevicePathAt(int32_t argc, Janet *argv)
{
    IDesktopWallpaper *self;
    UINT monitorIndex;

    HRESULT hrRet;
    JanetString monitor_id_str = NULL;

    LPWSTR monitorID = NULL;

    janet_fixarity(argc, 2);

    self = (IDesktopWallpaper *)jw32_com_get_obj_ref(argv, 0);
    monitorIndex = jw32_get_uint(argv, 1);

    hrRet = self->lpVtbl->GetMonitorDevicePathAt(self, monitorIndex, &monitorID);
    if (S_OK == hrRet) {
        monitor_id_str = jw32_bstr_to_string((BSTR)monitorID);
        if (!monitor_id_str) {
            hrRet = E_UNEXPECTED;
        }
    }
    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_string(monitor_id_str));
}


static Janet IDesktopWallpaper_GetMonitorDevicePathCount(int32_t argc, Janet *argv)
{
    IDesktopWallpaper *self;

    HRESULT hrRet;
    UINT count = 0;
    INT iCount = 0;

    janet_fixarity(argc, 1);

    self = (IDesktopWallpaper *)jw32_com_get_obj_ref(argv, 0);
    
    hrRet = self->lpVtbl->GetMonitorDevicePathCount(self, &count);
    if (S_OK == hrRet) {
        /* The return value from C API is a UINT, but we return
           an INT number, for ease of processing. There can't be
           that many monitors, right? */
        if (count > INT_MAX) {
            hrRet = E_BOUNDS;
        } else {
            iCount = (INT)count;
        }
    }
    JW32_HR_RETURN_OR_PANIC(hrRet, jw32_wrap_int(iCount));
}


static Janet IDesktopWallpaper_GetWallpaper(int32_t argc, Janet *argv)
{
    IDesktopWallpaper *self;
    JanetString monitor_id_str;

    HRESULT hrRet;
    JanetString wallpaper_str = NULL;

    BSTR monitorID = NULL;
    LPWSTR wallpaper = NULL;

    janet_fixarity(argc, 2);

    self = (IDesktopWallpaper *)jw32_com_get_obj_ref(argv, 0);
    if (janet_checktype(argv[1], JANET_NIL)) {
        monitor_id_str = NULL;
    } else {
        monitor_id_str = janet_getstring(argv, 1);
    }

    if (monitor_id_str) {
        monitorID = jw32_string_to_bstr(monitor_id_str);
        if (!monitorID) {
            /* XXX: always raise E_INVALIDARG when failed */
            janet_panicv(JW32_HRESULT_ERRORV(E_INVALIDARG));
        }
    } else {
        monitorID = NULL;
    }

    hrRet = self->lpVtbl->GetWallpaper(self, monitorID, &wallpaper);

    SysFreeString(monitorID);

    if (S_OK == hrRet) {
        /* XXX: jw32_bstr_to_string does not care about the BSTR header, so this works */
        wallpaper_str = jw32_bstr_to_string((BSTR)wallpaper);
        if (!wallpaper_str) {
            hrRet = E_UNEXPECTED;
        }
    }

    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_string(wallpaper_str));
}


static Janet IDesktopWallpaper_SetWallpaper(int32_t argc, Janet *argv)
{
    IDesktopWallpaper *self;
    JanetString monitor_id_str;
    JanetString wallpaper_str;

    BSTR monitorID = NULL;
    BSTR wallpaper = NULL;

    HRESULT hrRet;

    janet_fixarity(argc, 3);

    self = (IDesktopWallpaper *)jw32_com_get_obj_ref(argv, 0);
    if (janet_checktype(argv[1], JANET_NIL)) {
        monitor_id_str = NULL;
    } else {
        monitor_id_str = janet_getstring(argv, 1);
    }
    wallpaper_str = janet_getstring(argv, 2);

    if (monitor_id_str) {
        monitorID = jw32_string_to_bstr(monitor_id_str);
        if (!monitorID) {
            /* XXX: always raise E_INVALIDARG when failed */
            hrRet = E_INVALIDARG;
            goto free_and_return;
        }
    } else {
        monitorID = NULL;
    }

    wallpaper = jw32_string_to_bstr(wallpaper_str);
    if (!wallpaper) {
        /* XXX: always raise E_INVALIDARG when failed */
        hrRet = E_INVALIDARG;
        goto free_and_return;
    }

    hrRet = self->lpVtbl->SetWallpaper(self, monitorID, wallpaper);

free_and_return:
    SysFreeString(monitorID);
    SysFreeString(wallpaper);
    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_nil());
}


static const JanetMethod IDesktopWallpaper_methods[] = {
    //{"AdvanceSlideshow", IDesktopWallpaper_AdvanceSlideshow},
    //{"Enable", IDesktopWallpaper_Enable},
    //{"GetBackgroundColor", IDesktopWallpaper_GetBackgroundColor},
    {"GetMonitorDevicePathAt", IDesktopWallpaper_GetMonitorDevicePathAt},
    {"GetMonitorDevicePathCount", IDesktopWallpaper_GetMonitorDevicePathCount},
    //{"GetMonitorRECT", IDesktopWallpaper_GetMonitorRECT},
    //{"GetPosition", IDesktopWallpaper_GetPosition},
    //{"GetSlideshow", IDesktopWallpaper_GetSlideshow},
    //{"GetSlideshowOptions", IDesktopWallpaper_GetSlideshowOptions},
    //{"GetStatus", IDesktopWallpaper_GetStatus},
    {"GetWallpaper", IDesktopWallpaper_GetWallpaper},
    //{"SetBackgroundColor", IDesktopWallpaper_SetBackgroundColor},
    //{"SetPosition", IDesktopWallpaper_SetPosition},
    //{"SetSlideshow", IDesktopWallpaper_SetSlideshow},
    //{"SetSlideshowOptions", IDesktopWallpaper_SetSlideshowOptions},
    {"SetWallpaper", IDesktopWallpaper_SetWallpaper},
    {NULL, NULL},
};


static void init_table_protos(JanetTable *env)
{
#define __def_proto(name, parent, doc)                                  \
    do {                                                                \
        ##name##_proto = jw32_com_make_if_proto(#name, ##name##_methods, parent, &IID_##name##); \
        janet_def(env, #name, janet_wrap_table(##name##_proto), doc);   \
    } while (0)

    __def_proto(IVirtualDesktopManager,
                "IUnknown",
                "Prototype for COM IVirtualDesktopManager interface.");
    __def_proto(IDesktopWallpaper,
                "IUnknown",
                "Prototype for COM IDesktopWallpaper interface.");

#undef __def_proto
}


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_uuids(env);

    init_table_protos(env);
}
