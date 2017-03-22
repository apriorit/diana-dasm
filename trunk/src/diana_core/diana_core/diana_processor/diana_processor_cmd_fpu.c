#include "diana_processor_cmd_fpu.h"
#include "diana_proc_gen.h"
#include "diana_gen.h"
#include "diana_core_gen_tags.h"
#include "diana_processor_cmd_internal.h"
#include "softfloat/softfloatx80.h"
#include "diana_processor_core_impl_xmm.h"



#define DI_FPU_START   DI_CHECK(Diana_FPU_CheckFPU(pCallContext, 0)); &pDianaContext;
#define DI_FPU_START_IGNORE_EXCEPTIONS   DI_CHECK(Diana_FPU_CheckFPU(pCallContext, 1)); &pDianaContext;
#define DI_FPU_CLEAR_C1 pCallContext->m_fpu.statusWord &= ~DI_FPU_SW_C1;

#define DI_FPU_SET_CC(cc)  pCallContext->m_fpu.statusWord =(pCallContext->m_fpu.statusWord & ~(DI_FPU_SW_CC))|((cc) & DI_FPU_SW_CC);


#define DI_FPU_REG_IS_EMPTY(Number) !(pCallContext->m_fpu.registerFlags[(pCallContext->m_fpu.stackTop+(Number))&7]&DI_PROCESSOR_FPU_REGISTER_BUSY)

void Diana_FPU_Push(DianaProcessor * pCallContext)
{
    pCallContext->m_fpu.stackTop = (pCallContext->m_fpu.stackTop - 1)&7;
}
void Diana_FPU_Pop(DianaProcessor * pCallContext)
{
    pCallContext->m_fpu.registerFlags[pCallContext->m_fpu.stackTop] &= ~DI_PROCESSOR_FPU_REGISTER_BUSY;
    pCallContext->m_fpu.stackTop = (pCallContext->m_fpu.stackTop + 1)&7;
}

DI_UINT32 DI_FPU_ProcessException(DianaProcessor * pCallContext, DI_UINT32 in_exception)
{
    DI_UINT32 exception = in_exception & DI_FPU_SW_ALL_EXCEPTIONS;
    DI_UINT32 status = pCallContext->m_fpu.statusWord;
    DI_UINT32 unmasked = exception & ~pCallContext->m_fpu.controlWord&DI_FPU_CW_ALL_EXCEPTIONS;
    if (exception & (DI_FPU_EX_INVALID|DI_FPU_EX_ZERO_DIV))
        unmasked &= (DI_FPU_EX_INVALID|DI_FPU_EX_ZERO_DIV);
    
    if (unmasked)
    {
        pCallContext->m_fpu.statusWord |= DI_FPU_SW_SUMMARY | DI_FPU_SW_BACKWARD;
    }

    if (exception & DI_FPU_EX_INVALID) 
    {
        // FPU_EX_Invalid cannot come with any other exception but x87 stack fault
        pCallContext->m_fpu.statusWord |= exception;
        if (exception & DI_FPU_SW_STACK_FAULT) 
        {
            if (! (exception & DI_FPU_SW_C1)) 
            {
                /* This bit distinguishes over- from underflow for a stack fault,
                and roundup from round-down for precision loss. */
                pCallContext->m_fpu.statusWord &= ~DI_FPU_SW_C1;
            }
        }
        return unmasked;
    }

    if (exception & DI_FPU_EX_ZERO_DIV) 
    {
        pCallContext->m_fpu.statusWord |= DI_FPU_EX_ZERO_DIV;
        return unmasked;
    }

    if (exception & DI_FPU_EX_DENORMAL) 
    {
        pCallContext->m_fpu.statusWord |= DI_FPU_EX_DENORMAL;
        if (unmasked & DI_FPU_EX_DENORMAL)
            return unmasked;
    }

    // set the corresponding exception bits */
    pCallContext->m_fpu.statusWord |= exception;

    if (exception & DI_FPU_EX_PRECISION)
    {
        if (! (exception & DI_FPU_SW_C1)) 
        {
            pCallContext->m_fpu.statusWord&= ~DI_FPU_SW_C1;
        }
    }

    unmasked &= ~DI_FPU_EX_PRECISION;
    if (unmasked & (DI_FPU_EX_UNDERFLOW|DI_FPU_EX_OVERFLOW))
    {
        pCallContext->m_fpu.statusWord &= ~DI_FPU_SW_C1;
        if (! (status & DI_FPU_EX_PRECISION))
        {
            pCallContext->m_fpu.statusWord &= ~DI_FPU_EX_PRECISION;
        }
    }
    return unmasked;
}

