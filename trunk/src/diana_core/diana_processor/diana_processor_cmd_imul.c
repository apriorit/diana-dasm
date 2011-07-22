#include "diana_processor_cmd_i.h"
#include "diana_proc_gen.h"
#include "diana_gen.h"
#include "diana_core_gen_tags.h"

#define DI_UPDATE_IMUL_FLAGS(X) \
    if ((arg1 ^ arg2) < 0) \
    {\
        temp.value = 0 - result.value;\
    }\
    if (temp.h || (temp.value & DianaProcessor_GetSignMask(X)))\
    {\
        SET_FLAG_CF;\
        SET_FLAG_OF;\
    }

// IMUL 1
static
int Diana_Call_imul8(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext,
                    OPERAND_SIZE * pArgument)
{
    DianaRegisterValue16_signed_type result, temp;
    DI_SIGNED_CHAR arg1 = (DI_SIGNED_CHAR)GET_REG_AL;
    DI_SIGNED_CHAR arg2 = (DI_SIGNED_CHAR)*pArgument;

    result.value = (DI_INT32)arg1 * (DI_INT32)arg2;
    temp = result;

    DI_UPDATE_IMUL_FLAGS(1);

    SET_REG_AX(result.value);
    return DI_SUCCESS;
}

static
int Diana_Call_imul16(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext,
                    OPERAND_SIZE * pArgument)
{
    DianaRegisterValue32_signed_type result, temp;
    DI_INT16 arg1 = (DI_INT16)GET_REG_AX;
    DI_INT16 arg2 = (DI_INT16)*pArgument;

    result.value = (DI_INT32)arg1 * (DI_INT32)arg2;
    temp = result;

    DI_UPDATE_IMUL_FLAGS(2);

    SET_REG_AX(result.l.value);
    SET_REG_DX(result.h);
    return DI_SUCCESS;
}

static
int Diana_Call_imul32(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext,
                    OPERAND_SIZE * pArgument)
{
    DianaRegisterValue_signed_type result, temp;
    DI_INT32 arg1 = (DI_INT32)GET_REG_EAX;
    DI_INT32 arg2 = (DI_INT32)*pArgument;

    result.value = (DI_INT64)arg1 * (DI_INT64)arg2;
    temp = result;

    DI_UPDATE_IMUL_FLAGS(4);

    SET_REG_EAX(result.l.value);
    SET_REG_EDX(result.h);
    return DI_SUCCESS;
}

static
int Diana_Call_imul64(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext,
                    OPERAND_SIZE * pArgument)
{
    DianaRegisterValue_signed_type result;
    DI_INT64 argument = (DI_INT64)*pArgument;
    DI_INT64 rax = (DI_INT64)GET_REG_RAX;

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

int Diana_Call_imul_1(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext)
{
    DI_DEF_LOCAL(argument);
    DI_MEM_GET_DEST(argument);

    switch(pCallContext->m_result.linkedOperands->usedSize)
    {
    case DIANA_MODE64:
        DI_CHECK(Diana_Call_imul64(pDianaContext, pCallContext, &argument));
        break;

    case DIANA_MODE32:
        DI_CHECK(Diana_Call_imul32(pDianaContext, pCallContext, &argument));
        break;

    case DIANA_MODE16:
        DI_CHECK(Diana_Call_imul16(pDianaContext, pCallContext, &argument));
        break;

    case 1:
        DI_CHECK(Diana_Call_imul8(pDianaContext, pCallContext, &argument));
        break;

    default:
        Diana_DebugFatalBreak();
        return DI_ERROR;
    }
    DI_PROC_END
}


// IMUL 1, 2, 3
int Diana_Call_imul_impl(struct _dianaContext * pDianaContext,
                         DianaProcessor * pCallContext,
                         OPERAND_SIZE * op1,
                         OPERAND_SIZE * op2,
                         OPERAND_SIZE * op3,
                         int size)
{
    DianaRegisterValue_signed_type result, temp;
    OPERAND_SIZE_SIGNED arg1 = 0, arg2 =0;

    DI_CHECK(DianaProcessor_SignExtend(op1, 
                                       size,
                                       8));
    DI_CHECK(DianaProcessor_SignExtend(op2, 
                                       size,
                                       8));

    arg1 = (OPERAND_SIZE_SIGNED)*op1;
    arg2 = (OPERAND_SIZE_SIGNED)*op2;

    result.value = arg1 * arg2;
    temp = result;

    switch(pCallContext->m_result.linkedOperands->usedSize)
    {
    case DIANA_MODE64:
        if (arg1 && arg2)
        {
            //check overflow
            if (arg1 != result.value / arg2)
            {
                return DI_UNSUPPORTED_COMMAND;
            }
        }
        break;

    case DIANA_MODE32:
        if ((OPERAND_SIZE_SIGNED)(DI_INT32)result.value != result.value)
        {
            SET_FLAG_CF;
            SET_FLAG_OF;
        }  
        break;

    case DIANA_MODE16:
        if ((OPERAND_SIZE_SIGNED)(DI_INT16)result.value != result.value)
        {
            SET_FLAG_CF;
            SET_FLAG_OF;
        }  
        break;

    case 1:
        if ((OPERAND_SIZE_SIGNED)(DI_SIGNED_CHAR)result.value != result.value)
        {
            SET_FLAG_CF;
            SET_FLAG_OF;
        }          
        break;

    default:
        Diana_DebugFatalBreak();
        return DI_ERROR;
    }
    *op3 = result.value;
    return DI_SUCCESS;
}
int Diana_Call_imul_2(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext)
{
    DI_DEF_LOCALS(src, dest);
    DI_MEM_GET_DEST(dest);
    DI_MEM_GET_SRC(src);

    DI_CHECK(Diana_Call_imul_impl(pDianaContext, 
                                  pCallContext, 
                                  &src, 
                                  &dest, 
                                  &dest,
                                  dest_size));
    DI_MEM_SET_DEST(dest);
    DI_PROC_END
}

int Diana_Call_imul_3(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext)
{
    DI_DEF_LOCALS2(dest, src1, src2);
    DI_MEM_GET_DEST(dest);
    DI_MEM_GET_SRC(src1);
    DI_MEM_GET_SRC2(src2);

    DI_CHECK(Diana_Call_imul_impl(pDianaContext, 
                                  pCallContext, 
                                  &src1, 
                                  &src2, 
                                  &dest,
                                  dest_size));
    DI_MEM_SET_DEST(dest);
    DI_PROC_END
}

int Diana_Call_imul(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    CLEAR_FLAG_CF;
    CLEAR_FLAG_OF;

    switch(pCallContext->m_result.iLinkedOpCount)
    {
    case 1:
        return Diana_Call_imul_1(pDianaContext, pCallContext);
    case 2:
        return Diana_Call_imul_2(pDianaContext, pCallContext);
    case 3:
        return Diana_Call_imul_3(pDianaContext, pCallContext);
    }
    Diana_DebugFatalBreak();
    return DI_UNSUPPORTED_COMMAND;
}