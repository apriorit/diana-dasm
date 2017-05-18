#ifndef DIANA_ULTIMATE_PATCHER_H
#define DIANA_ULTIMATE_PATCHER_H

#include "diana_core.h"

#define DIANA_HOOK_FLASG_ALLOCATE_EXECUTABLE              1

typedef int ( * DianaHook_Alloc_type)(void * pThis, 
                                      OPERAND_SIZE size, 
                                      OPERAND_SIZE * pAddress,
                                      const OPERAND_SIZE * pHintAddress,
                                      int flags);
typedef void ( * DianaHook_Free_type)(void * pThis, 
                                      const OPERAND_SIZE * pAddress);

typedef struct _DianaHook_Allocator
{
    DianaHook_Alloc_type alloc;
    DianaHook_Free_type free;
}DianaHook_Allocator;

void DianaHook_Allocator_Init(DianaHook_Allocator * pDianaHookAllocator,
                              DianaHook_Alloc_type alloc,
                              DianaHook_Free_type free);

typedef struct _DianaHook_TargetMemoryProvider
{
    DianaReadWriteRandomStream * pReadWriteStream;
    DianaHook_Allocator * pAllocator;
}DianaHook_TargetMemoryProvider;

void DianaHook_TargetMemoryProvider_Init(DianaHook_TargetMemoryProvider * pDianaHookProvider,
                                         DianaReadWriteRandomStream * pReadWriteStream,
                                         DianaHook_Allocator * pAllocator);

typedef struct _DianaHook_CustomOptions
{
    OPERAND_SIZE originalFunctionPointer;
}DianaHook_CustomOptions;

int DianaHook_PatchStream(DianaHook_TargetMemoryProvider * pTargetMemoryProvider,
                          int processorMode,
                          OPERAND_SIZE addressToHook,
                          OPERAND_SIZE hookFunction,
                          DianaHook_CustomOptions * pCustomOptions);

#endif