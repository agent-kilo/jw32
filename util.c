#include "jw32.h"
#include <windowsx.h>

#define MOD_NAME "util"


static Janet cfun_IsNull(int32_t argc, Janet *argv)
{
    LPVOID p;
    janet_fixarity(argc, 1);
    p = jw32_get_lpvoid(argv, 0);
    return janet_wrap_boolean(p == NULL);
}

static Janet cfun_LOWORD(int32_t argc, Janet *argv)
{
    DWORD_PTR dwordptr;

    janet_fixarity(argc, 1);

    dwordptr = jw32_get_dword_ptr(argv, 0);
    return jw32_wrap_word(LOWORD(dwordptr));
}

static Janet cfun_HIWORD(int32_t argc, Janet *argv)
{
    DWORD_PTR dwordptr;

    janet_fixarity(argc, 1);

    dwordptr = jw32_get_dword_ptr(argv, 0);
    return jw32_wrap_word(HIWORD(dwordptr));
}


static Janet cfun_GET_X_LPARAM(int32_t argc, Janet *argv)
{
    LPARAM lParam;

    janet_fixarity(argc, 1);

    lParam = jw32_get_lparam(argv, 0);
    return jw32_wrap_int(GET_X_LPARAM(lParam));
}


static Janet cfun_GET_Y_LPARAM(int32_t argc, Janet *argv)
{
    LPARAM lParam;

    janet_fixarity(argc, 1);

    lParam = jw32_get_lparam(argv, 0);
    return jw32_wrap_int(GET_Y_LPARAM(lParam));
}


static Janet cfun_signed_to_unsigned_64(int32_t argc, Janet *argv)
{
    int64_t n;

    janet_fixarity(argc, 1);

    n = janet_getinteger64(argv, 0);
    return janet_wrap_u64((uint64_t)n);
}


static Janet cfun_unsigned_to_signed_64(int32_t argc, Janet *argv)
{
    uint64_t n;

    janet_fixarity(argc, 1);

    n = janet_getuinteger64(argv, 0);
    return janet_wrap_s64((int64_t)n);
}


static Janet cfun_signed_to_unsigned_32(int32_t argc, Janet *argv)
{
    int32_t n;

    janet_fixarity(argc, 1);

    n = janet_getinteger(argv, 0);
    return janet_wrap_u64((uint32_t)n);
}


static Janet cfun_unsigned_to_signed_32(int32_t argc, Janet *argv)
{
    uint64_t n;

    janet_fixarity(argc, 1);

    n = janet_getuinteger64(argv, 0);
    if (n > UINT_MAX) {
        janet_panicf("bad slop #0: expected 32 bit unsigned integer, got %v", argv[0]);
    }
    return janet_wrap_integer((int32_t)((uint32_t)n));
}


static Janet cfun_alloc_and_marshal(int32_t argc, Janet *argv)
{
    Janet x;

    janet_fixarity(argc, 1);

    x = argv[0];

    JanetBuffer *buf = janet_malloc(sizeof(JanetBuffer));
    if (!buf) {
        janet_panicf("out of memory");
    }
    janet_buffer_init(buf, 0);
    janet_marshal(buf, x, NULL, JANET_MARSHAL_UNSAFE);

    return janet_wrap_pointer((void *)buf);
}


static Janet cfun_unmarshal_and_free(int32_t argc, Janet *argv)
{
    JanetBuffer *buf;

    Janet ret;

    janet_fixarity(argc, 1);

    if (janet_checktype(argv[0], JANET_POINTER)) {
        buf = (JanetBuffer *)janet_getpointer(argv, 0);
    } else {
        buf = (JanetBuffer *)jw32_get_ulong_ptr(argv, 0);
    }

    ret = janet_unmarshal(buf->data, buf->count, JANET_MARSHAL_UNSAFE, NULL, NULL);
    janet_free(buf);
    return ret;
}