void DI_FPU_Overflow(DianaProcessor * pCallContext)
{
    if (pCallContext->m_fpu.controlWord & DI_FPU_CW_INVALID)
    {
        Diana_FPU_Push(pCallContext);
        SET_REG_FPU_ST0(&floatx80_default_nan);
    }
    DI_FPU_ProcessException(pCallContext, DI_FPU_EX_STACK_OVERFLOW);
}

#define DI_FPU_PR_32_BITS          0x000
#define DI_FPU_PR_RESERVED_BITS    0x100
#define DI_FPU_PR_64_BITS          0x200
#define DI_FPU_PR_80_BITS          0x300

float_status_t FPU_pre_exception_handling(DianaProcessor * pCallContext)
{
    float_status_t status;

    int precision = pCallContext->m_fpu.controlWord & DI_FPU_CW_PRECISION_CONTROL;
    switch(precision)
    {
     case DI_FPU_PR_32_BITS:
        status.float_rounding_precision = 32;
        break;
     case DI_FPU_PR_64_BITS:
        status.float_rounding_precision = 64;
        break;
     case DI_FPU_PR_80_BITS:
        status.float_rounding_precision = 80;
        break;
     default:
        status.float_rounding_precision = 80;
    }

    status.float_exception_flags = 0; 
    status.float_nan_handling_mode = float_first_operand_nan;
    status.float_rounding_mode = (pCallContext->m_fpu.controlWord & DI_FPU_CW_ROUNDING_CONTROL)>>10;
    status.flush_underflow_to_zero = 0;
    status.float_suppress_exception = 0;
    status.float_exception_masks = pCallContext->m_fpu.controlWord&DI_FPU_CW_ALL_EXCEPTIONS;
    status.denormals_are_zeros = 0;
    return status;
}

int Diana_GlobalInitFPU()
{
    #ifndef DIANA_PROCESSOR_USE_SOFTFLOAT_FPU
        return InitSoftFloat();
    #endif
    return 0;
}

int Diana_FPU_WriteFloatArgument(struct _dianaContext * pDianaContext,
                                 DianaProcessor * pCallContext,
                                 int number,
                                 floatx80_t * result)
{
    float_status_t status = {0};
    OPERAND_SIZE doneBytes;
    OPERAND_SIZE selector = 0, address = 0;
    DianaUnifiedRegister segReg = reg_DS;
    DI_CHECK(Diana_QueryAddress(pDianaContext, pCallContext, number, &selector, &address, &segReg));

    switch(pCallContext->m_result.linkedOperands[number].usedSize)
    {
    default:
        Diana_FatalBreak();
        return DI_ERROR;
    case 4:
        {
            float32 ft32 = floatx80_to_float32(*result, &status);
            DI_CHECK(DianaProcessor_WriteMemory(pCallContext, 
                                           selector,
                                           address,
                                           &ft32,
                                           4,
                                           &doneBytes,
                                           0,
                                           segReg));
        }
        break;
    case 8:
        {
            float64 ft64 = floatx80_to_float64(*result, &status);
            DI_CHECK(DianaProcessor_WriteMemory(pCallContext, 
                                           selector,
                                           address,
                                           &ft64,
                                           8,
                                           &doneBytes,
                                           0,
                                           segReg));
        }  
        break;
    case 10:
        DI_CHECK(DianaProcessor_WriteMemory(pCallContext, 
                                       selector,
                                       address,
                                       result,
                                       10,
                                       &doneBytes,
                                       0,
                                       segReg));
        break;
    }
    return DI_SUCCESS;
}

