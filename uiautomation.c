#include "jw32.h"
#include "jw32_com.h"
#include "UIAutomationClient.h"

#define MOD_NAME "uiautomation"


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_def(env, "CLSID_CUIAutomation", jw32_wrap_refclsid(&CLSID_CUIAutomation),
              "Class ID for CUIAutomation.");

    janet_def(env, "IID_IUIAutomation", jw32_wrap_refiid(&IID_IUIAutomation),
              "Interface ID for IUIAutomation.");
}
