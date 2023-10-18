#include "jw32.h"

#define MOD_NAME "errhandlingapi"


static void define_consts_sem(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_uint(const_name),     \
              "Constant for process error modes.")
    __def(SEM_FAILCRITICALERRORS);
    __def(SEM_NOALIGNMENTFAULTEXCEPT);
    __def(SEM_NOGPFAULTERRORBOX);
    __def(SEM_NOOPENFILEERRORBOX);
#undef __def
}

static void define_consts_noerror(JanetTable *env)
{
    janet_def(env, "NOERROR", jw32_wrap_hresult(NOERROR),
              "Constant for error codes returned by API functions.");
}

static void define_consts_s(JanetTable *env)
{
#define __def(const_name)                                               \
    janet_def(env, #const_name, jw32_wrap_hresult(const_name),          \
              "Constant for error codes returned by API functions.")
    __def(S_OK);
    __def(S_FALSE);
#undef __def
}

static void define_consts_e(JanetTable *env)
{
#define __def(const_name)                                               \
    janet_def(env, #const_name, jw32_wrap_hresult(const_name),          \
              "Constant for error codes returned by API functions.")
    __def(E_NOT_SET);
    __def(E_NOT_VALID_STATE);
    __def(E_NOT_SUFFICIENT_BUFFER);
    __def(E_TIME_SENSITIVE_THREAD);
    __def(E_NO_TASK_QUEUE);
    __def(E_UNEXPECTED);
    __def(E_NOTIMPL);
    __def(E_OUTOFMEMORY);
    __def(E_INVALIDARG);
    __def(E_NOINTERFACE);
    __def(E_POINTER);
    __def(E_HANDLE);
    __def(E_ABORT);
    __def(E_FAIL);
    __def(E_ACCESSDENIED);
    __def(E_PENDING);
    __def(E_BOUNDS);
    __def(E_CHANGED_STATE);
    __def(E_ILLEGAL_STATE_CHANGE);
    __def(E_ILLEGAL_METHOD_CALL);
    __def(E_STRING_NOT_NULL_TERMINATED);
    __def(E_ILLEGAL_DELEGATE_ASSIGNMENT);
    __def(E_ASYNC_OPERATION_NOT_STARTED);
    __def(E_APPLICATION_EXITING);
    __def(E_APPLICATION_VIEW_EXITING);
#undef __def
}

static void define_consts_co_e(JanetTable *env)
{
#define __def(const_name)                                               \
    janet_def(env, #const_name, jw32_wrap_hresult(const_name),          \
              "Constant for error codes returned by API functions.")
    __def(CO_E_FIRST);
    __def(CO_E_LAST);
    __def(CO_E_NOTINITIALIZED);
    __def(CO_E_ALREADYINITIALIZED);
    __def(CO_E_CANTDETERMINECLASS);
    __def(CO_E_CLASSSTRING);
    __def(CO_E_IIDSTRING);
    __def(CO_E_APPNOTFOUND);
    __def(CO_E_APPSINGLEUSE);
    __def(CO_E_ERRORINAPP);
    __def(CO_E_DLLNOTFOUND);
    __def(CO_E_ERRORINDLL);
    __def(CO_E_WRONGOSFORAPP);
    __def(CO_E_OBJNOTREG);
    __def(CO_E_OBJISREG);
    __def(CO_E_OBJNOTCONNECTED);
    __def(CO_E_APPDIDNTREG);
    __def(CO_E_RELEASED);
    __def(CO_E_INIT_TLS);
    __def(CO_E_INIT_SHARED_ALLOCATOR);
    __def(CO_E_INIT_MEMORY_ALLOCATOR);
    __def(CO_E_INIT_CLASS_CACHE);
    __def(CO_E_INIT_RPC_CHANNEL);
    __def(CO_E_INIT_TLS_SET_CHANNEL_CONTROL);
    __def(CO_E_INIT_TLS_CHANNEL_CONTROL);
    __def(CO_E_INIT_UNACCEPTED_USER_ALLOCATOR);
    __def(CO_E_INIT_SCM_MUTEX_EXISTS);
    __def(CO_E_INIT_SCM_FILE_MAPPING_EXISTS);
    __def(CO_E_INIT_SCM_MAP_VIEW_OF_FILE);
    __def(CO_E_INIT_SCM_EXEC_FAILURE);
    __def(CO_E_INIT_ONLY_SINGLE_THREADED);
    __def(CO_E_CANT_REMOTE);
    __def(CO_E_BAD_SERVER_NAME);
    __def(CO_E_WRONG_SERVER_IDENTITY);
    __def(CO_E_OLE1DDE_DISABLED);
    __def(CO_E_RUNAS_SYNTAX);
    __def(CO_E_CREATEPROCESS_FAILURE);
    __def(CO_E_RUNAS_CREATEPROCESS_FAILURE);
    __def(CO_E_RUNAS_LOGON_FAILURE);
    __def(CO_E_LAUNCH_PERMSSION_DENIED);
    __def(CO_E_START_SERVICE_FAILURE);
    __def(CO_E_REMOTE_COMMUNICATION_FAILURE);
    __def(CO_E_SERVER_START_TIMEOUT);
    __def(CO_E_CLSREG_INCONSISTENT);
    __def(CO_E_IIDREG_INCONSISTENT);
    __def(CO_E_NOT_SUPPORTED);
    __def(CO_E_RELOAD_DLL);
    __def(CO_E_MSI_ERROR);
    __def(CO_E_ATTEMPT_TO_CREATE_OUTSIDE_CLIENT_CONTEXT);
    __def(CO_E_SERVER_PAUSED);
    __def(CO_E_SERVER_NOT_PAUSED);
    __def(CO_E_CLASS_DISABLED);
    __def(CO_E_CLRNOTAVAILABLE);
    __def(CO_E_ASYNC_WORK_REJECTED);
    __def(CO_E_SERVER_INIT_TIMEOUT);
    __def(CO_E_NO_SECCTX_IN_ACTIVATE);
    __def(CO_E_TRACKER_CONFIG);
    __def(CO_E_THREADPOOL_CONFIG);
    __def(CO_E_SXS_CONFIG);
    __def(CO_E_MALFORMED_SPN);
    __def(CO_E_UNREVOKED_REGISTRATION_ON_APARTMENT_SHUTDOWN);
    __def(CO_E_PREMATURE_STUB_RUNDOWN);
#undef __def
}

