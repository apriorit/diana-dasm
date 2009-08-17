#include "diana_patchers.h"
#include "diana_streams.h"


int Diana_GetCmdSizeToMove(const DianaParserResult * pResult, size_t cmdSize, size_t * pNewSize)
{
    int i = 0;
    // get group info
    DianaGroupInfo * pGroupInfo = Diana_GetGroupInfo(pResult->pInfo->m_lGroupId);
    if (!pGroupInfo->m_pLinkedInfo)
    {
        *pNewSize = cmdSize;
        return DI_SUCCESS;
    }
    if (!pGroupInfo->m_pLinkedInfo->canChangeCSIP)
    {
        *pNewSize = cmdSize;
        return DI_SUCCESS;
    }

    if (pResult->iLinkedOpCount == 1)
    {
        const DianaLinkedOperand * pOp = pResult->linkedOperands + i;
        if (pOp->type == diana_rel)
        {
            *pNewSize = 5;
            if (pGroupInfo->m_pLinkedInfo->isJump)
            {
                //todo: if CONDITIONAL
                *pNewSize = 6;
            }
        }
        return DI_SUCCESS;
    }
    return DI_ERROR;
}

int Diana_CalculateSizeToMove(void * pPlaceToHook, 
                              size_t summSize,
                              size_t * pSizeAfterMove)
{
    int status = DI_ERROR;
    char * pBegin = (char * )pPlaceToHook;
    size_t cmdSize = 0;
    long tailSize = summSize;
    size_t resultSize = 0;

    *pSizeAfterMove = 0;

    for(;tailSize>0; pBegin+=cmdSize, tailSize-= cmdSize)
    {
        DianaParserResult result;
        
        status = Diana_ParseCmdOnBuffer(DIANA_MODE32,
                                        pBegin,
                                         tailSize,
                                         Diana_GetRootLine(),  // IN
                                         &result,  //OUT
                                         &cmdSize);    // OUT
        if (status)
           return status;

        {
            size_t newCmnSize;
            status = Diana_GetCmdSizeToMove(&result, cmdSize, &newCmnSize);
            if (status)
                return status;
    
            resultSize += newCmnSize;
        }
    }
    *pSizeAfterMove = resultSize;
    return status;
}

int
Diana_MoveSequence(void * pDestination, 
                   void * pSource,
                   size_t summSize,
                   size_t * pResSize)
{
    int status = DI_ERROR;
    char * pBegin = (char * )pSource;
    size_t cmdSize = 0;
    size_t realCmdSize = 0;
    
    size_t targetCmdOffset = 0;
    long tailSize = summSize;

    for(;tailSize>0; pBegin+=cmdSize, tailSize-= cmdSize, targetCmdOffset += realCmdSize)
    {
        int iNeedToCopy = 1;
        DianaParserResult result;
        DianaGroupInfo * pGroupInfo = 0;
        
        status = Diana_ParseCmdOnBuffer(DIANA_MODE32,
                                        pBegin,
                                         tailSize,
                                         Diana_GetRootLine(),  // IN
                                         &result,  //OUT
                                         &cmdSize);    // OUT
        if (status)
           return status;


        // analyze 
        pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId);
        if (pGroupInfo->m_pLinkedInfo &&  pGroupInfo->m_pLinkedInfo->canChangeCSIP)
        {
            if (result.iLinkedOpCount ==1 && result.linkedOperands[0].type == diana_rel)
            {
                // relative, need correction
                const DianaLinkedOperand * pOp = result.linkedOperands;
                switch(pOp->value.rel.m_size)
                {
                    case 1:
                        {
                            // find another command
                            return DI_ERROR;
                        }
                    case 2:
                        {
                            //if (result.pInfo->m_bHas32BitAnalog)
                            return DI_ERROR;
                        }
                    case 4:
                        realCmdSize = cmdSize;
                        memcpy((char*)pDestination+targetCmdOffset, pBegin, cmdSize);
                        
                        {
                            unsigned long * pChangedData   = (unsigned long *)((char*)pDestination + targetCmdOffset+ pOp->iOffset);
                            *pChangedData = *pChangedData - ((unsigned long)pChangedData-1)+ (unsigned long)pBegin;
                        }
                        iNeedToCopy = 0;
                        break;
                    default:
                        return DI_ERROR;
                }
            }
        }

        if (iNeedToCopy) // normal copy
        {
            realCmdSize = cmdSize;
            memcpy((char*)pDestination+targetCmdOffset, pBegin, cmdSize);
        }
    }

    *pResSize = targetCmdOffset;
    return status;
}
                   
