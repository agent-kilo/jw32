#ifndef __JW32_COM_H
#define __JW32_COM_H

#include "jw32.h"
#include <objbase.h>


#define JW32_COM_OBJ_REF_NAME "__obj_ref"
#define JW32_COM_IID_NAME "__iid"
#define JW32_COM_IF_NAME_NAME "__if_name"

/* XXX: this only works when the modules are properly installed */
#define IUNKNOWN_MOD_NAME "jw32/combaseapi"
#define IUNKNOWN_PROTO_NAME "combaseapi/IUnknown"


/* REFCLSID: pointer to class UUID struct */
#define jw32_wrap_refclsid(x) jw32_wrap_lpvoid((void *)x)
#define jw32_unwrap_refclsid(x) ((REFCLSID)jw32_unwrap_lpvoid(x))
#define jw32_get_refclsid(argv, n) ((REFCLSID)jw32_get_lpvoid(argv, n))


/* REFIID: pointer to interface UUID struct */
#define jw32_wrap_refiid(x) jw32_wrap_lpvoid((void *)x)
#define jw32_unwrap_refiid(x) ((REFIID)jw32_unwrap_lpvoid(x))
#define jw32_get_refiid(argv, n) ((REFIID)jw32_get_lpvoid(argv, n))


/* LPUNKNOWN: pointer to IUnknown interface */
#define jw32_wrap_lpunknown(x) jw32_wrap_lpvoid(x)
#define jw32_unwrap_lpunknown(x) ((LPUNKNOWN)jw32_unwrap_lpvoid(x))
#define jw32_get_lpunknown(argv, n) ((LPUNKNOWN)jw32_get_lpvoid(argv, n))


/* TODO: check interface name? */
static inline void *jw32_com_get_obj_ref(Janet *argv, int32_t n)
{
    if (janet_checktype(argv[n], JANET_NIL)) {
        return NULL;
    }

    JanetTable *tbl = janet_gettable(argv, n);
    /* XXX: this is slow? */
    Janet maybe_ref = janet_table_get(tbl, jw32_cstr_to_keyword(JW32_COM_OBJ_REF_NAME));
    void *ref;

    if (!janet_checktype(maybe_ref, JANET_POINTER)) {
        janet_panicf("invalid object reference: %v", maybe_ref);
    }

    ref = janet_unwrap_pointer(maybe_ref);
    if (!ref) {
        janet_panicf("invalid object reference: %v", maybe_ref);
    }

    return janet_unwrap_pointer(maybe_ref);
}

static inline REFIID jw32_com_normalize_iid(JanetTable *proto)
{
    Janet maybe_iid = janet_table_get(proto, janet_ckeywordv(JW32_COM_IID_NAME));
    if (!janet_checktype(maybe_iid, JANET_POINTER)) {
        janet_panicf("invalid interface ID: %v", maybe_iid);
    }

    return (REFIID)janet_unwrap_pointer(maybe_iid);
}

static inline JanetTable *jw32_com_resolve_iunknown_proto(void)
{
    Janet argv[] = {
        janet_wrap_string(janet_cstring(IUNKNOWN_MOD_NAME)),
    };
    /* (import* ...) may fail by raising a signal. */
    Janet cur_env = jw32_call_core_fn("import*", 1, argv);
    JanetTable *cur_env_tb = janet_unwrap_table(cur_env);

    Janet iunknown_proto;
    JanetBindingType b_type = janet_resolve(cur_env_tb,
                                            janet_csymbol(IUNKNOWN_PROTO_NAME),
                                            &iunknown_proto);
    if (JANET_BINDING_NONE == b_type) {
        janet_panicf("could not import variable %s", IUNKNOWN_PROTO_NAME);
    }
    if (!janet_checktype(iunknown_proto, JANET_TABLE)) {
        janet_panicf("expected %s to be a table, got %v",
                     IUNKNOWN_PROTO_NAME, iunknown_proto);
    }

    return janet_unwrap_table(iunknown_proto);
}

static inline JanetTable *jw32_com_make_if_proto(const char* name, const JanetMethod *methods, JanetTable *parent, REFIID riid)
{
    JanetTable *proto = janet_table(0);

    for (int i = 0; NULL != methods[i].name; i++) {
        janet_table_put(proto,
                        janet_ckeywordv(methods[i].name),
                        janet_wrap_cfunction((void *)methods[i].cfun));
    }

    if (riid) {
        janet_table_put(proto, janet_ckeywordv(JW32_COM_IID_NAME), jw32_wrap_refiid(riid));
    }
    janet_table_put(proto,
                    janet_ckeywordv(JW32_COM_IF_NAME_NAME),
                    janet_wrap_string(janet_cstring(name)));

    proto->proto = parent;

    return proto;
}

