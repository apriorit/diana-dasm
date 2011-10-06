#include "diana_processor_cmd_m.h"
#include "diana_proc_gen.h"
#include "diana_gen.h"
#include "diana_core_gen_tags.h"
#include "diana_processor_cmd_internal.h"

int Di_CheckZeroExtends(DianaProcessor * pCallContext,
                        OPERAND_SIZE * pDest,
                        int src_size,
                        int * dest_size)
{
    if (!pCallContext->m_context.iAMD64Mode)
    {
        return DI_SUCCESS;
    }

    if (src_size == 8)
    {
        return DI_SUCCESS;
    }

    if (pCallContext->m_context.iCurrentCmd_opsize == 8)
    {
        if (pCallContext->m_result.pInfo->m_flags & DI_FLAG_CMD_AMD64_SIGN_EXTENDS)
        {
            DI_CHECK(DianaProcessor_SignExtend(pDest, 
                                            src_size,
                                            8));
        }
    }
    else
    if (pCallContext->m_context.iCurrentCmd_opsize == 4)
    {
        if (pCallContext->m_result.linkedOperands->type == diana_register)
        {
            DianaUnifiedRegister reg64;
            DI_CHECK(DianaProcessor_Query64RegisterFor32( pCallContext->m_result.linkedOperands->value.recognizedRegister,
                                                          &reg64));

            DianaProcessor_SetValue(pCallContext, reg64, DianaProcessor_QueryReg(pCallContext, reg64), 0);
        }
    }
    return DI_SUCCESS;
}

int Diana_Call_mov(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    //DEST := SRC
    DI_DEF_LOCALS(src, dest);
    
    DI_MEM_GET_DEST(dest)
    DI_MEM_GET_SRC(src);

    dest = src;
    DI_CHECK(Di_CheckZeroExtends(pCallContext, &dest, src_size, &dest_size));

    DI_MEM_SET_DEST(dest);
    DI_PROC_END
}

int Diana_Call_movs(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    // SRC = DEST;
    OPERAND_SIZE selector = GET_REG_DS;
    DianaRmIndex srcIndex, destIndex;
    OPERAND_SIZE srcRegAddress = 0, destRegAddress = 0;
    DI_DEF_LOCALS(src, dest);
    
    DianaProcessor_CmdUsesNormalRep(pCallContext);

    DI_CHECK(Di_PrepareSIDI_regs(pCallContext, &srcIndex, &destIndex));
        
    DI_CHECK(Diana_ProcessorGetOperand_index_ex(pDianaContext,
                                                pCallContext,
                                                pCallContext->m_result.linkedOperands->usedSize,
                                                &srcRegAddress,
                                                &src,
                                                &srcIndex));

    DI_CHECK(DianaProcessor_CalcIndex(pDianaContext,
                                      pCallContext,
                                      &destIndex,
                                      &selector,
                                      &destRegAddress));

    // move src to dest
    DI_CHECK(DianaProcessor_SetMemValue(pCallContext, 
                                      selector,
                                      destRegAddress, 
                                      pCallContext->m_result.linkedOperands->usedSize,
                                      &src,
                                      0,
                                      destIndex.seg_reg));

    // shift registers
    Di_UpdateSIDI(pCallContext,
                  &srcRegAddress, 
                  &destRegAddress);

    src_size = pCallContext->m_result.linkedOperands->usedSize;

    //---
    DianaProcessor_SetValue(pCallContext, 
                            srcIndex.reg,
                            DianaProcessor_QueryReg(pCallContext,
                                                    srcIndex.reg), 
                            srcRegAddress);

    DianaProcessor_SetValue(pCallContext, 
                            destIndex.reg,
                            DianaProcessor_QueryReg(pCallContext,
                                                    destIndex.reg), 
                            destRegAddress);

    DI_PROC_END
}

int Diana_Call_movsx(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    //DEST := SRC
    DI_DEF_LOCALS(src, dest);
    
    DI_MEM_GET_DEST(dest)
    DI_MEM_GET_SRC(src);

    DI_SIGN_EXTEND(src, dest_size);

    dest = src;

    DI_MEM_SET_DEST(dest);
    DI_PROC_END
}

