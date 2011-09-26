#include "diana_processor_cmd_b.h"
#include "diana_proc_gen.h"
#include "diana_gen.h"
#include "diana_core_gen_tags.h"


int Diana_Call_bsf(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    //IF r/m = 0
    //THEN
    //   ZF := 1;
    //   register := UNDEFINED;
    //ELSE
    //   temp := 0;
    //   ZF := 0;
    //   WHILE BIT[r/m, temp = 0]
    //   DO
    //      temp := temp + 1;
    //      register := temp;
    //   OD;
    //FI; 

    DI_DEF_LOCALS(src, dest);
        
    DI_MEM_GET_DEST(dest);
    DI_MEM_GET_SRC(src);

    CLEAR_FLAG_ZF;
    if (!src)
    {
        SET_FLAG_ZF;
    }
    else
    {
        int i = 0;
        OPERAND_SIZE mask = 1;
        
        for(i = 0; i<64 && !(mask & src); ++i, mask<<=1)
        {
        }
        dest = i;
        DI_MEM_SET_DEST(dest);
    }
    DI_PROC_END;
}

int Diana_Call_bsr(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    //IF r/m = 0
    //THEN
    //   ZF := 1;
    //   register := UNDEFINED;
    //ELSE
    //   temp := OperandSize - 1;
    //   ZF := 0;
    //   WHILE BIT[r/m, temp] = 0
    //   DO
    //      temp := temp - 1;
    //      register := temp;
    //   OD;
    //FI; 
    DI_DEF_LOCALS(src, dest);
        
    DI_MEM_GET_DEST(dest);
    DI_MEM_GET_SRC(src);

    CLEAR_FLAG_ZF;
    if (!src)
    {
        SET_FLAG_ZF;
    }
    else
    {
        int i = 0;
        OPERAND_SIZE mask = 0x8000000000000000ULL;
        
        for(i = 63; i>=0 && !(mask & src); --i, mask>>=1)
        {
        }
        dest = i;
        DI_MEM_SET_DEST(dest);
    }
    DI_PROC_END;
}

int Diana_Call_bt(struct _dianaContext * pDianaContext,
                  DianaProcessor * pCallContext)
{
    // CF := BIT[LeftSRC, RightSRC]; 
    OPERAND_SIZE temp = 0;
    DI_DEF_LOCALS(src, dest);
        
    DI_MEM_GET_DEST(dest);
    DI_MEM_GET_SRC(src);

    CLEAR_FLAG_CF;

    temp = DianaProcessor_GetSignMaskSpecifyBit(src);

    if (dest & temp)
    {
        SET_FLAG_CF;
    }
    DI_PROC_END;
}

int Diana_Call_btc(struct _dianaContext * pDianaContext,
                   DianaProcessor * pCallContext)
{
    //CF := BIT[LeftSRC, RightSRC];
    //BIT[LeftSRC, RightSRC] := NOT BIT[LeftSRC, RightSRC]; 

    OPERAND_SIZE temp = 0;
    DI_DEF_LOCALS(src, dest);
        
    DI_MEM_GET_DEST(dest);
    DI_MEM_GET_SRC(src);

    CLEAR_FLAG_CF;

    temp = DianaProcessor_GetSignMaskSpecifyBit(src);

    if (dest & temp)
    {
        SET_FLAG_CF;
        dest = dest & ~temp;
    }
    else
    {
        dest |= temp;
    }
    DI_MEM_SET_DEST(dest);
    DI_PROC_END;
}

int Diana_Call_btr(struct _dianaContext * pDianaContext,
                   DianaProcessor * pCallContext)
{
    //CF := BIT[LeftSRC, RightSRC];
    //BIT[LeftSRC, RightSRC] := 0; 

    OPERAND_SIZE temp = 0;
    DI_DEF_LOCALS(src, dest);
        
    DI_MEM_GET_DEST(dest);
    DI_MEM_GET_SRC(src);

    CLEAR_FLAG_CF;

    temp = DianaProcessor_GetSignMaskSpecifyBit(src);

    if (dest & temp)
    {
        SET_FLAG_CF;
        dest = dest & ~temp;
    }
    DI_MEM_SET_DEST(dest);
    DI_PROC_END;
}

int Diana_Call_bts(struct _dianaContext * pDianaContext,
                   DianaProcessor * pCallContext)
{
    //CF := BIT[LeftSRC, RightSRC];
    //BIT[LeftSRC, RightSRC] := 0; 

    OPERAND_SIZE temp = 0;
    DI_DEF_LOCALS(src, dest);
        
    DI_MEM_GET_DEST(dest);
    DI_MEM_GET_SRC(src);

    CLEAR_FLAG_CF;

    temp = DianaProcessor_GetSignMaskSpecifyBit(src);

    if (dest & temp)
    {
        SET_FLAG_CF;
    }
    else
    {
        dest |= temp;
    }
    DI_MEM_SET_DEST(dest);
    DI_PROC_END;
}