int Diana_FPU_ReadFloatArgument(struct _dianaContext * pDianaContext,
                                 DianaProcessor * pCallContext,
                                 int number,
                                 floatx80_t * result)
{
    float_status_t status = {0};
    OPERAND_SIZE doneBytes;
    OPERAND_SIZE selector = 0, address = 0;
    DianaUnifiedRegister segReg = reg_DS;
    DI_CHECK(Diana_QueryAddress(pDianaContext, pCallContext, number, &selector, &address, &segReg));

    switch(pCallContext->m_result.linkedOperands[number].usedSize)
    {
    default:
        Diana_FatalBreak();
        return DI_ERROR;
    case 4:
        {
            float32 ft32 = 0;
            DI_CHECK(DianaProcessor_ReadMemory(pCallContext, 
                                           selector,
                                           address,
                                           &ft32,
                                           4,
                                           &doneBytes,
                                           0,
                                           segReg));


            *result = float32_to_floatx80(ft32, &status);
        }
        break;
    case 8:
        {
            float64 ft64 = 0;
            DI_CHECK(DianaProcessor_ReadMemory(pCallContext, 
                                           selector,
                                           address,
                                           &ft64,
                                           8,
                                           &doneBytes,
                                           0,
                                           segReg));
            *result = float64_to_floatx80(ft64, &status);
        }  
        break;
    case 10:
        DI_CHECK(DianaProcessor_ReadMemory(pCallContext, 
                                       selector,
                                       address,
                                       result,
                                       10,
                                       &doneBytes,
                                       0,
                                       segReg));
        break;
    }
    return DI_SUCCESS;
}
// commands
int Diana_Call_wait(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    DI_FPU_START
    DI_PROC_END
}
int Diana_Call_fnstsw(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext)
{
#ifdef DIANA_PROCESSOR_USE_SOFTFLOAT_FPU
    DI_FPU_START
    {
    
        DI_UINT16  statusWord = Diana_FPU_QueryStatusWord(pCallContext);
        OPERAND_SIZE doneBytes = 0;
        DI_CHECK(Diana_WriteRawBufferToArgMem(pDianaContext, 
                                             pCallContext, 
                                             0, 
                                             &statusWord, 
                                             sizeof(statusWord), 
                                             &doneBytes, 
                                             0));
    }
    DI_PROC_END
#else
  return DI_UNSUPPORTED_COMMAND;
#endif

}

int Diana_Call_fnstcw(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext)
{
#ifdef DIANA_PROCESSOR_USE_SOFTFLOAT_FPU
    DI_FPU_START
    {
        DI_UINT16  controlWord  = pCallContext->m_fpu.controlWord;
        OPERAND_SIZE doneBytes = 0;
        DI_CHECK(Diana_WriteRawBufferToArgMem(pDianaContext, 
                                             pCallContext, 
                                             0, 
                                             &controlWord, 
                                             sizeof(controlWord), 
                                             &doneBytes, 
                                             0));

    }
    DI_PROC_END
#else
    return DI_UNSUPPORTED_COMMAND;
#endif

}
int Diana_Call_fnclex(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext)
{
#ifdef DIANA_PROCESSOR_USE_SOFTFLOAT_FPU
    DI_FPU_START_IGNORE_EXCEPTIONS
    {
        pCallContext->m_fpu.statusWord &= 0x7F00;
    }
    DI_PROC_END
#else
    return DI_UNSUPPORTED_COMMAND;
#endif
}
int Diana_Call_fldcw(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext)
{
#ifdef DIANA_PROCESSOR_USE_SOFTFLOAT_FPU
    DI_FPU_START
    {
        DI_UINT16  controlWord  = 0;
        OPERAND_SIZE doneBytes = 0;
        DI_CHECK(Diana_ReadRawBufferFromArgMem(pDianaContext, 
                                             pCallContext, 
                                             0, 
                                             &controlWord, 
                                             sizeof(controlWord), 
                                             &doneBytes, 
                                             0));

        pCallContext->m_fpu.controlWord = (controlWord &  DI_FPU_CW_RESERVED_BITS) | DI_FPU_CW_RESERVED_40;

        
        if (pCallContext->m_fpu.statusWord & ~controlWord & DI_FPU_CW_ALL_EXCEPTIONS)
        {
            pCallContext->m_fpu.statusWord |= DI_FPU_SW_SUMMARY;
            pCallContext->m_fpu.statusWord |= DI_FPU_SW_BACKWARD;
        }
        else
        {
            pCallContext->m_fpu.statusWord &= ~(DI_FPU_SW_SUMMARY|DI_FPU_SW_BACKWARD);
        }
    }
    DI_PROC_END
#else
    return DI_UNSUPPORTED_COMMAND;
#endif
}

