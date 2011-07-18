#include "diana_analyze.h"
#include "stdlib.h"
#include "diana_stack.h"

void DianaAnalyzeReadStream_Init(DianaAnalyzeReadStream * pThis,
                                 DianaRead_fnc pReadFnc,
                                 DianaAnalyzeMoveTo_fnc pMoveFnc)
{
    pThis->m_parent.pReadFnc = pReadFnc;
    pThis->m_pMoveTo = pMoveFnc;
}

void Diana_Instruction_Init(Diana_Instruction * pInstruction,
                            OPERAND_SIZE offset,
                            int flags)
{
    memset(pInstruction, 0, sizeof(*pInstruction));
    
    pInstruction->m_offset = offset;
    pInstruction->m_flags = flags;
    Diana_InitList(&pInstruction->m_refsTo);
    Diana_InitList(&pInstruction->m_refsFrom);

}
int Diana_InstructionsOwner_Init(Diana_InstructionsOwner * pOwner,
                                 OPERAND_SIZE maxOffsetSize)
{
    int res = 0;
    memset(pOwner, 0, sizeof(*pOwner));

    pOwner->m_pInstructionsVec = malloc(sizeof(Diana_Instruction) * (size_t)maxOffsetSize);
    if (!pOwner->m_pInstructionsVec)
        return DI_OUT_OF_MEMORY;

    pOwner->m_ppPresenceVec = (Diana_Instruction ** )malloc(sizeof(Diana_Instruction*) * (size_t)maxOffsetSize);
    if (!pOwner->m_ppPresenceVec)
    {
        Diana_InstructionsOwner_Free(pOwner);
        return DI_OUT_OF_MEMORY;
    }
    memset(pOwner->m_ppPresenceVec, 0, (size_t)maxOffsetSize * sizeof(Diana_Instruction*));
    
    res = Diana_Stack_Init(&pOwner->m_xrefs, 1024, sizeof(Diana_XRef));
    if (res != DI_SUCCESS)
    {
        Diana_InstructionsOwner_Free(pOwner);
        return res;
    }
    pOwner->m_stackInited = 1;
    pOwner->m_maxSize = maxOffsetSize;
    pOwner->m_usedSize = 0; 
    return DI_SUCCESS;
}
void Diana_InstructionsOwner_Free(Diana_InstructionsOwner * pOwner)
{
    if (pOwner->m_pInstructionsVec)
    {
        free(pOwner->m_pInstructionsVec);
    }
    if (pOwner->m_ppPresenceVec)
    {
        free(pOwner->m_ppPresenceVec);
    }
    if (pOwner->m_stackInited)
    {
        Diana_Stack_Free(&pOwner->m_xrefs);
    }
}
Diana_Instruction * Diana_InstructionsOwner_AllocateInstruction(Diana_InstructionsOwner * pOwner)
{
    Diana_Instruction * pEntry = pOwner->m_pInstructionsVec + pOwner->m_usedSize;
    if (pOwner->m_usedSize >= pOwner->m_maxSize)
        return 0;

    ++pOwner->m_usedSize;
    return pEntry;
}

int Diana_InstructionsOwner_QueryOrAllocateInstruction(Diana_InstructionsOwner * pOwner,
                                                       OPERAND_SIZE address,
                                                       Diana_Instruction ** ppInstruction,
                                                       int flags)
{
    *ppInstruction = pOwner->m_ppPresenceVec[address];
    if (!*ppInstruction)
    {
        *ppInstruction = Diana_InstructionsOwner_AllocateInstruction(pOwner);
        if (!*ppInstruction)
            return DI_OUT_OF_MEMORY;

        Diana_Instruction_Init(*ppInstruction, 
                               address,
                               flags);
        pOwner->m_ppPresenceVec[address] = *ppInstruction;
    }
    return DI_SUCCESS;
}

static int Diana_CreateXRef(Diana_InstructionsOwner * pOwner,
                            Diana_Instruction * pInstruction,
                            Diana_XRef ** ppXref)
{
    Diana_XRef xref;
    memset(&xref, 0, sizeof(xref));
    xref.m_pInstruction = pInstruction;
    DI_CHECK(Diana_Stack_Push(&pOwner->m_xrefs, &xref));
    *ppXref = (Diana_XRef *)Diana_Stack_GetTopPtr(&pOwner->m_xrefs);
    return DI_SUCCESS;
}

static
int AcquireNewOffset(DianaParserResult * pResult,
                     Diana_LinkedAdditionalGroupInfo * pLinkedInfo,
                     OPERAND_SIZE * pNewOffset,
                     OPERAND_SIZE offset)
{
    const DianaLinkedOperand * pOp = pResult->linkedOperands + pLinkedInfo->relArgrumentNumber;
    *pNewOffset = 0;

    switch(pOp->type)
    {
    case diana_rel:
        {
            *pNewOffset = offset + pOp->value.rel.m_value;
            break;
        }
    case diana_call_ptr:
    case diana_index:
    case diana_register:
        return DI_UNSUPPORTED_COMMAND;
    }
    return DI_SUCCESS;
}

