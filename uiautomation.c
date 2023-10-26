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
        JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),                   \
                            maybe_make_object(hrRet, out, #__prop_if)); \
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
        JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),                   \
                            jw32_wrap_##__prop_jw32_type##(out));       \
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
        return jw32_wrap_hresult(hrRet);                                \
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
        return jw32_wrap_hresult(hrRet);                                \
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


static inline Janet maybe_make_object(HRESULT hr, LPVOID pv, const char *proto_name)
{
    if (SUCCEEDED(hr)) {
        return jw32_com_make_object_in_env(pv, proto_name, uia_thread_state.env);
    }

    return janet_wrap_nil();
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

    uia_thread_state.env = unmarshal_handler_env(handler);
    uia_thread_state.vm_initialized = 1;
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

static HRESULT STDMETHODCALLTYPE Jw32UIAEventHandler_HandleAutomationEvent(
    Jw32UIAEventHandler *self,
    IUIAutomationElement *sender,
    EVENTID eventId)
{
    init_event_handler_thread_vm(self);

    JanetFunction *callback = unmarshal_handler_cb(self);
    JanetTable *env = uia_thread_state.env;
    Janet argv[] = {
        jw32_com_make_object_in_env(sender, "IUIAutomationElement", env),
        jw32_wrap_int(eventId),
    };
    Janet ret;

    /* XXX: i'm not sure this is the right code.... */
    HRESULT hrRet = E_UNEXPECTED;
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

    JanetFunction *callback = unmarshal_handler_cb(self);
    JanetTable *env = uia_thread_state.env;
    Janet argv[] = {
        jw32_com_make_object_in_env(sender, "IUIAutomationElement", env),
    };
    Janet ret;

    HRESULT hrRet = E_UNEXPECTED;
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

    JanetFunction *callback = unmarshal_handler_cb(self);
    JanetTable *env = uia_thread_state.env;
    Janet argv[] = {
        jw32_com_make_object_in_env(sender, "IUIAutomationElement", env),
        jw32_wrap_int(propertyId),
        /* TODO: VARIANT type */
        janet_wrap_nil(),
    };
    Janet ret;

    HRESULT hrRet = E_UNEXPECTED;
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

    JanetFunction *callback = unmarshal_handler_cb(self);
    JanetTable *env = uia_thread_state.env;
    Janet argv[] = {
        jw32_com_make_object_in_env(sender, "IUIAutomationElement", env),
        jw32_wrap_int(changeType),
        /* TODO: SAFEARRAY type */
        janet_wrap_nil(),
    };
    Janet ret;

    HRESULT hrRet = E_UNEXPECTED;
    if (uia_call_event_handler_fn(callback, 3, argv, env, &ret)) {
        hrRet = jw32_unwrap_hresult(ret);
    }

    return hrRet;
}


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

static Janet IUIAutomation_GetRootElement(int32_t argc, Janet *argv)
{
    IUIAutomation *self;

    HRESULT hrRet;
    IUIAutomationElement *root = NULL;

    janet_fixarity(argc, 1);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->GetRootElement(self, &root);

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        maybe_make_object(hrRet, root, "IUIAutomationElement"));
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

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        maybe_make_object(hrRet, root, "IUIAutomationElement"));
}

static Janet IUIAutomation_GetFocusedElement(int32_t argc, Janet *argv)
{
    IUIAutomation *self;

    HRESULT hrRet;
    IUIAutomationElement *element = NULL;

    janet_fixarity(argc, 1);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->GetFocusedElement(self, &element);

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        maybe_make_object(hrRet, element, "IUIAutomationElement"));
}

static Janet IUIAutomation_CreateCacheRequest(int32_t argc, Janet *argv)
{
    IUIAutomation *self;

    HRESULT hrRet;
    IUIAutomationCacheRequest *cacheRequest;

    janet_fixarity(argc, 1);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->CreateCacheRequest(self, &cacheRequest);

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        maybe_make_object(hrRet, cacheRequest, "IUIAutomationCacheRequest"));
}

static Janet IUIAutomation_CreateTrueCondition(int32_t argc, Janet *argv)
{
    IUIAutomation *self;

    HRESULT hrRet;
    IUIAutomationCondition *newCondition;

    janet_fixarity(argc, 1);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->CreateTrueCondition(self, &newCondition);

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        maybe_make_object(hrRet, newCondition, "IUIAutomationCondition"));
}

