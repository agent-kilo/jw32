#include "jw32.h"
#include "jw32_com.h"
#include <UIAutomationClient.h>
#include "debug.h"

#define MOD_NAME "uiautomation"

#define PROPERTY_GETTER(__if, __prop) __##__if##_get_##__prop##__

#define DEFINE_OBJ_PROPERTY_GETTER(__if, __prop, __prop_if)    \
    static Janet PROPERTY_GETTER(__if, __prop)(int32_t argc, Janet *argv) \
    {                                                                   \
        __if *self;                                                     \
        HRESULT hrRet;                                                  \
        __prop_if *out = NULL;                                          \
        janet_fixarity(argc, 1);                                        \
        self = (__if *)jw32_com_get_obj_ref(argv, 0);                   \
        hrRet = self->lpVtbl->get_##__prop##(self, &out);               \
        JW32_HR_RETURN_OR_PANIC(hrRet, jw32_com_make_object_in_env(out, #__prop_if, uia_thread_state.env)); \
    }

#define DEFINE_SIMPLE_PROPERTY_GETTER(__if, __prop, __prop_type, __prop_jw32_type) \
    static Janet PROPERTY_GETTER(__if, __prop)(int32_t argc, Janet *argv) \
    {                                                                   \
        __if *self;                                                     \
        HRESULT hrRet;                                                  \
        __prop_type out = 0;                                            \
        janet_fixarity(argc, 1);                                        \
        self = (__if *)jw32_com_get_obj_ref(argv, 0);                   \
        hrRet = self->lpVtbl->get_##__prop##(self, &out);               \
        JW32_HR_RETURN_OR_PANIC(hrRet, jw32_wrap_##__prop_jw32_type##(out)); \
    }

#define DEFINE_BSTR_PROPERTY_GETTER(__if, __prop)                       \
    static Janet PROPERTY_GETTER(__if, __prop)(int32_t argc, Janet *argv) \
    {                                                                   \
        __if *self;                                                     \
        HRESULT hrRet;                                                  \
        BSTR retVal = NULL;                                             \
        janet_fixarity(argc, 1);                                        \
        self = (__if *)jw32_com_get_obj_ref(argv, 0);                   \
        hrRet = self->lpVtbl->get_##__prop##(self, &retVal);            \
        if (S_OK == hrRet) {                                            \
            Janet jstr = janet_wrap_string(jw32_bstr_to_string(retVal)); \
            SysFreeString(retVal);                                      \
            return jstr;                                                \
        } else {                                                        \
            janet_panicv(JW32_HRESULT_ERRORV(hrRet));                   \
        }                                                               \
    }

#define PROPERTY_SETTER(__if, __prop) __##__if##_put_##__prop##__

#define DEFINE_OBJ_PROPERTY_SETTER(__if, __prop, __prop_if)    \
    static Janet PROPERTY_SETTER(__if, __prop)(int32_t argc, Janet *argv) \
    {                                                                   \
        __if *self;                                                     \
        __prop_if *val;                                                 \
        HRESULT hrRet;                                                  \
        janet_fixarity(argc, 2);                                        \
        self = (__if *)jw32_com_get_obj_ref(argv, 0);                   \
        val = (__prop_if *)jw32_com_get_obj_ref(argv, 1);               \
        hrRet = self->lpVtbl->put_##__prop##(self, val);                \
        JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_nil());               \
    }

#define DEFINE_SIMPLE_PROPERTY_SETTER(__if, __prop, __prop_type, __prop_jw32_type) \
    static Janet PROPERTY_SETTER(__if, __prop)(int32_t argc, Janet *argv) \
    {                                                                   \
        __if *self;                                                     \
        __prop_type val;                                                \
        HRESULT hrRet;                                                  \
        janet_fixarity(argc, 2);                                        \
        self = (__if *)jw32_com_get_obj_ref(argv, 0);                   \
        val = (__prop_type)jw32_get_##__prop_jw32_type##(argv, 1);      \
        hrRet = self->lpVtbl->put_##__prop##(self, val);                \
        JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_nil());               \
    }

#define DEFINE_OBJ_PROPERTY(__if, __prop, __prop_if)    \
    DEFINE_OBJ_PROPERTY_GETTER(__if, __prop, __prop_if) \
    DEFINE_OBJ_PROPERTY_SETTER(__if, __prop, __prop_if)

#define DEFINE_SIMPLE_PROPERTY(__if, __prop, __prop_type, __prop_jw32_type) \
    DEFINE_SIMPLE_PROPERTY_GETTER(__if, __prop, __prop_type, __prop_jw32_type) \
    DEFINE_SIMPLE_PROPERTY_SETTER(__if, __prop, __prop_type, __prop_jw32_type)

#define PROPERTY_GETTER_METHOD(__if, __prop)           \
    {"get_" #__prop, PROPERTY_GETTER(__if, __prop)}

#define PROPERTY_SETTER_METHOD(__if, __prop)           \
    {"put_" #__prop, PROPERTY_SETTER(__if, __prop)}

#define PROPERTY_METHODS(__if, __prop)    \
    PROPERTY_GETTER_METHOD(__if, __prop), \
    PROPERTY_SETTER_METHOD(__if, __prop)


struct Jw32UIAEventHandlerThreadState {
    int vm_initialized;
    JanetTable *env;
};
typedef struct Jw32UIAEventHandlerThreadState Jw32UIAEventHandlerThreadState;

JANET_THREAD_LOCAL Jw32UIAEventHandlerThreadState uia_thread_state = {
    0,
    NULL,
};

/* make these global so that IUIAutomation methods can find them */
JanetTable *IUIAutomation_proto;
JanetTable *IUIAutomationElement_proto;
JanetTable *IUIAutomationElementArray_proto;
JanetTable *IUIAutomationCacheRequest_proto;
JanetTable *IUIAutomationCondition_proto;
JanetTable *IUIAutomationAndCondition_proto;
JanetTable *IUIAutomationBoolCondition_proto;
JanetTable *IUIAutomationNotCondition_proto;
JanetTable *IUIAutomationOrCondition_proto;
JanetTable *IUIAutomationPropertyCondition_proto;
JanetTable *IUIAutomationTransformPattern_proto;
JanetTable *IUIAutomationWindowPattern_proto;


static SAFEARRAY *make_condition_safearray(JanetView view)
{
    SAFEARRAY *psa = SafeArrayCreateVector(VT_UNKNOWN, 0, view.len);
    if (!psa) {
        janet_panicv(JW32_HRESULT_ERRORV(E_OUTOFMEMORY));
    }

    for (LONG i = 0; i < view.len; i++) {
        Janet item = view.items[i];
        if (!janet_checktype(item, JANET_TABLE)) {
            SafeArrayDestroy(psa);
            janet_panicf("bad condition #%d: expected a table, got %v", i, item);
        }
        JanetTable *obj = janet_unwrap_table(item);
        Janet maybe_ref = janet_table_get(obj, jw32_cstr_to_keyword(JW32_COM_OBJ_REF_NAME));
        if (!janet_checktype(maybe_ref, JANET_POINTER)) {
        cleanup_and_panic:
            SafeArrayDestroy(psa);
            janet_panicf("invalid object reference in slot %d: %v", i, maybe_ref);
        }
        IUIAutomationCondition *cond = (IUIAutomationCondition *)janet_unwrap_pointer(maybe_ref);
        if (!cond) {
            goto cleanup_and_panic;
        }
        HRESULT hr = SafeArrayPutElement(psa, &i, cond);
        jw32_dbg_val(hr, "%d");
    }

    return psa;
}


/*******************************************************************
 *
 * EVENT HANDLERS
 *
 *******************************************************************/

struct Jw32UIAEventHandler;
typedef struct Jw32UIAEventHandler Jw32UIAEventHandler;

/* XXX: not the actual type, just a place-holder type for all the
   Handle*Event() methods */
typedef void (STDMETHODCALLTYPE *Jw32UIAEventHandlerFunc)(void);

typedef struct Jw32UIAEventHandlerVtbl
{
    BEGIN_INTERFACE

    HRESULT (STDMETHODCALLTYPE *QueryInterface)( 
        __RPC__in Jw32UIAEventHandler *self,
        __RPC__in REFIID riid,
        _COM_Outptr_ void **ppvObject);

    ULONG (STDMETHODCALLTYPE *AddRef)( 
        __RPC__in Jw32UIAEventHandler *self);

    ULONG (STDMETHODCALLTYPE *Release)( 
        __RPC__in Jw32UIAEventHandler *self);
        
    Jw32UIAEventHandlerFunc HandleEvent;

    END_INTERFACE
} Jw32UIAEventHandlerVtbl;

/* the only thing that's different is the callback function pointer
   in the virtual table, so we are re-using the same struct for
   everything else. */
struct Jw32UIAEventHandler {
    CONST_VTBL Jw32UIAEventHandlerVtbl *lpVtbl;
    const IID *_riid;
    LONG _refCount;
    JanetBuffer *marshaled_cb;
    JanetBuffer *marshaled_env;
};


static JanetFunction *unmarshal_handler_cb(Jw32UIAEventHandler *handler)
{
    JanetBuffer *marshaled_cb = handler->marshaled_cb;
    Janet cb = janet_unmarshal(marshaled_cb->data, marshaled_cb->count,
                               JANET_MARSHAL_UNSAFE, NULL, NULL);
    return janet_unwrap_function(cb);
}

static JanetTable *unmarshal_handler_env(Jw32UIAEventHandler *handler)
{
    JanetBuffer *marshaled_env = handler->marshaled_env;
    Janet env = janet_unmarshal(marshaled_env->data, marshaled_env->count,
                                JANET_MARSHAL_UNSAFE, NULL, NULL);
    return janet_unwrap_table(env);
}

static void init_event_handler_thread_vm(Jw32UIAEventHandler *handler)
{
    jw32_dbg_val(GetCurrentThreadId(), "0x%x");
    jw32_dbg_val(uia_thread_state.vm_initialized, "%d");

    if (uia_thread_state.vm_initialized) {
        return;
    }

    janet_init();

    /* TODO: pass abstract type registry & cfun registry by marshaling?
       see cfun_ev_thread() & janet_go_thread_subr() */

    JanetTryState tstate;
    JanetSignal signal = janet_try(&tstate);
    if (JANET_SIGNAL_OK == signal) {
        uia_thread_state.env = unmarshal_handler_env(handler);
        uia_thread_state.vm_initialized = 1;
    } else {
        jw32_dbg_val(signal, "%d");
        jw32_dbg_jval(tstate.payload);
    }
    janet_restore(&tstate);
}

static JanetSignal uia_call_event_handler_fn(JanetFunction *fn,
                                             int32_t argc, const Janet *argv,
                                             JanetTable *env, Janet *out) {
    JanetFiber *fiber;
    JanetSignal signal;

    fiber = janet_fiber(fn, 64, argc, argv);
    if (NULL == fiber) {
        *out = janet_cstringv("arity mismatch");
        return JANET_SIGNAL_ERROR;
    }
    fiber->env = janet_table(0);
    fiber->env->proto = env;

    signal = janet_continue(fiber, janet_wrap_nil(), out);

    if (signal != JANET_SIGNAL_OK) {
        janet_stacktrace(fiber, *out);
        return 0;
    }
    return 1;
}


static ULONG STDMETHODCALLTYPE Jw32UIAEventHandler_AddRef(Jw32UIAEventHandler *self)
{
    ULONG ret;
    ret = InterlockedIncrement(&(self->_refCount));
    jw32_dbg_val(ret, "%lu");
    return ret;
}

static ULONG STDMETHODCALLTYPE Jw32UIAEventHandler_Release(Jw32UIAEventHandler *self)
{
    ULONG ret = InterlockedDecrement(&(self->_refCount));
    jw32_dbg_val(ret, "%lu");
    if (0 == ret) {
        janet_buffer_deinit(self->marshaled_cb);
        janet_free(self->marshaled_cb);

        janet_buffer_deinit(self->marshaled_env);
        janet_free(self->marshaled_env);

        GlobalFree(self);
    }
    return ret;
}

static HRESULT STDMETHODCALLTYPE Jw32UIAEventHandler_QueryInterface(
    Jw32UIAEventHandler *self,
    REFIID riid,
    void **ppInterface)
{
    if (!IsEqualIID(riid, &IID_IUnknown) && !IsEqualIID(riid, self->_riid)) {
        *ppInterface = NULL;
        return E_NOINTERFACE;
    }

    self->lpVtbl->AddRef(self);
    *ppInterface = (void *)self;
    return S_OK;
}

#define __JANET_TRY()                            \
    JanetTryState __tstate;                      \
    JanetSignal __signal = janet_try(&__tstate); \
    if (JANET_SIGNAL_OK == __signal) {

#define __JANET_TRY_END(__code)                 \
    } else {                                    \
        jw32_dbg_val(__signal, "%d");           \
        jw32_dbg_jval(__tstate.payload);        \
        janet_restore(&__tstate);               \
        return __code;                          \
    }                                           \
    janet_restore(&__tstate);

static HRESULT STDMETHODCALLTYPE Jw32UIAEventHandler_HandleAutomationEvent(
    Jw32UIAEventHandler *self,
    IUIAutomationElement *sender,
    EVENTID eventId)
{
    init_event_handler_thread_vm(self);

    JanetTable *env = uia_thread_state.env;
    JanetFunction *callback;
    Janet argv[2];
    Janet ret;
    /* XXX: i'm not sure this is the right code.... */
    HRESULT hrRet = E_UNEXPECTED;

    __JANET_TRY()

    callback = unmarshal_handler_cb(self);
    argv[0] = jw32_com_make_object_in_env(sender, "IUIAutomationElement", env);
    argv[1] = jw32_wrap_int(eventId);

    __JANET_TRY_END(hrRet)

    if (uia_call_event_handler_fn(callback, 2, argv, env, &ret)) {
        hrRet = jw32_unwrap_hresult(ret);
    }

    return hrRet;
}

static HRESULT STDMETHODCALLTYPE Jw32UIAEventHandler_HandleFocusChangedEvent(
    Jw32UIAEventHandler *self,
    IUIAutomationElement *sender)
{
    init_event_handler_thread_vm(self);

    JanetTable *env = uia_thread_state.env;
    JanetFunction *callback;
    Janet argv[1];
    Janet ret;
    HRESULT hrRet = E_UNEXPECTED;

    __JANET_TRY()

    callback = unmarshal_handler_cb(self);
    argv[0] = jw32_com_make_object_in_env(sender, "IUIAutomationElement", env);

    __JANET_TRY_END(hrRet)

    if (uia_call_event_handler_fn(callback, 1, argv, env, &ret)) {
        hrRet = jw32_unwrap_hresult(ret);
    }

    return hrRet;
}

static HRESULT STDMETHODCALLTYPE Jw32UIAEventHandler_HandlePropertyChangedEvent(
    Jw32UIAEventHandler *self,
    IUIAutomationElement *sender,
    PROPERTYID propertyId,
    VARIANT newValue)
{
    init_event_handler_thread_vm(self);

    JanetTable *env = uia_thread_state.env;
    JanetFunction *callback;
    Janet argv[3];
    Janet ret;
    HRESULT hrRet = E_UNEXPECTED;

    jw32_dbg_val(V_VT(&newValue), "0x%hx");

    __JANET_TRY()

    Janet new_jval = jw32_parse_variant(&newValue);
    callback = unmarshal_handler_cb(self);
    /* TODO: wrap VT_UNKNOWN values in IUnknown */
    argv[0] = jw32_com_make_object_in_env(sender, "IUIAutomationElement", env);
    argv[1] = jw32_wrap_int(propertyId);
    argv[2] = new_jval;

    __JANET_TRY_END(hrRet)

    if (uia_call_event_handler_fn(callback, 3, argv, env, &ret)) {
        hrRet = jw32_unwrap_hresult(ret);
    }

    return hrRet;
}

static HRESULT STDMETHODCALLTYPE Jw32UIAEventHandler_HandleStructureChangedEvent(
    Jw32UIAEventHandler *self,
    IUIAutomationElement *sender,
    enum StructureChangeType changeType,
    SAFEARRAY *runtimeId)
{
    init_event_handler_thread_vm(self);

    JanetTable *env = uia_thread_state.env;
    JanetFunction *callback;
    Janet argv[3];
    Janet ret;
    HRESULT hrRet = E_UNEXPECTED;

    __JANET_TRY()

    callback = unmarshal_handler_cb(self);
    argv[0] = jw32_com_make_object_in_env(sender, "IUIAutomationElement", env);
    argv[1] = jw32_wrap_int(changeType);
    /* opaque pointer, don't need to access its content,
       use IUIAutomation::CompareRuntimeIds() to compare */
    argv[2] = janet_wrap_pointer(runtimeId);

    __JANET_TRY_END(hrRet)

    if (uia_call_event_handler_fn(callback, 3, argv, env, &ret)) {
        hrRet = jw32_unwrap_hresult(ret);
    }

    return hrRet;
}

#undef __JANET_TRY
#undef __JANET_TRY_END


#define __COMMON_METHODS                \
    Jw32UIAEventHandler_QueryInterface, \
    Jw32UIAEventHandler_AddRef,         \
    Jw32UIAEventHandler_Release

static CONST_VTBL Jw32UIAEventHandlerVtbl UIAEventHandler_Vtbl = {
    __COMMON_METHODS,
    (Jw32UIAEventHandlerFunc)Jw32UIAEventHandler_HandleAutomationEvent,
};

static CONST_VTBL Jw32UIAEventHandlerVtbl UIAFocusChangedEventHandler_Vtbl = {
    __COMMON_METHODS,
    (Jw32UIAEventHandlerFunc)Jw32UIAEventHandler_HandleFocusChangedEvent,
};

static CONST_VTBL Jw32UIAEventHandlerVtbl UIAPropertyChangedEventHandler_Vtbl = {
    __COMMON_METHODS,
    (Jw32UIAEventHandlerFunc)Jw32UIAEventHandler_HandlePropertyChangedEvent,
};

static CONST_VTBL Jw32UIAEventHandlerVtbl UIAStructureChangedEventHandler_Vtbl = {
    __COMMON_METHODS,
    (Jw32UIAEventHandlerFunc)Jw32UIAEventHandler_HandleStructureChangedEvent,
};

#undef __COMMON_METHODS


static Jw32UIAEventHandler *create_uia_event_handler(
    REFIID riid,
    CONST_VTBL Jw32UIAEventHandlerVtbl *pVtbl,
    JanetFunction *callback,
    JanetTable *env)
{
    Jw32UIAEventHandler *handler = GlobalAlloc(GPTR, sizeof(Jw32UIAEventHandler));

    if (!handler) {
        return NULL;
    }

    handler->_riid = riid,
    handler->lpVtbl = pVtbl;

    JanetBuffer *marshaled_cb = janet_malloc(sizeof(JanetBuffer));
    if (!marshaled_cb) {
        goto error_cb;
    }
    janet_buffer_init(marshaled_cb, 0);
    janet_marshal(marshaled_cb, janet_wrap_function(callback), NULL, JANET_MARSHAL_UNSAFE);
    handler->marshaled_cb = marshaled_cb;

    JanetBuffer *marshaled_env = janet_malloc(sizeof(JanetBuffer));
    if (!marshaled_env) {
        goto error_env;
    }
    janet_buffer_init(marshaled_env, 0);
    janet_marshal(marshaled_env, janet_wrap_table(env), NULL, JANET_MARSHAL_UNSAFE);
    handler->marshaled_env = marshaled_env;

    handler->lpVtbl->AddRef(handler);

    return handler;

error_env:
    janet_free(marshaled_cb);
error_cb:
    GlobalFree(handler);
    return NULL;
}

/*******************************************************************
 *
 * CONSTANT DEFINITIONS
 *
 *******************************************************************/

static void define_uuids(JanetTable *env)
{
    /* UIAutomation */
    janet_def(env, "CLSID_CUIAutomation", jw32_wrap_refclsid(&CLSID_CUIAutomation),
              "Class ID for CUIAutomation.");
    janet_def(env, "CLSID_CUIAutomation8", jw32_wrap_refclsid(&CLSID_CUIAutomation8),
              "Class ID for CUIAutomation8.");
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

static void define_consts_uia_propertyid(JanetTable *env)
{
#define __def(const_name)                                        \
    janet_def(env, #const_name, jw32_wrap_int(const_name),       \
              "Constant for UI Automation property types.")
    __def(UIA_RuntimeIdPropertyId);
    __def(UIA_BoundingRectanglePropertyId);
    __def(UIA_ProcessIdPropertyId);
    __def(UIA_ControlTypePropertyId);
    __def(UIA_LocalizedControlTypePropertyId);
    __def(UIA_NamePropertyId);
    __def(UIA_AcceleratorKeyPropertyId);
    __def(UIA_AccessKeyPropertyId);
    __def(UIA_HasKeyboardFocusPropertyId);
    __def(UIA_IsKeyboardFocusablePropertyId);
    __def(UIA_IsEnabledPropertyId);
    __def(UIA_AutomationIdPropertyId);
    __def(UIA_ClassNamePropertyId);
    __def(UIA_HelpTextPropertyId);
    __def(UIA_ClickablePointPropertyId);
    __def(UIA_CulturePropertyId);
    __def(UIA_IsControlElementPropertyId);
    __def(UIA_IsContentElementPropertyId);
    __def(UIA_LabeledByPropertyId);
    __def(UIA_IsPasswordPropertyId);
    __def(UIA_NativeWindowHandlePropertyId);
    __def(UIA_ItemTypePropertyId);
    __def(UIA_IsOffscreenPropertyId);
    __def(UIA_OrientationPropertyId);
    __def(UIA_FrameworkIdPropertyId);
    __def(UIA_IsRequiredForFormPropertyId);
    __def(UIA_ItemStatusPropertyId);
    __def(UIA_IsDockPatternAvailablePropertyId);
    __def(UIA_IsExpandCollapsePatternAvailablePropertyId);
    __def(UIA_IsGridItemPatternAvailablePropertyId);
    __def(UIA_IsGridPatternAvailablePropertyId);
    __def(UIA_IsInvokePatternAvailablePropertyId);
    __def(UIA_IsMultipleViewPatternAvailablePropertyId);
    __def(UIA_IsRangeValuePatternAvailablePropertyId);
    __def(UIA_IsScrollPatternAvailablePropertyId);
    __def(UIA_IsScrollItemPatternAvailablePropertyId);
    __def(UIA_IsSelectionItemPatternAvailablePropertyId);
    __def(UIA_IsSelectionPatternAvailablePropertyId);
    __def(UIA_IsTablePatternAvailablePropertyId);
    __def(UIA_IsTableItemPatternAvailablePropertyId);
    __def(UIA_IsTextPatternAvailablePropertyId);
    __def(UIA_IsTogglePatternAvailablePropertyId);
    __def(UIA_IsTransformPatternAvailablePropertyId);
    __def(UIA_IsValuePatternAvailablePropertyId);
    __def(UIA_IsWindowPatternAvailablePropertyId);
    __def(UIA_ValueValuePropertyId);
    __def(UIA_ValueIsReadOnlyPropertyId);
    __def(UIA_RangeValueValuePropertyId);
    __def(UIA_RangeValueIsReadOnlyPropertyId);
    __def(UIA_RangeValueMinimumPropertyId);
    __def(UIA_RangeValueMaximumPropertyId);
    __def(UIA_RangeValueLargeChangePropertyId);
    __def(UIA_RangeValueSmallChangePropertyId);
    __def(UIA_ScrollHorizontalScrollPercentPropertyId);
    __def(UIA_ScrollHorizontalViewSizePropertyId);
    __def(UIA_ScrollVerticalScrollPercentPropertyId);
    __def(UIA_ScrollVerticalViewSizePropertyId);
    __def(UIA_ScrollHorizontallyScrollablePropertyId);
    __def(UIA_ScrollVerticallyScrollablePropertyId);
    __def(UIA_SelectionSelectionPropertyId);
    __def(UIA_SelectionCanSelectMultiplePropertyId);
    __def(UIA_SelectionIsSelectionRequiredPropertyId);
    __def(UIA_GridRowCountPropertyId);
    __def(UIA_GridColumnCountPropertyId);
    __def(UIA_GridItemRowPropertyId);
    __def(UIA_GridItemColumnPropertyId);
    __def(UIA_GridItemRowSpanPropertyId);
    __def(UIA_GridItemColumnSpanPropertyId);
    __def(UIA_GridItemContainingGridPropertyId);
    __def(UIA_DockDockPositionPropertyId);
    __def(UIA_ExpandCollapseExpandCollapseStatePropertyId);
    __def(UIA_MultipleViewCurrentViewPropertyId);
    __def(UIA_MultipleViewSupportedViewsPropertyId);
    __def(UIA_WindowCanMaximizePropertyId);
    __def(UIA_WindowCanMinimizePropertyId);
    __def(UIA_WindowWindowVisualStatePropertyId);
    __def(UIA_WindowWindowInteractionStatePropertyId);
    __def(UIA_WindowIsModalPropertyId);
    __def(UIA_WindowIsTopmostPropertyId);
    __def(UIA_SelectionItemIsSelectedPropertyId);
    __def(UIA_SelectionItemSelectionContainerPropertyId);
    __def(UIA_TableRowHeadersPropertyId);
    __def(UIA_TableColumnHeadersPropertyId);
    __def(UIA_TableRowOrColumnMajorPropertyId);
    __def(UIA_TableItemRowHeaderItemsPropertyId);
    __def(UIA_TableItemColumnHeaderItemsPropertyId);
    __def(UIA_ToggleToggleStatePropertyId);
    __def(UIA_TransformCanMovePropertyId);
    __def(UIA_TransformCanResizePropertyId);
    __def(UIA_TransformCanRotatePropertyId);
    __def(UIA_IsLegacyIAccessiblePatternAvailablePropertyId);
    __def(UIA_LegacyIAccessibleChildIdPropertyId);
    __def(UIA_LegacyIAccessibleNamePropertyId);
    __def(UIA_LegacyIAccessibleValuePropertyId);
    __def(UIA_LegacyIAccessibleDescriptionPropertyId);
    __def(UIA_LegacyIAccessibleRolePropertyId);
    __def(UIA_LegacyIAccessibleStatePropertyId);
    __def(UIA_LegacyIAccessibleHelpPropertyId);
    __def(UIA_LegacyIAccessibleKeyboardShortcutPropertyId);
    __def(UIA_LegacyIAccessibleSelectionPropertyId);
    __def(UIA_LegacyIAccessibleDefaultActionPropertyId);
    __def(UIA_AriaRolePropertyId);
    __def(UIA_AriaPropertiesPropertyId);
    __def(UIA_IsDataValidForFormPropertyId);
    __def(UIA_ControllerForPropertyId);
    __def(UIA_DescribedByPropertyId);
    __def(UIA_FlowsToPropertyId);
    __def(UIA_ProviderDescriptionPropertyId);
    __def(UIA_IsItemContainerPatternAvailablePropertyId);
    __def(UIA_IsVirtualizedItemPatternAvailablePropertyId);
    __def(UIA_IsSynchronizedInputPatternAvailablePropertyId);
    __def(UIA_OptimizeForVisualContentPropertyId);
    __def(UIA_IsObjectModelPatternAvailablePropertyId);
    __def(UIA_AnnotationAnnotationTypeIdPropertyId);
    __def(UIA_AnnotationAnnotationTypeNamePropertyId);
    __def(UIA_AnnotationAuthorPropertyId);
    __def(UIA_AnnotationDateTimePropertyId);
    __def(UIA_AnnotationTargetPropertyId);
    __def(UIA_IsAnnotationPatternAvailablePropertyId);
    __def(UIA_IsTextPattern2AvailablePropertyId);
    __def(UIA_StylesStyleIdPropertyId);
    __def(UIA_StylesStyleNamePropertyId);
    __def(UIA_StylesFillColorPropertyId);
    __def(UIA_StylesFillPatternStylePropertyId);
    __def(UIA_StylesShapePropertyId);
    __def(UIA_StylesFillPatternColorPropertyId);
    __def(UIA_StylesExtendedPropertiesPropertyId);
    __def(UIA_IsStylesPatternAvailablePropertyId);
    __def(UIA_IsSpreadsheetPatternAvailablePropertyId);
    __def(UIA_SpreadsheetItemFormulaPropertyId);
    __def(UIA_SpreadsheetItemAnnotationObjectsPropertyId);
    __def(UIA_SpreadsheetItemAnnotationTypesPropertyId);
    __def(UIA_IsSpreadsheetItemPatternAvailablePropertyId);
    __def(UIA_Transform2CanZoomPropertyId);
    __def(UIA_IsTransformPattern2AvailablePropertyId);
    __def(UIA_LiveSettingPropertyId);
    __def(UIA_IsTextChildPatternAvailablePropertyId);
    __def(UIA_IsDragPatternAvailablePropertyId);
    __def(UIA_DragIsGrabbedPropertyId);
    __def(UIA_DragDropEffectPropertyId);
    __def(UIA_DragDropEffectsPropertyId);
    __def(UIA_IsDropTargetPatternAvailablePropertyId);
    __def(UIA_DropTargetDropTargetEffectPropertyId);
    __def(UIA_DropTargetDropTargetEffectsPropertyId);
    __def(UIA_DragGrabbedItemsPropertyId);
    __def(UIA_Transform2ZoomLevelPropertyId);
    __def(UIA_Transform2ZoomMinimumPropertyId);
    __def(UIA_Transform2ZoomMaximumPropertyId);
    __def(UIA_FlowsFromPropertyId);
    __def(UIA_IsTextEditPatternAvailablePropertyId);
    __def(UIA_IsPeripheralPropertyId);
    __def(UIA_IsCustomNavigationPatternAvailablePropertyId);
    __def(UIA_PositionInSetPropertyId);
    __def(UIA_SizeOfSetPropertyId);
    __def(UIA_LevelPropertyId);
    __def(UIA_AnnotationTypesPropertyId);
    __def(UIA_AnnotationObjectsPropertyId);
    __def(UIA_LandmarkTypePropertyId);
    __def(UIA_LocalizedLandmarkTypePropertyId);
    __def(UIA_FullDescriptionPropertyId);
    __def(UIA_FillColorPropertyId);
    __def(UIA_OutlineColorPropertyId);
    __def(UIA_FillTypePropertyId);
    __def(UIA_VisualEffectsPropertyId);
    __def(UIA_OutlineThicknessPropertyId);
    __def(UIA_CenterPointPropertyId);
    __def(UIA_RotationPropertyId);
    __def(UIA_SizePropertyId);
    __def(UIA_IsSelectionPattern2AvailablePropertyId);
    __def(UIA_Selection2FirstSelectedItemPropertyId);
    __def(UIA_Selection2LastSelectedItemPropertyId);
    __def(UIA_Selection2CurrentSelectedItemPropertyId);
    __def(UIA_Selection2ItemCountPropertyId);
    __def(UIA_HeadingLevelPropertyId);
    __def(UIA_IsDialogPropertyId);
#undef __def
}

static void define_consts_uia_patternid(JanetTable *env)
{
#define __def(const_name)                                        \
    janet_def(env, #const_name, jw32_wrap_int(const_name),       \
              "Constant for UI Automation pattern types.")
    __def(UIA_InvokePatternId);
    __def(UIA_SelectionPatternId);
    __def(UIA_ValuePatternId);
    __def(UIA_RangeValuePatternId);
    __def(UIA_ScrollPatternId);
    __def(UIA_ExpandCollapsePatternId);
    __def(UIA_GridPatternId);
    __def(UIA_GridItemPatternId);
    __def(UIA_MultipleViewPatternId);
    __def(UIA_WindowPatternId);
    __def(UIA_SelectionItemPatternId);
    __def(UIA_DockPatternId);
    __def(UIA_TablePatternId);
    __def(UIA_TableItemPatternId);
    __def(UIA_TextPatternId);
    __def(UIA_TogglePatternId);
    __def(UIA_TransformPatternId);
    __def(UIA_ScrollItemPatternId);
    __def(UIA_LegacyIAccessiblePatternId);
    __def(UIA_ItemContainerPatternId);
    __def(UIA_VirtualizedItemPatternId);
    __def(UIA_SynchronizedInputPatternId);
    __def(UIA_ObjectModelPatternId);
    __def(UIA_AnnotationPatternId);
    __def(UIA_TextPattern2Id);
    __def(UIA_StylesPatternId);
    __def(UIA_SpreadsheetPatternId);
    __def(UIA_SpreadsheetItemPatternId);
    __def(UIA_TransformPattern2Id);
    __def(UIA_TextChildPatternId);
    __def(UIA_DragPatternId);
    __def(UIA_DropTargetPatternId);
    __def(UIA_TextEditPatternId);
    __def(UIA_CustomNavigationPatternId);
    __def(UIA_SelectionPattern2Id);
#undef __def
}

static void define_consts_uia_eventid(JanetTable *env)
{
#define __def(const_name)                                        \
    janet_def(env, #const_name, jw32_wrap_int(const_name),       \
              "Constant for UI Automation event types.")
    __def(UIA_ToolTipOpenedEventId);
    __def(UIA_ToolTipClosedEventId);
    __def(UIA_StructureChangedEventId);
    __def(UIA_MenuOpenedEventId);
    __def(UIA_AutomationPropertyChangedEventId);
    __def(UIA_AutomationFocusChangedEventId);
    __def(UIA_AsyncContentLoadedEventId);
    __def(UIA_MenuClosedEventId);
    __def(UIA_LayoutInvalidatedEventId);
    __def(UIA_Invoke_InvokedEventId);
    __def(UIA_SelectionItem_ElementAddedToSelectionEventId);
    __def(UIA_SelectionItem_ElementRemovedFromSelectionEventId);
    __def(UIA_SelectionItem_ElementSelectedEventId);
    __def(UIA_Selection_InvalidatedEventId);
    __def(UIA_Text_TextSelectionChangedEventId);
    __def(UIA_Text_TextChangedEventId);
    __def(UIA_Window_WindowOpenedEventId);
    __def(UIA_Window_WindowClosedEventId);
    __def(UIA_MenuModeStartEventId);
    __def(UIA_MenuModeEndEventId);
    __def(UIA_InputReachedTargetEventId);
    __def(UIA_InputReachedOtherElementEventId);
    __def(UIA_InputDiscardedEventId);
    __def(UIA_SystemAlertEventId);
    __def(UIA_LiveRegionChangedEventId);
    __def(UIA_HostedFragmentRootsInvalidatedEventId);
    __def(UIA_Drag_DragStartEventId);
    __def(UIA_Drag_DragCancelEventId);
    __def(UIA_Drag_DragCompleteEventId);
    __def(UIA_DropTarget_DragEnterEventId);
    __def(UIA_DropTarget_DragLeaveEventId);
    __def(UIA_DropTarget_DroppedEventId);
    __def(UIA_TextEdit_TextChangedEventId);
    __def(UIA_TextEdit_ConversionTargetChangedEventId);
    __def(UIA_ChangesEventId);
    __def(UIA_NotificationEventId);
    __def(UIA_ActiveTextPositionChangedEventId);
#undef __def
}

static void define_consts_treescope(JanetTable *env)
{
#define __def(const_name)                                        \
    janet_def(env, #const_name, jw32_wrap_int(const_name),       \
              "Constant for UI Automation caching scopes.")
    __def(TreeScope_None);
    __def(TreeScope_Element);
    __def(TreeScope_Children);
    __def(TreeScope_Descendants);
    __def(TreeScope_Parent);
    __def(TreeScope_Ancestors);
    __def(TreeScope_Subtree);
#undef __def
}


/*******************************************************************
 *
 * IUIAutomation
 *
 *******************************************************************/

static Janet IUIAutomation_CheckNotSupported(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    IUnknown *unk; /* XXX: internal detail, may change at some point */
    VARIANT value;

    HRESULT hrRet;
    BOOL isNotSupported = 0;

    janet_fixarity(argc, 2);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    unk = (IUnknown *)janet_getpointer(argv, 1);
    VariantInit(&value);
    V_VT(&value) = VT_UNKNOWN;
    V_UNKNOWN(&value) = unk;

    hrRet = self->lpVtbl->CheckNotSupported(self, value, &isNotSupported);

    JW32_HR_RETURN_OR_PANIC(hrRet, jw32_wrap_bool(isNotSupported));
}

static Janet IUIAutomation_GetRootElement(int32_t argc, Janet *argv)
{
    IUIAutomation *self;

    HRESULT hrRet;
    IUIAutomationElement *root = NULL;

    janet_fixarity(argc, 1);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->GetRootElement(self, &root);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            root,
            "IUIAutomationElement",
            uia_thread_state.env));
}

static Janet IUIAutomation_GetRootElementBuildCache(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    IUIAutomationCacheRequest *cacheRequest;

    HRESULT hrRet;
    IUIAutomationElement *root = NULL;

    janet_fixarity(argc, 2);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    cacheRequest = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 1);
    hrRet = self->lpVtbl->GetRootElementBuildCache(self, cacheRequest, &root);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            root,
            "IUIAutomationElement",
            uia_thread_state.env));
}

