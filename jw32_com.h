#ifndef __JW32_COM_H
#define __JW32_COM_H

#include "jw32.h"
#include <objbase.h>

#define JW32_COM_OBJ_REF_NAME "__obj_ref"

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


static inline void *jw32_com_get_obj_ref(Janet *argv, int32_t n)
{
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

#endif /* __JW32_COM_H */
