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
    Diana_RouteInfo curRouteInfo;
}DianaAnalyzeSession;

void DianaAnalyzeObserver_Init(DianaAnalyzeObserver * pThis,
                               DianaMovableReadStream * pStream,
                               DianaAnalyzeAddress_type pAnalyzeJumpAddress)
{
    pThis->m_pStream = pStream;
    pThis->m_pAnalyzeAddress = pAnalyzeJumpAddress;
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
static
int Diana_InstructionsOwner_ExternalInstructionsDeleter(Diana_ListNode * pNode,
                                                        void * pContext,
                                                        int * pbDone)
{
    Diana_Instruction * pInstruction = (Diana_Instruction * )pNode;
    free(pInstruction);
    &pContext;
	*pbDone = 0;
    return DI_SUCCESS;
}
void Diana_InstructionsOwner_Free(Diana_InstructionsOwner * pOwner)
{
    Diana_ListForEach(&pOwner->m_externalInstructionsList, Diana_InstructionsOwner_ExternalInstructionsDeleter, 0);
    Diana_InitList(&pOwner->m_externalInstructionsList);
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
    ++pOwner->m_actualSize;
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
                               flags
                               );
        pOwner->m_ppPresenceVec[address] = *ppInstruction;
    }
    return DI_SUCCESS;
}

Diana_Instruction * Diana_InstructionsOwner_GetInstruction(Diana_InstructionsOwner * pOwner, 
                                                           OPERAND_SIZE address)
{
    return pOwner->m_ppPresenceVec[address];
}

static int Diana_CreateXRef(Diana_InstructionsOwner * pOwner,
                            Diana_Instruction * pFromInstruction,
                            Diana_Instruction * pToInstruction,
                            Diana_XRef ** ppXref)
{
    Diana_XRef * pXref = 0;
    Diana_XRef xref;
    memset(&xref, 0, sizeof(xref));
    xref.m_subrefs[0].m_pInstruction = pFromInstruction;
    xref.m_subrefs[1].m_pInstruction = pToInstruction;
    DI_CHECK(Diana_Stack_Push(&pOwner->m_xrefs, &xref));

    pXref = (Diana_XRef *)Diana_Stack_GetTopPtr(&pOwner->m_xrefs);
    Diana_PushBack(&pToInstruction->m_refsFrom, &(pXref)->m_subrefs[0].m_instructionEntry);
    Diana_PushBack(&pFromInstruction->m_refsTo, &(pXref)->m_subrefs[1].m_instructionEntry);
    if (ppXref)
    {
        *ppXref = pXref;
    }
    return DI_SUCCESS;
}

static
int AnalyzeJumps(DianaParserResult * pResult,
                 Diana_LinkedAdditionalGroupInfo * pLinkedInfo,
                 OPERAND_SIZE * pNewOffset,
                 OPERAND_SIZE offset,
                 int * pAbsoluteAddress)
{
    const DianaLinkedOperand * pOp = pResult->linkedOperands + pLinkedInfo->relArgrumentNumber;
    *pNewOffset = 0;
    *pAbsoluteAddress = 0;

    switch(pOp->type)
    {
    // usual call smth
    case diana_rel:
        *pNewOffset = offset + pOp->value.rel.m_value;
        break;
    // crazy call for really crazy guys
    case diana_call_ptr:
        *pAbsoluteAddress = 1;
        *pNewOffset = pOp->value.ptr.m_address;
        break;
    // call [rbx+0x5435345] 
    case diana_index:
        *pAbsoluteAddress = 1;
        *pNewOffset = pOp->value.rmIndex.dispValue;
        break;
    case diana_register:
        return DI_UNSUPPORTED_COMMAND;
    }
    return DI_SUCCESS;
}

static
int SaveNewExternalRoute(DianaAnalyzeSession * pSession,
                         Diana_Instruction * pInstruction,
                         OPERAND_SIZE newOffset)
{
    Diana_XRef * pXref = 0;
    Diana_Instruction * pTargetInstruction = 0;
    pTargetInstruction = malloc(sizeof(Diana_Instruction));
    DI_CHECK_ALLOC(pTargetInstruction);
    Diana_Instruction_Init(pTargetInstruction, newOffset, DI_INSTRUCTION_EXTERNAL);
    Diana_PushBack(&pSession->pOwner->m_externalInstructionsList, &pTargetInstruction->m_routeEntry);
    DI_CHECK(Diana_CreateXRef(pSession->pOwner, pInstruction, pTargetInstruction, &pXref));
    pXref->m_flags |= DI_XREF_EXTERNAL;
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
    Diana_RouteInfo newRouteInfo;

    if (newOffset >= pSession->maxOffset)
        return DI_SUCCESS;

    memset(&newRouteInfo, 0, sizeof(newRouteInfo));
    newRouteInfo.startOffset = newOffset;
    newRouteInfo.flags = newRouteFlags;

    // create new instruction
    pOldTargetInstruction = Diana_InstructionsOwner_GetInstruction(pSession->pOwner, newOffset);

    DI_CHECK(Diana_InstructionsOwner_QueryOrAllocateInstruction(pSession->pOwner, 
                                                                newOffset, 
                                                                &pTargetInstruction,
                                                                DI_INSTRUCTION_IS_LOADING)); 

    if (!newRouteFlags & DI_ROUTE_QUESTIONABLE)
    {
        pTargetInstruction->m_flags |= DI_INSTRUCTION_ROOT;
    }

    // register xref-from Instruction to Target
    DI_CHECK(Diana_CreateXRef(pSession->pOwner, pInstruction, pTargetInstruction, 0));

    // do not push new route to stack twice
    if (pOldTargetInstruction)
        return DI_SUCCESS;

    // push to stack
    DI_CHECK(Diana_Stack_Push(&pSession->stack, &newRouteInfo));

    return DI_SUCCESS;
}

