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
}


static Janet iuiautomation_GetRootElement(int32_t argc, Janet *argv)
{
    IUIAutomation *self;

    HRESULT hrRet;
    IUIAutomationElement *root = NULL;
    Janet ret_tuple[2];

    janet_fixarity(argc, 1);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->GetRootElement(self, &root);
    ret_tuple[0] = jw32_wrap_hresult(hrRet);
    /* TODO: return an object? */
    ret_tuple[1] = jw32_wrap_lpvoid(root);

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static const JanetMethod iuiautomation_methods[] = {
    {"GetRootElement", iuiautomation_GetRootElement},
    {NULL, NULL},
};

static void init_table_protos(JanetTable *env)
{
    JanetTable *iunknown_proto = jw32_com_resolve_iunknown_proto();
    /* XXX: do we need this to keep the prototype from being gc-ed? does the table
       mark its proto when doing gc_mark? */
    janet_def(env, "IUnknown", janet_wrap_table(iunknown_proto),
              "Prototype for COM IUnknown interface.");

    JanetTable *iuiautomation_proto = jw32_com_make_if_proto("IUIAutomation",
                                                             iuiautomation_methods,
                                                             iunknown_proto,
                                                             &IID_IUIAutomation);
    janet_def(env, "IUIAutomation", janet_wrap_table(iuiautomation_proto),
              "Prototype for COM IUIAutomation interface.");
}


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_uuids(env);

    init_table_protos(env);
}
