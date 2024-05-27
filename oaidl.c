#include "jw32.h"
#include "jw32_com.h"
#include <oaidl.h>
#include "debug.h"

#define MOD_NAME "oaidl"


JanetTable *IDispatch_proto;


static void define_uuids(JanetTable *env)
{
    janet_def(env, "IID_IDispatch", jw32_wrap_refclsid(&IID_IDispatch),
              "Interface ID for IDispatch.");
}


static const JanetMethod IDispatch_methods[] = {
    {NULL, NULL},
};


static void init_table_protos(JanetTable *env)
{
#define __def_proto(name, parent, doc)                                  \
    do {                                                                \
        ##name##_proto = jw32_com_make_if_proto(#name, ##name##_methods, parent, &IID_##name##); \
        janet_def(env, #name, janet_wrap_table(##name##_proto), doc);   \
    } while (0)

    __def_proto(IDispatch, "IUnknown", "Prototype for COM IDispatch interface.");

#undef __def_proto
}


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_uuids(env);

    init_table_protos(env);
}