static inline Janet jw32_com_make_object(LPVOID pv, JanetTable *if_proto)
{
    JanetTable *if_obj = janet_table(0);
    janet_table_put(if_obj,
                    janet_ckeywordv(JW32_COM_OBJ_REF_NAME),
                    janet_wrap_pointer(pv));
    if_obj->proto = if_proto;
    return janet_wrap_table(if_obj);
}

static inline Janet jw32_com_make_object_in_env(LPVOID pv, const char *proto_name, JanetTable *env)
{
    Janet proto;
    JanetBindingType b_type = janet_resolve(env, janet_csymbol(proto_name), &proto);

    if (JANET_BINDING_NONE == b_type) {
        janet_panicf("could not resolve variable %s", proto_name);
    }
    if (!janet_checktype(proto, JANET_TABLE)) {
        janet_panicf("expected %s to be a table, got %v", proto_name, proto);
    }

    return jw32_com_make_object(pv, janet_unwrap_table(proto));
}

static inline JanetString jw32_com_bstr_to_string(BSTR from)
{
    int count = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, from, -1, NULL, 0, NULL, NULL);

    if (count <= 0) {
        /* XXX: free the BSTR when panicked? */
        janet_panicf("WideCharToMultiByte failed: 0x%lx", GetLastError());
    } else {
        /* janet_string_begin() adds one more byte for the trailing zero,
           and count includes the trailing zero, so */
        uint8_t *to = janet_string_begin(count - 1);
        int count_again = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                                              from, -1, (LPSTR)to, count,
                                              NULL, NULL);
        if (count_again != count) {
            janet_panicf("calculated buffer len is %d, but %d bytes are copied", count, count_again);
        }
        return janet_string_end(to);
    }
}

#define JW32_COM_PROPERTY_GETTER(__if, __prop) __##__if##_get_##__prop##__

#define JW32_COM_DEFINE_OBJ_PROPERTY_GETTER(__if, __prop, __prop_if)    \
    static Janet JW32_COM_PROPERTY_GETTER(__if, __prop)(int32_t argc, Janet *argv) \
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

#define JW32_COM_DEFINE_SIMPLE_PROPERTY_GETTER(__if, __prop, __prop_type, __prop_jw32_type) \
    static Janet JW32_COM_PROPERTY_GETTER(__if, __prop)(int32_t argc, Janet *argv) \
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

#define JW32_COM_PROPERTY_SETTER(__if, __prop) __##__if##_put_##__prop##__

#define JW32_COM_DEFINE_OBJ_PROPERTY_SETTER(__if, __prop, __prop_if)    \
    static Janet JW32_COM_PROPERTY_SETTER(__if, __prop)(int32_t argc, Janet *argv) \
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

#define JW32_COM_DEFINE_SIMPLE_PROPERTY_SETTER(__if, __prop, __prop_type, __prop_jw32_type) \
    static Janet JW32_COM_PROPERTY_SETTER(__if, __prop)(int32_t argc, Janet *argv) \
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

#define JW32_COM_DEFINE_OBJ_PROPERTY(__if, __prop, __prop_if)    \
    JW32_COM_DEFINE_OBJ_PROPERTY_GETTER(__if, __prop, __prop_if) \
    JW32_COM_DEFINE_OBJ_PROPERTY_SETTER(__if, __prop, __prop_if)

#define JW32_COM_DEFINE_SIMPLE_PROPERTY(__if, __prop, __prop_type, __prop_jw32_type) \
    JW32_COM_DEFINE_SIMPLE_PROPERTY_GETTER(__if, __prop, __prop_type, __prop_jw32_type) \
    JW32_COM_DEFINE_SIMPLE_PROPERTY_SETTER(__if, __prop, __prop_type, __prop_jw32_type)

#define JW32_COM_PROPERTY_GETTER_METHOD(__if, __prop)           \
    {"get_" #__prop, JW32_COM_PROPERTY_GETTER(__if, __prop)}

#define JW32_COM_PROPERTY_SETTER_METHOD(__if, __prop)           \
    {"put_" #__prop, JW32_COM_PROPERTY_SETTER(__if, __prop)}

#define JW32_COM_PROPERTY_METHODS(__if, __prop)    \
    JW32_COM_PROPERTY_GETTER_METHOD(__if, __prop), \
    JW32_COM_PROPERTY_SETTER_METHOD(__if, __prop)

#endif /* __JW32_COM_H */
