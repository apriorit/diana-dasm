
#include "diana_processor_core_impl_xmm.h"

// special
#include "diana_processor_specific_xmm.h"

void DianaProcessor_XMM_SetValue(DianaProcessor * pCallContext,
                                 DianaUnifiedRegister regId,
                                 DianaRegInfo * pReg,
                                 DianaRegisterXMM_type * value)
{
    DianaProcessor_XMM_SetValueEx(pCallContext,
                                  regId,
                                  pReg,
                                  value,
                                  pReg->m_size);
}

int Diana_ProcessorSetGetOperand_XMM_register(struct _dianaContext * pDianaContext,
                                              DianaProcessor * pCallContext,
                                              DianaLinkedOperand * pLinkedOp,
                                              DianaRegisterXMM_type * pResult,
                                              int bSet)
{
    pDianaContext;

    if (bSet)
    {
        DianaProcessor_XMM_SetValue(pCallContext, 
                                    pLinkedOp->value.recognizedRegister,
                                    DianaProcessor_QueryReg(pCallContext,
                                                            pLinkedOp->value.recognizedRegister), 
                                    pResult);
    }
    else
    {
        *pResult = DianaProcessor_XMM_GetValue(pCallContext, 
                                               DianaProcessor_QueryReg(pCallContext,
                                                                       pLinkedOp->value.recognizedRegister));
    }
    return DI_SUCCESS;
}

int Diana_ProcessorSetGetOperand_XMM_index(struct _dianaContext * pDianaContext,
                                           DianaProcessor * pCallContext,
                                           int usedSize,
                                           DianaRegisterXMM_type * pResult,
                                           int bSet,
                                           DianaRmIndex * pRmIndex,
                                           int flags)
{
    OPERAND_SIZE selector = GET_REG_DS;
    OPERAND_SIZE address = 0;

    DI_CHECK(DianaProcessor_CalcIndex(pDianaContext,
                                      pCallContext,
                                      pRmIndex,
                                      &selector,
                                      &address));

    if (pCallContext->m_result.pInfo->m_flags & DI_FLAG_CMD_MUST_BE_ALIGNED &&
        0 != (address % 16))
    {
        return DI_GP;
    }

    if (bSet)
    {
        return DianaProcessor_XMM_SetMemValue(pCallContext, 
                                              selector,
                                              address, 
                                              usedSize,
                                              pResult,
                                              flags,
                                              pRmIndex->seg_reg);
    }

    return DianaProcessor_XMM_GetMemValue(pCallContext, 
                                          selector,
                                          address, 
                                          usedSize,
                                          pResult,
                                          flags,
                                          pRmIndex->seg_reg);
}

int DianaProcessor_XMM_SetGetOperand(struct _dianaContext * pDianaContext,
                                     DianaProcessor * pCallContext,
                                     int opNumber,
                                     DianaRegisterXMM_type * pResult,
                                     int bSet,
                                     int sizeOfOperand,
                                     int flags)
{
    DianaLinkedOperand * pLinkedOp = pCallContext->m_result.linkedOperands + opNumber;

    flags;

    if (opNumber >= pCallContext->m_result.iLinkedOpCount ||
        opNumber < 0)
    {
        Diana_FatalBreak();
        return DI_ERROR;
    }

	pLinkedOp->usedSize = sizeOfOperand;
    switch(pLinkedOp->type)
    {
    case diana_register:
        return Diana_ProcessorSetGetOperand_XMM_register(pDianaContext,
                                                         pCallContext,
                                                         pLinkedOp,
                                                         pResult,
                                                         bSet);
    case diana_index:
        return Diana_ProcessorSetGetOperand_XMM_index(pDianaContext,
                                                      pCallContext,
                                                      pLinkedOp->usedSize,
                                                      pResult,
                                                      bSet,
                                                      &pLinkedOp->value.rmIndex,
                                                      flags);
    default:
        break;
    }

    Diana_FatalBreak();
    return DI_ERROR;
}