int Diana_Call_fild(struct _dianaContext * pDianaContext,
                   DianaProcessor * pCallContext)
{
#ifdef DIANA_PROCESSOR_USE_SOFTFLOAT_FPU
    DI_FPU_START
    {
        floatx80_t result;
        DI_DEF_LOCAL_1(src)
        DI_MEM_GET_DEST(src)

        DI_FPU_CLEAR_C1;
        if (DI_FPU_REG_IS_EMPTY(-1)) 
        {
            result = int64_to_floatx80(src);
            Diana_FPU_Push(pCallContext);
            SET_REG_FPU_ST0(&result);
        }
        else 
        {
            DI_FPU_Overflow(pCallContext);
        }
    }
    DI_PROC_END
#else
    return DI_UNSUPPORTED_COMMAND;
#endif
}

// read float argument
int Diana_Call_fld(struct _dianaContext * pDianaContext,
                   DianaProcessor * pCallContext)
{
#ifdef DIANA_PROCESSOR_USE_SOFTFLOAT_FPU
    DI_FPU_START
    {
        floatx80_t result;

        if (pCallContext->m_result.linkedOperands->type == diana_register)
        {
            // st(i) version
            DianaProcessor_FPU_GetValueEx(pCallContext, DianaProcessor_FPU_QueryReg(pCallContext, pCallContext->m_result.linkedOperands->value.recognizedRegister), &result);
        }
        else
        {
            DI_CHECK(Diana_FPU_ReadFloatArgument(pDianaContext, pCallContext, 0, &result));
        }
       
        DI_FPU_CLEAR_C1;
        if (DI_FPU_REG_IS_EMPTY(-1)) 
        {
            Diana_FPU_Push(pCallContext);
            SET_REG_FPU_ST0(&result);
        }
        else 
        {
            DI_FPU_Overflow(pCallContext);
        }
    }
    DI_PROC_END
#else
    return DI_UNSUPPORTED_COMMAND;
#endif
}