static Janet IUIAutomation_GetFocusedElement(int32_t argc, Janet *argv)
{
    IUIAutomation *self;

    HRESULT hrRet;
    IUIAutomationElement *element = NULL;

    janet_fixarity(argc, 1);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->GetFocusedElement(self, &element);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            element,
            "IUIAutomationElement",
            uia_thread_state.env));
}

static Janet IUIAutomation_CreateCacheRequest(int32_t argc, Janet *argv)
{
    IUIAutomation *self;

    HRESULT hrRet;
    IUIAutomationCacheRequest *cacheRequest;

    janet_fixarity(argc, 1);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->CreateCacheRequest(self, &cacheRequest);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            cacheRequest,
            "IUIAutomationCacheRequest",
            uia_thread_state.env));
}

static Janet IUIAutomation_CreateTrueCondition(int32_t argc, Janet *argv)
{
    IUIAutomation *self;

    HRESULT hrRet;
    IUIAutomationCondition *newCondition;

    janet_fixarity(argc, 1);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->CreateTrueCondition(self, &newCondition);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            newCondition,
            "IUIAutomationCondition",
            uia_thread_state.env));
}

static Janet IUIAutomation_CreateFalseCondition(int32_t argc, Janet *argv)
{
    IUIAutomation *self;

    HRESULT hrRet;
    IUIAutomationCondition *newCondition;

    janet_fixarity(argc, 1);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->CreateFalseCondition(self, &newCondition);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            newCondition,
            "IUIAutomationCondition",
            uia_thread_state.env));
}

