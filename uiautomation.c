#include "jw32.h"
#include "jw32_com.h"
#include <UIAutomationClient.h>
#include "debug.h"

#define MOD_NAME "uiautomation"


static void define_uuids(JanetTable *env)
{
    /* UIAutomation */
    janet_def(env, "CLSID_CUIAutomation", jw32_wrap_refclsid(&CLSID_CUIAutomation),
              "Class ID for CUIAutomation.");
    janet_def(env, "IID_IUIAutomation", jw32_wrap_refiid(&IID_IUIAutomation),
              "Interface ID for IUIAutomation.");
}


static Janet iuiautomation_GetRootElement(int32_t argc, Janet *argv)
{
    IUIAutomation *self;

    HRESULT hrRet;
    IUIAutomationElement *root = NULL;
    Janet ret_tuple[2];

    hrRet = self->lpVtbl->GetRootElement(self, &root);
    ret_tuple[0] = hrRet;
    /* TODO: return an object? */
    ret_tuple[1] = jw32_wrap_lpvoid(root);

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static const JanetMethod iuiautomation_methods[] = {
    {"GetRootElement", iuiautomation_GetRootElement},
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_uuids(env);

    JanetTable *iunknown_proto = jw32_com_resolve_iunknown_proto();
    JanetTable *iuiautomation_proto = jw32_com_make_proto(iuiautomation_methods);

    iuiautomation_proto->proto = iunknown_proto;

    janet_def(env, "IUnknown", janet_wrap_table(iunknown_proto),
	      "Prototype for COM IUnknown interface.");
    janet_def(env, "IUIAutomation", janet_wrap_table(iuiautomation_proto),
	      "Prototype for COM IUIAutomation interface.");
}