static void define_consts_rpc_e(JanetTable *env)
{
#define __def(const_name)                                               \
    janet_def(env, #const_name, jw32_wrap_hresult(const_name),          \
              "Constant for error codes returned by API functions.")
    __def(RPC_E_CHANGED_MODE);
#undef __def
}


static Janet cfun_SUCCEEDED(int32_t argc, Janet *argv)
{
    HRESULT code;

    janet_fixarity(argc, 1);
    code = jw32_get_hresult(argv, 0); /* may also be an SCODE */
    return janet_wrap_boolean(SUCCEEDED(code));
}

static Janet cfun_FAILED(int32_t argc, Janet *argv)
{
    HRESULT code;

    janet_fixarity(argc, 1);
    code = jw32_get_hresult(argv, 0); /* may also be an SCODE */
    return janet_wrap_boolean(FAILED(code));
}

static Janet cfun_HRESULT_CODE(int32_t argc, Janet *argv)
{
    HRESULT code;

    janet_fixarity(argc, 1);
    code = jw32_get_hresult(argv, 0);
    return jw32_wrap_int(HRESULT_CODE(code));
}

static Janet cfun_SCODE_CODE(int32_t argc, Janet *argv)
{
    SCODE code;

    janet_fixarity(argc, 1);
    code = jw32_get_hresult(argv, 0);
    return jw32_wrap_int(SCODE_CODE(code));
}

static Janet cfun_SetLastError(int32_t argc, Janet *argv)
{
    DWORD dwErrCode;

    janet_fixarity(argc, 1);

    dwErrCode = jw32_get_dword(argv, 0);
    SetLastError(dwErrCode);
    return janet_wrap_nil();
}

static Janet cfun_GetLastError(int32_t argc, Janet *argv)
{
    (void)argv;
    janet_fixarity(argc, 0);

    return jw32_wrap_dword(GetLastError());
}

static Janet cfun_SetErrorMode(int32_t argc, Janet *argv)
{
    UINT uMode, uRet;

    janet_fixarity(argc, 1);

    uMode = jw32_get_uint(argv, 0);

    uRet = SetErrorMode(uMode);
    return jw32_wrap_uint(uRet);
}

static Janet cfun_GetErrorMode(int32_t argc, Janet *argv)
{
    UINT uRet;

    (void)argv;
    janet_fixarity(argc, 0);

    uRet = GetErrorMode();
    return jw32_wrap_uint(uRet);
}


static const JanetReg cfuns[] = {
    {
        "SUCCEEDED",
        cfun_SUCCEEDED,
        "(" MOD_NAME "/SUCCEEDED hr)\n\n"
        "Check if an HRESULT or SCODE represents success.",
    },
    {
        "FAILED",
        cfun_FAILED,
        "(" MOD_NAME "/FAILED hr)\n\n"
        "Check if an HRESULT or SCODE represents failure.",
    },
    {
        "HRESULT_CODE",
        cfun_HRESULT_CODE,
        "(" MOD_NAME "/HRESULT_CODE hr)\n\n"
        "Extracts the code portion of the specified HRESULT.",
    },
    {
        "SCODE_CODE",
        cfun_SCODE_CODE,
        "(" MOD_NAME "/SCODE_CODE sc)\n\n"
        "Extracts the code portion of the specified SCODE.",
    },
    {
        "SetLastError",
        cfun_SetLastError,
        "(" MOD_NAME "/SetLastError dwErrCode)\n\n"
        "Sets the calling thread's last-error code value.",
    },
    {
        "GetLastError",
        cfun_GetLastError,
        "(" MOD_NAME "/GetLastError)\n\n"
        "Retrieves the calling thread's last-error code value.",
    },
    {
        "SetErrorMode",
        cfun_SetErrorMode,
        "(" MOD_NAME "/SetErrorMode uMode)\n\n"
        "Sets the process error mode.",
    },
    {
        "GetErrorMode",
        cfun_GetErrorMode,
        "(" MOD_NAME "/GetErrorMode)\n\n"
        "Gets the process error mode.",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_consts_sem(env);
    define_consts_noerror(env);
    define_consts_s(env);
    define_consts_e(env);
    define_consts_co_e(env);
    define_consts_rpc_e(env);

    janet_cfuns(env, MOD_NAME, cfuns);
}
