#include "jw32.h"
#include <windowsx.h>

#include "gc.h"       /* for JANET_MEM_* constants */
#include "state.h"    /* for JanetVM internals */

#if __has_include("vcs-version.h")
#include "vcs-version.h"
#endif

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
    JanetTable *rlookup;

    janet_arity(argc, 1, 2);

    x = argv[0];
    if (argc <= 1) {
        rlookup = NULL;
    } else {
        rlookup = janet_gettable(argv, 1);
    }

    JanetBuffer *buf = janet_malloc(sizeof(JanetBuffer));
    if (!buf) {
        janet_panicf("out of memory");
    }
    janet_buffer_init(buf, 0);
    janet_marshal(buf, x, rlookup, JANET_MARSHAL_UNSAFE);

    return janet_wrap_pointer((void *)buf);
}


static Janet cfun_unmarshal_and_free(int32_t argc, Janet *argv)
{
    JanetBuffer *buf;
    JanetTable *lookup;

    Janet ret;

    janet_arity(argc, 1, 2);

    if (janet_checktype(argv[0], JANET_POINTER)) {
        buf = (JanetBuffer *)janet_getpointer(argv, 0);
    } else {
        buf = (JanetBuffer *)jw32_get_ulong_ptr(argv, 0);
    }
    if (argc <= 1) {
        lookup = NULL;
    } else {
        lookup = janet_gettable(argv, 1);
    }

    ret = janet_unmarshal(buf->data, buf->count, JANET_MARSHAL_UNSAFE, lookup, NULL);
    janet_buffer_deinit(buf);
    janet_free(buf);
    return ret;
}


static Janet cfun_alloc_console_and_reopen_streams(int32_t argc, Janet *argv)
{
    janet_fixarity(argc, 0);
    (void)argv;

    HWND h_con = GetConsoleWindow();
    if (NULL != h_con) {
        /* We already have a console attached */
        return janet_wrap_boolean(0);
    }

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

    return janet_wrap_boolean(1);
}


#define __JANET_MEMORY_TYPE_COUNT (JANET_MEMORY_ARRAY_WEAK + 1)

static const char *mem_type_names[__JANET_MEMORY_TYPE_COUNT] = {
    "none",
    "string",
    "symbol",
    "array",
    "tuple",
    "table",
    "struct",
    "fiber",
    "buffer",
    "function",
    "abstract",
    "funcenv",
    "funcdef",
    "threaded-abstract",
    "table-weakk",
    "table-weakv",
    "table-weakkv",
    "array-weak",
};


static SIZE_T heap_size(HANDLE hHeap, LPVOID p)
{
    SIZE_T ret = HeapSize(hHeap, 0, p);
    if (((SIZE_T)-1) == ret) {
        janet_panic("HeapSize failed");
    }
    return ret;
}


/* See src/core/table.c in Janet source tree */
#define JANET_TABLE_FLAG_STACK 0x10000

static size_t calc_memory_size(JanetGCObject *mem, HANDLE hHeap)
{
    switch (mem->flags & JANET_MEM_TYPEBITS) {
    default:
    case JANET_MEMORY_NONE:
    case JANET_MEMORY_STRING:
    case JANET_MEMORY_SYMBOL:
    case JANET_MEMORY_TUPLE:
    case JANET_MEMORY_STRUCT:
    case JANET_MEMORY_FUNCTION:
    case JANET_MEMORY_ABSTRACT:
    case JANET_MEMORY_THREADED_ABSTRACT:
    {
        return heap_size(hHeap, mem);
    }
    case JANET_MEMORY_ARRAY:
    case JANET_MEMORY_ARRAY_WEAK:
    {
        JanetArray *arr = (JanetArray *)mem;
        size_t data_size = 0;
        if (arr->data) {
            data_size = heap_size(hHeap, arr->data);
        }
        return heap_size(hHeap, mem) + data_size;
    }
    case JANET_MEMORY_TABLE:
    case JANET_MEMORY_TABLE_WEAKK:
    case JANET_MEMORY_TABLE_WEAKV:
    case JANET_MEMORY_TABLE_WEAKKV:
    {
        JanetTable *tab = (JanetTable *)mem;
        size_t data_size = 0;
        if (tab->data && !(mem->flags & JANET_TABLE_FLAG_STACK)) {
            data_size = heap_size(hHeap, tab->data);
        }
        return heap_size(hHeap, mem) + data_size;
    }
    case JANET_MEMORY_FIBER:
    {
        JanetFiber *fib = (JanetFiber *)mem;
        size_t data_size = heap_size(hHeap, fib->data);
        return heap_size(hHeap, mem) + data_size;
    }
    case JANET_MEMORY_BUFFER:
    {
        JanetBuffer *buf = (JanetBuffer *)mem;
        size_t data_size = 0;
        if (!(mem->flags & JANET_BUFFER_FLAG_NO_REALLOC)) {
            data_size = heap_size(hHeap, buf->data);
        }
        return heap_size(hHeap, mem) + data_size;
    }
    case JANET_MEMORY_FUNCENV:
    {
        JanetFuncEnv *env = (JanetFuncEnv *)mem;
        size_t data_size = 0;
        if (!(env->offset)) {
            data_size = heap_size(hHeap, env->as.values);
        }
        return heap_size(hHeap, mem) + data_size;
    }
    case JANET_MEMORY_FUNCDEF:
    {
        JanetFuncDef *def = (JanetFuncDef *)mem;
        size_t data_size = 0;
        if (def->constants) {
            data_size += heap_size(hHeap, def->constants);
        }
        if (def->symbolmap) {
            data_size += heap_size(hHeap, def->symbolmap);
        }
        if (def->bytecode) {
            data_size += heap_size(hHeap, def->bytecode);
        }
        if (def->environments) {
            data_size += heap_size(hHeap, def->environments);
        }
        if (def->defs) {
            data_size += heap_size(hHeap, def->defs);
        }
        if (def->sourcemap) {
            data_size += heap_size(hHeap, def->sourcemap);
        }
        if (def->closure_bitset) {
            data_size += heap_size(hHeap, def->closure_bitset);
        }
        return heap_size(hHeap, mem) + data_size;
    }
    }
}


