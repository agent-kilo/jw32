#include "jw32.h"

#define MOD_NAME "processthreadsapi"


static void define_consts_token(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),    \
              "Access rights for access tokens.")

    __def(TOKEN_ASSIGN_PRIMARY);
    __def(TOKEN_DUPLICATE);
    __def(TOKEN_IMPERSONATE);
    __def(TOKEN_QUERY);
    __def(TOKEN_QUERY_SOURCE);
    __def(TOKEN_ADJUST_PRIVILEGES);
    __def(TOKEN_ADJUST_GROUPS);
    __def(TOKEN_ADJUST_DEFAULT);
    __def(TOKEN_ADJUST_SESSIONID);
    __def(TOKEN_ALL_ACCESS_P);
#if ((defined(_WIN32_WINNT) && (_WIN32_WINNT > 0x0400)) || (!defined(_WIN32_WINNT)))
    __def(TOKEN_ALL_ACCESS);
#else
    __def(TOKEN_ALL_ACCESS);
#endif
    __def(TOKEN_READ);
    __def(TOKEN_WRITE);
    __def(TOKEN_EXECUTE);
    __def(TOKEN_TRUST_CONSTRAINT_MASK);
#if (NTDDI_VERSION >= NTDDI_WIN8)
    __def(TOKEN_ACCESS_PSEUDO_HANDLE_WIN8);
    __def(TOKEN_ACCESS_PSEUDO_HANDLE);
#endif

#undef __def
}


static void define_consts_process(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),    \
              "Process-specific access rights.")

    __def(PROCESS_TERMINATE);
    __def(PROCESS_CREATE_THREAD);
    __def(PROCESS_SET_SESSIONID);
    __def(PROCESS_VM_OPERATION);
    __def(PROCESS_VM_READ);
    __def(PROCESS_VM_WRITE);
    __def(PROCESS_DUP_HANDLE);
    __def(PROCESS_CREATE_PROCESS);
    __def(PROCESS_SET_QUOTA);
    __def(PROCESS_SET_INFORMATION);
    __def(PROCESS_QUERY_INFORMATION);
    __def(PROCESS_SUSPEND_RESUME);
    __def(PROCESS_QUERY_LIMITED_INFORMATION);
    __def(PROCESS_SET_LIMITED_INFORMATION);
    __def(PROCESS_ALL_ACCESS);

#undef __def
}


static Janet cfun_GetCurrentProcess(int32_t argc, Janet *argv)
{
    (void)argv;
    janet_fixarity(argc, 0);

    /* XXX: janet will truncate the handle values if we used jw32_wrap_handle() here.
       use jw32_wrap_uint() instead to preserve all the bits */
    return jw32_wrap_uint(GetCurrentProcess());
}


static Janet cfun_GetCurrentProcessId(int32_t argc, Janet *argv)
{
    (void)argv;
    janet_fixarity(argc, 0);

    return jw32_wrap_dword(GetCurrentProcessId());
}


static Janet cfun_GetCurrentThreadId(int32_t argc, Janet *argv)
{
    (void)argv;
    janet_fixarity(argc, 0);

    return jw32_wrap_dword(GetCurrentThreadId());
}


static Janet cfun_OpenProcess(int32_t argc, Janet *argv)
{
    DWORD dwDesiredAccess;
    BOOL  bInheritHandle;
    DWORD dwProcessId;

    HANDLE hProcess;

    janet_fixarity(argc, 3);

    dwDesiredAccess = jw32_get_dword(argv, 0);
    bInheritHandle = jw32_get_bool(argv, 1);
    dwProcessId = jw32_get_dword(argv, 2);

    hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
    return jw32_wrap_handle(hProcess);
}


static Janet cfun_OpenProcessToken(int32_t argc, Janet *argv)
{
    HANDLE processHandle;
    DWORD desiredAccess;

    HANDLE token = NULL;
    Janet ret_tuple[2];

    janet_fixarity(argc, 2);

    processHandle = jw32_get_handle(argv, 0);
    desiredAccess = jw32_get_dword(argv, 1);

    ret_tuple[0] = jw32_wrap_bool(OpenProcessToken(processHandle, desiredAccess, &token));
    ret_tuple[1] = jw32_wrap_handle(token);

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}


static Janet cfun_OpenThreadToken(int32_t argc, Janet *argv)
{
    HANDLE threadHandle;
    DWORD desiredAccess;
    BOOL openAsSelf;

    HANDLE token = NULL;
    Janet ret_tuple[2];

    janet_fixarity(argc, 3);

    threadHandle = jw32_get_handle(argv, 0);
    desiredAccess = jw32_get_dword(argv, 1);
    openAsSelf = jw32_get_bool(argv, 2);

    ret_tuple[0] = jw32_wrap_bool(OpenThreadToken(threadHandle, desiredAccess, openAsSelf, &token));
    ret_tuple[1] = jw32_wrap_handle(token);

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}


static Janet cfun_ProcessIdToSessionId(int32_t argc, Janet *argv)
{
    DWORD dwProcessId;

    BOOL bRet;

    DWORD dwSessionId = 0;
    Janet ret_tuple[2];

    janet_fixarity(argc, 1);

    dwProcessId = jw32_get_dword(argv, 0);

    bRet = ProcessIdToSessionId(dwProcessId, &dwSessionId);

    ret_tuple[0] = jw32_wrap_bool(bRet);
    ret_tuple[1] = jw32_wrap_dword(dwSessionId);

    return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
}


static const JanetReg cfuns[] = {
    {
        "GetCurrentProcess",
        cfun_GetCurrentProcess,
        "(" MOD_NAME "/GetCurrentProcess)\n\n"
        "Returns the current process handle.",
    },
    {
        "GetCurrentProcessId",
        cfun_GetCurrentProcessId,
        "(" MOD_NAME "/GetCurrentProcessId)\n\n"
        "Returns the current process ID.",
    },
    {
        "GetCurrentThreadId",
        cfun_GetCurrentThreadId,
        "(" MOD_NAME "/GetCurrentThreadId)\n\n"
        "Returns the current thread id.",
    },
    {
        "OpenProcess",
        cfun_OpenProcess,
        "(" MOD_NAME "/OpenProcess dwDesiredAccess bInheritHandle dwProcessId)\n\n"
        "Opens an existing local process object.",
    },
    {
        "OpenProcessToken",
        cfun_OpenProcessToken,
        "(" MOD_NAME "/OpenProcessToken ProcessHandle DesiredAccess)\n\n"
        "Opens the access token associated with a process.",
    },
    {
        "OpenThreadToken",
        cfun_OpenThreadToken,
        "(" MOD_NAME "/OpenThreadToken ThreadHandle DesiredAccess OpenAsSelf)\n\n"
        "Opens the access token associated with a thread.",
    },
    {
        "ProcessIdToSessionId",
        cfun_ProcessIdToSessionId,
        "(" MOD_NAME "/ProcessIdToSessionId dwProcessId)\n\n"
        "Retrieves the Remote Desktop Services session associated with a process.",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_consts_token(env);
    define_consts_process(env);

    janet_cfuns(env, MOD_NAME, cfuns);
}