static Janet IUIAutomation_CreateNotCondition(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    IUIAutomationCondition *condition;

    HRESULT hrRet;
    IUIAutomationCondition *newCondition = NULL;

    janet_fixarity(argc, 2);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    condition = (IUIAutomationCondition *)jw32_com_get_obj_ref(argv, 1);

    hrRet = self->lpVtbl->CreateNotCondition(self, condition, &newCondition);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            newCondition,
            "IUIAutomationCondition",
            uia_thread_state.env));
}

static Janet IUIAutomation_CreateOrCondition(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    IUIAutomationCondition *condition1;
    IUIAutomationCondition *condition2;

    HRESULT hrRet;
    IUIAutomationCondition *newCondition = NULL;

    janet_fixarity(argc, 3);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    condition1 = (IUIAutomationCondition *)jw32_com_get_obj_ref(argv, 1);
    condition2 = (IUIAutomationCondition *)jw32_com_get_obj_ref(argv, 2);
    hrRet = self->lpVtbl->CreateOrCondition(self, condition1, condition2, &newCondition);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            newCondition,
            "IUIAutomationCondition",
            uia_thread_state.env));
}

static Janet IUIAutomation_CreateOrConditionFromArray(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    JanetView cond_view;

    HRESULT hrRet;
    IUIAutomationCondition *newCondition = NULL;

    SAFEARRAY *conditions;

    janet_fixarity(argc, 2);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    cond_view = janet_getindexed(argv, 1);
    conditions = make_condition_safearray(cond_view);

    hrRet = self->lpVtbl->CreateOrConditionFromArray(self, conditions, &newCondition);

    SafeArrayDestroy(conditions);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            newCondition,
            "IUIAutomationCondition",
            uia_thread_state.env));
}

