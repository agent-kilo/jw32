#include "jw32.h"
#include "jw32_com.h"

#define MOD_NAME "combaseapi"


static void define_consts_coinit(JanetTable *env)
{
#define __def(const_name)                                        \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),     \
              "Constant for COM concurrency models.")
    __def(COINIT_APARTMENTTHREADED);
    __def(COINIT_MULTITHREADED);
    __def(COINIT_DISABLE_OLE1DDE);
    __def(COINIT_SPEED_OVER_MEMORY);
#undef __def
}


static Janet iunknown_AddRef(int32_t argc, Janet *argv)
{
    IUnknown *self;

    ULONG uRet;

    janet_fixarity(argc, 1);

    self = (IUnknown *)jw32_com_get_obj_ref(argv, 0);
    uRet = self->lpVtbl->AddRef(self);
    return jw32_wrap_ulong(uRet);
}

static Janet iunknown_Release(int32_t argc, Janet *argv)
{
    IUnknown *self;

    ULONG uRet;

    janet_fixarity(argc, 1);

    self = (IUnknown *)jw32_com_get_obj_ref(argv, 0);
    uRet = self->lpVtbl->Release(self);
    return jw32_wrap_ulong(uRet);
}

static Janet iunknown_QueryInterface(int32_t argc, Janet *argv)
{
    HRESULT hrRet;
    void *pvObject = NULL;
    Janet ret_tuple[2];

    janet_fixarity(argc, 1);

    IUnknown *self = (IUnknown *)jw32_com_get_obj_ref(argv, 0);
    REFIID riid = jw32_get_refiid(argv, 1);

    hrRet = self->lpVtbl->QueryInterface(self, riid, &pvObject);
    ret_tuple[0] = jw32_wrap_hresult(hrRet);
    ret_tuple[1] = jw32_wrap_lpvoid(pvObject);
    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}

static const JanetMethod iunknown_methods[] = {
    {"AddRef", iunknown_AddRef},
    {"Release", iunknown_Release},
    {"QueryInterface", iunknown_QueryInterface},
    {NULL, NULL},
};

static void init_table_protos(JanetTable *env)
{
    JanetTable *iunknown_proto = jw32_com_make_proto(iunknown_methods);
    janet_def(env, "IUnknown", janet_wrap_table(iunknown_proto),
              "Prototype for COM IUnknown interface.");
}


static Janet cfun_CoInitializeEx(int32_t argc, Janet *argv)
{
    LPVOID pvReserved;
    DWORD dwCoInit;

    HRESULT hrRet;

    janet_fixarity(argc, 2);

    pvReserved = jw32_get_lpvoid(argv, 0);
    dwCoInit = jw32_get_dword(argv, 1);

    hrRet = CoInitializeEx(pvReserved, dwCoInit);
    return jw32_wrap_hresult(hrRet);
}

static Janet cfun_CoUninitialize(int32_t argc, Janet *argv)
{
    janet_fixarity(argc, 0);

    (void)argc;
    (void)argv;

    CoUninitialize();
    return janet_wrap_nil();
}

static Janet cfun_CoCreateInstance(int32_t argc, Janet *argv)
{
    LPVOID pv = NULL;
    HRESULT hrRet;
    Janet ret_tuple[2];

    janet_fixarity(argc, 4);

    /* REFCLSID & REFIID have the const prefix,
       have to be declared & initialized at the same time. */
    REFCLSID rclsid = jw32_get_refclsid(argv, 0);
    LPUNKNOWN pUnkOuter = jw32_get_lpunknown(argv, 1);
    DWORD dwClsContext = jw32_get_dword(argv, 2);
    REFIID riid = jw32_get_refiid(argv, 3);

    hrRet = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, &pv);
    ret_tuple[0] = jw32_wrap_hresult(hrRet);
    ret_tuple[1] = jw32_wrap_lpvoid(pv);
    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}


static const JanetReg cfuns[] = {
    {
        "CoInitializeEx",
        cfun_CoInitializeEx,
        "(" MOD_NAME "/CoInitializeEx pvReserved dwCoInit)\n\n"
        "Initializes the COM library."
    },
    {
        "CoUninitialize",
        cfun_CoUninitialize,
        "(" MOD_NAME "/CoUninitialize)\n\n"
        "Uninitializes the COM library."
    },
    {
        "CoCreateInstance",
        cfun_CoCreateInstance,
        "(" MOD_NAME "/CoCreateInstance rclsid pUnkOuter dwClsContext riid)\n\n"
        "Creates a COM object instance. Returns a tuple [HRESULT LPVOID]."
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_consts_coinit(env);

    init_table_protos(env);

    janet_cfuns(env, MOD_NAME, cfuns);
}