static
int Diana_AnalyzeCodeImpl(Diana_InstructionsOwner * pOwner,
                      DianaAnalyzeReadStream * pStream,
                      int mode,
                      OPERAND_SIZE initialOffset,
                      OPERAND_SIZE maxOffset,
                      Diana_Stack * pStack
                      )
{
    OPERAND_SIZE offset = initialOffset;
    DianaGroupInfo * pGroupInfo=0;
    DianaParserResult result;
    Diana_Instruction * pInstruction = 0;

    int iRes = 0;
    OPERAND_SIZE newOffset = 0, prevOffset = 0;

    DianaContext context;
    int bNeedReset = 0;
    Diana_InitContext(&context, mode);

    while(1)
    {
        Diana_LinkedAdditionalGroupInfo * pLinkedInfo = 0;

        if (offset > maxOffset || bNeedReset)
        {
            if (!pStack->m_count)
                break;
            DI_CHECK(Diana_Stack_Pop(pStack, &offset));
            DI_CHECK(pStream->m_pMoveTo(pStream, offset));
            Diana_ClearCache(&context);
        }
        bNeedReset = 0;

        prevOffset = offset;
        iRes = Diana_ParseCmd(&context,
                               Diana_GetRootLine(),
                               &pStream->m_parent,
                               &result);

        offset += result.iFullCmdSize;
        if (iRes == DI_END)
            break;

        if (iRes)
        {
            // instruction not recognized properly
            if (!result.iFullCmdSize)
            {
                Diana_CacheEatOneSafe(&context);
            }
            Diana_DebugFatalBreak();
            continue;
        }

        // check if instruction is always registered
        DI_CHECK(Diana_InstructionsOwner_QueryOrAllocateInstruction(pOwner, 
                                                                    prevOffset, 
                                                                    &pInstruction,
                                                                    0)); // flags for new instruction

        // clear DI_INSTRUCTION_IS_LOADING 
        pInstruction->m_flags = 0;
        
        pLinkedInfo = result.pInfo->m_pGroupInfo->m_pLinkedInfo;
        if (!pLinkedInfo)
            continue;

        if (!(pLinkedInfo->flags & DIANA_GT_CAN_CHANGE_RIP))
            continue;

        // special instruction found: jmps, jcc's, call, ret, leave
        pInstruction->m_flags |= DI_INSTRUCTION_CAN_CHANGE_CSIP;

        // handle ret case
        if (pLinkedInfo->flags & DIANA_GT_RET)
        {
            pInstruction->m_flags |= DI_INSTRUCTION_IS_RET;
            bNeedReset = 1;
            continue;
        }

        // handle jumps and others
        newOffset = 0;
        iRes = AcquireNewOffset(&result, 
                                  pLinkedInfo, 
                                  &newOffset,
                                  offset);
        if (iRes == DI_UNSUPPORTED_COMMAND)
            continue;

        if (!(pLinkedInfo->flags & DIANA_GT_CAN_GO_TO_THE_NEXT_INSTRUCTION))
        {
            bNeedReset = 1;
        }

        DI_CHECK(iRes);
        {
            Diana_Instruction * pTargetInstruction = 0;
            Diana_Instruction * pOldTargetInstruction = 0;
            Diana_XRef * pXref = 0;

            if (newOffset > maxOffset)
            {
                Diana_DebugFatalBreak();
                continue;
            }
            // create new instruction
            pOldTargetInstruction = pOwner->m_ppPresenceVec[newOffset];

            DI_CHECK(Diana_InstructionsOwner_QueryOrAllocateInstruction(pOwner, 
                                                                        newOffset, 
                                                                        &pTargetInstruction,
                                                                        DI_INSTRUCTION_IS_LOADING)); 

            // register xref-from Instruction to Target
            DI_CHECK(Diana_CreateXRef(pOwner, pTargetInstruction, &pXref));
            Diana_PushBack(&pInstruction->m_refsFrom, &pXref->m_instructionEntry);

            // register xref: to Target from Instruction
            DI_CHECK(Diana_CreateXRef(pOwner, pInstruction, &pXref));
            Diana_PushBack(&pTargetInstruction->m_refsTo, &pXref->m_instructionEntry);

            // do not push new route to stack twice
            if (pOldTargetInstruction)
                continue;

            // push to stack
            DI_CHECK(Diana_Stack_Push(pStack, &newOffset));

        }
    }
    return DI_SUCCESS;
}

int Diana_AnalyzeCode(Diana_InstructionsOwner * pOwner,
                      DianaAnalyzeReadStream * pStream,
                      int mode,
                      OPERAND_SIZE initialOffset,
                      OPERAND_SIZE maxOffset)
{
    int res = 0;
    Diana_Stack stack;
    
    DI_CHECK(Diana_Stack_Init(&stack, 4096, sizeof(OPERAND_SIZE)));

    res = Diana_AnalyzeCodeImpl(pOwner,
                                pStream,
                                mode,
                                initialOffset,
                                maxOffset,
                                &stack);
    Diana_Stack_Free(&stack);
    return res;
}

