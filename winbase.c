#include "jw32.h"
#include "debug.h"

#define MOD_NAME "winbase"


static Janet cfun_AddAtom(int32_t argc, Janet *argv)
{
    LPCSTR lpString;

    ATOM aRet;

    janet_fixarity(argc, 1);

    lpString = jw32_get_lpcstr(argv, 0);
    aRet = AddAtom(lpString);
    return jw32_wrap_atom(aRet);
}

static Janet cfun_DeleteAtom(int32_t argc, Janet *argv)
{
    ATOM nAtom;

    ATOM aRet;

    janet_fixarity(argc, 1);

    nAtom = jw32_get_atom(argv, 0);
    aRet = DeleteAtom(nAtom);
    return jw32_wrap_atom(aRet);
}

static Janet cfun_FindAtom(int32_t argc, Janet *argv)
{
    LPCSTR lpString;

    ATOM aRet;

    janet_fixarity(argc, 1);

    lpString = jw32_get_lpcstr(argv, 0);
    aRet = FindAtom(lpString);
    return jw32_wrap_atom(aRet);
}

static Janet cfun_GetAtomName(int32_t argc, Janet *argv)
{
    ATOM nAtom;
    JanetBuffer *name_buf;

    UINT uRet;

    int32_t cap = JW32_BUFFER_INIT_CAPACITY;

    janet_fixarity(argc, 2);

    nAtom = jw32_get_atom(argv, 0);
    name_buf = janet_getbuffer(argv, 1);

    if (name_buf->capacity > JW32_BUFFER_INIT_CAPACITY) {
        cap = lower_power_of_two(name_buf->capacity);
    }

    jw32_dbg_val(cap, "%d");

    do {
        janet_buffer_ensure(name_buf, cap, 1);
        jw32_dbg_val(name_buf->capacity, "%d");
        uRet = GetAtomName(nAtom, (LPSTR)name_buf->data, name_buf->capacity);
        cap *= 2;
    } while (uRet >= (UINT)(name_buf->capacity - 1));

    if (uRet > 0) {
        name_buf->count = uRet;
    }

    return jw32_wrap_uint(uRet);
}

static Janet cfun_GlobalAddAtom(int32_t argc, Janet *argv)
{
    LPCSTR lpString;

    ATOM aRet;

    janet_fixarity(argc, 1);

    lpString = jw32_get_lpcstr(argv, 0);
    aRet = GlobalAddAtom(lpString);
    return jw32_wrap_atom(aRet);
}

static Janet cfun_GlobalDeleteAtom(int32_t argc, Janet *argv)
{
    ATOM nAtom;

    ATOM aRet;

    janet_fixarity(argc, 1);

    nAtom = jw32_get_atom(argv, 0);
    aRet = GlobalDeleteAtom(nAtom);
    return jw32_wrap_atom(aRet);
}

static Janet cfun_GlobalFindAtom(int32_t argc, Janet *argv)
{
    LPCSTR lpString;

    ATOM aRet;

    janet_fixarity(argc, 1);

    lpString = jw32_get_lpcstr(argv, 0);
    aRet = GlobalFindAtom(lpString);
    return jw32_wrap_atom(aRet);
}

static Janet cfun_GlobalGetAtomName(int32_t argc, Janet *argv)
{
    ATOM nAtom;
    JanetBuffer *name_buf;

    UINT uRet;

    int32_t cap = JW32_BUFFER_INIT_CAPACITY;

    janet_fixarity(argc, 2);

    nAtom = jw32_get_atom(argv, 0);
    name_buf = janet_getbuffer(argv, 1);

    if (name_buf->capacity > JW32_BUFFER_INIT_CAPACITY) {
        cap = lower_power_of_two(name_buf->capacity);
    }

    jw32_dbg_val(cap, "%d");

    do {
        janet_buffer_ensure(name_buf, cap, 1);
        jw32_dbg_val(name_buf->capacity, "%d");
        uRet = GlobalGetAtomName(nAtom, (LPSTR)name_buf->data, name_buf->capacity);
        cap *= 2;
    } while (uRet >= (UINT)(name_buf->capacity - 1));

    if (uRet > 0) {
        name_buf->count = uRet;
    }

    return jw32_wrap_uint(uRet);
}