// fst
static
int Diana_Call_fstp_common(struct _dianaContext * pDianaContext,
                           DianaProcessor * pCallContext,
                           int pop)
{
#ifdef DIANA_PROCESSOR_USE_SOFTFLOAT_FPU
    DI_FPU_START
    {
        floatx80_t result = floatx80_default_nan;
        if (DI_FPU_REG_IS_EMPTY(0)) 
        {
            DI_FPU_ProcessException(pCallContext, DI_FPU_EX_STACK_UNDERFLOW);
        }
        else
        {
            GET_REG_FPU_ST0(&result);
        }

        if (pCallContext->m_result.linkedOperands->type == diana_register)
        {
            // st version
            DianaProcessor_FPU_SetValueEx(pCallContext, DianaProcessor_FPU_QueryReg(pCallContext, pCallContext->m_result.linkedOperands->value.recognizedRegister), &result);
        }
        else
        {
            // mem version
            DI_CHECK(Diana_FPU_WriteFloatArgument(pDianaContext, pCallContext, 0, &result));
        }
        if (pop)
        {
            Diana_FPU_Pop(pCallContext);
        }
    }
    DI_PROC_END;
#else
    return DI_UNSUPPORTED_COMMAND;
#endif
}
int Diana_Call_fstp(struct _dianaContext * pDianaContext,
                   DianaProcessor * pCallContext)
{
    return Diana_Call_fstp_common(pDianaContext,  pCallContext, 1);
}
int Diana_Call_fst(struct _dianaContext * pDianaContext,
                   DianaProcessor * pCallContext)
{
    return Diana_Call_fstp_common(pDianaContext,  pCallContext, 0);
}
// done fst

// fist
static
int Diana_Call_fistp_common(struct _dianaContext * pDianaContext,
                           DianaProcessor * pCallContext,
                           int pop)
{
#ifdef DIANA_PROCESSOR_USE_SOFTFLOAT_FPU
    DI_FPU_START
    {
        float_status_t status;
        floatx80_t result = floatx80_default_nan;
        DI_DEF_LOCAL_1(dest)
        DI_MEM_GET_DEST(dest)
        
        if (DI_FPU_REG_IS_EMPTY(0)) 
        {
            DI_FPU_ProcessException(pCallContext, DI_FPU_EX_STACK_UNDERFLOW);
        }
        else
        {
            GET_REG_FPU_ST0(&result);
        }

        dest = floatx80_to_int64(result, &status);
        DI_MEM_SET_DEST(dest)

        if (pop)
        {
            Diana_FPU_Pop(pCallContext);
        }
    }
    DI_PROC_END;
#else
    return DI_UNSUPPORTED_COMMAND;
#endif
}
int Diana_Call_fistp(struct _dianaContext * pDianaContext,
                   DianaProcessor * pCallContext)
{
    return Diana_Call_fistp_common(pDianaContext,  pCallContext, 1);
}
int Diana_Call_fist(struct _dianaContext * pDianaContext,
                   DianaProcessor * pCallContext)
{
    return Diana_Call_fistp_common(pDianaContext,  pCallContext, 0);
}
// done fst

// sub
//FSUB m32fp | D8 /4
//FSUB m64fp | DC /4
//FSUB ST(0), ST(i) | D8 E0 + fpu_i
//FSUB ST(i), ST(0) | DC E8 + fpu_i
//FSUBP ST(i), ST(0) | DE E8 + fpu_i

//FSUBR m32fp | D8 /5
//FSUBR m64fp | DC /5
//FSUBR ST(0), ST(i) | D8 E8 + fpu_i
//FSUBR ST(i), ST(0) | DC E0 + fpu_i
//FSUBRP ST(i), ST(0) | DE E0 + fpu_i