static
void DispatchMode32(DianaAnalyzeSession * pSession,
                    Diana_Instruction * pInstruction,
                    const DianaLinkedOperand * pOp,
                    int * pbFound,
                    OPERAND_SIZE * pSuspectedOp)
{
	&pInstruction;
	&pSession;
    *pbFound = 0;
    switch(pOp->type)
    {
    case diana_imm:
        *pbFound = 1;
        *pSuspectedOp = pOp->value.imm;
        break;
    case diana_index:
        if (pOp->value.rmIndex.dispSize == 4 ||
            pOp->value.rmIndex.dispSize ==  8)
        {
            *pbFound = 1;
            *pSuspectedOp = pOp->value.rmIndex.dispValue;
        }
        break;
    default:
        ;
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
    &pSession;
    *pbFound = 0;
    switch(pOp->type)
    {
    case diana_register:
        if (pOp->value.recognizedRegister == reg_RIP || 
            pOp->value.recognizedRegister == reg_IP)
        {
            pInstruction->m_flags |= DI_INSTRUCTION_USES_RIP;
        }
        break;
    case diana_imm:
        *pbFound = 1;
        *pSuspectedOp = pOp->value.imm;
        break;
    case diana_index:
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
    default:
        ;
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
            // TODO: add analyze 
            // this is not realy valid command
            DI_CHECK(SaveNewRoute(pSession, 
                                  pInstruction,
                                  suspectedOp,
                                  DI_ROUTE_QUESTIONABLE));
            
        }
    }
    return DI_SUCCESS;
}

static
int AnalyzeAndConstructInstruction(DianaAnalyzeSession * pSession,
                                   OPERAND_SIZE instructionOffset,
                                   OPERAND_SIZE nextInstructionOffset,
                                   int * pbNeedReset)
{
    int bAbsoluteAddress = 0;
    int iRes = 0;
    Diana_Instruction * pInstruction = 0;
    Diana_LinkedAdditionalGroupInfo * pLinkedInfo = 0;
    int bIsNormalInstruction = 0;
    OPERAND_SIZE newOffset = 0;
    DianaAnalyzeAddressResult_type jumpFlags = diaJumpNormal;

    *pbNeedReset = 0;

    DI_CHECK(Diana_InstructionsOwner_QueryOrAllocateInstruction(pSession->pOwner, 
                                                                instructionOffset, 
                                                                &pInstruction,
                                                                0)); // flags for new instruction

    // clear DI_INSTRUCTION_IS_LOADING flag
    Diana_PushBack(&pSession->curRouteInfo.instructions, 
                    &pInstruction->m_routeEntry);

    pInstruction->m_flags = 0;

    if (!pSession->curRouteInfo.flags & DI_ROUTE_QUESTIONABLE)
    {
        pInstruction->m_flags |= DI_INSTRUCTION_ROOT;
    }

    // analyze command
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

    // analyze normal command
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
                        &bAbsoluteAddress);
    
    if (iRes == DI_UNSUPPORTED_COMMAND)
        return DI_SUCCESS;

    DI_CHECK(iRes);
    
    // check new jump address
    DI_CHECK(pSession->pObserver->m_pAnalyzeAddress(pSession->pObserver, 
                                                        newOffset,
                                                        bAbsoluteAddress?DIANA_ANALYZE_ABSOLUTE_ADDRESS:0,
                                                        &newOffset,
                                                        &jumpFlags
                                                        ));
    
    switch(jumpFlags)
    {
    case diaJumpNormal:
        // save new route
        DI_CHECK(SaveNewRoute(pSession,
                              pInstruction,
                              newOffset,
                              pSession->curRouteInfo.flags));
        break;
    case diaJumpExternal:
        DI_CHECK(SaveNewExternalRoute(pSession,
                                      pInstruction,
                                      newOffset));
        break;
    default:
        Diana_FatalBreak();
    case diaJumpInvalid: ;
    }

    return DI_SUCCESS;
}