static Janet IUIAutomation_CreateAndCondition(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    IUIAutomationCondition *condition1;
    IUIAutomationCondition *condition2;

    HRESULT hrRet;
    IUIAutomationCondition *newCondition = NULL;

    janet_fixarity(argc, 3);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    condition1 = (IUIAutomationCondition *)jw32_com_get_obj_ref(argv, 1);
    condition2 = (IUIAutomationCondition *)jw32_com_get_obj_ref(argv, 2);
    hrRet = self->lpVtbl->CreateAndCondition(self, condition1, condition2, &newCondition);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            newCondition,
            "IUIAutomationCondition",
            uia_thread_state.env));
}

static Janet IUIAutomation_CreateAndConditionFromArray(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    JanetView cond_view;

    HRESULT hrRet;
    IUIAutomationCondition *newCondition = NULL;

    SAFEARRAY *conditions;

    janet_fixarity(argc, 2);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    cond_view = janet_getindexed(argv, 1);
    conditions = make_condition_safearray(cond_view);

    hrRet = self->lpVtbl->CreateAndConditionFromArray(self, conditions, &newCondition);

    SafeArrayDestroy(conditions);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            newCondition,
            "IUIAutomationCondition",
            uia_thread_state.env));
}

static Janet IUIAutomation_AddAutomationEventHandler(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    EVENTID eventId;
    IUIAutomationElement *element;
    enum TreeScope scope;
    IUIAutomationCacheRequest *cacheRequest;
    JanetFunction *callback;

    HRESULT hrRet;

    Jw32UIAEventHandler *handler;

    janet_fixarity(argc, 6);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    eventId = jw32_get_int(argv, 1);
    element = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 2);
    scope = jw32_get_int(argv, 3);
    cacheRequest = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 4);
    callback = janet_getfunction(argv, 5);

    handler = create_uia_event_handler(&IID_IUIAutomationEventHandler,
                                       &UIAEventHandler_Vtbl,
                                       callback,
                                       uia_thread_state.env);
    if (!handler) {
        janet_panicv(JW32_HRESULT_ERRORV(E_OUTOFMEMORY));
    }

    hrRet = self->lpVtbl->AddAutomationEventHandler(self,
                                                    eventId,
                                                    element,
                                                    scope,
                                                    cacheRequest,
                                                    (IUIAutomationEventHandler *)handler);

    if (FAILED(hrRet)) {
        handler->lpVtbl->Release(handler);
    }

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            handler,
            "IUnknown",
            uia_thread_state.env));
}

