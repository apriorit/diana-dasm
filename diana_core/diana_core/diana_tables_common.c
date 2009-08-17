#include "diana_tables_common.h"

typedef struct _dianaRM16_4
{
    DianaUnifiedRegister regs[3];
}DianaRM16_4;


static DianaRM16_4 table2[8]= 
{
    {{reg_EAX, reg_AX, reg_AL}},
    {{reg_ECX, reg_CX, reg_CL}},
    {{reg_EDX, reg_DX, reg_DL}},
    {{reg_EBX, reg_BX, reg_BL}},
    {{reg_ESP, reg_SP, reg_AH}},
    {{reg_EBP, reg_BP, reg_CH}},
    {{reg_ESI, reg_SI, reg_DH}},
    {{reg_EDI, reg_DI, reg_BH}}
};


int Diana_DispatchMod3(int rm,
                       int iRegSizeInBytes,
                       DianaUnifiedRegister *pOut)
{
    int iIndex = 0;
    switch (iRegSizeInBytes)
    {
    default:
        return DI_ERROR;

    case 4:
        iIndex = 0;
        break;
    case 2:
        iIndex = 1;
        break;
    case 1: 
        iIndex = 2;
    };
    *pOut = table2[rm].regs[iIndex];
    return DI_SUCCESS;
}
