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

#ifndef JW32_COM_IUNKNOWN_EXPORT
#ifdef JW32_DLL
__declspec(dllimport) JanetTable *IUnknown_proto;
#else
extern JanetTable *IUnknown_proto; /* For static linking */
#endif /* JW32_DLL */
#endif /* JW32_COM_IUNKNOWN_EXPORT */

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
    Janet maybe_ref = janet_table_get(tbl, janet_ckeywordv(JW32_COM_OBJ_REF_NAME));
    void *ref;

    if (!janet_checktype(maybe_ref, JANET_POINTER)) {
        janet_panicf("invalid object reference: %v", maybe_ref);
    }

    ref = janet_unwrap_pointer(maybe_ref);
    if (!ref) {
        janet_panicf("invalid object reference: %v", maybe_ref);
    }

    return ref;
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
    if (pv) {
        JanetTable *if_obj = janet_table(0);
        janet_table_put(if_obj,
                        janet_ckeywordv(JW32_COM_OBJ_REF_NAME),
                        janet_wrap_pointer(pv));
        if_obj->proto = if_proto;
        return janet_wrap_table(if_obj);
    } else {
        return janet_wrap_nil();
    }
}

static inline Janet jw32_com_make_object_in_env(LPVOID pv, const char *proto_name, JanetTable *env)
{
    if (pv) {
        Janet proto;
        JanetBindingType b_type = janet_resolve(env, janet_csymbol(proto_name), &proto);

        if (JANET_BINDING_NONE == b_type) {
            janet_panicf("could not resolve variable %s", proto_name);
        }
        if (!janet_checktype(proto, JANET_TABLE)) {
            janet_panicf("expected %s to be a table, got %v", proto_name, proto);
        }

        return jw32_com_make_object(pv, janet_unwrap_table(proto));
    } else {
        return janet_wrap_nil();
    }
}

#endif /* __JW32_COM_H */