static Janet IUIAutomation_CreateFalseCondition(int32_t argc, Janet *argv)
{
    IUIAutomation *self;

    HRESULT hrRet;
    IUIAutomationCondition *newCondition;

    janet_fixarity(argc, 1);

    self = (IUIAutomation *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->CreateFalseCondition(self, &newCondition);

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        maybe_make_object(hrRet, newCondition, "IUIAutomationCondition"));
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

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        maybe_make_object(hrRet, newCondition, "IUIAutomationCondition"));
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
        return jw32_wrap_hresult(E_OUTOFMEMORY);
    }

    hrRet = self->lpVtbl->AddAutomationEventHandler(self,
                                                    eventId,
                                                    element,
                                                    scope,
                                                    cacheRequest,
                                                    (IUIAutomationEventHandler *)handler);
    return jw32_wrap_hresult(hrRet);
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
        return jw32_wrap_hresult(E_OUTOFMEMORY);
    }

    hrRet = self->lpVtbl->AddFocusChangedEventHandler(self,
                                                      cacheRequest,
                                                      (IUIAutomationFocusChangedEventHandler *)handler);
    return jw32_wrap_hresult(hrRet);
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
        return jw32_wrap_hresult(E_OUTOFMEMORY);
    }
    for (LONG i = 0; i < properties.len; i++) {
        Janet item = properties.items[i];
        if (!janet_checkint(item)) {
            SafeArrayDestroy(propertyArray);
            janet_panicf("bad property #%d: expected an integer, got %v", i, item);
        }
        enum PROPERTYID pid = jw32_unwrap_int(item);
        HRESULT hr = SafeArrayPutElement(propertyArray, &i, &pid);
        jw32_dbg_val(hr, "%d");
    }

    handler = create_uia_event_handler(&IID_IUIAutomationPropertyChangedEventHandler,
                                       &UIAPropertyChangedEventHandler_Vtbl,
                                       callback,
                                       uia_thread_state.env);
    if (!handler) {
        SafeArrayDestroy(propertyArray);
        return jw32_wrap_hresult(E_OUTOFMEMORY);
    }

    hrRet = self->lpVtbl->AddPropertyChangedEventHandler(self,
                                                         element,
                                                         scope,
                                                         cacheRequest,
                                                         (IUIAutomationPropertyChangedEventHandler *)handler,
                                                         propertyArray);
    SafeArrayDestroy(propertyArray);
    return jw32_wrap_hresult(hrRet);
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
        return jw32_wrap_hresult(E_OUTOFMEMORY);
    }

    hrRet = self->lpVtbl->AddStructureChangedEventHandler(self,
                                                          element,
                                                          scope,
                                                          cacheRequest,
                                                          (IUIAutomationStructureChangedEventHandler *)handler);
    return jw32_wrap_hresult(hrRet);
}

DEFINE_OBJ_PROPERTY_GETTER(IUIAutomation, ContentViewCondition, IUIAutomationCondition)
DEFINE_OBJ_PROPERTY_GETTER(IUIAutomation, ControlViewCondition, IUIAutomationCondition)
DEFINE_OBJ_PROPERTY_GETTER(IUIAutomation, RawViewCondition, IUIAutomationCondition)

static const JanetMethod IUIAutomation_methods[] = {
    {"GetRootElement", IUIAutomation_GetRootElement},
    {"GetRootElementBuildCache", IUIAutomation_GetRootElementBuildCache},
    {"GetFocusedElement", IUIAutomation_GetFocusedElement},
    {"CreateCacheRequest", IUIAutomation_CreateCacheRequest},
    {"CreateTrueCondition", IUIAutomation_CreateTrueCondition},
    {"CreateFalseCondition", IUIAutomation_CreateFalseCondition},
    {"CreateAndCondition", IUIAutomation_CreateAndCondition},
    {"AddAutomationEventHandler", IUIAutomation_AddAutomationEventHandler},
    {"AddFocusChangedEventHandler", IUIAutomation_AddFocusChangedEventHandler},
    {"AddPropertyChangedEventHandler", IUIAutomation_AddPropertyChangedEventHandler},
    {"AddStructureChangedEventHandler", IUIAutomation_AddStructureChangedEventHandler},

    PROPERTY_GETTER_METHOD(IUIAutomation, ContentViewCondition),
    PROPERTY_GETTER_METHOD(IUIAutomation, ControlViewCondition),
    PROPERTY_GETTER_METHOD(IUIAutomation, RawViewCondition),

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

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        maybe_make_object(hrRet, updatedElement, "IUIAutomationElement"));
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

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        maybe_make_object(hrRet, found, "IUIAutomationElementArray"));
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

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        maybe_make_object(hrRet, found, "IUIAutomationElementArray"));
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

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        maybe_make_object(hrRet, found, "IUIAutomationElement"));
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

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        maybe_make_object(hrRet, found, "IUIAutomationElement"));
}

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentControlType, CONTROLTYPEID, int)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedControlType, CONTROLTYPEID, int)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentIsControlElement, BOOL, bool)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedIsControlElement, BOOL, bool)

DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CurrentIsContentElement, BOOL, bool)
DEFINE_SIMPLE_PROPERTY_GETTER(IUIAutomationElement, CachedIsContentElement, BOOL, bool)

