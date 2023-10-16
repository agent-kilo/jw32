#include "jw32.h"
#include "jw32_com.h"
#include <UIAutomationClient.h>
#include "debug.h"

#define MOD_NAME "uiautomation"


/* make these global so that IUIAutomation methods can find them */
JanetTable *IUIAutomation_proto;
JanetTable *IUIAutomationElement_proto;
JanetTable *IUIAutomationCacheRequest_proto;
JanetTable *IUIAutomationCondition_proto;


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
                        jw32_com_maybe_make_object(hrRet, root, IUIAutomationElement_proto));
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
                        jw32_com_maybe_make_object(hrRet, cacheRequest, IUIAutomationCacheRequest_proto));
}

static const JanetMethod IUIAutomation_methods[] = {
    {"GetRootElement", IUIAutomation_GetRootElement},
    {"CreateCacheRequest", IUIAutomation_CreateCacheRequest},
    {NULL, NULL},
};


/*******************************************************************
 *
 * IUIAutomationElement
 *
 *******************************************************************/

static Janet IUIAutomationElement_get_CurrentControlType(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;

    HRESULT hrRet;
    CONTROLTYPEID retVal;

    janet_fixarity(argc, 1);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->get_CurrentControlType(self, &retVal);

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        jw32_wrap_int(retVal));
}

static Janet IUIAutomationElement_get_CurrentName(int32_t argc, Janet *argv)
{
    IUIAutomationElement *self;

    HRESULT hrRet;
    BSTR retVal;
    Janet ret_tuple[2];

    janet_fixarity(argc, 1);

    self = (IUIAutomationElement *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->get_CurrentName(self, &retVal);

    ret_tuple[0] = jw32_wrap_hresult(hrRet);
    if (SUCCEEDED(hrRet)) {
        ret_tuple[1] = janet_wrap_string(jw32_com_bstr_to_string(retVal));
    } else {
        ret_tuple[1] = janet_wrap_nil();
    }

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static const JanetMethod IUIAutomationElement_methods[] = {
    {"get_CurrentControlType", IUIAutomationElement_get_CurrentControlType},
    {"get_CurrentName", IUIAutomationElement_get_CurrentName},
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
    IUIAutomationCacheRequest *clonedRequest;

    janet_fixarity(argc, 1);

    self = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->Clone(self, &clonedRequest);

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        jw32_com_maybe_make_object(hrRet, clonedRequest, IUIAutomationCacheRequest_proto));
}

static Janet IUIAutomationCacheRequest_get_AutomationElementMode(int32_t argc, Janet *argv)
{
    IUIAutomationCacheRequest *self;
    
    HRESULT hrRet;
    enum AutomationElementMode mode;

    janet_fixarity(argc, 1);

    self = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->get_AutomationElementMode(self, &mode);

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        jw32_wrap_int(mode));
}

static Janet IUIAutomationCacheRequest_get_TreeFilter(int32_t argc, Janet *argv)
{
    IUIAutomationCacheRequest *self;

    HRESULT hrRet;
    IUIAutomationCondition *filter;

    janet_fixarity(argc, 1);

    self = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->get_TreeFilter(self, &filter);

    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet),
                        jw32_com_maybe_make_object(hrRet, filter, IUIAutomationCondition_proto));
}

static Janet IUIAutomationCacheRequest_get_TreeScope(int32_t argc, Janet *argv)
{
    IUIAutomationCacheRequest *self;

    HRESULT hrRet;
    enum TreeScope scope;

    janet_fixarity(argc, 1);

    self = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 0);
    hrRet = self->lpVtbl->get_TreeScope(self, &scope);
    
    JW32_RETURN_TUPLE_2(jw32_wrap_hresult(hrRet), jw32_wrap_int(scope));
}

static Janet IUIAutomationCacheRequest_put_AutomationElementMode(int32_t argc, Janet *argv)
{
    IUIAutomationCacheRequest *self;
    enum AutomationElementMode mode;

    HRESULT hrRet;

    janet_fixarity(argc, 2);

    self = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 0);
    mode = jw32_get_int(argv, 1);
    hrRet = self->lpVtbl->put_AutomationElementMode(self, mode);
    return jw32_wrap_hresult(hrRet);
}

static Janet IUIAutomationCacheRequest_put_TreeFilter(int32_t argc, Janet *argv)
{
    IUIAutomationCacheRequest *self;
    IUIAutomationCondition *filter;

    HRESULT hrRet;

    janet_fixarity(argc, 2);

    self = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 0);
    filter = (IUIAutomationCondition *)jw32_com_get_obj_ref(argv, 1);
    hrRet = self->lpVtbl->put_TreeFilter(self, filter);
    return jw32_wrap_hresult(hrRet);
}

static Janet IUIAutomationCacheRequest_put_TreeScope(int32_t argc, Janet *argv)
{
    IUIAutomationCacheRequest *self;
    enum TreeScope scope;

    HRESULT hrRet;

    janet_fixarity(argc, 2);

    self = (IUIAutomationCacheRequest *)jw32_com_get_obj_ref(argv, 0);
    scope = jw32_get_int(argv, 1);
    hrRet = self->lpVtbl->put_TreeScope(self, scope);
    return jw32_wrap_hresult(hrRet);
}

static const JanetMethod IUIAutomationCacheRequest_methods[] = {
    {"AddPattern", IUIAutomationCacheRequest_AddPattern},
    {"AddProperty", IUIAutomationCacheRequest_AddProperty},
    {"Clone", IUIAutomationCacheRequest_Clone},
    {"get_AutomationElementMode", IUIAutomationCacheRequest_get_AutomationElementMode},
    {"get_TreeFilter", IUIAutomationCacheRequest_get_TreeFilter},
    {"get_TreeScope", IUIAutomationCacheRequest_get_TreeScope},
    {"put_AutomationElementMode", IUIAutomationCacheRequest_put_AutomationElementMode},
    {"put_TreeFilter", IUIAutomationCacheRequest_put_TreeFilter},
    {"put_TreeScope", IUIAutomationCacheRequest_put_TreeScope},
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
 * MODULE ENTRY & OTHER STUFF
 *
 *******************************************************************/

static void init_table_protos(JanetTable *env)
{
    JanetTable *IUnknown_proto = jw32_com_resolve_iunknown_proto();

#define __def_proto(name, parent, doc)                                  \
    do {                                                                \
        ##name##_proto = jw32_com_make_if_proto(#name, ##name##_methods, parent, &IID_##name##); \
        janet_def(env, #name, janet_wrap_table(##name##_proto), doc);   \
    } while (0)

    __def_proto(IUIAutomation,
                IUnknown_proto,
                "Prototype for COM IUIAutomation interface.");
    __def_proto(IUIAutomationElement,
                IUnknown_proto,
                "Prototype for COM IUIAutomationElement interface.");
    __def_proto(IUIAutomationCacheRequest,
                IUnknown_proto,
                "Prototype for COM IUIAutomationCacheRequest interface.");
    __def_proto(IUIAutomationCondition,
                IUnknown_proto,
                "Prototype for COM IUIAutomationCondition interface.");

#undef __def_proto

}


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_uuids(env);
    define_consts_uia_controltypeid(env);
    define_consts_uia_propertyid(env);
    define_consts_uia_patternid(env);
    define_consts_treescope(env);

    init_table_protos(env);
}
