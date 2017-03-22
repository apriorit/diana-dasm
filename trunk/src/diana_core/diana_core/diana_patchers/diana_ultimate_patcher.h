#ifndef DIANA_ULTIMATE_PATCHER_H
#define DIANA_ULTIMATE_PATCHER_H

#include "diana_core.h"

typedef int ( * DianaHook_Alloc_type)(void * pThis, 
                                      DIANA_SIZE_T size, 
                                      OPERAND_SIZE * pAddress,
                                      const OPERAND_SIZE * pHintAddress);
typedef void ( * DianaHook_Free_type)(void * pThis, 
                                      const OPERAND_SIZE * pAddress);
typedef int ( *DianaHook_Patcher_type)(void * pThis, 
                                       const OPERAND_SIZE * pDest, 
                                       const OPERAND_SIZE * pSource, 
                                       OPERAND_SIZE sizeInBytes);

typedef struct _DianaHook_Allocator
{
    DianaHook_Alloc_type alloc;
    DianaHook_Alloc_type free;
    DianaHook_Patcher_type patch;
}DianaHook_Allocator;

void DianaHook_Allocator_Init(DianaHook_Allocator * pDianaHookAllocator,
                              DianaHook_Alloc_type alloc,
                              DianaHook_Alloc_type free,
                              DianaHook_Patcher_type patch);

int DianaHook_PatchStream(DianaReadWriteRandomStream * pReadWriteStream,
                          DianaHook_Allocator * pAllocator,
                          int processorMode);

#endif