//FISUB m32int | DA /4
//FISUB m16int | DE /4
//FISUBR m32int | DA /5
//FISUBR m16int | DE /5
static
int Diana_Call_fsub_common(struct _dianaContext * pDianaContext,
                           DianaProcessor * pCallContext,
                           int pop,
                           int reverse,
                           int add_cmd)
{
#ifdef DIANA_PROCESSOR_USE_SOFTFLOAT_FPU
    DI_FPU_START
    {
        floatx80_t a, b;
        int bStstVersion = 0;

        if (pCallContext->m_result.linkedOperands->type == diana_register)
        {
            // st(i) version
            bStstVersion = 1;
            DianaProcessor_FPU_GetValueEx(pCallContext, DianaProcessor_FPU_QueryReg(pCallContext, pCallContext->m_result.linkedOperands[0].value.recognizedRegister), &a);
            DianaProcessor_FPU_GetValueEx(pCallContext, DianaProcessor_FPU_QueryReg(pCallContext, pCallContext->m_result.linkedOperands[1].value.recognizedRegister), &b);

        }
        else
        {
            GET_REG_FPU_ST0(&a);
            DI_CHECK(Diana_FPU_ReadFloatArgument(pDianaContext, pCallContext, 0, &b));
        }
       
        DI_FPU_CLEAR_C1;
  
        {
            floatx80_t result = {0};
            float_status_t status = FPU_pre_exception_handling(pCallContext);
            if (reverse)
            {
                if (add_cmd)
                {
                    result = floatx80_add(b, a, &status);
                }
                else
                {
                    result = floatx80_sub(b, a, &status);
                }
            }
            else
            {
                if (add_cmd)
                {
                    result = floatx80_add(a, b, &status);
                }
                else
                {
                    result = floatx80_sub(a, b, &status);
                }
            }
           
            if (!DI_FPU_ProcessException(pCallContext, status.float_exception_flags))
            {
                if (bStstVersion)
                {
                    DianaProcessor_FPU_SetValueEx(pCallContext, DianaProcessor_FPU_QueryReg(pCallContext, pCallContext->m_result.linkedOperands[0].value.recognizedRegister), &result);
                }
                else
                {
                    SET_REG_FPU_ST0(&result);
                }
                if (pop)
                {
                    Diana_FPU_Pop(pCallContext);
                }
            }
        }
    }
    DI_PROC_END
#else
    return DI_UNSUPPORTED_COMMAND;
#endif
}
int Diana_Call_fsub(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext)
{
    return Diana_Call_fsub_common(pDianaContext, pCallContext, 0, 0, 0);
}
int Diana_Call_fsubp(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext)
{
    return Diana_Call_fsub_common(pDianaContext, pCallContext, 1, 0, 0);
}

int Diana_Call_fsubr(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext)
{
    return Diana_Call_fsub_common(pDianaContext, pCallContext, 0, 1, 0);
}
int Diana_Call_fsubrp(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext)
{
    return Diana_Call_fsub_common(pDianaContext, pCallContext, 1, 1, 0);
}

static
int Diana_Call_common_fisub(struct _dianaContext * pDianaContext,
                            DianaProcessor * pCallContext,
                            int reverse,
                            int add_cmd)
{
#ifdef DIANA_PROCESSOR_USE_SOFTFLOAT_FPU
    DI_FPU_START
    {
        floatx80_t st0 = floatx80_default_nan;
        DI_DEF_LOCAL_1(src)
        DI_MEM_GET_DEST(src)
        
        if (DI_FPU_REG_IS_EMPTY(0)) 
        {
            DI_FPU_ProcessException(pCallContext, DI_FPU_EX_STACK_UNDERFLOW);
        }
        else
        {
            GET_REG_FPU_ST0(&st0);
        }

        {
            floatx80_t arg = {0}, result = {0};
            float_status_t status = FPU_pre_exception_handling(pCallContext);

            arg = int64_to_floatx80(src);
            if (reverse)
            {
                if (add_cmd)
                {
                    result = floatx80_add(arg, st0, &status);
                }
                else
                {
                    result = floatx80_sub(arg, st0, &status);
                }
            }
            else
            {
                if (add_cmd)
                {
                    result = floatx80_add(st0, arg, &status);
                }
                else
                {
                    result = floatx80_sub(st0, arg, &status);
                }
            }
           
            if (!DI_FPU_ProcessException(pCallContext, status.float_exception_flags))
            {
                SET_REG_FPU_ST0(&result);
            }
        }
    }
    DI_PROC_END;
#else
    return DI_UNSUPPORTED_COMMAND;
#endif
}

int Diana_Call_fisub(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext)
{
    return Diana_Call_common_fisub(pDianaContext, pCallContext, 0, 0);
}
int Diana_Call_fisubr(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext)
{
    return Diana_Call_common_fisub(pDianaContext, pCallContext, 1, 0);
}
// done sub
// mul
// FMUL m32fp | D8 /1
// FMUL m64fp | DC /1
// FMUL ST(0), ST(i) | D8 C8 + fpu_i
// FMUL ST(i), ST(0) | DC C8 + fpu_i
// FMULP ST(i), ST(0) | DE C8 + fpu_i