static Janet cfun_SetDllDirectory(int32_t argc, Janet *argv)
{
    LPCSTR lpPathName;

    BOOL bRet;

    janet_fixarity(argc, 1);

    lpPathName = jw32_get_lpcstr(argv, 0);

    bRet = SetDllDirectory(lpPathName);
    return jw32_wrap_bool(bRet);
}


static Janet cfun_QueryFullProcessImageName(int32_t argc, Janet *argv)
{
    HANDLE hProcess;
    DWORD dwFlags;

    BOOL bRet;

    JanetBuffer *buf;
    DWORD dwSize;

    janet_fixarity(argc, 2);

    hProcess = jw32_get_handle(argv, 0);
    dwFlags = jw32_get_dword(argv, 1);

    buf = janet_buffer(MAX_PATH);
    dwSize = buf->capacity;

    while (1) {
        bRet = QueryFullProcessImageName(hProcess, dwFlags, (LPSTR)buf->data, &dwSize);
        if (0 == bRet) {
            break;
        }
        /* plus one NULL byte */
        if ((dwSize + 1) < (DWORD)buf->capacity) {
            /* We have the full path */
            break;
        }
        if (buf->capacity * 2 > INT_MAX) {
            /* We don't have the full path, but can't expand buf any further */
            break;
        }

        janet_buffer_ensure(buf, buf->capacity * 2, 1);
        dwSize = buf->capacity;
    }

    if (bRet) {
        buf->count = dwSize;  /* Sans the trailing NULL byte */
        return janet_wrap_buffer(buf);
    } else {
        return janet_wrap_nil();
    }
}


static const JanetReg cfuns[] = {
    {
        "AddAtom",
        cfun_AddAtom,
        "(" MOD_NAME "/AddAtom lpString)\n\n"
        "Adds an atom.",
    },
    {
        "DeleteAtom",
        cfun_DeleteAtom,
        "(" MOD_NAME "/DeleteAtom nAtom)\n\n"
        "Deletes an atom.",
    },
    {
        "FindAtom",
        cfun_FindAtom,
        "(" MOD_NAME "/FindAtom lpString)\n\n"
        "Finds an atom.",
    },
    {
        "GetAtomName",
        cfun_GetAtomName,
        "(" MOD_NAME "/GetAtomName nAtom lpBuffer)\n\n"
        "lpBuffer should be a janet buffer, who's content will be overridden upon a successful call",
    },
    {
        "GlobalAddAtom",
        cfun_GlobalAddAtom,
        "(" MOD_NAME "/GlobalAddAtom lpString)\n\n"
        "Adds a global atom.",
    },
    {
        "GlobalDeleteAtom",
        cfun_GlobalDeleteAtom,
        "(" MOD_NAME "/GlobalDeleteAtom nAtom)\n\n"
        "Deletes a global atom.",
    },
    {
        "GlobalFindAtom",
        cfun_GlobalFindAtom,
        "(" MOD_NAME "/GlobalFindAtom lpString)\n\n"
        "Finds a global atom.",
    },
    {
        "GlobalGetAtomName",
        cfun_GlobalGetAtomName,
        "(" MOD_NAME "/GlobalGetAtomName nAtom lpBuffer)\n\n"
        "lpBuffer should be a janet buffer, who's content will be overridden upon a successful call",
    },
    {
        "SetDllDirectory",
        cfun_SetDllDirectory,
        "(" MOD_NAME "/SetDllDirectory lpPathName)\n\n"
        "Adds a directory to the DLL search path.",
    },
    {
        "QueryFullProcessImageName",
        cfun_QueryFullProcessImageName,
        "(" MOD_NAME "/QueryFullProcessImageName hProcess dwFlags)\n\n"
        "Retrieves the full name of the executable image for the specified process.",
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, MOD_NAME, cfuns);
}
