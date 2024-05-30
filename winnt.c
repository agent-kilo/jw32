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


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_consts_token_information_class(env);
}