static Janet PROPERTY_GETTER(IUIAutomationElement, CurrentName)(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;

    HRESULT hrRet;
    BSTR retVal = NULL;
    Janet ret_tuple[2];

    janet_fixarity(argc, 1);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->get_CurrentName(self, &retVal);

    ret_tuple[0] = jw32_wrap_hresult(hrRet);
    if (SUCCEEDED(hrRet)) {
        ret_tuple[1] = janet_wrap_string(jw32_com_bstr_to_string(retVal));
        SysFreeString(retVal);
    } else {
        ret_tuple[1] = janet_wrap_nil();
    }

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static Janet PROPERTY_GETTER(IUIAutomationElement, CachedName)(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;

    HRESULT hrRet;
    BSTR retVal = NULL;
    Janet ret_tuple[2];

    janet_fixarity(argc, 1);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->get_CachedName(self, &retVal);

    ret_tuple[0] = jw32_wrap_hresult(hrRet);
    if (SUCCEEDED(hrRet)) {
        ret_tuple[1] = janet_wrap_string(jw32_com_bstr_to_string(retVal));
        SysFreeString(retVal);
    } else {
        ret_tuple[1] = janet_wrap_nil();
    }

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static Janet PROPERTY_GETTER(IUIAutomationElement, CurrentAcceleratorKey)(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;

    HRESULT hrRet;
    BSTR retVal = NULL;
    Janet ret_tuple[2];

    janet_fixarity(argc, 1);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->get_CurrentAcceleratorKey(self, &retVal);

    ret_tuple[0] = jw32_wrap_hresult(hrRet);
    if (SUCCEEDED(hrRet)) {
        ret_tuple[1] = janet_wrap_string(jw32_com_bstr_to_string(retVal));
        SysFreeString(retVal);
    } else {
        ret_tuple[1] = janet_wrap_nil();
    }

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static Janet PROPERTY_GETTER(IUIAutomationElement, CachedAcceleratorKey)(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;

    HRESULT hrRet;
    BSTR retVal = NULL;
    Janet ret_tuple[2];

    janet_fixarity(argc, 1);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->get_CachedAcceleratorKey(self, &retVal);

    ret_tuple[0] = jw32_wrap_hresult(hrRet);
    if (SUCCEEDED(hrRet)) {
        ret_tuple[1] = janet_wrap_string(jw32_com_bstr_to_string(retVal));
        SysFreeString(retVal);
    } else {
        ret_tuple[1] = janet_wrap_nil();
    }

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static Janet PROPERTY_GETTER(IUIAutomationElement, CurrentAccessKey)(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;

    HRESULT hrRet;
    BSTR retVal = NULL;
    Janet ret_tuple[2];

    janet_fixarity(argc, 1);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->get_CurrentAccessKey(self, &retVal);

    ret_tuple[0] = jw32_wrap_hresult(hrRet);
    if (SUCCEEDED(hrRet)) {
        ret_tuple[1] = janet_wrap_string(jw32_com_bstr_to_string(retVal));
        SysFreeString(retVal);
    } else {
        ret_tuple[1] = janet_wrap_nil();
    }

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static Janet PROPERTY_GETTER(IUIAutomationElement, CachedAccessKey)(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;

    HRESULT hrRet;
    BSTR retVal = NULL;
    Janet ret_tuple[2];

    janet_fixarity(argc, 1);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->get_CachedAccessKey(self, &retVal);

    ret_tuple[0] = jw32_wrap_hresult(hrRet);
    if (SUCCEEDED(hrRet)) {
        ret_tuple[1] = janet_wrap_string(jw32_com_bstr_to_string(retVal));
        SysFreeString(retVal);
    } else {
        ret_tuple[1] = janet_wrap_nil();
    }

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static const JanetMethod IUIAutomationElement_methods[] = {
    {"BuildUpdatedCache", IUIAutomationElement_BuildUpdatedCache},
    {"FindAll", IUIAutomationElement_FindAll},
    {"FindAllBuildCache", IUIAutomationElement_FindAllBuildCache},
    {"FindFirstBuildCache", IUIAutomationElement_FindFirstBuildCache},

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentIsContentElement),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedIsContentElement),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentIsControlElement),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedIsControlElement),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentControlType),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedControlType),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentName),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedName),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentAcceleratorKey),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedAcceleratorKey),

    PROPERTY_GETTER_METHOD(IUIAutomationElement, CurrentAccessKey),
    PROPERTY_GETTER_METHOD(IUIAutomationElement, CachedAccessKey),

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

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        maybe_make_object(hrRet, element, "IUIAutomationElement"));
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

    return jw32_wrap_hresult(hrRet);
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

    return jw32_wrap_hresult(hrRet);
}

static Janet IUIAutomationCacheRequest_Clone(int32_t argc, Janet *argv)
{
    IUIAutomationCacheRequest *self;
    
    HRESULT hrRet;
    IUIAutomationCacheRequest *clonedRequest = NULL;

    janet_fixarity(argc, 1);

    self = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->Clone(self, &clonedRequest);

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        maybe_make_object(hrRet, clonedRequest, "IUIAutomationCacheRequest"));
}

DEFINE_SIMPLE_PROPERTY(IUIAutomationCacheRequest, AutomationElementMode,
                                enum AutomationElementMode, int)
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
