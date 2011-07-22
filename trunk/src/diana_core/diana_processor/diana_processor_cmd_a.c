#include "diana_processor_cmd_a.h"
#include "diana_proc_gen.h"
#include "diana_gen.h"
#include "diana_core_gen_tags.h"


int Diana_Call_aaa(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
  
    //IF ((AL AND 0FH) > 9) OR (AF = 1)
    //THEN
    //AL := (AL + 6) AND 0FH;
    //AH := AH + 1;
    //AF := 1;
    //CF := 1;
    //ELSE
    //CF := 0;
    //AF := 0;
    //FI; 

    if (((GET_REG_AL & 0x0F) > 9) || (GET_FLAG_AF == 1))
    {
        SET_REG_AL((GET_REG_AL + 6) & 0x0F);
        SET_REG_AH(GET_REG_AH + 1);
        SET_FLAG_AF;
        SET_FLAG_CF;
    }
    else
    {
        CLEAR_FLAG_AF;
        CLEAR_FLAG_CF;
    }
    DI_PROC_END;
}

int Diana_Call_aad(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    //AL := AH * 10 + AL;
    //AH := 0;
    DianaRegisterValue16_type ax;

    ax.value = (unsigned int)GET_REG_AX;

    ax.l = ax.h*10 + ax.l;
    ax.h = 0;

    DI_UPDATE_FLAGS_PSZ(SET_REG_AX(ax.value));
    DI_PROC_END;
}

int Diana_Call_aam(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    //AH := AL / 10;
    //AL := AL MOD 10;
    DianaRegisterValue16_type ax;

    ax.value = (unsigned int)GET_REG_AX;

    ax.h = ax.l / 10;
    ax.l = ax.l % 10;

    DI_UPDATE_FLAGS_PSZ(SET_REG_AX(ax.value));
    DI_PROC_END;
}

int Diana_Call_aas(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    //IF (AL AND 0FH) > 9 OR AF = 1
    //THEN
    //    AL := AL - 6;
    //    AL := AL AND 0FH;
    //    AH := AH - 1;
    //    AF := 1;
    //    CF := 1;
    //ELSE
    //    CF := 0;
    //    AF := 0;
    //FI;

    if ((GET_REG_AL & 0xF) > 9 || GET_FLAG_AF)
    {
        SET_REG_AL(GET_REG_AL - 6);
        SET_REG_AL(GET_REG_AL & 0xF);
        SET_REG_AH(GET_REG_AH - 1);

        SET_FLAG_AF;
        SET_FLAG_CF;
    }
    else
    {
        CLEAR_FLAG_AF;
        CLEAR_FLAG_CF;
    }
    DI_PROC_END;
}

    
int Diana_Call_adc(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{    
    //DEST := DEST + SRC + CF;
    int cfValue = 0;
    DI_DEF_LOCALS(src, dest);

    DI_MEM_GET_DEST(dest);
    DI_MEM_GET_SRC(src);

    DI_SIGN_EXTEND(src, DI_VAR_SIZE(dest));

    cfValue = GET_FLAG_CF;
    DI_START_UPDATE_COA_FLAGS(dest);
    dest += src + cfValue;
    DI_END_UPDATE_COA_FLAGS_ADD(dest, src);

    DI_UPDATE_FLAGS_PSZ(DI_MEM_SET_DEST(dest));
    DI_PROC_END
}

int Diana_Call_add(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    //DEST := DEST + SRC;
    DI_DEF_LOCALS(src, dest);
        
    DI_MEM_GET_DEST(dest);
    DI_MEM_GET_SRC(src);

    DI_SIGN_EXTEND(src, DI_VAR_SIZE(dest));
    // update AF, OF, CF
    DI_START_UPDATE_COA_FLAGS(dest);

    dest += src;

    DI_END_UPDATE_COA_FLAGS_ADD(dest, src);

    DI_UPDATE_FLAGS_PSZ(DI_MEM_SET_DEST(dest));
    DI_PROC_END
}

int Diana_Call_and(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    //DEST := DEST AND SRC;
    //CF := 0;
    //OF := 0;

    DI_DEF_LOCALS(src, dest);

    DI_MEM_GET_DEST(dest);
    DI_MEM_GET_SRC(src);

    DI_SIGN_EXTEND(src, DI_VAR_SIZE(dest));

    dest = dest & src;
    CLEAR_FLAG_CF;
    CLEAR_FLAG_OF;

    DI_UPDATE_FLAGS_PSZ(DI_MEM_SET_DEST(dest));
    DI_PROC_END
}