int Diana_Call_movzx(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    //DEST := SRC
    DI_DEF_LOCALS(src, dest);
    
    DI_MEM_GET_DEST(dest)
    DI_MEM_GET_SRC(src);

    dest = src;

    DI_MEM_SET_DEST(dest);
    DI_PROC_END
}

// MUL
static
int Diana_Call_mul8(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext,
                    OPERAND_SIZE * pArgument)
{
    DianaRegisterValue16_type result, temp;
    DI_UINT16 arg1 = (DI_UINT16)GET_REG_AL;
    DI_UINT16 arg2 = (DI_UINT16)*pArgument;

    result.value = arg1 * arg2;
    temp = result;

    if (result.h)
    {
       SET_FLAG_CF;
       SET_FLAG_OF;
    }

    SET_REG_AX(result.value);
    return DI_SUCCESS;
}

static
int Diana_Call_mul16(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext,
                    OPERAND_SIZE * pArgument)
{
    DianaRegisterValue32_type result, temp;
    DI_UINT32 arg1 = (DI_UINT32)GET_REG_AX;
    DI_UINT32 arg2 = (DI_UINT32)*pArgument;

    result.value = arg1 * arg2;
    temp = result;

    if (result.h)
    {
       SET_FLAG_CF;
       SET_FLAG_OF;
    }

    SET_REG_AX(result.l.value);
    SET_REG_DX(result.h);
    return DI_SUCCESS;
}

static
int Diana_Call_mul32(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext,
                    OPERAND_SIZE * pArgument)
{
    DianaRegisterValue_type result, temp;
    DI_UINT64 arg1 = (DI_UINT64)GET_REG_EAX;
    DI_UINT64 arg2 = (DI_UINT64)*pArgument;

    result.value = arg1 * arg2;
    temp = result;

    if (result.h)
    {
       SET_FLAG_CF;
       SET_FLAG_OF;
    }

    SET_REG_EAX(result.l.value);
    SET_REG_EDX(result.h);
    return DI_SUCCESS;
}

static
int Diana_Call_mul64(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext,
                    OPERAND_SIZE * pArgument)
{
    DianaRegisterValue_type result;
    DI_UINT64 argument = *pArgument;
    DI_UINT64 rax = GET_REG_RAX;

    result.value = rax * argument;

    if (rax && argument)
    {
        //check overflow
        if (rax != result.value / argument)
        {
            return DI_UNSUPPORTED_COMMAND;
        }
    }

    SET_REG_RAX(result.value);
    SET_REG_RDX(0);
    return DI_SUCCESS;
}

int Diana_Call_mul(struct _dianaContext * pDianaContext,
                   DianaProcessor * pCallContext)
{
    DI_DEF_LOCAL(argument);
    DI_MEM_GET_SRC(argument);

    CLEAR_FLAG_CF;
    CLEAR_FLAG_OF;

    switch(pCallContext->m_result.linkedOperands->usedSize)
    {
    case DIANA_MODE64:
        DI_CHECK(Diana_Call_mul64(pDianaContext, pCallContext, &argument));
        break;

    case DIANA_MODE32:
        DI_CHECK(Diana_Call_mul32(pDianaContext, pCallContext, &argument));
        break;

    case DIANA_MODE16:
        DI_CHECK(Diana_Call_mul16(pDianaContext, pCallContext, &argument));
        break;

    case 1:
        DI_CHECK(Diana_Call_mul8(pDianaContext, pCallContext, &argument));
        break;

    default:
        Diana_DebugFatalBreak();
        return DI_ERROR;
    }

    DI_PROC_END
}

int Diana_Call_movsxd(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext)
{
    //DEST := SRC
    DI_DEF_LOCALS(src, dest);
    
    DI_MEM_GET_DEST(dest)
    DI_MEM_GET_SRC(src);

    DI_SIGN_EXTEND(src, dest_size);

    if (pCallContext->m_context.iCurrentCmd_opsize == DIANA_MODE16)
    {
        dest &= ~0xFFFFULL;
        dest |= src & 0xFFFFULL;
    }
    else
    {
        dest = src;
    }

    DI_MEM_SET_DEST(dest);
    DI_PROC_END
}