static
int Diana_Call_fmul_common(struct _dianaContext * pDianaContext,
                           DianaProcessor * pCallContext,
                           int pop)
{
#ifdef DIANA_PROCESSOR_USE_SOFTFLOAT_FPU
    DI_FPU_START
    {
        floatx80_t a, b;
        int bStstVersion = 0;

        if (pCallContext->m_result.linkedOperands->type == diana_register)
        {
            // st(i) version
            bStstVersion = 1;
            DianaProcessor_FPU_GetValueEx(pCallContext, DianaProcessor_FPU_QueryReg(pCallContext, pCallContext->m_result.linkedOperands[0].value.recognizedRegister), &a);
            DianaProcessor_FPU_GetValueEx(pCallContext, DianaProcessor_FPU_QueryReg(pCallContext, pCallContext->m_result.linkedOperands[1].value.recognizedRegister), &b);

        }
        else
        {
            GET_REG_FPU_ST0(&a);
            DI_CHECK(Diana_FPU_ReadFloatArgument(pDianaContext, pCallContext, 0, &b));
        }
       
        DI_FPU_CLEAR_C1;
  
        {
            floatx80_t result = {0};
            float_status_t status = FPU_pre_exception_handling(pCallContext);

            result = floatx80_mul(a, b, &status);
           
            if (!DI_FPU_ProcessException(pCallContext, status.float_exception_flags))
            {
                if (bStstVersion)
                {
                    DianaProcessor_FPU_SetValueEx(pCallContext, DianaProcessor_FPU_QueryReg(pCallContext, pCallContext->m_result.linkedOperands[0].value.recognizedRegister), &result);
                }
                else
                {
                    SET_REG_FPU_ST0(&result);
                }
                if (pop)
                {
                    Diana_FPU_Pop(pCallContext);
                }
            }
        }
    }
    DI_PROC_END
#else
    return DI_UNSUPPORTED_COMMAND;
#endif
}
int Diana_Call_fmulp(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext)
{
    return Diana_Call_fmul_common(pDianaContext, pCallContext, 1);
}

int Diana_Call_fmul(struct _dianaContext * pDianaContext,
                   DianaProcessor * pCallContext)
{
    return Diana_Call_fmul_common(pDianaContext, pCallContext, 0);
}
// done mul
// fadd
int Diana_Call_fadd(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext)
{
    return Diana_Call_fsub_common(pDianaContext, pCallContext, 0, 0, 1);
}
int Diana_Call_faddp(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext)
{
    return Diana_Call_fsub_common(pDianaContext, pCallContext, 1, 0, 1);
}
int Diana_Call_fiadd(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext)
{
    return Diana_Call_common_fisub(pDianaContext, pCallContext, 0, 1);
}
// fcom

static
int Diana_Call_common_fcom(struct _dianaContext * pDianaContext,
                           DianaProcessor * pCallContext,
                           int popCount)
{
    &pDianaContext;
    &pCallContext;
    &popCount;
    return DI_UNSUPPORTED_COMMAND;
}

int Diana_Call_fcom(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    return Diana_Call_common_fcom(pDianaContext, pCallContext, 0);
}
int Diana_Call_fcomp(struct _dianaContext * pDianaContext,
                   DianaProcessor * pCallContext)
{
    return Diana_Call_common_fcom(pDianaContext, pCallContext, 1);
}
int Diana_Call_fcompp(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext)
{
    return Diana_Call_common_fcom(pDianaContext, pCallContext, 2);
}
// done fcomp
int Diana_Call_fsqrt(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext)
{
    &pDianaContext;
    &pCallContext;
    return DI_UNSUPPORTED_COMMAND;
}