static
int SwitchToLastState(DianaAnalyzeSession * pSession,
                      Diana_RouteInfo * pCurRouteInfo,
                      OPERAND_SIZE * pOffset)
{
    DI_CHECK(Diana_Stack_Pop(&pSession->stack, pCurRouteInfo));
    *pOffset = pCurRouteInfo->startOffset;
    DI_CHECK(pSession->pObserver->m_pStream->pMoveTo(pSession->pObserver->m_pStream, pCurRouteInfo->startOffset));
    Diana_ClearCache(&pSession->context);
    return DI_SUCCESS;
}

Diana_XRef * Diana_CastXREF(Diana_ListNode * pNode,
                            int index)
{
    Diana_SubXRef * pSubXREF = (Diana_SubXRef * )pNode;
    return DIANA_CONTAINING_RECORD(pSubXREF, Diana_XRef, m_subrefs[index]);
}

static
int XrefRouteMarker(Diana_ListNode * pNode,
                    void * pContext,
                    int * pbDone)
{
    Diana_XRef * pXRef = Diana_CastXREF(pNode, (int)(size_t)pContext);
    pXRef->m_flags |= DI_XREF_INVALID;
	&pbDone;
    return DI_SUCCESS;
}

static
int RouteMarker(Diana_ListNode * pNode,
                void * pContext,
                int * pbDone)
{
    Diana_Instruction * pInstruction = (Diana_Instruction * )pNode;
    DianaAnalyzeSession * pSession  = pContext;
	&pbDone;
    pInstruction->m_flags |= DI_INSTRUCTION_INVALID;

    Diana_ListForEach(&pInstruction->m_refsFrom, 
                      XrefRouteMarker, 
                      (void*)0);

    Diana_ListForEach(&pInstruction->m_refsTo, 
                      XrefRouteMarker, 
                      #pragma warning( suppress : 4306 ) // conversion from ' type1 ' to ' type2 ' of greater size
                      (void*)1);

    --pSession->pOwner->m_actualSize;
    return DI_SUCCESS;
}

static
void RollbackCurrentRoute(DianaAnalyzeSession * pSession)
{
    Diana_ListForEach(&pSession->curRouteInfo.instructions,
                      RouteMarker,
                      pSession);
}

static
int Diana_AnalyzeCodeImpl(DianaAnalyzeSession * pSession,
                          OPERAND_SIZE initialOffset)
{
    int iRes = 0;
    OPERAND_SIZE offset = initialOffset;
    OPERAND_SIZE prevOffset = 0;
    
    int bNeedReset = 0;
    Diana_InitContext(&pSession->context, pSession->mode);

    // init cur route
    pSession->curRouteInfo.startOffset = offset;
    pSession->curRouteInfo.flags = 0;
	#pragma warning( suppress : 4127 ) // conditional expression is constant
    while(1)
    {
        if (offset >= pSession->maxOffset || bNeedReset)
        {
            // just go out
            if (!pSession->stack.m_count)
                break;

            DI_CHECK(SwitchToLastState(pSession, 
                                       &pSession->curRouteInfo, 
                                       &offset));
        }
        bNeedReset = 0;

        prevOffset = offset;

        iRes = Diana_ParseCmd(&pSession->context,
                              Diana_GetRootLine(),
                              &pSession->pObserver->m_pStream->parent,
                              &pSession->result);

        offset += pSession->result.iFullCmdSize;
        if (iRes)
        {
            // instruction not recognized properly
            if (pSession->curRouteInfo.flags & DI_ROUTE_QUESTIONABLE)
            {
                RollbackCurrentRoute(pSession);
                bNeedReset = 1;
                continue;
            }

            offset = prevOffset + 1;
            DI_CHECK(pSession->pObserver->m_pStream->pMoveTo(pSession->pObserver->m_pStream, 
                                                             offset));
            Diana_ClearCache(&pSession->context);

            continue;
        }
        // check if instruction is always registered
        {
            Diana_Instruction * pInstruction = Diana_InstructionsOwner_GetInstruction(pSession->pOwner, prevOffset);
            if (pInstruction)
            {
                if (pInstruction->m_flags & DI_INSTRUCTION_INVALID)
                {
                    if (pSession->curRouteInfo.flags & DI_ROUTE_QUESTIONABLE)
                    {
                        RollbackCurrentRoute(pSession);
                    }
                    bNeedReset = 1;
                    continue;
                }
                if (!(pInstruction->m_flags & DI_INSTRUCTION_IS_LOADING))
                {
                    bNeedReset = 1;                 
                    continue;
                }
            }
        }
        // analyze instruction
        DI_CHECK(AnalyzeAndConstructInstruction(pSession,
                                                prevOffset,
                                                offset,
                                                &bNeedReset));
        if (offset >= pSession->maxOffset)
        {
            bNeedReset = 1;
            continue;
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

    res = pObserver->m_pStream->pMoveTo(pObserver->m_pStream, initialOffset);
    if (res)
        goto clean;
    res = Diana_AnalyzeCodeImpl(&session,
                                initialOffset);
    if (res)
        goto clean;

clean:
    Diana_Stack_Free(&session.stack);
    return res;
}