static Janet IUIAutomation_AddFocusChangedEventHandler(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    IUIAutomationCacheRequest *cacheRequest;
    JanetFunction *callback;

    HRESULT hrRet;

    Jw32UIAEventHandler *handler;

    janet_fixarity(argc, 3);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    cacheRequest = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 1);
    callback = janet_getfunction(argv, 2);

    handler = create_uia_event_handler(&IID_IUIAutomationFocusChangedEventHandler,
                                       &UIAFocusChangedEventHandler_Vtbl,
                                       callback,
                                       uia_thread_state.env);
    if (!handler) {
        janet_panicv(JW32_HRESULT_ERRORV(E_OUTOFMEMORY));
    }

    hrRet = self->lpVtbl->AddFocusChangedEventHandler(self,
                                                      cacheRequest,
                                                      (IUIAutomationFocusChangedEventHandler *)handler);

    if (FAILED(hrRet)) {
        handler->lpVtbl->Release(handler);
    }

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            handler,
            "IUnknown",
            uia_thread_state.env));
}

static Janet IUIAutomation_AddPropertyChangedEventHandler(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    IUIAutomationElement *element;
    enum TreeScope scope;
    IUIAutomationCacheRequest *cacheRequest;
    JanetFunction *callback;
    JanetView properties;

    HRESULT hrRet;

    Jw32UIAEventHandler *handler;
    SAFEARRAY *propertyArray;

    janet_fixarity(argc, 6);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    element = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 1);
    scope = jw32_get_int(argv, 2);
    cacheRequest = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 3);
    callback = janet_getfunction(argv, 4);
    properties = janet_getindexed(argv, 5);

    propertyArray = SafeArrayCreateVector(VT_INT, 0, properties.len);
    if (!propertyArray) {
        janet_panicv(JW32_HRESULT_ERRORV(E_OUTOFMEMORY));
    }
    for (LONG i = 0; i < properties.len; i++) {
        Janet item = properties.items[i];
        if (!janet_checkint(item)) {
            SafeArrayDestroy(propertyArray);
            janet_panicf("bad property #%d: expected an integer, got %v", i, item);
        }
        PROPERTYID pid = jw32_unwrap_int(item);
        HRESULT hr = SafeArrayPutElement(propertyArray, &i, &pid);
        jw32_dbg_val(hr, "%d");
    }

    handler = create_uia_event_handler(&IID_IUIAutomationPropertyChangedEventHandler,
                                       &UIAPropertyChangedEventHandler_Vtbl,
                                       callback,
                                       uia_thread_state.env);
    if (!handler) {
        SafeArrayDestroy(propertyArray);
        janet_panicv(JW32_HRESULT_ERRORV(E_OUTOFMEMORY));
    }

    hrRet = self->lpVtbl->AddPropertyChangedEventHandler(self,
                                                         element,
                                                         scope,
                                                         cacheRequest,
                                                         (IUIAutomationPropertyChangedEventHandler *)handler,
                                                         propertyArray);

    SafeArrayDestroy(propertyArray);

    if (FAILED(hrRet)) {
        handler->lpVtbl->Release(handler);
    }

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            handler,
            "IUnknown",
            uia_thread_state.env));
}

static Janet IUIAutomation_AddStructureChangedEventHandler(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    IUIAutomationElement *element;
    enum TreeScope scope;
    IUIAutomationCacheRequest *cacheRequest;
    JanetFunction *callback;

    HRESULT hrRet;

    Jw32UIAEventHandler *handler;

    janet_fixarity(argc, 5);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    element = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 1);
    scope = jw32_get_int(argv, 2);
    cacheRequest = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 3);
    callback = janet_getfunction(argv, 4);

    handler = create_uia_event_handler(&IID_IUIAutomationStructureChangedEventHandler,
                                       &UIAStructureChangedEventHandler_Vtbl,
                                       callback,
                                       uia_thread_state.env);
    if (!handler) {
        janet_panicv(JW32_HRESULT_ERRORV(E_OUTOFMEMORY));
    }

    hrRet = self->lpVtbl->AddStructureChangedEventHandler(self,
                                                          element,
                                                          scope,
                                                          cacheRequest,
                                                          (IUIAutomationStructureChangedEventHandler *)handler);

    if (FAILED(hrRet)) {
        handler->lpVtbl->Release(handler);
    }

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            handler,
            "IUnknown",
            uia_thread_state.env));
}

static Janet IUIAutomation_CompareElements(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    IUIAutomationElement *el1, *el2;

    HRESULT hrRet;
    BOOL areSame = 0;

    janet_fixarity(argc, 3);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    el1 = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 1);
    el2 = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 2);

    hrRet = self->lpVtbl->CompareElements(self, el1, el2, &areSame);

    JW32_HR_RETURN_OR_PANIC(hrRet, jw32_wrap_bool(areSame));
}

static Janet IUIAutomation_CompareRuntimeIds(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    SAFEARRAY *runtimeId1, *runtimeId2;

    HRESULT hrRet;
    BOOL areSame = 0;

    janet_fixarity(argc, 3);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    runtimeId1 = janet_getpointer(argv, 1);
    runtimeId2 = janet_getpointer(argv, 2);

    hrRet = self->lpVtbl->CompareRuntimeIds(self, runtimeId1, runtimeId2, &areSame);

    JW32_HR_RETURN_OR_PANIC(hrRet, jw32_wrap_bool(areSame));
}

static Janet IUIAutomation_RemoveAllEventHandlers(int32_t argc, Janet *argv)
{
    IUIAutomation *self;

    HRESULT hrRet;

    janet_fixarity(argc, 1);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->RemoveAllEventHandlers(self);

    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_nil());
}

static Janet IUIAutomation_RemoveAutomationEventHandler(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    EVENTID eventId;
    IUIAutomationElement *element;
    IUIAutomationEventHandler *handler;

    HRESULT hrRet;

    janet_fixarity(argc, 4);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    eventId = jw32_get_int(argv, 1);
    element = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 2);
    handler = (IUIAutomationEventHandler *)jw32_com_get_obj_ref(argv, 3);

    hrRet = self->lpVtbl->RemoveAutomationEventHandler(self, eventId, element, handler);

    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_nil());
}

static Janet IUIAutomation_RemoveFocusChangedEventHandler(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    IUIAutomationFocusChangedEventHandler *handler;

    HRESULT hrRet;

    janet_fixarity(argc, 2);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    handler = (IUIAutomationFocusChangedEventHandler *)jw32_com_get_obj_ref(argv, 1);

    hrRet = self->lpVtbl->RemoveFocusChangedEventHandler(self, handler);

    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_nil());
}

static Janet IUIAutomation_RemovePropertyChangedEventHandler(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    IUIAutomationElement *element;
    IUIAutomationPropertyChangedEventHandler *handler;

    HRESULT hrRet;

    janet_fixarity(argc, 3);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    element = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 1);
    handler = (IUIAutomationPropertyChangedEventHandler *)jw32_com_get_obj_ref(argv, 2);

    hrRet = self->lpVtbl->RemovePropertyChangedEventHandler(self, element, handler);

    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_nil());
}

static Janet IUIAutomation_RemoveStructureChangedEventHandler(int32_t argc, Janet *argv)
{
    IUIAutomation *self;
    IUIAutomationElement *element;
    IUIAutomationStructureChangedEventHandler *handler;

    HRESULT hrRet;

    janet_fixarity(argc, 3);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    element = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 1);
    handler = (IUIAutomationStructureChangedEventHandler *)jw32_com_get_obj_ref(argv, 2);

    hrRet = self->lpVtbl->RemoveStructureChangedEventHandler(self, element, handler);

    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_nil());
}

