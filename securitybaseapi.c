#include "jw32.h"

#define MOD_NAME "securitybaseapi"


static Janet cfun_GetTokenInformation(int32_t argc, Janet *argv)
{
    HANDLE tokenHandle;
    TOKEN_INFORMATION_CLASS tokenInformationClass;

    DWORD retLen;
    Janet ret_tuple[2];

    janet_fixarity(argc, 2);

    tokenHandle = jw32_get_handle(argv, 0);
    tokenInformationClass = jw32_get_int(argv, 1);

    switch (tokenInformationClass) {
    case TokenElevation: {
        TOKEN_ELEVATION elev = {0};
        BOOL bRet = GetTokenInformation(tokenHandle, tokenInformationClass, &elev, sizeof(elev), &retLen);
        ret_tuple[0] = jw32_wrap_bool(bRet);
        ret_tuple[1] = janet_wrap_boolean(0);
        if (bRet) {
            ret_tuple[1] = janet_wrap_boolean(elev.TokenIsElevated != 0);
        }
        return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
    }
    case TokenUIAccess: {
        DWORD ua = 0;
        BOOL bRet = GetTokenInformation(tokenHandle, tokenInformationClass, &ua, sizeof(ua), &retLen);
        ret_tuple[0] = jw32_wrap_bool(bRet);
        ret_tuple[1] = janet_wrap_boolean(0);
        if (bRet) {
            ret_tuple[1] = janet_wrap_boolean(ua != 0);
        }
        return janet_wrap_tuple(janet_tuple_n(ret_tuple, 2));
    }
    default:
        janet_panicf("unsupported token information class: %d", tokenInformationClass);
    }
}


static const JanetReg cfuns[] = {
    {
        "GetTokenInformation",
        cfun_GetTokenInformation,
        "(" MOD_NAME "/GetTokenInformation TokenHandle TokenInformationClass)\n\n"
        "Retrieves a specified type of information about an access token.",
    },
    {NULL, NULL, NULL}
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, MOD_NAME, cfuns);
}
