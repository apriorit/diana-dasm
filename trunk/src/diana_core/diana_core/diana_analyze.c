#include "diana_analyze.h"
#include "stdlib.h"
#include "diana_stack.h"

typedef struct _DianaAnalyzeSession
{
    Diana_InstructionsOwner * pOwner;
    DianaAnalyzeObserver * pObserver;
    int mode;

    Diana_Stack stack;
    DianaContext context;
    DianaParserResult result;
    OPERAND_SIZE maxOffset;
}DianaAnalyzeSession;

void DianaAnalyzeObserver_Init(DianaAnalyzeObserver * pThis,
                                 DianaRead_fnc pReadFnc,
                                 DianaAnalyzeMoveTo_fnc pMoveFnc,
                                 ConvertAddressToRelative_fnc pConvertAddress,
                                 AddSuspectedDataAddress_fnc pSuspectedDataAddress)
{
    pThis->m_stream.pReadFnc = pReadFnc;
    pThis->m_pMoveTo = pMoveFnc;
    pThis->m_pConvertAddress = pConvertAddress;
    pThis->m_pSuspectedDataAddress = pSuspectedDataAddress;
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
int AnalyzeJumps(DianaParserResult * pResult,
                 Diana_LinkedAdditionalGroupInfo * pLinkedInfo,
                 OPERAND_SIZE * pNewOffset,
                 OPERAND_SIZE offset,
                 DianaAnalyzeObserver * pObserver)
{
    const DianaLinkedOperand * pOp = pResult->linkedOperands + pLinkedInfo->relArgrumentNumber;
    *pNewOffset = 0;

    switch(pOp->type)
    {
        // usual call smth
    case diana_rel:
        {
            *pNewOffset = offset + pOp->value.rel.m_value;
            break;
        }
        // crazy call for really crazy guys
    case diana_call_ptr:
        {
            *pNewOffset = pOp->value.ptr.m_address;
            break;
        }

        // call [rbx+0x5435345] 
    case diana_index:
        {
            if (pOp->value.rmIndex.dispSize == 4)
            {
                // save it, it is interesting
                DI_CHECK(pObserver->m_pSuspectedDataAddress(pObserver, 
                                                          pOp->value.rmIndex.dispValue));

            }
            break;
        }
    case diana_register:
        return DI_UNSUPPORTED_COMMAND;
    }
    return DI_SUCCESS;
}

static
int SaveNewRoute(DianaAnalyzeSession * pSession,
                 Diana_Instruction * pInstruction,
                 OPERAND_SIZE newOffset,
                 int newRouteFlags)
{
    Diana_Instruction * pTargetInstruction = 0;
    Diana_Instruction * pOldTargetInstruction = 0;
    Diana_XRef * pXref = 0;

    Diana_RouteInfo curRouteInfo;

    if (newOffset > pSession->maxOffset)
        return DI_SUCCESS;

    curRouteInfo.startOffset = newOffset;
    curRouteInfo.flags = newRouteFlags;

    // create new instruction
    pOldTargetInstruction = pSession->pOwner->m_ppPresenceVec[newOffset];

    DI_CHECK(Diana_InstructionsOwner_QueryOrAllocateInstruction(pSession->pOwner, 
                                                                newOffset, 
                                                                &pTargetInstruction,
                                                                DI_INSTRUCTION_IS_LOADING)); 

    // register xref-from Instruction to Target
    DI_CHECK(Diana_CreateXRef(pSession->pOwner, pTargetInstruction, &pXref));
    Diana_PushBack(&pInstruction->m_refsFrom, &pXref->m_instructionEntry);

    // register xref: to Target from Instruction
    DI_CHECK(Diana_CreateXRef(pSession->pOwner, pInstruction, &pXref));
    Diana_PushBack(&pTargetInstruction->m_refsTo, &pXref->m_instructionEntry);

    // do not push new route to stack twice
    if (pOldTargetInstruction)
        return DI_SUCCESS;

    // push to stack
    DI_CHECK(Diana_Stack_Push(&pSession->stack, &newOffset));
    return DI_SUCCESS;
}

static
void DispatchMode32(DianaAnalyzeSession * pSession,
                    Diana_Instruction * pInstruction,
                    const DianaLinkedOperand * pOp,
                    int * pbFound,
                    OPERAND_SIZE * pSuspectedOp)
{
    *pbFound = 0;

    switch(pOp->type)
    {
    case diana_imm:
        {
            *pbFound = 1;
            *pSuspectedOp = pOp->value.imm;
            break;
        }
    case diana_index:
        {
            if (pOp->value.rmIndex.dispSize == 4 ||
                pOp->value.rmIndex.dispSize ==  8)
            {
                *pbFound = 1;
                *pSuspectedOp = pOp->value.rmIndex.dispValue;
            }
            break;
        }
    default:;
    } 
}

static
int DispatchMode64(DianaAnalyzeSession * pSession,
                    Diana_Instruction * pInstruction,
                    const DianaLinkedOperand * pOp,
                    OPERAND_SIZE nextInstructionOffset,
                    int * pbFound,
                    OPERAND_SIZE * pSuspectedOp)
{
    switch(pOp->type)
    {
    case diana_register:
        {
            if (pOp->value.recognizedRegister == reg_RIP || 
                pOp->value.recognizedRegister == reg_IP)
            {
                pInstruction->m_flags |= DI_INSTRUCTION_USES_RIP;
            }
            break;
        }
    case diana_imm:
        {
            *pbFound = 1;
            *pSuspectedOp = pOp->value.imm;
            break;
        }
    case diana_index:
        {
            if (pOp->value.rmIndex.reg == reg_RIP ||
                pOp->value.rmIndex.indexed_reg == reg_RIP)
            {
                // ooops 
                if ((pOp->value.rmIndex.indexed_reg == reg_none ||
                    pOp->value.rmIndex.index == 0) ||
                    (pOp->value.rmIndex.reg == reg_none &&
                     pOp->value.rmIndex.index == 1))
                {
                    // good case, no index
                    DI_CHECK(SaveNewRoute(pSession, 
                                          pInstruction,
                                          nextInstructionOffset + pOp->value.rmIndex.dispValue,
                                          DI_ROUTE_QUESTIONABLE)); 

                    pInstruction->m_flags |= DI_INSTRUCTION_USES_RIP;
                    break;
                }
                
                // bad case
                pInstruction->m_flags |= DI_INSTRUCTION_USES_UNKNOWN_RIP;
                break;
            }

            // common check
            if (pOp->value.rmIndex.dispSize == 4 ||
                pOp->value.rmIndex.dispSize ==  8)
            {
                *pbFound = 1;
                *pSuspectedOp = pOp->value.rmIndex.dispValue;
            }
            break;
        }
    default:;
    } 
    return DI_SUCCESS;
}        

static
int AnalyzeAndConstructNormalInstruction(DianaAnalyzeSession * pSession,
                                         Diana_Instruction * pInstruction,
                                         OPERAND_SIZE nextInstructionOffset)
{
    const DianaLinkedOperand * pOp = pSession->result.linkedOperands;
    int i = 0;

    for(i = 0; i < pSession->result.iLinkedOpCount; ++i, ++pOp)
    {
        OPERAND_SIZE suspectedOp = 0;
        int bFound = 0;

        if (pSession->mode == DIANA_MODE32)
        {
            DispatchMode32(pSession,
                           pInstruction,
                           pOp,
                           &bFound,
                           &suspectedOp);
        }
        else
        {
            DI_CHECK(DispatchMode64(pSession,
                                    pInstruction,
                                    pOp,
                                    nextInstructionOffset,
                                    &bFound,
                                    &suspectedOp));
        }

        if (bFound)
        {
            OPERAND_SIZE newOffset = 0;
            int bInvalidPointer = 0;
            DI_CHECK(pSession->pObserver->m_pConvertAddress(pSession->pObserver, 
                                                  suspectedOp,
                                                  &newOffset,
                                                  &bInvalidPointer));
            if (!bInvalidPointer)
            {
                // this is not realy valid command
                DI_CHECK(SaveNewRoute(pSession, 
                                      pInstruction,
                                      newOffset,
                                      DI_ROUTE_QUESTIONABLE));
            }
        }
    }
    return DI_SUCCESS;
}

static
int AnalyzeAndConstructInstruction(DianaAnalyzeSession * pSession,
                                   OPERAND_SIZE instructionOffset,
                                   OPERAND_SIZE nextInstructionOffset,
                                   int * pbNeedReset
                                   )
{
    int iRes = 0;
    Diana_Instruction * pInstruction = 0;
    Diana_LinkedAdditionalGroupInfo * pLinkedInfo = 0;
    int newRouteFlags =0;
    int bIsNormalInstruction = 0;
    OPERAND_SIZE newOffset = 0;

    *pbNeedReset = 0;

    // check if instruction is always registered
    DI_CHECK(Diana_InstructionsOwner_QueryOrAllocateInstruction(pSession->pOwner, 
                                                                instructionOffset, 
                                                                &pInstruction,
                                                                0)); // flags for new instruction

    // clear DI_INSTRUCTION_IS_LOADING 
    pInstruction->m_flags = 0;

    pLinkedInfo = pSession->result.pInfo->m_pGroupInfo->m_pLinkedInfo;
    if (!pLinkedInfo)
    {
        bIsNormalInstruction = 1;
    }
    else
    if (!(pLinkedInfo->flags & DIANA_GT_CAN_CHANGE_RIP))
    {
        bIsNormalInstruction = 1;
    }

    // acquire newOffset
    if (bIsNormalInstruction)
    {
        return AnalyzeAndConstructNormalInstruction(pSession, 
                                                    pInstruction,
                                                    nextInstructionOffset);
    }

    // special instruction found: jmps, jcc's, call, ret, leave
    pInstruction->m_flags |= DI_INSTRUCTION_CAN_CHANGE_CSIP;

    // handle ret case
    if (pLinkedInfo->flags & DIANA_GT_RET)
    {
        pInstruction->m_flags |= DI_INSTRUCTION_IS_RET;
        *pbNeedReset = 1;
        return DI_SUCCESS;
    }

    // handle jumps and others
    if (!(pLinkedInfo->flags & DIANA_GT_CAN_GO_TO_THE_NEXT_INSTRUCTION))
    {
        *pbNeedReset = 1;
    }

    newOffset = 0;
    iRes = AnalyzeJumps(&pSession->result, 
                        pLinkedInfo, 
                        &newOffset,
                        nextInstructionOffset,
                        pSession->pObserver);
    
    if (iRes == DI_UNSUPPORTED_COMMAND)
        return DI_SUCCESS;

    DI_CHECK(iRes);
    

    // check bounds
    if (newOffset > pSession->maxOffset)
    {
        return DI_SUCCESS;
    }

    DI_CHECK(SaveNewRoute(pSession,
                          pInstruction,
                          newOffset,
                          newRouteFlags));
    return DI_SUCCESS;
}

static
int Diana_AnalyzeCodeImpl(DianaAnalyzeSession * pSession,
                          OPERAND_SIZE initialOffset
                      )
{
    int iRes = 0;
    OPERAND_SIZE offset = initialOffset;
    OPERAND_SIZE prevOffset = 0;
    
    Diana_RouteInfo curRouteInfo;
    
    int bNeedReset = 0;
    Diana_InitContext(&pSession->context, pSession->mode);

    // init cur route
    curRouteInfo.startOffset = offset;
    curRouteInfo.flags = 0;
    while(1)
    {
        prevOffset = offset;
        iRes = Diana_ParseCmd(&pSession->context,
                               Diana_GetRootLine(),
                               &pSession->pObserver->m_stream,
                               &pSession->result);

        offset += pSession->result.iFullCmdSize;
        if (iRes == DI_END)
            break;

        if (iRes)
        {
            // instruction not recognized properly
            if (!pSession->result.iFullCmdSize)
            {
                Diana_CacheEatOneSafe(&pSession->context);
            }
            Diana_DebugFatalBreak();
            continue;
        }


        // analyze instruction
        bNeedReset = 0;
        DI_CHECK(AnalyzeAndConstructInstruction(pSession,
                                                prevOffset,
                                                offset,
                                                &bNeedReset));

        if (offset > pSession->maxOffset || bNeedReset)
        {
            if (!pSession->stack.m_count)
                break;
            DI_CHECK(Diana_Stack_Pop(&pSession->stack, &curRouteInfo));
            offset = curRouteInfo.startOffset;
            DI_CHECK(pSession->pObserver->m_pMoveTo(pSession->pObserver, offset));
            Diana_ClearCache(&pSession->context);
        }
    }
    return DI_SUCCESS;
}

int Diana_AnalyzeCode(Diana_InstructionsOwner * pOwner,
                      DianaAnalyzeObserver * pObserver,
                      int mode,
                      OPERAND_SIZE initialOffset,
                      OPERAND_SIZE maxOffset)
{
    int res = 0;
    DianaAnalyzeSession session;
    memset(&session, 0, sizeof(session));

    session.maxOffset = maxOffset;
    session.pOwner = pOwner;
    session.pObserver = pObserver;
    session.mode = mode;
    
    DI_CHECK(Diana_Stack_Init(&session.stack, 4096, sizeof(Diana_RouteInfo)));

    res = Diana_AnalyzeCodeImpl(&session,
                                initialOffset);
    Diana_Stack_Free(&session.stack);
    return res;
}

