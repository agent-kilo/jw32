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
        uRet = GetAtomName(nAtom, name_buf->data, name_buf->capacity);
        cap *= 2;
    } while (uRet >= name_buf->capacity - 1);

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
        uRet = GlobalGetAtomName(nAtom, name_buf->data, name_buf->capacity);
        cap *= 2;
    } while (uRet >= name_buf->capacity - 1);

    if (uRet > 0) {
        name_buf->count = uRet;
    }

    return jw32_wrap_uint(uRet);
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
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, MOD_NAME, cfuns);
}