static Janet cfun_alloc_console_and_reopen_streams(int32_t argc, Janet *argv)
{
    janet_fixarity(argc, 0);
    (void)argv;

    if (!AllocConsole()) {
        janet_panic("AllocConsole() failed");
    }

    errno_t err = 0;
    FILE *con_stdin;
    FILE *con_stdout;
    FILE *con_stderr;

    err = freopen_s(&con_stdin, "CONIN$", "r", stdin);
    if (err) {
        janet_panicf("freopen_s() failed for stdin: %n", err);
    }

    err = freopen_s(&con_stdout, "CONOUT$", "w", stdout);
    if (err) {
        janet_panicf("freopen_s() failed for stdout: %n", err);
    }

    err = freopen_s(&con_stderr, "CONOUT$", "w", stderr);
    if (err) {
        janet_panicf("freopen_s() failed for stderr: %n", err);
    }

    return janet_wrap_nil();
}


static const JanetReg cfuns[] = {
    {
        "null?",
        cfun_IsNull,
        "(" MOD_NAME "/null? pointer)\n\n"
        "Check if pointer is NULL.",
    },
    {
        "LOWORD",
        cfun_LOWORD,
        "(" MOD_NAME "/LOWORD dwordptr)\n\n"
        "Retrieves LOWORD.",
    },
    {
        "HIWORD",
        cfun_HIWORD,
        "(" MOD_NAME "/HIWORD dwordptr)\n\n"
        "Retrieves HIWORD.",
    },
    {
        "GET_X_LPARAM",
        cfun_GET_X_LPARAM,
        "(" MOD_NAME "/GET_X_LPARAM lparam)\n\n"
        "Retrieves the signed x-coordinate from the specified value.",
    },
    {
        "GET_Y_LPARAM",
        cfun_GET_Y_LPARAM,
        "(" MOD_NAME "/GET_Y_LPARAM lparam)\n\n"
        "Retrieves the signed y-coordinate from the specified value.",
    },
    {
        "signed-to-unsigned-64",
        cfun_signed_to_unsigned_64,
        "(" MOD_NAME "/signed-to-unsigned-64 n)\n\n"
        "Converts a signed integer to an unsigned integer."
    },
    {
        "unsigned-to-signed-64",
        cfun_unsigned_to_signed_64,
        "(" MOD_NAME "/unsigned-to-signed-64 n)\n\n"
        "Converts an unsigned integer to a signed integer."
    },
    {
        "signed-to-unsigned-32",
        cfun_signed_to_unsigned_32,
        "(" MOD_NAME "/signed-to-unsigned-32 n)\n\n"
        "Converts a signed integer to an unsigned integer."
    },
    {
        "unsigned-to-signed-32",
        cfun_unsigned_to_signed_32,
        "(" MOD_NAME "/unsigned-to-signed-32 n)\n\n"
        "Converts an unsigned integer to a signed integer."
    },
    {
        "alloc-and-marshal",
        cfun_alloc_and_marshal,
        "(" MOD_NAME "/alloc-and-marshal x)\n\n"
        "Marshals a Janet object into a newly allocated buffer, and returns a pointer pointing to the buffer."
    },
    {
        "unmarshal-and-free",
        cfun_unmarshal_and_free,
        "(" MOD_NAME "/unmarshal-and-free ptr)\n\n"
        "Unmarshals a Janet object from the buffer pointed to by ptr, and then frees the buffer."
    },
    {
        "alloc-console-and-reopen-streams",
        cfun_alloc_console_and_reopen_streams,
        "(" MOD_NAME "/alloc-console-and-reopen-streams)\n\n"
        "Opens a console and redirect stdin, stdout and stderr."
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    janet_cfuns(env, MOD_NAME, cfuns);

    janet_def(env, "NULL", jw32_wrap_lpvoid(NULL),
              "The NULL pointer, for comparison with API return values.");
    janet_def(env, "TRUE", jw32_wrap_bool(TRUE),
              "TRUE value, for comparison with API return values.");
    janet_def(env, "FALSE", jw32_wrap_bool(FALSE),
              "FALSE value, for comparison with API return values.");
}
