#include "diana_analyze2.h"
#include "diana_core.h"

int Diana_AnalyzeInstructionStackDiff_KnownCommands(struct _dianaContext * pContext,
                                                    struct _dianaParserResult * pResult,
                                                    OPERAND_SIZE_SIGNED * pESPDiff,
                                                    int * pIsCall)
{
    DianaGroupInfo * pGroupInfo = Diana_GetGroupInfo(pResult->pInfo->m_lGroupId);
    int scanMode = 0;
    &pContext;

    *pESPDiff = 0;
    *pIsCall = 0;
    switch(pGroupInfo->m_commandId)
    {
    case diana_cmd_push:
        {
            *pESPDiff = 0-pContext->iCurrentCmd_opsize;
            return 0;
        }
    case diana_cmd_pop:
        {
            *pESPDiff = pContext->iCurrentCmd_opsize;
            return 0;
        }
    case diana_cmd_pusha:
        {
            *pESPDiff = 0-pContext->iCurrentCmd_opsize*8;
            return 0;
        }
    case diana_cmd_popa:
        {
            *pESPDiff = pContext->iCurrentCmd_opsize*8;
            return 0;
        }
    case diana_cmd_sub:
        scanMode = -1;
        break;
    case diana_cmd_add:
        scanMode = 1;
        break;
    case diana_cmd_mov:
        break;
    default:
        return DI_SUCCESS;
    }
    
    if (pResult->iLinkedOpCount != 2)
    {
        return DI_SUCCESS;
    }

    if (pResult->linkedOperands[0].type == diana_register &&
        (pResult->linkedOperands[0].value.recognizedRegister == reg_ESP ||
         pResult->linkedOperands[0].value.recognizedRegister == reg_RSP ||
         pResult->linkedOperands[0].value.recognizedRegister == reg_SP))
    {
        if (pGroupInfo->m_commandId == diana_cmd_mov)
        {
            return DI_END;
        }
        if (pResult->linkedOperands[1].type == diana_imm)
        {
            if (scanMode > 0)
            {
                *pESPDiff = pResult->linkedOperands[1].value.imm;
            }
            else
            {
                *pESPDiff = 0LL - pResult->linkedOperands[1].value.imm;
            }
        }
        else
        {
            return DI_UNSUPPORTED_COMMAND;
        }
    }
    return DI_SUCCESS;
}
int Diana_AnalyzeInstructionStackDiff(struct _dianaContext * pContext,
                                      struct _dianaParserResult * pResult,
                                      OPERAND_SIZE_SIGNED * pESPDiff,
                                      int * pIsCall)
{
    DianaGroupInfo * pGroupInfo = Diana_GetGroupInfo(pResult->pInfo->m_lGroupId);

    *pIsCall = 0;
    *pESPDiff = 0;

    if (!pGroupInfo)
    {
        return DI_ERROR;
    }
    if (pGroupInfo->m_pLinkedInfo)
    {
        if (pGroupInfo->m_pLinkedInfo->flags & DIANA_GT_IS_CALL)
        {
            *pIsCall = 1;
            *pESPDiff = -pContext->iCurrentCmd_opsize;
            return DI_SUCCESS;
        }
        if (pGroupInfo->m_pLinkedInfo->flags & DIANA_GT_IS_JUMP)
        {
            return DI_SUCCESS;
        }
        // check ret
        if (pGroupInfo->m_pLinkedInfo->flags & DIANA_GT_RET)
        {
            *pESPDiff = pContext->iCurrentCmd_opsize;
            if (pResult->iLinkedOpCount > 0  && pResult->linkedOperands[0].type == diana_imm)
            {
                *pESPDiff += pResult->linkedOperands[0].value.imm;
            }
            return DI_SUCCESS;
        }
    }
  
    return Diana_AnalyzeInstructionStackDiff_KnownCommands(pContext,
                                                           pResult,
                                                           pESPDiff,
                                                            pIsCall);
}