static Janet cfun_local_heap_info(int32_t argc, Janet *argv)
{
    (void)argv;

    janet_fixarity(argc, 0);

    size_t weak_block_count = 0;
    size_t weak_disabled_block_count = 0;
    size_t weak_reachable_block_count = 0;
    size_t weak_heap_size = 0;
    size_t weak_heap_type_counts[__JANET_MEMORY_TYPE_COUNT] = { 0 };

    size_t block_count = 0;
    size_t disabled_block_count = 0;
    size_t reachable_block_count = 0;
    size_t heap_size = 0;
    size_t heap_type_counts[__JANET_MEMORY_TYPE_COUNT] = { 0 };

    size_t threaded_block_count = 0;
    size_t threaded_non_abstract_block_count = 0;
    size_t threaded_reachable_block_count = 0;
    size_t threaded_heap_size = 0;

    int add_to_heap_size = 1;

    HANDLE hHeap = GetProcessHeap();
    if (!hHeap) {
        janet_panic("failed to retrieve default heap handle");
    }

    JanetGCObject *current = janet_local_vm()->weak_blocks;
    while (NULL != current) {
        weak_block_count++;
        weak_heap_type_counts[janet_gc_type(current)]++;

        add_to_heap_size = 1;

        if (current->flags & JANET_MEM_REACHABLE) {
            weak_reachable_block_count++;
        }
        if (current->flags & JANET_MEM_DISABLED) {
            weak_disabled_block_count++;
            /* Currently only janet_malloc'ed buffers have this flag,
               and they SHOULD NOT appear in the heap */
            add_to_heap_size = 0;
        }

        if (add_to_heap_size) {
            weak_heap_size += calc_memory_size(current, hHeap);
        }

        current = current->data.next;
    }

    current = janet_local_vm()->blocks;
    while (NULL != current) {
        block_count++;
        heap_type_counts[janet_gc_type(current)]++;

        add_to_heap_size = 1;

        if (current->flags & JANET_MEM_REACHABLE) {
            reachable_block_count++;
        }
        if (current->flags & JANET_MEM_DISABLED) {
            disabled_block_count++;
            /* Currently only janet_malloc'ed buffers have this flag,
               and they SHOULD NOT appear in the heap */
            add_to_heap_size = 0;
        }

        if (add_to_heap_size) {
            heap_size += calc_memory_size(current, hHeap);
        }

        current = current->data.next;
    }

    JanetKV *items = janet_local_vm()->threaded_abstracts.data;
    for (int32_t i = 0; i < janet_local_vm()->threaded_abstracts.capacity; i++) {
        if (janet_checktype(items[i].key, JANET_NIL)) {
            /* Hit a tombstone */
            continue;
        }

        threaded_block_count++;

        add_to_heap_size = 1;

        if (!janet_checktype(items[i].key, JANET_ABSTRACT)) {
            threaded_non_abstract_block_count++;
            /* Currently there're only abstract types in the threaded heap */
            add_to_heap_size = 0;
        }
        if (janet_truthy(items[i].value)) {
            threaded_reachable_block_count++;
        }

        if (add_to_heap_size) {
            void *p = janet_unwrap_abstract(items[i].key);
            threaded_heap_size += calc_memory_size((JanetGCObject *)(janet_abstract_head(p)), hHeap);
        }
    }


    JanetTable *ret_table = janet_table(12);

    janet_table_put(ret_table, janet_ckeywordv("weak_block_count"), jw32_wrap_ulong_ptr(weak_block_count));
    janet_table_put(ret_table, janet_ckeywordv("weak_disabled_block_count"), jw32_wrap_ulong_ptr(weak_disabled_block_count));
    janet_table_put(ret_table, janet_ckeywordv("weak_reachable_block_count"), jw32_wrap_ulong_ptr(weak_reachable_block_count));
    janet_table_put(ret_table, janet_ckeywordv("weak_heap_size"), jw32_wrap_ulong_ptr(weak_heap_size));
    JanetTable *weak_heap_type_count_table = janet_table(__JANET_MEMORY_TYPE_COUNT);
    for (int i = 0; i < __JANET_MEMORY_TYPE_COUNT; i++) {
        janet_table_put(weak_heap_type_count_table, janet_ckeywordv(mem_type_names[i]), jw32_wrap_ulong_ptr(weak_heap_type_counts[i]));
    }
    janet_table_put(ret_table, janet_ckeywordv("weak_heap_type_counts"), janet_wrap_table(weak_heap_type_count_table));

    janet_table_put(ret_table, janet_ckeywordv("block_count"), jw32_wrap_ulong_ptr(block_count));
    janet_table_put(ret_table, janet_ckeywordv("disabled_block_count"), jw32_wrap_ulong_ptr(disabled_block_count));
    janet_table_put(ret_table, janet_ckeywordv("reachable_block_count"), jw32_wrap_ulong_ptr(reachable_block_count));
    janet_table_put(ret_table, janet_ckeywordv("heap_size"), jw32_wrap_ulong_ptr(heap_size));
    JanetTable *heap_type_count_table = janet_table(__JANET_MEMORY_TYPE_COUNT);
    for (int i = 0; i < __JANET_MEMORY_TYPE_COUNT; i++) {
        janet_table_put(heap_type_count_table, janet_ckeywordv(mem_type_names[i]), jw32_wrap_ulong_ptr(heap_type_counts[i]));
    }
    janet_table_put(ret_table, janet_ckeywordv("heap_type_counts"), janet_wrap_table(heap_type_count_table));

    janet_table_put(ret_table, janet_ckeywordv("threaded_block_count"), jw32_wrap_ulong_ptr(threaded_block_count));
    janet_table_put(ret_table, janet_ckeywordv("threaded_non_abstract_block_count"), jw32_wrap_ulong_ptr(threaded_non_abstract_block_count));
    janet_table_put(ret_table, janet_ckeywordv("threaded_reachable_block_count"), jw32_wrap_ulong_ptr(threaded_reachable_block_count));
    janet_table_put(ret_table, janet_ckeywordv("threaded_heap_size"), jw32_wrap_ulong_ptr(threaded_heap_size));

    janet_table_put(ret_table, janet_ckeywordv("gc_interval"), jw32_wrap_ulong_ptr(janet_local_vm()->gc_interval));
    janet_table_put(ret_table, janet_ckeywordv("next_collection"), jw32_wrap_ulong_ptr(janet_local_vm()->next_collection));
    janet_table_put(ret_table, janet_ckeywordv("root_count"), jw32_wrap_ulong_ptr(janet_local_vm()->root_count));

    return janet_wrap_table(ret_table);
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
        "(" MOD_NAME "/alloc-and-marshal x &opt rlookup-table)\n\n"
        "Marshals a Janet object into a newly allocated buffer, and returns a pointer pointing to the buffer. Accepts an optional reverse lookup table rlookup-table."
    },
    {
        "unmarshal-and-free",
        cfun_unmarshal_and_free,
        "(" MOD_NAME "/unmarshal-and-free ptr &opt lookup)\n\n"
        "Unmarshals a Janet object from the buffer pointed to by ptr, and then frees the buffer. Accepts an optional lookup table lookup-table."
    },
    {
        "alloc-console-and-reopen-streams",
        cfun_alloc_console_and_reopen_streams,
        "(" MOD_NAME "/alloc-console-and-reopen-streams)\n\n"
        "Opens a console and redirect stdin, stdout and stderr."
    },
    {
        "local-heap-info",
        cfun_local_heap_info,
        "(" MOD_NAME "/local-heap-info)\n\n"
        "Retrieves debug info for the thread-local heap."
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

#ifdef JW32_VCS_VERSION
    janet_def(env, "_jw32-vcs-version", janet_cstringv(JW32_VCS_VERSION),
              "The current JW32 version, from the VCS (fossil or git).");
#else
    janet_def(env, "_jw32-vcs-version", janet_wrap_nil(),
              "The current JW32 version, from the VCS (fossil or git).");
#endif
}
