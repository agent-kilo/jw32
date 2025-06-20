#include "jw32.h"

#define MOD_NAME "winnt"


static void define_consts_token_information_class(JanetTable *env)
{
#define __def(const_name)                                     \
    janet_def(env, #const_name, jw32_wrap_int(const_name),    \
              "Values for TOKEN_INFORMATION_CLASS.")

    __def(TokenUser);
    __def(TokenGroups);
    __def(TokenPrivileges);
    __def(TokenOwner);
    __def(TokenPrimaryGroup);
    __def(TokenDefaultDacl);
    __def(TokenSource);
    __def(TokenType);
    __def(TokenImpersonationLevel);
    __def(TokenStatistics);
    __def(TokenRestrictedSids);
    __def(TokenSessionId);
    __def(TokenGroupsAndPrivileges);
    __def(TokenSessionReference);
    __def(TokenSandBoxInert);
    __def(TokenAuditPolicy);
    __def(TokenOrigin);
    __def(TokenElevationType);
    __def(TokenLinkedToken);
    __def(TokenElevation);
    __def(TokenHasRestrictions);
    __def(TokenAccessInformation);
    __def(TokenVirtualizationAllowed);
    __def(TokenVirtualizationEnabled);
    __def(TokenIntegrityLevel);
    __def(TokenUIAccess);
    __def(TokenMandatoryPolicy);
    __def(TokenLogonSid);
    __def(TokenIsAppContainer);
    __def(TokenCapabilities);
    __def(TokenAppContainerSid);
    __def(TokenAppContainerNumber);
    __def(TokenUserClaimAttributes);
    __def(TokenDeviceClaimAttributes);
    __def(TokenRestrictedUserClaimAttributes);
    __def(TokenRestrictedDeviceClaimAttributes);
    __def(TokenDeviceGroups);
    __def(TokenRestrictedDeviceGroups);
    __def(TokenSecurityAttributes);
    __def(TokenIsRestricted);
    __def(TokenProcessTrustLevel);
    __def(TokenPrivateNameSpace);
    __def(TokenSingletonAttributes);
    __def(TokenBnoIsolation);
    __def(TokenChildProcessFlags);
    __def(TokenIsLessPrivilegedAppContainer);
    __def(TokenIsSandboxed);
    __def(TokenIsAppSilo);
    __def(MaxTokenInfoClass);

#undef __def
}


static void define_consts_key(JanetTable *env)
{
#define __def(const_name)                                     \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),    \
              "Constant for registry key access rights.")

    __def(KEY_QUERY_VALUE);
    __def(KEY_SET_VALUE);
    __def(KEY_CREATE_SUB_KEY);
    __def(KEY_ENUMERATE_SUB_KEYS);
    __def(KEY_NOTIFY);
    __def(KEY_CREATE_LINK);
    __def(KEY_WOW64_32KEY);
    __def(KEY_WOW64_64KEY);
    __def(KEY_WOW64_RES);
    __def(KEY_READ);
    __def(KEY_WRITE);
    __def(KEY_EXECUTE);
    __def(KEY_ALL_ACCESS);

#undef __def
}


static void define_consts_reg_option(JanetTable *env)
{
#define __def(const_name)                                     \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),    \
              "Constant for registry key open/create options.")

    __def(REG_OPTION_RESERVED);
    __def(REG_OPTION_NON_VOLATILE);
    __def(REG_OPTION_VOLATILE);
    __def(REG_OPTION_CREATE_LINK);
    __def(REG_OPTION_BACKUP_RESTORE);
    __def(REG_OPTION_OPEN_LINK);
    __def(REG_OPTION_DONT_VIRTUALIZE);
    __def(REG_LEGAL_OPTION);
    __def(REG_OPEN_LEGAL_OPTION);

#undef __def
}


static void define_consts_reg(JanetTable *env)
{
#define __def(const_name)                                     \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),    \
              "Constant for registry value types.")

    __def(REG_NONE);
    __def(REG_SZ);
    __def(REG_EXPAND_SZ);
    __def(REG_BINARY);
    __def(REG_DWORD);
    __def(REG_DWORD_LITTLE_ENDIAN);
    __def(REG_DWORD_BIG_ENDIAN);
    __def(REG_LINK);
    __def(REG_MULTI_SZ);
    __def(REG_RESOURCE_LIST);
    __def(REG_FULL_RESOURCE_DESCRIPTOR);
    __def(REG_RESOURCE_REQUIREMENTS_LIST);
    __def(REG_QWORD);
    __def(REG_QWORD_LITTLE_ENDIAN);

#undef __def
}


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_consts_token_information_class(env);
    define_consts_key(env);
    define_consts_reg_option(env);
    define_consts_reg(env);
}
