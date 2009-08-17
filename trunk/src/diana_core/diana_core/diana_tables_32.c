#include "diana_tables_32.h"
#include "diana_tables_common.h"

// 32 bit
//[EAX]                000   00    08    10    18    20    28    30    38
//[ECX]                001   01    09    11    19    21    29    31    39
//[EDX]                010   02    0A    12    1A    22    2A    32    3A
//[EBX]                011   03    0B    13    1B    23    2B    33    3B
//[--] [--]        00  100   04    0C    14    1C    24    2C    34    3C
//disp32               101   05    0D    15    1D    25    2D    35    3D
//[ESI]                110   06    0E    16    1E    26    2E    36    3E
//[EDI]                111   07    0F    17    1F    27    2F    37    3F
//
//disp8[EAX]           000   40    48    50    58    60    68    70    78
//disp8[ECX]           001   41    49    51    59    61    69    71    79
//disp8[EDX]           010   42    4A    52    5A    62    6A    72    7A
//disp8[EPX];          011   43    4B    53    5B    63    6B    73    7B
//disp8[--] [--]   01  100   44    4C    54    5C    64    6C    74    7C
//disp8[ebp]           101   45    4D    55    5D    65    6D    75    7D
//disp8[ESI]           110   46    4E    56    5E    66    6E    76    7E
//disp8[EDI]           111   47    4F    57    5F    67    6F    77    7F
//
//disp32[EAX]          000   80    88    90    98    A0    A8    B0    B8
//disp32[ECX]          001   81    89    91    99    A1    A9    B1    B9
//disp32[EDX]          010   82    8A    92    9A    A2    AA    B2    BA
//disp32[EBX]          011   83    8B    93    9B    A3    AB    B3    BB
//disp32[--] [--]  10  100   84    8C    94    9C    A4    AC    B4    BC
//disp32[EBP]          101   85    8D    95    9D    A5    AD    B5    BD
//disp32[ESI]          110   86    8E    96    9E    A6    AE    B6    BE
//disp32[EDI]          111   87    8F    97    9F    A7    AF    B7    BF
//
//EAX/AX/AL            000   C0    C8    D0    D8    E0    E8    F0    F8
//ECX/CX/CL            001   C1    C9    D1    D9    E1    E9    F1    F9
//EDX/DX/DL            010   C2    CA    D2    DA    E2    EA    F2    FA
//EBX/BX/BL            011   C3    CB    D3    DB    E3    EB    F3    FB
//ESP/SP/AH        11  100   C4    CC    D4    DC    E4    EC    F4    FC
//EBP/BP/CH            101   C5    CD    D5    DD    E5    ED    F5    FD
//ESI/SI/DH            110   C6    CE    D6    DE    E6    EE    F6    FE
//EDI/DI/BH            111   C7    CF    D7    DF    E7    EF    F7    FF

typedef struct _DianaEntry32
{
    int iHasSIB;
    DianaRmIndex index;
}DianaEntry32;

static DianaEntry32 g_table1[3][8] =
{
    {
        {0, {reg_none, reg_EAX, reg_none, 0, 0, 0}},
        {0, {reg_none, reg_ECX, reg_none, 0, 0, 0}},
        {0, {reg_none, reg_EDX, reg_none, 0, 0, 0}},
        {0, {reg_none, reg_EBX, reg_none, 0, 0, 0}},
        {1, {reg_none, reg_none, reg_none, 0, 0, 0}}, // SIB
        {0, {reg_none, reg_none, reg_none, 0, 4, 0}},
        {0, {reg_none, reg_ESI, reg_none, 0, 0, 0}},
        {0, {reg_none, reg_EDI, reg_none, 0, 0, 0}}
    },
    {
        {0, {reg_none, reg_EAX, reg_none, 0, 1, 0}},
        {0, {reg_none, reg_ECX, reg_none, 0, 1, 0}},
        {0, {reg_none, reg_EDX, reg_none, 0, 1, 0}},
        {0, {reg_none, reg_EBX, reg_none, 0, 1, 0}},
        {1, {reg_none, reg_none, reg_none, 0, 1, 0}}, // SIB
        {0, {reg_none, reg_EBP, reg_none, 0, 1, 0}},
        {0, {reg_none, reg_ESI, reg_none, 0, 1, 0}},
        {0, {reg_none, reg_EDI, reg_none, 0, 1, 0}}
    },
    {
        {0, {reg_none, reg_EAX, reg_none, 0, 4, 0}},
        {0, {reg_none, reg_ECX, reg_none, 0, 4, 0}},
        {0, {reg_none, reg_EDX, reg_none, 0, 4, 0}},
        {0, {reg_none, reg_EBX, reg_none, 0, 4, 0}},
        {1, {reg_none, reg_none, reg_none, 0, 4, 0}}, // SIB
        {0, {reg_none, reg_EBP, reg_none, 0, 4, 0}},
        {0, {reg_none, reg_ESI, reg_none, 0, 4, 0}},
        {0, {reg_none, reg_EDI, reg_none, 0, 4, 0}}
    }
};


static DI_CHAR disp_ss[4] = {1,2,4,8};

int Diana_ReadIndexStructure32(DI_CHAR iOpSize,
                               unsigned char postByte,
                               DianaReadStream * pStream, 
                               DianaOperandValue * pValue,
                               DianaValueType * pType)
{
    DI_CHAR mod = Diana_GetMod(postByte);
    DI_CHAR rm = Diana_GetRm(postByte);

    if (mod == 3)
    {
        *pType = diana_register;
        return Diana_DispatchMod3(rm, iOpSize, &pValue->recognizedRegister);
    }
    if (mod>3)
        return DI_ERROR;

    *pType = diana_index;
    {
        DianaRmIndex  * pIndex = &pValue->rmIndex;
        DianaEntry32 * pOp = &g_table1[mod][rm];
        
        *pIndex = pOp->index;

        if (!pOp->iHasSIB)
            return DI_SUCCESS;

        // check all
        {
            unsigned char sibByte = 0;
            int readed = 0;
            int ss=0, index=0, base=0;
            int iRes  = pStream->pReadFnc(pStream, &sibByte, 1, &readed);
            if (iRes)
                return iRes;
            if(readed !=1)
                return DI_ERROR;

            Diana_DispatchSIB(sibByte, &ss, &index, &base);

            // todo test index==4
            if (base == 5)
            {
               if (mod == 0)
               {
                   pIndex->dispSize = 4;
               }
               else
               {
                   pIndex->reg = reg_EBP;
               }
            
            } 
            else
            {
                iRes = DianaRecognizeCommonReg(4,
                                               base, 
                                               &pIndex->reg);
                if (iRes)
                    return DI_ERROR;

            }

            // recognize indexes
            iRes = DianaRecognizeCommonReg(4,
                                            index, 
                                            &pIndex->indexed_reg);
            if (iRes)
                return DI_ERROR;

            if (index == 5)
            {
                pIndex->indexed_reg = reg_none; 
                pIndex->index = 0;
            } else
            {
                pIndex->index = disp_ss[ss];
            }
        }
    }
    return DI_SUCCESS;

}