DEFINE_OBJ_PROPERTY_GETTER(IUIAutomation, ContentViewCondition, IUIAutomationCondition)
DEFINE_OBJ_PROPERTY_GETTER(IUIAutomation, ControlViewCondition, IUIAutomationCondition)
DEFINE_OBJ_PROPERTY_GETTER(IUIAutomation, RawViewCondition, IUIAutomationCondition)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomation, ReservedMixedAttributeValue, IUnknown *, handle)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomation, ReservedNotSupportedValue, IUnknown *, handle)

static const JanetMethod IUIAutomation_methods[] = {
    {"AddAutomationEventHandler", IUIAutomation_AddAutomationEventHandler},
    {"AddFocusChangedEventHandler", IUIAutomation_AddFocusChangedEventHandler},
    {"AddPropertyChangedEventHandler", IUIAutomation_AddPropertyChangedEventHandler},
    {"AddStructureChangedEventHandler", IUIAutomation_AddStructureChangedEventHandler},

    {"CheckNotSupported", IUIAutomation_CheckNotSupported},
    {"CompareElements", IUIAutomation_CompareElements},
    {"CompareRuntimeIds", IUIAutomation_CompareRuntimeIds},

    {"CreateAndCondition", IUIAutomation_CreateAndCondition},
    {"CreateAndConditionFromArray", IUIAutomation_CreateAndConditionFromArray},
    {"CreateCacheRequest", IUIAutomation_CreateCacheRequest},
    {"CreateFalseCondition", IUIAutomation_CreateFalseCondition},
    {"CreateNotCondition", IUIAutomation_CreateNotCondition},
    {"CreateOrCondition", IUIAutomation_CreateOrCondition},
    {"CreateOrConditionFromArray", IUIAutomation_CreateOrConditionFromArray},
    {"CreateTrueCondition", IUIAutomation_CreateTrueCondition},

    {"GetFocusedElement", IUIAutomation_GetFocusedElement},
    {"GetRootElement", IUIAutomation_GetRootElement},
    {"GetRootElementBuildCache", IUIAutomation_GetRootElementBuildCache},

    {"RemoveAllEventHandlers", IUIAutomation_RemoveAllEventHandlers},
    {"RemoveAutomationEventHandler", IUIAutomation_RemoveAutomationEventHandler},
    {"RemoveFocusChangedEventHandler", IUIAutomation_RemoveFocusChangedEventHandler},
    {"RemovePropertyChangedEventHandler", IUIAutomation_RemovePropertyChangedEventHandler},
    {"RemoveStructureChangedEventHandler", IUIAutomation_RemoveStructureChangedEventHandler},

    PROPERTY_GETTER_METHOD(IUIAutomation, ContentViewCondition),
    PROPERTY_GETTER_METHOD(IUIAutomation, ControlViewCondition),
    PROPERTY_GETTER_METHOD(IUIAutomation, RawViewCondition),
    PROPERTY_GETTER_METHOD(IUIAutomation, ReservedMixedAttributeValue),
    PROPERTY_GETTER_METHOD(IUIAutomation, ReservedNotSupportedValue),

    {NULL, NULL},
};


/*******************************************************************
 *
 * IUIAutomationElement
 *
 *******************************************************************/

static Janet IUIAutomationElement_BuildUpdatedCache(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;
    IUIAutomationCacheRequest *cacheRequest;

    HRESULT hrRet;
    IUIAutomationElement *updatedElement = NULL;

    janet_fixarity(argc, 2);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    cacheRequest = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 1);
    hrRet = self->lpVtbl->BuildUpdatedCache(self, cacheRequest, &updatedElement);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            updatedElement,
            "IUIAutomationElement",
            uia_thread_state.env));
}

static Janet IUIAutomationElement_FindAll(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;
    enum TreeScope scope;
    IUIAutomationCondition *condition;

    HRESULT hrRet;
    IUIAutomationElementArray *found = NULL;

    janet_fixarity(argc, 3);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    scope = jw32_get_int(argv, 1);
    condition = (IUIAutomationCondition *)jw32_com_get_obj_ref(argv, 2);
    hrRet = self->lpVtbl->FindAll(self, scope, condition, &found);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            found,
            "IUIAutomationElementArray",
            uia_thread_state.env));
}

static Janet IUIAutomationElement_FindAllBuildCache(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;
    enum TreeScope scope;
    IUIAutomationCondition *condition;
    IUIAutomationCacheRequest *cacheRequest;

    HRESULT hrRet;
    IUIAutomationElementArray *found = NULL;

    janet_fixarity(argc, 4);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    scope = jw32_get_int(argv, 1);
    condition = (IUIAutomationCondition *)jw32_com_get_obj_ref(argv, 2);
    cacheRequest = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 3);
    hrRet = self->lpVtbl->FindAllBuildCache(self, scope, condition, cacheRequest, &found);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            found,
            "IUIAutomationElementArray",
            uia_thread_state.env));
}

static Janet IUIAutomationElement_FindFirst(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;
    enum TreeScope scope;
    IUIAutomationCondition *condition;

    HRESULT hrRet;
    IUIAutomationElement *found = NULL;

    janet_fixarity(argc, 3);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    scope = jw32_get_int(argv, 1);
    condition = (IUIAutomationCondition *)jw32_com_get_obj_ref(argv, 2);
    hrRet = self->lpVtbl->FindFirst(self, scope, condition, &found);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            found,
            "IUIAutomationElement",
            uia_thread_state.env));
}

static Janet IUIAutomationElement_FindFirstBuildCache(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;
    enum TreeScope scope;
    IUIAutomationCondition *condition;
    IUIAutomationCacheRequest *cacheRequest;

    HRESULT hrRet;
    IUIAutomationElement *found = NULL;

    janet_fixarity(argc, 4);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    scope = jw32_get_int(argv, 1);
    condition = (IUIAutomationCondition *)jw32_com_get_obj_ref(argv, 2);
    cacheRequest = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 3);
    hrRet = self->lpVtbl->FindFirst(self, scope, condition, &found);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            found,
            "IUIAutomationElement",
            uia_thread_state.env));
}

/* TODO: the api returns NULL arrays when there are no cached children */
static Janet IUIAutomationElement_GetCachedChildren(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;

    HRESULT hrRet;
    IUIAutomationElementArray *children = NULL;

    janet_fixarity(argc, 1);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->GetCachedChildren(self, &children);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            children,
            "IUIAutomationElementArray",
            uia_thread_state.env));
}

static Janet IUIAutomationElement_GetCachedParent(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;

    HRESULT hrRet;
    IUIAutomationElement *parent = NULL;

    janet_fixarity(argc, 1);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->GetCachedParent(self, &parent);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            parent,
            "IUIAutomationElement",
            uia_thread_state.env));
}

static Janet IUIAutomationElement_GetCurrentPattern(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;
    PATTERNID patternId;

    HRESULT hrRet;
    IUnknown *patternObject = NULL;

    janet_fixarity(argc, 2);
    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    patternId = jw32_get_int(argv, 1);
    hrRet = self->lpVtbl->GetCurrentPattern(self, patternId, &patternObject);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            patternObject,
            "IUnknown",
            uia_thread_state.env));
}

static Janet IUIAutomationElement_GetCachedPattern(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;
    PATTERNID patternId;

    HRESULT hrRet;
    IUnknown *patternObject = NULL;

    janet_fixarity(argc, 2);
    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    patternId = jw32_get_int(argv, 1);
    hrRet = self->lpVtbl->GetCachedPattern(self, patternId, &patternObject);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            patternObject,
            "IUnknown",
            uia_thread_state.env));
}

static Janet IUIAutomationElement_GetCurrentPatternAs(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;
    PATTERNID patternId;
    JanetTable *if_proto;

    HRESULT hrRet;
    void *patternObject = NULL;

    janet_fixarity(argc, 3);
    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    patternId = jw32_get_int(argv, 1);
    if_proto = janet_gettable(argv, 2);

    REFIID riid = jw32_com_normalize_iid(if_proto);

    hrRet = self->lpVtbl->GetCurrentPatternAs(self, patternId, riid, &patternObject);

    JW32_HR_RETURN_OR_PANIC(hrRet, jw32_com_make_object(patternObject, if_proto));
}

static Janet IUIAutomationElement_GetCachedPatternAs(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;
    PATTERNID patternId;
    JanetTable *if_proto;

    HRESULT hrRet;
    void *patternObject = NULL;

    janet_fixarity(argc, 3);
    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    patternId = jw32_get_int(argv, 1);
    if_proto = janet_gettable(argv, 2);

    REFIID riid = jw32_com_normalize_iid(if_proto);

    hrRet = self->lpVtbl->GetCachedPatternAs(self, patternId, riid, &patternObject);

    JW32_HR_RETURN_OR_PANIC(hrRet, jw32_com_make_object(patternObject, if_proto));
}

static Janet IUIAutomationElement_GetCurrentPropertyValue(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;
    PROPERTYID propertyId;

    HRESULT hrRet;
    VARIANT retVal;

    janet_fixarity(argc, 2);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    propertyId = jw32_get_int(argv, 1);

    hrRet = self->lpVtbl->GetCurrentPropertyValue(self, propertyId, &retVal);

    if (SUCCEEDED(hrRet)) {
        jw32_dbg_val(V_VT(&retVal), "0x%x");
    }

    JW32_HR_RETURN_OR_PANIC(hrRet, jw32_parse_variant(&retVal));
}

static Janet IUIAutomationElement_GetCachedPropertyValue(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;
    PROPERTYID propertyId;

    HRESULT hrRet;
    VARIANT retVal;

    janet_fixarity(argc, 2);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    propertyId = jw32_get_int(argv, 1);

    hrRet = self->lpVtbl->GetCachedPropertyValue(self, propertyId, &retVal);

    if (SUCCEEDED(hrRet)) {
        jw32_dbg_val(V_VT(&retVal), "0x%x");
    }

    JW32_HR_RETURN_OR_PANIC(hrRet, jw32_parse_variant(&retVal));
}

