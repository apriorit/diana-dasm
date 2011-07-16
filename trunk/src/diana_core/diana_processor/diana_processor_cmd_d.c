#include "diana_processor_cmd_d.h"
#include "diana_proc_gen.h"
#include "diana_gen.h"
#include "diana_core_gen_tags.h"


int Diana_Call_daa(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    //IF ((AL AND 0FH) > 9) OR (AF = 1)
    //THEN
    //   AL := AL + 6;
    //   AF := 1;
    //ELSE
    //   AF := 0;
    //FI;
    //IF (AL > 9FH) OR (CF = 1)
    //THEN
    //   AL := AL + 60H;
    //   CF := 1;
    //ELSE CF := 0;
    //FI; 

    unsigned char al = 0;

    al = (unsigned char)GET_REG_AL;

    if ( ((al & 0x0F) > 9) || GET_FLAG_AF )
    {
       al = al + 6;
       SET_FLAG_AF;
    }
    else
    {
       CLEAR_FLAG_AF;
    }
    
    if ( (al > 0x9F) || GET_FLAG_CF )
    {
       al = al + 0x60;
       SET_FLAG_CF;
    }
    else
    {
        CLEAR_FLAG_CF;
    }
    DI_UPDATE_FLAGS_PSZ(SET_REG_AL(al));
    DI_PROC_END;
}

int Diana_Call_das(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    //IF (AL AND 0FH) > 9 OR AF = 1
    //THEN
    //   AL := AL - 6;
    //   AF := 1;
    //ELSE
    //   AF := 0;
    //FI;
    //IF (AL > 9FH) OR (CF = 1)
    //THEN
    //   AL := AL - 60H;
    //   CF := 1;
    //ELSE CF := 0;
    //FI; 
    unsigned char al = 0;
    al = (unsigned char)GET_REG_AL;

    if ( (al & 0x0F) > 9 || GET_FLAG_AF )
    {
        al = al - 6;
        SET_FLAG_AF;
    }
    else
    {
        CLEAR_FLAG_AF;
    }

    if ( (al > 0x9F) || GET_FLAG_CF )
    {
        al = al - 0x60;
        SET_FLAG_CF;
    }
    else
    {
        CLEAR_FLAG_CF;
    }
    DI_UPDATE_FLAGS_PSZ(SET_REG_AL(al));
    DI_PROC_END;
}

int Diana_Call_dec(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    //DEST := DEST + SRC;
    DI_DEF_LOCALS(src, dest);
        
    DI_MEM_GET_DEST(dest);

    src = 1;

    // update AF, OF
    DI_START_UPDATE_OA_FLAGS(dest);

    dest = dest - src;

    DI_END_UPDATE_OA_FLAGS_SUB(dest, src);

    DI_UPDATE_FLAGS_PSZ(DI_MEM_SET_DEST(dest));
    DI_PROC_END
}

static
int Diana_Call_div8(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext,
                    OPERAND_SIZE * pDivisor)
{
    //byte    AX           r/m8       AL          AH
    DI_UINT16 src = (DI_UINT16)GET_REG_AX;
    DI_UINT16 quotient = 0;
    unsigned char remainder = 0,
                  divisor = (unsigned char)*pDivisor;

    if (divisor == 0)
        return DI_DIVISION_BY_ZERO;

    quotient = src / divisor;
    remainder = src % divisor;

    if (quotient > 0xFF)
        return DI_DIVISION_BY_ZERO;

    SET_REG_AL(quotient);
    SET_REG_AH(remainder);
    return DI_SUCCESS;
}

static
int Diana_Call_div16(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext,
                    OPERAND_SIZE * pDivisor)
{
    //word    DX:AX        r/m16      AX          DX
    DianaRegisterValue32_type src;
    DI_UINT32 quotient = 0; 
    DI_UINT16 remainder = 0,
               divisor = (DI_UINT16)*pDivisor;

    if (divisor == 0)
        return DI_DIVISION_BY_ZERO;

    src.h       = (DI_UINT16)GET_REG_DX;
    src.l.value = (DI_UINT16)GET_REG_AX;

    quotient = src.value / divisor;
    remainder = src.value % divisor;

    if (quotient > 0xFFFF)
        return DI_DIVISION_OVERFLOW;

    SET_REG_AX(quotient);
    SET_REG_DX(remainder);
    return DI_SUCCESS;
}

static
int Diana_Call_div32(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext,
                    OPERAND_SIZE * pDivisor)
{
    //dword   EDX:EAX      r/m32      EAX         EDX 
    DianaRegisterValue_type src;
    DI_UINT64  quotient = 0;
    DI_UINT32  remainder = 0,
               divisor = (DI_UINT32)*pDivisor;

    if (divisor == 0)
        return DI_DIVISION_BY_ZERO;

    src.h       = (DI_UINT32)GET_REG_EDX;
    src.l.value = (DI_UINT32)GET_REG_EAX;


    quotient = src.value / divisor;
    remainder = src.value % divisor;

    if (quotient > 0xFFFFFFFFULL)
        return DI_DIVISION_OVERFLOW;

    SET_REG_EAX(quotient);
    SET_REG_EDX(remainder);
    return DI_SUCCESS;
}

static
int Diana_Call_div64(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext,
                    OPERAND_SIZE * pDivisor)
{
    //qword   RDX:RAX      r/m64      RAX         RDX  
    OPERAND_SIZE rax = GET_REG_RAX;
    DI_UINT64  quotient = 0,
               remainder = 0,
               divisor = *pDivisor;

    if (GET_REG_RDX)
    {
        // this is so sad, but C doesnt have appropriate 
        // intrumentary to do this
        // __int128 is available only on amd64 platforms
        return DI_UNSUPPORTED_COMMAND;
    }

    if (divisor == 0)
        return DI_DIVISION_BY_ZERO;

    quotient = rax / divisor;
    remainder = rax % divisor;

    SET_REG_RAX(quotient);
    SET_REG_RDX(remainder);
    return DI_SUCCESS;
}

int Diana_Call_div(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    //temp := dividend / divisor;
    //IF temp does not fit in quotient
    //THEN Interrupt 0;
    //ELSE
    //   quotient := temp;
    //   remainder := dividend MOD (r/m);
    //FI; 
    //Size    Dividend     Divisor   Quotient   Remainder
    //byte    AX           r/m8       AL          AH
    //word    DX:AX        r/m16      AX          DX
    //dword   EDX:EAX      r/m32      EAX         EDX 
    //qword   RDX:RAX      r/m64      RAX         RDX     
    DI_DEF_LOCAL(divisor);
    DI_MEM_GET_SRC(divisor);

    switch(pCallContext->m_result.linkedOperands->usedSize)
    {
    case DIANA_MODE64:
        DI_CHECK(Diana_Call_div64(pDianaContext, pCallContext, &divisor));
        break;

    case DIANA_MODE32:
        DI_CHECK(Diana_Call_div32(pDianaContext, pCallContext, &divisor));
        break;

    case DIANA_MODE16:
        DI_CHECK(Diana_Call_div16(pDianaContext, pCallContext, &divisor));
        break;

    case 1:
        DI_CHECK(Diana_Call_div8(pDianaContext, pCallContext, &divisor));
        break;

    default:
        Diana_DebugFatalBreak();
        return DI_ERROR;
    }

    DI_PROC_END
}
