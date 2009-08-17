#ifndef DIANA_PATCHERS_H
#define DIANA_PATCHERS_H

#include "diana_core.h"

typedef void (__stdcall *Diana_PatchHandlerFunction_type)(void * pContext, void * pOriginalESP);

#define DIANA_RETURN(sizeN, x) {\
    __asm mov eax, x \
    __asm mov esp, ebp \
    __asm pop ebp \
    __asm add esp, 0xC \
    __asm ret sizeN }

#define DIANA_ARG(espVal, num, type)  (*((type*)espVal+num))
#define DIANA_RET(espVal)  (*((unsigned long*)espVal))


int Diana_PatchSomething32(void * pPlaceToHook,
                           size_t size,
                           Diana_PatchHandlerFunction_type pPatchFnc,
                           void * pPatchContext,
                           Diana_Allocator * pAllocator);

#endif