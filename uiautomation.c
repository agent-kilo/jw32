#include "jw32.h"
#include "jw32_com.h"
#include <UIAutomationClient.h>
#include "debug.h"

#define MOD_NAME "uiautomation"


JanetTable *iuiautomationelement_proto;


static void define_uuids(JanetTable *env)
{
    /* UIAutomation */
    janet_def(env, "CLSID_CUIAutomation", jw32_wrap_refclsid(&CLSID_CUIAutomation),
              "Class ID for CUIAutomation.");
}

static void define_consts_uia_controltypeid(JanetTable *env)
{
#define __def(const_name)                                        \
    janet_def(env, #const_name, jw32_wrap_int(const_name),       \
              "Constant for UI Automation control types.")
    __def(UIA_ButtonControlTypeId);
    __def(UIA_CalendarControlTypeId);
    __def(UIA_CheckBoxControlTypeId);
    __def(UIA_ComboBoxControlTypeId);
    __def(UIA_EditControlTypeId);
    __def(UIA_HyperlinkControlTypeId);
    __def(UIA_ImageControlTypeId);
    __def(UIA_ListItemControlTypeId);
    __def(UIA_ListControlTypeId);
    __def(UIA_MenuControlTypeId);
    __def(UIA_MenuBarControlTypeId);
    __def(UIA_MenuItemControlTypeId);
    __def(UIA_ProgressBarControlTypeId);
    __def(UIA_RadioButtonControlTypeId);
    __def(UIA_ScrollBarControlTypeId);
    __def(UIA_SliderControlTypeId);
    __def(UIA_SpinnerControlTypeId);
    __def(UIA_StatusBarControlTypeId);
    __def(UIA_TabControlTypeId);
    __def(UIA_TabItemControlTypeId);
    __def(UIA_TextControlTypeId);
    __def(UIA_ToolBarControlTypeId);
    __def(UIA_ToolTipControlTypeId);
    __def(UIA_TreeControlTypeId);
    __def(UIA_TreeItemControlTypeId);
    __def(UIA_CustomControlTypeId);
    __def(UIA_GroupControlTypeId);
    __def(UIA_ThumbControlTypeId);
    __def(UIA_DataGridControlTypeId);
    __def(UIA_DataItemControlTypeId);
    __def(UIA_DocumentControlTypeId);
    __def(UIA_SplitButtonControlTypeId);
    __def(UIA_WindowControlTypeId);
    __def(UIA_PaneControlTypeId);
    __def(UIA_HeaderControlTypeId);
    __def(UIA_HeaderItemControlTypeId);
    __def(UIA_TableControlTypeId);
    __def(UIA_TitleBarControlTypeId);
    __def(UIA_SeparatorControlTypeId);
    __def(UIA_SemanticZoomControlTypeId);
    __def(UIA_AppBarControlTypeId);
#undef __def
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
    ret_tuple[1] = jw32_com_maybe_make_object(hrRet, root, iuiautomationelement_proto);

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static const JanetMethod iuiautomation_methods[] = {
    {"GetRootElement", iuiautomation_GetRootElement},
    {NULL, NULL},
};


static Janet iuiautomationelement_get_CurrentControlType(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;

    HRESULT hrRet;
    CONTROLTYPEID retVal;
    Janet ret_tuple[2];

    janet_fixarity(argc, 1);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->get_CurrentControlType(self, &retVal);

    ret_tuple[0] = jw32_wrap_hresult(hrRet);
    ret_tuple[1] = jw32_wrap_int(retVal);

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static const JanetMethod iuiautomationelement_methods[] = {
    {"get_CurrentControlType", iuiautomationelement_get_CurrentControlType},
    {NULL, NULL},
};


static void init_table_protos(JanetTable *env)
{
    JanetTable *iunknown_proto = jw32_com_resolve_iunknown_proto();
    /* TODO: do we need this to keep the prototype from being gc-ed? does the table
       mark its proto when doing gc_mark? */
    janet_def(env, "IUnknown", janet_wrap_table(iunknown_proto),
              "Prototype for COM IUnknown interface.");

    JanetTable *iuiautomation_proto = jw32_com_make_if_proto("IUIAutomation",
                                                             iuiautomation_methods,
                                                             iunknown_proto,
                                                             &IID_IUIAutomation);
    janet_def(env, "IUIAutomation", janet_wrap_table(iuiautomation_proto),
              "Prototype for COM IUIAutomation interface.");

    /* make it global so that IUIAutomation methods can find it */
    iuiautomationelement_proto = jw32_com_make_if_proto("IUIAutomationElement",
                                                        iuiautomationelement_methods,
                                                        iunknown_proto,
                                                        &IID_IUIAutomationElement);
    janet_def(env, "IUIAutomationElement", janet_wrap_table(iuiautomationelement_proto),
              "Prototype for COM IUIAutomationElement interface.");
}


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_uuids(env);
    define_consts_uia_controltypeid(env);

    init_table_protos(env);
}
