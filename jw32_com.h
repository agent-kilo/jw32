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

typedef struct {
    JanetTable *proto_map;
    JanetTable *deferred_init;
    JanetTable *hook_map;
} jw32_com_proto_registry_t;

#ifndef JW32_COM_PROTO_EXPORT
#ifdef JW32_DLL
__declspec(dllimport) jw32_com_proto_registry_t jw32_com_proto_registry;
#else
extern jw32_com_proto_registry_t jw32_com_proto_registry;
#endif /* JW32_DLL */
#else /* JW32_COM_IUNKNOWN_EXPORT */
__declspec(dllexport) jw32_com_proto_registry_t jw32_com_proto_registry = {
    .proto_map = NULL,
    .deferred_init = NULL,
    .hook_map = NULL,
};
#endif /* !JW32_COM_IUNKNOWN_EXPORT */


static inline GUID *jw32_unwrap_guid(Janet x, GUID *pguid)
{
    switch (janet_type(x)) {
    case JANET_STRING: {
        JanetString guid_str = janet_unwrap_string(x);
        if (jw32_string_to_guid(guid_str, pguid)) {
            return pguid;
        } else {
            janet_panicf("invalid guid string: %v", x);
        }
    }
    case JANET_POINTER:
        return jw32_unwrap_lpvoid(x);
    default:
        janet_panicf("invalid guid value, expected %T or %T, got %v",
                     JANET_TFLAG_STRING, JANET_TFLAG_POINTER, x);
    }
}


static inline GUID *jw32_get_guid(const Janet *argv, int32_t n, GUID *pguid)
{
    Janet x = argv[n];
    return jw32_unwrap_guid(x, pguid);
}


/* REFCLSID: pointer to class UUID struct */
#define jw32_wrap_refclsid(x) jw32_wrap_lpvoid((void *)x)
#define jw32_unwrap_refclsid(x) ((REFCLSID)jw32_unwrap_lpvoid(x))
#define jw32_get_refclsid(argv, n, pguid) ((REFCLSID)jw32_get_guid(argv, n, pguid))


/* REFIID: pointer to interface UUID struct */
#define jw32_wrap_refiid(x) jw32_wrap_lpvoid((void *)x)
#define jw32_unwrap_refiid(x) ((REFIID)jw32_unwrap_lpvoid(x))
#define jw32_get_refiid(argv, n, pguid) ((REFIID)jw32_get_guid(argv, n, pguid))


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

static inline REFIID jw32_com_normalize_iid(JanetTable *proto, GUID *pguid)
{
    Janet maybe_iid = janet_table_get(proto, janet_ckeywordv(JW32_COM_IID_NAME));
    return (REFIID)jw32_unwrap_guid(maybe_iid, pguid);
}

static inline JanetTable *jw32_com_find_if_proto(const char* name)
{
    if (!jw32_com_proto_registry.proto_map) {
        return NULL;
    }

    Janet pv = janet_table_get(jw32_com_proto_registry.proto_map, janet_cstringv(name));
    if (janet_checktype(pv, JANET_NIL)) {
        return NULL;
    }

    return janet_unwrap_table(pv);
}

/* A hook function that's called when a prototype is ready for use. */
typedef void (*jw32_com_proto_hook_t)(JanetTable *proto);

static inline void jw32_com_reg_proto_hook(const char* proto_name, jw32_com_proto_hook_t hook)
{
    if (!jw32_com_proto_registry.hook_map) {
        jw32_com_proto_registry.hook_map = janet_table(0);
        janet_gcroot(janet_wrap_table(jw32_com_proto_registry.hook_map));
    }

    JanetTable *proto = jw32_com_find_if_proto(proto_name);
    if (proto) {
        hook(proto);
        return;
    }

    /* The prototype is not registered yet, register a hook function for future execution */
    Janet arrv = janet_table_get(jw32_com_proto_registry.hook_map, janet_cstringv(proto_name));
    JanetArray *arr;
    if (janet_checktype(arrv, JANET_NIL)) {
        arr = janet_array(1);
    } else {
        arr = janet_unwrap_array(arrv);
    }
    janet_array_push(arr, janet_wrap_pointer((void *)hook));
    janet_table_put(jw32_com_proto_registry.hook_map,
                    janet_cstringv(proto_name),
                    janet_wrap_array(arr));
}

static inline JanetTable *jw32_com_make_if_proto(const char* name, const JanetMethod *methods, const char *parent, REFIID riid)
{
    if (!jw32_com_proto_registry.proto_map) {
        jw32_com_proto_registry.proto_map = janet_table(0);
        janet_gcroot(janet_wrap_table(jw32_com_proto_registry.proto_map));
    }
    if (!jw32_com_proto_registry.deferred_init) {
        jw32_com_proto_registry.deferred_init = janet_table(0);
        janet_gcroot(janet_wrap_table(jw32_com_proto_registry.deferred_init));
    }

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

    if (parent) {
        JanetTable *pt = jw32_com_find_if_proto(parent);
        if (pt) {
            proto->proto = pt;
        } else {
            /* The required prototype is not registered yet, defer the initialization */
            Janet arrv = janet_table_get(jw32_com_proto_registry.deferred_init, janet_cstringv(parent));
            JanetArray *arr;
            if (janet_checktype(arrv, JANET_NIL)) {
                arr = janet_array(1);
            } else {
                arr = janet_unwrap_array(arrv);
            }
            janet_array_push(arr, janet_cstringv(name));
            janet_table_put(jw32_com_proto_registry.deferred_init,
                            janet_cstringv(parent),
                            janet_wrap_array(arr));
        }
    }

    janet_table_put(jw32_com_proto_registry.proto_map, janet_cstringv(name), janet_wrap_table(proto));

    Janet deferred_arrv = janet_table_remove(jw32_com_proto_registry.deferred_init, janet_cstringv(name));
    if (!janet_checktype(deferred_arrv, JANET_NIL)) {
        JanetArray *deferred_arr = janet_unwrap_array(deferred_arrv);
        for (int32_t i = 0; i < deferred_arr->count; i++) {
            const char *child_name = (const char *)janet_unwrap_string(deferred_arr->data[i]);
            JanetTable *child_proto = jw32_com_find_if_proto(child_name);
            if (!child_proto) {
                janet_panicf("deferred initialization for unknown prototype: %s", child_name);
            }
            child_proto->proto = proto;
        }
    }

    if (jw32_com_proto_registry.hook_map) {
        Janet hook_arrv = janet_table_remove(jw32_com_proto_registry.hook_map, janet_cstringv(name));
        if (!janet_checktype(hook_arrv, JANET_NIL)) {
            JanetArray *hook_arr = janet_unwrap_array(hook_arrv);
            for (int32_t i = 0; i < hook_arr->count; i++) {
                jw32_com_proto_hook_t hook = (jw32_com_proto_hook_t)janet_unwrap_pointer(hook_arr->data[i]);
                hook(proto);
            }
        }
    }

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
