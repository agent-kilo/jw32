#include "jw32.h"
#include "jw32_com.h"
#include <shobjidl_core.h>
#include <shobjidl.h>
#include "debug.h"

#define MOD_NAME "shobjidl_core"


JanetTable *IVirtualDesktopManager_proto;


static void define_uuids(JanetTable *env)
{
    janet_def(env, "CLSID_VirtualDesktopManager", jw32_wrap_refclsid(&CLSID_VirtualDesktopManager),
              "Class ID for VirtualDesktopManager.");
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

#undef __def_proto
}


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_uuids(env);

    init_table_protos(env);
}
