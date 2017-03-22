#include "diana_ultimate_patcher.h"
#include "diana_analyze.h"


void DianaHook_Allocator_Init(DianaHook_Allocator * pDianaHookAllocator,
                              DianaHook_Alloc_type alloc,
                              DianaHook_Alloc_type free,
                              DianaHook_Patcher_type patch)
{
    pDianaHookAllocator->alloc = alloc;
    pDianaHookAllocator->free = free;
    pDianaHookAllocator->patch = patch;
}

int DianaHook_PatchStream(DianaReadWriteRandomStream * pReadWriteStream,
                          DianaHook_Allocator * pAllocator,
                          int processorMode)
{
    &pReadWriteStream;
    &pAllocator;
    &processorMode;
    return DI_ERROR_NOT_IMPLEMENTED;
}