static Janet IUIAutomationElement_GetCurrentPropertyValueEx(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;
    PROPERTYID propertyId;
    BOOL ignoreDefaultValue;

    HRESULT hrRet;
    VARIANT retVal;

    janet_fixarity(argc, 3);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    propertyId = jw32_get_int(argv, 1);
    ignoreDefaultValue = jw32_get_bool(argv, 2);

    hrRet = self->lpVtbl->GetCurrentPropertyValueEx(self, propertyId, ignoreDefaultValue, &retVal);

    if (SUCCEEDED(hrRet)) {
        jw32_dbg_val(V_VT(&retVal), "0x%x");
    }

    /* XXX: a VARIANT of type VT_UNKNOWN can mean this kind of property is not supported.
       see IUIAutomation_CheckNotSupported() */
    JW32_HR_RETURN_OR_PANIC(hrRet, jw32_parse_variant(&retVal));
}

static Janet IUIAutomationElement_GetCachedPropertyValueEx(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;
    PROPERTYID propertyId;
    BOOL ignoreDefaultValue;

    HRESULT hrRet;
    VARIANT retVal;

    janet_fixarity(argc, 3);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    propertyId = jw32_get_int(argv, 1);
    ignoreDefaultValue = jw32_get_bool(argv, 2);

    hrRet = self->lpVtbl->GetCachedPropertyValueEx(self, propertyId, ignoreDefaultValue, &retVal);

    if (SUCCEEDED(hrRet)) {
        jw32_dbg_val(V_VT(&retVal), "0x%x");
    }

    JW32_HR_RETURN_OR_PANIC(hrRet, jw32_parse_variant(&retVal));
}

static Janet IUIAutomationElement_GetClickablePoint(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;

    HRESULT hrRet;
    POINT clickable = {0, 0};
    BOOL gotClickable = 0;
    Janet ret_tuple[2];

    janet_fixarity(argc, 1);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->GetClickablePoint(self, &clickable, &gotClickable);

    if (S_OK == hrRet) {
        ret_tuple[0] = janet_wrap_tuple(jw32_point_to_tuple(&clickable));
        ret_tuple[1] = jw32_wrap_bool(gotClickable);
    }

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        janet_wrap_tuple(janet_tuple_n(ret_tuple, 2)));
}

static Janet IUIAutomationElement_GetRuntimeId(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;

    HRESULT hrRet;
    SAFEARRAY *runtimeId = NULL;

    janet_fixarity(argc, 1);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->GetRuntimeId(self, &runtimeId);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        /* opaque pointer, don't need to access its content,
           use IUIAutomation::CompareRuntimeIds() to compare */
        janet_wrap_pointer(runtimeId));
}

static Janet IUIAutomationElement_SetFocus(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;
    HRESULT hrRet;

    janet_fixarity(argc, 1);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->SetFocus(self);

    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_nil());
}

DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CurrentAcceleratorKey)
DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CachedAcceleratorKey)

DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CurrentAccessKey)
DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CachedAccessKey)

DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CurrentAriaProperties)
DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CachedAriaProperties)

DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CurrentAriaRole)
DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CachedAriaRole)

DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CurrentAutomationId)
DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CachedAutomationId)

static Janet PROPERTY_GETTER(IUIAutomationElement, CurrentBoundingRectangle)(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;
    HRESULT hrRet;
    RECT out = {0, 0, 0, 0};

    janet_fixarity(argc, 1);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->get_CurrentBoundingRectangle(self, &out);

    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_table(jw32_rect_to_table(&out)));
}

static Janet PROPERTY_GETTER(IUIAutomationElement, CachedBoundingRectangle)(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;
    HRESULT hrRet;
    RECT out = {0, 0, 0, 0};

    janet_fixarity(argc, 1);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->get_CachedBoundingRectangle(self, &out);

    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_table(jw32_rect_to_table(&out)));
}

DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CurrentClassName)
DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CachedClassName)

DEFINE_OBJ_PROPERTY_GETTER(IUIAutomationElement, CurrentControllerFor, IUIAutomationElementArray)
DEFINE_OBJ_PROPERTY_GETTER(IUIAutomationElement, CachedControllerFor, IUIAutomationElementArray)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentControlType, CONTROLTYPEID, int)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedControlType, CONTROLTYPEID, int)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentCulture, int, int)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedCulture, int, int)

DEFINE_OBJ_PROPERTY_GETTER(IUIAutomationElement, CurrentDescribedBy, IUIAutomationElementArray)
DEFINE_OBJ_PROPERTY_GETTER(IUIAutomationElement, CachedDescribedBy, IUIAutomationElementArray)

DEFINE_OBJ_PROPERTY_GETTER(IUIAutomationElement, CurrentFlowsTo, IUIAutomationElementArray)
DEFINE_OBJ_PROPERTY_GETTER(IUIAutomationElement, CachedFlowsTo, IUIAutomationElementArray)

DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CurrentFrameworkId)
DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CachedFrameworkId)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentHasKeyboardFocus, BOOL, bool)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedHasKeyboardFocus, BOOL, bool)

DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CurrentHelpText)
DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CachedHelpText)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentIsContentElement, BOOL, bool)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedIsContentElement, BOOL, bool)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentIsControlElement, BOOL, bool)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedIsControlElement, BOOL, bool)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentIsDataValidForForm, BOOL, bool)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedIsDataValidForForm, BOOL, bool)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentIsEnabled, BOOL, bool)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedIsEnabled, BOOL, bool)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentIsKeyboardFocusable, BOOL, bool)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedIsKeyboardFocusable, BOOL, bool)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentIsOffscreen, BOOL, bool)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedIsOffscreen, BOOL, bool)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentIsPassword, BOOL, bool)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedIsPassword, BOOL, bool)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentIsRequiredForForm, BOOL, bool)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedIsRequiredForForm, BOOL, bool)

DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CurrentItemStatus)
DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CachedItemStatus)

DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CurrentItemType)
DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CachedItemType)

DEFINE_OBJ_PROPERTY_GETTER(IUIAutomationElement, CurrentLabeledBy, IUIAutomationElement)
DEFINE_OBJ_PROPERTY_GETTER(IUIAutomationElement, CachedLabeledBy, IUIAutomationElement)

DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CurrentLocalizedControlType)
DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CachedLocalizedControlType)

DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CurrentName)
DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CachedName)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentNativeWindowHandle, UIA_HWND, handle)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedNativeWindowHandle, UIA_HWND, handle)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentOrientation, enum OrientationType, int)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedOrientation, enum OrientationType, int)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentProcessId, int, int)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedProcessId, int, int)

DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CurrentProviderDescription)
DEFINE_BSTR_PROPERTY_GETTER(IUIAutomationElement, CachedProviderDescription)

static const JanetMethod IUIAutomationElement_methods[] = {
    {"BuildUpdatedCache", IUIAutomationElement_BuildUpdatedCache},
    {"FindAll", IUIAutomationElement_FindAll},
    {"FindAllBuildCache", IUIAutomationElement_FindAllBuildCache},
    {"FindFirst", IUIAutomationElement_FindFirst},
    {"FindFirstBuildCache", IUIAutomationElement_FindFirstBuildCache},
    {"GetCachedChildren", IUIAutomationElement_GetCachedChildren},
    {"GetCachedParent", IUIAutomationElement_GetCachedParent},
    {"GetCurrentPattern", IUIAutomationElement_GetCurrentPattern},
    {"GetCachedPattern", IUIAutomationElement_GetCachedPattern},
    {"GetCurrentPatternAs", IUIAutomationElement_GetCurrentPatternAs},
    {"GetCachedPatternAs", IUIAutomationElement_GetCachedPatternAs},
    {"GetCurrentPropertyValue", IUIAutomationElement_GetCurrentPropertyValue},
    {"GetCachedPropertyValue", IUIAutomationElement_GetCachedPropertyValue},
    {"GetCurrentPropertyValueEx", IUIAutomationElement_GetCurrentPropertyValueEx},
    {"GetCachedPropertyValueEx", IUIAutomationElement_GetCachedPropertyValueEx},
    {"GetClickablePoint", IUIAutomationElement_GetClickablePoint},
    {"GetRuntimeId", IUIAutomationElement_GetRuntimeId},
    {"SetFocus", IUIAutomationElement_SetFocus},

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentAcceleratorKey),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedAcceleratorKey),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentAccessKey),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedAccessKey),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentAriaProperties),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedAriaProperties),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentAriaRole),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedAriaRole),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentAutomationId),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedAutomationId),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentBoundingRectangle),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedBoundingRectangle),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentClassName),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedClassName),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentControllerFor),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedControllerFor),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentControlType),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedControlType),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentCulture),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedCulture),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentDescribedBy),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedDescribedBy),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentFlowsTo),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedFlowsTo),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentFrameworkId),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedFrameworkId),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentHasKeyboardFocus),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedHasKeyboardFocus),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentHelpText),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedHelpText),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentIsContentElement),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedIsContentElement),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentIsControlElement),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedIsControlElement),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentIsDataValidForForm),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedIsDataValidForForm),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentIsEnabled),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedIsEnabled),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentIsKeyboardFocusable),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedIsKeyboardFocusable),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentIsOffscreen),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedIsOffscreen),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentIsPassword),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedIsPassword),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentIsRequiredForForm),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedIsRequiredForForm),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentItemStatus),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedItemStatus),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentItemType),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedItemType),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentLabeledBy),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedLabeledBy),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentLocalizedControlType),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedLocalizedControlType),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentName),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedName),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentNativeWindowHandle),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedNativeWindowHandle),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentOrientation),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedOrientation),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentProcessId),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedProcessId),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentProviderDescription),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedProviderDescription),

    {NULL, NULL},
};


/*******************************************************************
 *
 * IUIAutomationElementArray
 *
 *******************************************************************/

static Janet IUIAutomationElementArray_GetElement(int32_t argc, Janet *argv)
{
    IUIAutomationElementArray *self;
    int index;

    HRESULT hrRet;
    IUIAutomationElement *element = NULL;

    janet_fixarity(argc, 2);

    self = (IUIAutomationElementArray *)jw32_com_get_obj_ref(argv, 0);
    index = jw32_get_int(argv, 1);
    hrRet = self->lpVtbl->GetElement(self, index, &element);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            element,
            "IUIAutomationElement",
            uia_thread_state.env));
}

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElementArray, Length, int, int)

