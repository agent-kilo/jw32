#include "jw32.h"

#define JW32_COM_PROTO_EXPORT
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

static void define_consts_clsctx(JanetTable *env)
{
#define __def(const_name)                                        \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),     \
              "Constant for COM object execution contexts.")
    __def(CLSCTX_INPROC_SERVER);
    __def(CLSCTX_INPROC_HANDLER);
    __def(CLSCTX_LOCAL_SERVER);
    __def(CLSCTX_INPROC_SERVER16);
    __def(CLSCTX_REMOTE_SERVER);
    __def(CLSCTX_INPROC_HANDLER16);
    __def(CLSCTX_RESERVED1);
    __def(CLSCTX_RESERVED2);
    __def(CLSCTX_RESERVED3);
    __def(CLSCTX_RESERVED4);
    __def(CLSCTX_NO_CODE_DOWNLOAD);
    __def(CLSCTX_RESERVED5);
    __def(CLSCTX_NO_CUSTOM_MARSHAL);
    __def(CLSCTX_ENABLE_CODE_DOWNLOAD);
    __def(CLSCTX_NO_FAILURE_LOG);
    __def(CLSCTX_DISABLE_AAA);
    __def(CLSCTX_ENABLE_AAA);
    __def(CLSCTX_FROM_DEFAULT_CONTEXT);
    __def(CLSCTX_ACTIVATE_X86_SERVER);
    __def(CLSCTX_ACTIVATE_32_BIT_SERVER);
    __def(CLSCTX_ACTIVATE_64_BIT_SERVER);
    __def(CLSCTX_ENABLE_CLOAKING);
    __def(CLSCTX_APPCONTAINER);
    __def(CLSCTX_ACTIVATE_AAA_AS_IU);
    __def(CLSCTX_RESERVED6);
    __def(CLSCTX_ACTIVATE_ARM32_SERVER);
    __def(CLSCTX_PS_DLL);
#undef __def
}


static Janet IUnknown_AddRef(int32_t argc, Janet *argv)
{
    IUnknown *self;

    ULONG uRet;

    janet_fixarity(argc, 1);

    self = (IUnknown *)jw32_com_get_obj_ref(argv, 0);
    uRet = self->lpVtbl->AddRef(self);
    return jw32_wrap_ulong(uRet);
}

static Janet IUnknown_Release(int32_t argc, Janet *argv)
{
    IUnknown *self;

    ULONG uRet;

    janet_fixarity(argc, 1);

    self = (IUnknown *)jw32_com_get_obj_ref(argv, 0);
    uRet = self->lpVtbl->Release(self);
    return jw32_wrap_ulong(uRet);
}

static Janet IUnknown_QueryInterface(int32_t argc, Janet *argv)
{
    HRESULT hrRet;
    void *pvObject = NULL;

    GUID iid;
    GUID *piid;

    janet_fixarity(argc, 2);

    IUnknown *self = (IUnknown *)jw32_com_get_obj_ref(argv, 0);

    JanetTable *if_proto = NULL;
    if (janet_checktype(argv[1], JANET_TABLE)) {
        if_proto = janet_unwrap_table(argv[1]);
        piid = (GUID *)jw32_com_normalize_iid(if_proto, &iid);
    } else {
        piid = (GUID *)jw32_get_refiid(argv, 1, &iid);
    }
    REFIID riid = piid;

    hrRet = self->lpVtbl->QueryInterface(self, riid, &pvObject);

    JW32_HR_RETURN_OR_PANIC(hrRet, jw32_com_make_object(pvObject, if_proto));
}

static const JanetMethod IUnknown_methods[] = {
    {"AddRef", IUnknown_AddRef},
    {"Release", IUnknown_Release},
    {"QueryInterface", IUnknown_QueryInterface},
    {NULL, NULL},
};

static void init_table_protos(JanetTable *env)
{
    JanetTable *IUnknown_proto = jw32_com_make_if_proto("IUnknown", IUnknown_methods, NULL, &IID_IUnknown);
    janet_def(env, "IUnknown", janet_wrap_table(IUnknown_proto),
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

    JW32_HR_RETURN_OR_PANIC(hrRet, janet_wrap_nil());
}

static Janet cfun_CoUninitialize(int32_t argc, Janet *argv)
{
    janet_fixarity(argc, 0);

    (void)argv;

    CoUninitialize();
    return janet_wrap_nil();
}

static Janet cfun_CoCreateInstance(int32_t argc, Janet *argv)
{
    LPVOID pv = NULL;
    HRESULT hrRet;

    GUID clsid_from_str;
    GUID iid_from_str;
    GUID *piid = NULL;

    janet_fixarity(argc, 4);

    REFCLSID rclsid = jw32_get_refclsid(argv, 0, &clsid_from_str);
    LPUNKNOWN pUnkOuter = jw32_get_lpunknown(argv, 1);
    DWORD dwClsContext = jw32_get_dword(argv, 2);

    JanetTable *if_proto = NULL;
    if (janet_checktype(argv[3], JANET_TABLE)) {
        if_proto = janet_unwrap_table(argv[3]);
        piid = (GUID *)jw32_com_normalize_iid(if_proto, &iid_from_str);
    } else {
        piid = (GUID *)jw32_get_refiid(argv, 3, &iid_from_str);
    }
    REFIID riid = piid;

    hrRet = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, &pv);

    /* if_proto may be NULL when a prototype is not provided */
    JW32_HR_RETURN_OR_PANIC(hrRet, jw32_com_make_object(pv, if_proto));
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
        "Creates a COM object instance."
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_consts_coinit(env);
    define_consts_clsctx(env);

    init_table_protos(env);

    janet_cfuns(env, MOD_NAME, cfuns);
}
