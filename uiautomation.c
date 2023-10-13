#include "jw32.h"
#include "jw32_com.h"
#include <UIAutomationClient.h>
#include "debug.h"

#define MOD_NAME "uiautomation"


JANET_MODULE_ENTRY(JanetTable *env)
{
    JanetTable *iunknown_proto = jw32_com_resolve_iunknown_proto();

    /* TODO */
    jw32_dbg_jval(janet_wrap_table(iunknown_proto));

    janet_def(env, "CLSID_CUIAutomation", jw32_wrap_refclsid(&CLSID_CUIAutomation),
              "Class ID for CUIAutomation.");

    janet_def(env, "IID_IUIAutomation", jw32_wrap_refiid(&IID_IUIAutomation),
              "Interface ID for IUIAutomation.");
}