static const JanetMethod IUIAutomationElementArray_methods[] = {
    {"GetElement", IUIAutomationElementArray_GetElement},

    PROPERTY_GETTER_METHOD(IUIAutomationElementArray, Length),

    {NULL, NULL},
};


/*******************************************************************
 *
 * IUIAutomationCacheRequest
 *
 *******************************************************************/

static Janet IUIAutomationCacheRequest_AddProperty(int32_t argc, Janet *argv)
{
    IUIAutomationCacheRequest *self;
    PROPERTYID propertyId;

    HRESULT hrRet;

    janet_fixarity(argc, 2);

    self = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 0);
    propertyId = jw32_get_int(argv, 1);
    hrRet = self->lpVtbl->AddProperty(self, propertyId);

    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_nil());
}

static Janet IUIAutomationCacheRequest_AddPattern(int32_t argc, Janet *argv)
{
    IUIAutomationCacheRequest *self;
    PATTERNID patternId;

    HRESULT hrRet;

    janet_fixarity(argc, 2);

    self = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 0);
    patternId = jw32_get_int(argv, 1);
    hrRet = self->lpVtbl->AddPattern(self, patternId);

    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_nil());
}

static Janet IUIAutomationCacheRequest_Clone(int32_t argc, Janet *argv)
{
    IUIAutomationCacheRequest *self;
    
    HRESULT hrRet;
    IUIAutomationCacheRequest *clonedRequest = NULL;

    janet_fixarity(argc, 1);

    self = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->Clone(self, &clonedRequest);

    JW32_HR_RETURN_OR_PANIC(
        hrRet,
        jw32_com_make_object_in_env(
            clonedRequest,
            "IUIAutomationCacheRequest",
            uia_thread_state.env));
}

DEFINE_SIMPLE_PROPERTY(IUIAutomationCacheRequest, AutomationElementMode, enum AutomationElementMode, int)
DEFINE_SIMPLE_PROPERTY(IUIAutomationCacheRequest, TreeScope, enum TreeScope, int)
DEFINE_OBJ_PROPERTY(IUIAutomationCacheRequest, TreeFilter, IUIAutomationCondition)

static const JanetMethod IUIAutomationCacheRequest_methods[] = {
    {"AddPattern", IUIAutomationCacheRequest_AddPattern},
    {"AddProperty", IUIAutomationCacheRequest_AddProperty},
    {"Clone", IUIAutomationCacheRequest_Clone},

    PROPERTY_METHODS(IUIAutomationCacheRequest, AutomationElementMode),
    PROPERTY_METHODS(IUIAutomationCacheRequest, TreeScope),
    PROPERTY_METHODS(IUIAutomationCacheRequest, TreeFilter),

    {NULL, NULL},
};


/*******************************************************************
 *
 * IUIAutomationCondition
 *
 *******************************************************************/

static const JanetMethod IUIAutomationCondition_methods[] = {
    {NULL, NULL},
};


/*******************************************************************
 *
 * IUIAutomationAndCondition
 *
 *******************************************************************/

static const JanetMethod IUIAutomationAndCondition_methods[] = {
    {NULL, NULL},
};


/*******************************************************************
 *
 * IUIAutomationBoolCondition
 *
 *******************************************************************/

static const JanetMethod IUIAutomationBoolCondition_methods[] = {
    {NULL, NULL},
};


/*******************************************************************
 *
 * IUIAutomationNotCondition
 *
 *******************************************************************/

static const JanetMethod IUIAutomationNotCondition_methods[] = {
    {NULL, NULL},
};


/*******************************************************************
 *
 * IUIAutomationOrCondition
 *
 *******************************************************************/

static const JanetMethod IUIAutomationOrCondition_methods[] = {
    {NULL, NULL},
};


/*******************************************************************
 *
 * IUIAutomationPropertyCondition
 *
 *******************************************************************/

static const JanetMethod IUIAutomationPropertyCondition_methods[] = {
    {NULL, NULL},
};


/*******************************************************************
 *
 * IUIAutomationTransformPattern
 *
 *******************************************************************/

static Janet IUIAutomationTransformPattern_Move(int32_t argc, Janet *argv)
{
    IUIAutomationTransformPattern *self;
    double x, y;

    HRESULT hrRet;

    janet_fixarity(argc, 3);

    self = (IUIAutomationTransformPattern *)jw32_com_get_obj_ref(argv, 0);
    x = jw32_get_double(argv, 1);
    y = jw32_get_double(argv, 2);
    hrRet = self->lpVtbl->Move(self, x, y);

    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_nil());
}

static Janet IUIAutomationTransformPattern_Resize(int32_t argc, Janet *argv)
{
    IUIAutomationTransformPattern *self;
    double width, height;

    HRESULT hrRet;

    janet_fixarity(argc, 3);

    self = (IUIAutomationTransformPattern *)jw32_com_get_obj_ref(argv, 0);
    width = jw32_get_double(argv, 1);
    height = jw32_get_double(argv, 2);
    hrRet = self->lpVtbl->Resize(self, width, height);

    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_nil());
}

static Janet IUIAutomationTransformPattern_Rotate(int32_t argc, Janet *argv)
{
    IUIAutomationTransformPattern *self;
    double degrees;

    HRESULT hrRet;

    janet_fixarity(argc, 2);

    self = (IUIAutomationTransformPattern *)jw32_com_get_obj_ref(argv, 0);
    degrees = jw32_get_double(argv, 1);
    hrRet = self->lpVtbl->Rotate(self, degrees);

    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_nil());
}

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationTransformPattern, CurrentCanMove, BOOL, bool)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationTransformPattern, CachedCanMove, BOOL, bool)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationTransformPattern, CurrentCanResize, BOOL, bool)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationTransformPattern, CachedCanResize, BOOL, bool)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationTransformPattern, CurrentCanRotate, BOOL, bool)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationTransformPattern, CachedCanRotate, BOOL, bool)

static const JanetMethod IUIAutomationTransformPattern_methods[] = {
    {"Move", IUIAutomationTransformPattern_Move},
    {"Resize", IUIAutomationTransformPattern_Resize},
    {"Rotate", IUIAutomationTransformPattern_Rotate},

    PROPERTY_GETTER_METHOD(IUIAutomationTransformPattern, CurrentCanMove),
    PROPERTY_GETTER_METHOD(IUIAutomationTransformPattern, CachedCanMove),

    PROPERTY_GETTER_METHOD(IUIAutomationTransformPattern, CurrentCanResize),
    PROPERTY_GETTER_METHOD(IUIAutomationTransformPattern, CachedCanResize),

    PROPERTY_GETTER_METHOD(IUIAutomationTransformPattern, CurrentCanRotate),
    PROPERTY_GETTER_METHOD(IUIAutomationTransformPattern, CachedCanRotate),

    {NULL, NULL},
};


/*******************************************************************
 *
 * IUIAutomationWindowPattern
 *
 *******************************************************************/

static const JanetMethod IUIAutomationWindowPattern_methods[] = {
    /* TODO */
    {NULL, NULL},
};


/*******************************************************************
 *
 * MODULE ENTRY & OTHER STUFF
 *
 *******************************************************************/

static void init_table_protos(JanetTable *env)
{
    JanetTable *IUnknown_proto = jw32_com_resolve_iunknown_proto();

    uia_thread_state.env = janet_table(0);
    janet_def(uia_thread_state.env, "IUnknown", janet_wrap_table(IUnknown_proto), NULL);

#define __def_proto(name, parent, doc)                                  \
    do {                                                                \
        ##name##_proto = jw32_com_make_if_proto(#name, ##name##_methods, parent, &IID_##name##); \
        janet_def(env, #name, janet_wrap_table(##name##_proto), doc);   \
        janet_def(uia_thread_state.env, #name, janet_wrap_table(##name##_proto), NULL); \
    } while (0)

    __def_proto(IUIAutomation,
                IUnknown_proto,
                "Prototype for COM IUIAutomation interface.");
    __def_proto(IUIAutomationElement,
                IUnknown_proto,
                "Prototype for COM IUIAutomationElement interface.");
    __def_proto(IUIAutomationElementArray,
                IUnknown_proto,
                "Prototype for COM IUIAutomationElementArray interface.");

    __def_proto(IUIAutomationCacheRequest,
                IUnknown_proto,
                "Prototype for COM IUIAutomationCacheRequest interface.");

    __def_proto(IUIAutomationCondition,
                IUnknown_proto,
                "Prototype for COM IUIAutomationCondition interface.");
    __def_proto(IUIAutomationAndCondition,
                IUIAutomationCondition_proto,
                "Prototype for COM IUIAutomationAndCondition interface.");
    __def_proto(IUIAutomationBoolCondition,
                IUIAutomationCondition_proto,
                "Prototype for COM IUIAutomationBoolCondition interface.");
    __def_proto(IUIAutomationNotCondition,
                IUIAutomationCondition_proto,
                "Prototype for COM IUIAutomationNotCondition interface.");
    __def_proto(IUIAutomationOrCondition,
                IUIAutomationCondition_proto,
                "Prototype for COM IUIAutomationOrCondition interface.");
    __def_proto(IUIAutomationPropertyCondition,
                IUIAutomationCondition_proto,
                "Prototype for COM IUIAutomationPropertyCondition interface.");

    __def_proto(IUIAutomationTransformPattern,
                IUnknown_proto,
                "Prototype for COM IUIAutomationTransformPattern interface.");
    __def_proto(IUIAutomationWindowPattern,
                IUnknown_proto,
                "Prototype for COM IUIAutomationWindowPattern interface.");

#undef __def_proto

    janet_def(env, "uia_thread_state_env", janet_wrap_table(uia_thread_state.env),
              "Environment for UI Automation event handler functions.");
}


JANET_MODULE_ENTRY(JanetTable *env)
{
    /* we are loading this module in the "main" thread */
    uia_thread_state.vm_initialized = 1;

    define_uuids(env);
    define_consts_uia_controltypeid(env);
    define_consts_uia_propertyid(env);
    define_consts_uia_patternid(env);
    define_consts_uia_eventid(env);
    define_consts_treescope(env);

    init_table_protos(env);
}