static int PatchSequence32(void * pPlaceToHook, 
                         size_t summSize,
                         Diana_PatchHandlerFunction_type pPatchFnc,
                         void * pPatchContext,
                         Diana_Allocator * pAllocator)
{
    // alloc stub: summSize + 5
    void * pStub = 0;
    int status = DI_ERROR;
    size_t codeSizeAfterMove = 0;
    size_t stubSize = 0;
    size_t jumpOffset = 0;
    unsigned long * pFunctionArg = 0;
    
    status = Diana_CalculateSizeToMove(pPlaceToHook, summSize, &codeSizeAfterMove);
    if (status)
        goto err;


    // CREATE STUB
    /*
    push ESP
    push context
    call stdcall_function
    */

    stubSize = codeSizeAfterMove 
                            + 6 // push size
                            + 5 // call stub
                            + 5 // jmp res
                            ;

    pStub = (*pAllocator->m_alloc)(pAllocator, stubSize);
    if (!pStub)
        goto err;

    // set begin and end
    *(unsigned char*)pStub = 0x54;
    *((unsigned char*)pStub+1) = 0x68;
    memcpy((unsigned char*)pStub+2, &pPatchContext, 4);
    
    *((unsigned char*)pStub+6) = 0xE8;
    pFunctionArg = (unsigned long*)((unsigned char*)pStub+7);
    *pFunctionArg = (unsigned long)pPatchFnc - (unsigned long)pFunctionArg-4;

    status = Diana_MoveSequence((unsigned char*)pStub+stubSize-codeSizeAfterMove-5, pPlaceToHook, summSize, &jumpOffset);
    if (status)
        goto err;

    // put jump
    jumpOffset += stubSize-codeSizeAfterMove-5;

    *((unsigned char*)pStub+jumpOffset) = 0xE9;
    pFunctionArg = (unsigned long*)((unsigned char*)pStub+jumpOffset+1);
    *pFunctionArg = ((unsigned long)pPlaceToHook + summSize) - (unsigned long)pFunctionArg-4;
    
    // stub is done - set hook!
    {
        char buf[5] = {0xE9, 0,0,0,0};
        pFunctionArg = (unsigned long *)(buf+1);
        *pFunctionArg = (unsigned long)pStub - (unsigned long)pPlaceToHook-5;

        status = pAllocator->m_patch(pAllocator, pPlaceToHook, buf, 5);
        if (status)
            goto err;
    }
    return DI_SUCCESS;

err:
    if (pStub)
        (*pAllocator->m_free)(pAllocator, pStub);
    return status;
}

int Diana_PatchSomething32(void * pPlaceToHook,
                           size_t size,
                           Diana_PatchHandlerFunction_type pPatchFnc,
                           void * pPatchContext,
                           Diana_Allocator * pAllocator)
{
    DianaParserResult result;
    size_t cmdSize = 0;
    size_t cmdOffset = 0;
    size_t summSize = 0;
    int iRes = DI_ERROR;

    while(cmdOffset<size)
    {
        iRes = Diana_ParseCmdOnBuffer(DIANA_MODE32,
                                          (char*)pPlaceToHook+cmdOffset,
                                          size-summSize,
                                          Diana_GetRootLine(),  // IN
                                          &result,  //OUT
                                          &cmdSize);    // OUT
        if (iRes)
            return iRes;

        summSize += cmdSize;

        // logic:
        if (summSize >= 5)
            return PatchSequence32(pPlaceToHook, summSize, pPatchFnc, pPatchContext, pAllocator);

        // next:
        cmdOffset = summSize;
    }
    return iRes;
}