#include "jw32.h"
#include "debug.h"

#include <heapapi.h>

#define MOD_NAME "heapapi"


static void define_consts_heap(JanetTable *env)
{
#define __def(const_name)                                       \
    janet_def(env, #const_name, jw32_wrap_dword(const_name),    \
              "Constant for heap flags.")

    __def(HEAP_NO_SERIALIZE);
    __def(HEAP_GROWABLE);
    __def(HEAP_GENERATE_EXCEPTIONS);
    __def(HEAP_ZERO_MEMORY);
    __def(HEAP_REALLOC_IN_PLACE_ONLY);
    __def(HEAP_TAIL_CHECKING_ENABLED);
    __def(HEAP_FREE_CHECKING_ENABLED);
    __def(HEAP_DISABLE_COALESCE_ON_FREE);
    __def(HEAP_CREATE_ALIGN_16);
    __def(HEAP_CREATE_ENABLE_TRACING);
    __def(HEAP_CREATE_ENABLE_EXECUTE);
    __def(HEAP_MAXIMUM_TAG);
    __def(HEAP_PSEUDO_TAG_FLAG);
    __def(HEAP_TAG_SHIFT);
    __def(HEAP_CREATE_SEGMENT_HEAP);
    __def(HEAP_CREATE_HARDENED);

#undef __def
}


static Janet cfun_GetProcessHeap(int32_t argc, Janet *argv)
{
    (void)argc;
    (void)argv;

    janet_fixarity(argc, 0);
    return jw32_wrap_handle(GetProcessHeap());
}


static Janet cfun_GetProcessHeaps(int32_t argc, Janet *argv)
{
    (void)argc;
    (void)argv;

    DWORD dwCount;
    DWORD dwHeapCount;
    PHANDLE aHeaps;
    SIZE_T sBufSize;

    JanetArray *heap_arr;

    dwCount = GetProcessHeaps(0, NULL);
    if (dwCount <= 0) {
        return janet_wrap_nil();
    }

    sBufSize = dwCount * sizeof(*aHeaps);
    aHeaps = (PHANDLE)janet_malloc(sBufSize);

    if (!aHeaps) {
        janet_panic("failed to allocate memory for heaps array");
    }

    dwHeapCount = dwCount;
    dwCount = GetProcessHeaps(dwHeapCount, aHeaps);
    if (dwCount <= 0) {
        janet_free(aHeaps);
        return janet_wrap_nil();
    }

    heap_arr = janet_array(dwCount);
    for (SIZE_T i = 0; i < dwCount; i++) {
        janet_array_push(heap_arr, jw32_wrap_handle(aHeaps[i]));
    }

    janet_free(aHeaps);
    return janet_wrap_array(heap_arr);
}


static Janet cfun_HeapAlloc(int32_t argc, Janet *argv)
{
    HANDLE hHeap;
    DWORD dwFlags;
    SIZE_T dwBytes;

    LPVOID pRet;

    janet_fixarity(argc, 3);

    hHeap = jw32_get_handle(argv, 0);
    dwFlags = jw32_get_dword(argv, 1);
    dwBytes = jw32_get_ulong_ptr(argv, 2);

    pRet = HeapAlloc(hHeap, dwFlags, dwBytes);
    if (pRet) {
        return jw32_wrap_lpvoid(pRet);
    } else {
        return janet_wrap_nil();
    }
}


static Janet cfun_HeapReAlloc(int32_t argc, Janet *argv)
{
    HANDLE hHeap;
    DWORD dwFlags;
    LPVOID lpMem;
    SIZE_T dwBytes;

    LPVOID pRet;

    janet_fixarity(argc, 4);

    hHeap = jw32_get_handle(argv, 0);
    dwFlags = jw32_get_dword(argv, 1);
    lpMem = jw32_get_lpvoid(argv, 2);
    dwBytes = jw32_get_ulong_ptr(argv, 3);

    pRet = HeapReAlloc(hHeap, dwFlags, lpMem, dwBytes);
    if (pRet) {
        return jw32_wrap_lpvoid(pRet);
    } else {
        return janet_wrap_nil();
    }
}


static Janet cfun_HeapFree(int32_t argc, Janet *argv)
{
    HANDLE hHeap;
    DWORD dwFlags;
    LPVOID lpMem;

    BOOL bRet;

    janet_fixarity(argc, 3);

    hHeap = jw32_get_handle(argv, 0);
    dwFlags = jw32_get_dword(argv, 1);
    lpMem = jw32_get_lpvoid(argv, 2);

    bRet = HeapFree(hHeap, dwFlags, lpMem);
    return jw32_wrap_bool(bRet);
}


static const JanetReg cfuns[] = {
    {
        "GetProcessHeap",
        cfun_GetProcessHeap,
        "(" MOD_NAME "/GetProcessHeap)\n\n"
        "Retrieves a handle to the default heap of the calling process."
    },
    {
        "GetProcessHeaps",
        cfun_GetProcessHeaps,
        "(" MOD_NAME "/GetProcessHeaps)\n\n"
        "Retrieves handles to all of the active heaps for the calling process."
    },
    {
        "HeapAlloc",
        cfun_HeapAlloc,
        "(" MOD_NAME "/HeapAlloc hHeap dwFlags dwBytes)\n\n"
        "Allocates a block of memory from a heap."
    },
    {
        "HeapReAlloc",
        cfun_HeapReAlloc,
        "(" MOD_NAME "/HeapReAlloc hHeap dwFlags lpMem dwBytes)\n\n"
        "Reallocates a block of memory from a heap."
    },
    {
        "HeapFree",
        cfun_HeapFree,
        "(" MOD_NAME "/HeapFree hHeap dwFlags lpMem)\n\n"
        "Frees a memory block allocated from a heap."
    },
    {NULL, NULL, NULL},
};


JANET_MODULE_ENTRY(JanetTable *env)
{
    define_consts_heap(env);

    janet_cfuns(env, MOD_NAME, cfuns);
}
