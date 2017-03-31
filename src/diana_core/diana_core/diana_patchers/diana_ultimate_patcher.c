#include "diana_ultimate_patcher.h"
#include "diana_analyze.h"
#include "diana_ultimate_patcher_impl.h"

int DianaHook_PatchStream32(DianaHook_InternalMessage * pMessage);
int DianaHook_PatchStream64(DianaHook_InternalMessage * pMessage);


void DianaHook_Allocator_Init(DianaHook_Allocator * pDianaHookAllocator,
                              DianaHook_Alloc_type alloc,
                              DianaHook_Free_type free)
{
    pDianaHookAllocator->alloc = alloc;
    pDianaHookAllocator->free = free;
}

int DianaHook_PatchStream(DianaHook_TargetMemoryProvider * pTargetMemoryProvider,
                          int processorMode,
                          OPERAND_SIZE addressToHook,
                          DianaHook_CustomOptions * pCustomOptions)
{
    int result = 0;
    DianaHook_InternalMessage * pMessage = 0;

    pMessage = (DianaHook_InternalMessage *)DIANA_MALLOC(sizeof(DianaHook_InternalMessage));
    DI_CHECK_ALLOC(pMessage);

    DIANA_MEMSET(pMessage, 0, sizeof(DianaHook_InternalMessage));
    pMessage->pReadWriteStream = pTargetMemoryProvider->pReadWriteStream;
    pMessage->pAllocator = pTargetMemoryProvider->pAllocator;
    pMessage->processorMode = processorMode;
    pMessage->pCustomOptions = pCustomOptions;
    pMessage->addressToHook = addressToHook;
    switch (processorMode)
    {
    case DIANA_MODE32:
        result = DianaHook_PatchStream32(pMessage);
        break;
    case DIANA_MODE64:
        result = DianaHook_PatchStream64(pMessage);
        break;
    case DIANA_MODE16:
        result = DI_ERROR_NOT_IMPLEMENTED;
        break;
    default:
        result = DI_ERROR;
    }
    if (pMessage)
    {
        DIANA_FREE(pMessage);
    }
    return result;
}