#ifndef DIANA_PROCESSOR_CORE_IMPL_H
#define DIANA_PROCESSOR_CORE_IMPL_H

#include "diana_core.h"
#include "diana_proc_gen.h"
#include "diana_processor_streams.h"
#include "diana_uids.h"

typedef struct _DianaRegInfo
{
    int m_size;
    int m_offset;
}DianaRegInfo;


// FLAGS
#define flag_CF         0x000001
#define flag_1             0x000002 // 1
#define flag_PF         0x000004
#define flag_3             0x000008 // 0
#define flag_AF         0x000010
#define flag_5             0x000020 // 0
#define flag_ZF         0x000040
#define flag_SF         0x000080
#define flag_TF         0x000100
#define flag_IF         0x000200
#define flag_DF         0x000400
#define flag_OF         0x000800
#define flag_IOPL_0     0x001000
#define flag_IOPL_1     0x002000    
#define flag_NT         0x004000
#define flag_15         0x008000 // 0
//EFLAGS
#define flag_RF         0x010000
#define flag_VM         0x020000
#define flag_AC         0x040000
#define flag_VIF        0x080000
#define flag_VIP        0x100000
#define flag_ID         0x200000
//RFLAGS


typedef union _DianaRegisterValue16
{
    struct 
    {
        unsigned char l;
        unsigned char h;
    };
    DI_UINT16 value;
} DianaRegisterValue16_type;

typedef union _DianaRegisterValue32
{
    struct 
    {
        DianaRegisterValue16_type l;
        DI_UINT16 h;
    };
    DI_UINT32 value;
} DianaRegisterValue32_type;


typedef union _DianaRegisterValue
{
    struct 
    {
        DianaRegisterValue32_type l;
        DI_UINT32 h;
    };
    OPERAND_SIZE value;
} DianaRegisterValue_type;


// SIGNED
typedef union _DianaRegisterValue16_signed
{
    struct 
    {
        DI_SIGNED_CHAR l;
        DI_SIGNED_CHAR h;
    };
    DI_INT16 value;
} DianaRegisterValue16_signed_type;

typedef union _DianaRegisterValue32_signed
{
    struct 
    {
        DianaRegisterValue16_signed_type l;
        DI_INT16 h;
    };
    DI_INT32 value;
} DianaRegisterValue32_signed_type;


typedef union _DianaRegisterValue_signed
{
    struct 
    {
        DianaRegisterValue32_signed_type l;
        DI_INT32 h;
    };
    OPERAND_SIZE_SIGNED value;
} DianaRegisterValue_signed_type;

#define DI_PROC_STATE_TEMP_RIP_IS_VALID       0x1
#define DI_PROC_STATE_RIP_CHANGED             0x2
#define DI_PROC_STATE_UPDATE_FLAGS_PSZ        0x4 
#define DI_PROC_STATE_CMD_USES_NORMAL_REP     0x8 

typedef void (*FireAction_type)(struct _dianaProcessorFirePoint * pPoint,
                                struct _dianaProcessor * pProcessor);
typedef struct _dianaProcessorFirePoint
{
    void * pContext;
    OPERAND_SIZE address;
    FireAction_type action;
}DianaProcessorFirePoint;

#define DIANA_PROCESSOR_MAX_FIRE_POINTS     10
typedef struct _dianaProcessor
{
    DianaBase m_base;
    int m_iLastRegistersOffset;
    char * m_pRegistersVector;
    int m_registersVectorSize;

    DianaRegInfo m_registers[count_of_DianaUnifiedRegister];
    DianaRandomReadWriteStream * m_pMemoryStream;
    Diana_Allocator * m_pAllocator;
    
    DianaRegisterValue_type m_flags;
    
    DianaReadStream m_readStream;
    DianaContext m_context;
    DianaParserResult m_result;

    int m_stateFlags;
    int m_stateFlagsToRemove;
    OPERAND_SIZE m_tempRIP;

    DianaProcessorFirePoint m_firePoints[DIANA_PROCESSOR_MAX_FIRE_POINTS];
    int m_firePointsCount;
}DianaProcessor;

#define UPDATE_PSZ(X, highMask) \
    if (pCallContext->m_stateFlags & DI_PROC_STATE_UPDATE_FLAGS_PSZ)\
    {\
        CLEAR_FLAG_PF;\
        CLEAR_FLAG_SF;\
        CLEAR_FLAG_ZF;\
        if ((X))\
        {\
            if ((X)&highMask)\
                SET_FLAG_SF;\
            if (IsParity((unsigned char)((X))))\
                SET_FLAG_PF;\
        }\
        else\
        {\
            SET_FLAG_ZF;\
            SET_FLAG_PF;\
        }\
    }


const char * Diana_GenerateStuff();

int DianaProcessor_RegisterFirePoint(DianaProcessor * pCallContext,
                                     const DianaProcessorFirePoint * pPoint);

void DianaProcessor_CmdUsesNormalRep(DianaProcessor * pCallContext);

int DianaProcessor_InitProcessorImpl(DianaProcessor * pThis);

OPERAND_SIZE DianaProcessor_GetValue(DianaProcessor * pCallContext,
                            DianaRegInfo * pReg);

OPERAND_SIZE DianaProcessor_GetValueEx(DianaProcessor * pCallContext,
                                       DianaRegInfo * pReg,
                                       int size);

void DianaProcessor_SetValueEx(DianaProcessor * pCallContext,
                               DianaUnifiedRegister regId,
                               DianaRegInfo * pReg,
                               OPERAND_SIZE value,
                               int size);
// These functions only for size <= sizeof(*pResult):
int DianaProcessor_GetMemValue(DianaProcessor * pThis,
                               OPERAND_SIZE selector,
                               OPERAND_SIZE offset,
                               OPERAND_SIZE size,
                               OPERAND_SIZE * pResult,
                               int flags,
                               DianaUnifiedRegister segReg);

int DianaProcessor_SetMemValue(DianaProcessor * pThis,
                               OPERAND_SIZE selector,
                               OPERAND_SIZE offset,
                               OPERAND_SIZE size,
                               OPERAND_SIZE * pResult,
                               int flags,
                               DianaUnifiedRegister segReg);
//-------------
DianaRegInfo * DianaProcessor_QueryReg(DianaProcessor * pThis, 
                              DianaUnifiedRegister reg);

OPERAND_SIZE DianaProcessor_GetSignMask(int sizeInBytes);
OPERAND_SIZE DianaProcessor_GetSignMaskSpecifyBit(OPERAND_SIZE sizeInBits);

void DianaProcessor_SetValue(DianaProcessor * pCallContext,
                    DianaUnifiedRegister regId,
                    DianaRegInfo * pReg,
                    OPERAND_SIZE value);
int  DianaProcessor_QueryFlag(DianaProcessor * pThis, 
                     OPERAND_SIZE flag);
void DianaProcessor_SetFlag(DianaProcessor * pThis, 
                   OPERAND_SIZE flag);
void DianaProcessor_ClearFlag(DianaProcessor * pThis, 
                     OPERAND_SIZE flag);

int DianaProcessor_SetGetOperand(struct _dianaContext * pDianaContext,
                                 DianaProcessor * pCallContext,
                                 int opNumber,
                                 OPERAND_SIZE * pResult,
                                 int bSet,
                                 int * pSizeOfOperand,
                                 int flags);

void DianaProcessor_ProcImplInit();

void DianaProcessor_UpdatePSZ(DianaProcessor * pCallContext,
                              OPERAND_SIZE value,
                              int opSize);
OPERAND_SIZE DianaProcessor_CutValue(OPERAND_SIZE value,
                                     int size);

int DianaProcessor_SetCOA_Add(struct _dianaContext * pDianaContext,
                              DianaProcessor * pCallContext,
                              const OPERAND_SIZE * pOldValue,
                              const OPERAND_SIZE * pNewValue,
                              const OPERAND_SIZE * pOperand,
                              int opSize,
                              int bSetCF
                              );
int DianaProcessor_SetCOA_Sub(struct _dianaContext * pDianaContext,
                              DianaProcessor * pCallContext,
                              const OPERAND_SIZE * pOldValue,
                              const OPERAND_SIZE * pNewValue,
                              const OPERAND_SIZE * pOperand,
                              int opSize,
                              int bSetCF
                              );

int DianaProcessor_SignExtend(OPERAND_SIZE * pVariable, 
                              int size, 
                              int newSize);

int DianaProcessor_CalcIndex(struct _dianaContext * pDianaContext,
                             DianaProcessor * pCallContext,
                             const DianaRmIndex * pIndex,
                             OPERAND_SIZE * pSelector,
                             OPERAND_SIZE * pAddress);

int DianaProcessor_GetAddress(struct _dianaContext * pDianaContext,
                              DianaProcessor * pCallContext,
                              DianaLinkedOperand * pLinkedOp,
                              OPERAND_SIZE * pSelector,
                              OPERAND_SIZE * pAddress);

int Diana_ProcessorGetOperand_index_ex2(struct _dianaContext * pDianaContext,
                                          DianaProcessor * pCallContext,
                                          int usedSize,
                                          OPERAND_SIZE * pAddress,
                                          OPERAND_SIZE * pResult,
                                          DianaRmIndex * pRmIndex,
                                          OPERAND_SIZE * pSelector);

int DianaProcessor_ReadMemory(DianaProcessor * pThis,
                              OPERAND_SIZE selector,
                              OPERAND_SIZE offset,
                              void * pBuffer, 
                              OPERAND_SIZE iBufferSize, 
                              OPERAND_SIZE * readed,
                              int flags,
                              DianaUnifiedRegister segReg);

int DianaProcessor_WriteMemory(DianaProcessor * pThis,
                               OPERAND_SIZE selector,
                               OPERAND_SIZE offset,
                               void * pBuffer, 
                               OPERAND_SIZE iBufferSize, 
                               OPERAND_SIZE * readed,
                               int flags,
                               DianaUnifiedRegister segReg);

int Diana_ProcessorSetGetOperand_index(struct _dianaContext * pDianaContext,
                                       DianaProcessor * pCallContext,
                                       int usedSize,
                                       OPERAND_SIZE * pResult,
                                       int bSet,
                                       DianaRmIndex * pRmIndex,
                                       int flags);

int Diana_ProcessorGetOperand_index_ex(struct _dianaContext * pDianaContext,
                                          DianaProcessor * pCallContext,
                                          int usedSize,
                                          OPERAND_SIZE * pAddress,
                                          OPERAND_SIZE * pResult,
                                          DianaRmIndex * pRmIndex);

void DianaProcessor_SetResetDefaultFlags(DianaProcessor * pThis);

int DianaProcessor_Query64RegisterFor32(DianaUnifiedRegister registerIn,
                                        DianaUnifiedRegister * pUsedReg);

int DianaProcessor_QueryRaxRegister(int size, 
                                    DianaUnifiedRegister * pUsedReg);
int DianaProcessor_QueryRdxRegister(int size, 
                                    DianaUnifiedRegister * pUsedReg);
int DianaProcessor_QueryRcxRegister(int size, 
                                    DianaUnifiedRegister * pUsedReg);
#define DI_UPDATE_FLAGS_PSZ(X) \
    pCallContext->m_stateFlags |= DI_PROC_STATE_UPDATE_FLAGS_PSZ;\
    pCallContext->m_stateFlagsToRemove |= DI_PROC_STATE_UPDATE_FLAGS_PSZ;\
    X; \
    pCallContext->m_stateFlags &= ~DI_PROC_STATE_UPDATE_FLAGS_PSZ;

//----
// MEMORY ACCESS MACROSES
#define DI_DEF_LOCAL(X) \
    OPERAND_SIZE oldDestValue = 0;\
    OPERAND_SIZE X = 0; \
    int X##_size = 0;
//---
#define DI_DEF_LOCALS(X, Y) \
    OPERAND_SIZE oldDestValue = 0;\
    OPERAND_SIZE X = 0, Y = 0;\
    int X##_size = 0, Y##_size = 0;
//---
#define DI_DEF_LOCALS2(X, Y, Z) \
    OPERAND_SIZE oldDestValue = 0;\
    OPERAND_SIZE X = 0, Y = 0, Z = 0;\
    int X##_size = 0, Y##_size, Z##_size = 0;
//---
#define DI_MEM_GET(variable, number)  \
    DI_CHECK(DianaProcessor_SetGetOperand(pDianaContext, \
                                          pCallContext, \
                                          number, \
                                          &variable, \
                                          0, \
                                          &variable##_size, \
                                          0));

#define DI_MEM_SET(variable, number)  \
    DI_CHECK(DianaProcessor_SetGetOperand(pDianaContext, \
                                         pCallContext, \
                                         number, \
                                         &variable, \
                                         1,  \
                                         &variable##_size, \
                                         0));

#define DI_MEM_GET_SRC2(variable)    DI_MEM_GET(variable, 2)
#define DI_MEM_SET_SRC2(variable)    DI_MEM_SET(variable, 2)
#define DI_MEM_GET_SRC(variable)    DI_MEM_GET(variable, 1)
#define DI_MEM_SET_SRC(variable)    DI_MEM_SET(variable, 1)
#define DI_MEM_GET_DEST(variable)   DI_MEM_GET(variable, 0)
#define DI_MEM_SET_DEST(variable)   DI_MEM_SET(variable, 0)

#define DI_PROC_END \
    return DI_SUCCESS;

#define DI_VAR_SIZE(variable)   (variable##_size)

//---
#define DI_START_UPDATE_COA_FLAGS(dest) \
    CLEAR_FLAG_AF;\
    CLEAR_FLAG_OF;\
    CLEAR_FLAG_CF;\
    oldDestValue = dest;
//-
#define DI_END_UPDATE_COA_FLAGS_ADD(dest, src) \
    DI_CHECK(DianaProcessor_SetCOA_Add(pDianaContext, \
                                   pCallContext,\
                                   &oldDestValue,\
                                   &(dest),\
                                   &(src),\
                                   DI_VAR_SIZE(dest), \
                                   flag_CF|flag_OF|flag_AF));
//-
#define DI_END_UPDATE_COA_FLAGS_SUB(dest, src) \
    DI_CHECK(DianaProcessor_SetCOA_Sub(pDianaContext, \
                                   pCallContext,\
                                   &oldDestValue,\
                                   &(dest),\
                                   &(src),\
                                   DI_VAR_SIZE(dest), \
                                   flag_CF|flag_OF|flag_AF));

// OA
#define DI_START_UPDATE_OA_FLAGS(dest) \
    CLEAR_FLAG_AF;\
    CLEAR_FLAG_OF;\
    oldDestValue = dest;

//-
#define DI_END_UPDATE_OA_FLAGS_ADD(dest, src) \
    DI_CHECK(DianaProcessor_SetCOA_Add(pDianaContext, \
                                   pCallContext,\
                                   &oldDestValue,\
                                   &(dest),\
                                   &(src),\
                                   DI_VAR_SIZE(dest), \
                                   flag_OF|flag_AF));
//-
#define DI_END_UPDATE_OA_FLAGS_SUB(dest, src) \
    DI_CHECK(DianaProcessor_SetCOA_Sub(pDianaContext, \
                                   pCallContext,\
                                   &oldDestValue,\
                                   &(dest),\
                                   &(src),\
                                   DI_VAR_SIZE(dest), \
                                   flag_OF|flag_AF));

// A
#define DI_START_UPDATE_O_FLAGS(dest) \
    CLEAR_FLAG_OF;\
    oldDestValue = dest;

//-
#define DI_END_UPDATE_O_FLAGS_ADD(dest, src) \
    DI_CHECK(DianaProcessor_SetCOA_Add(pDianaContext, \
                                   pCallContext,\
                                   &oldDestValue,\
                                   &(dest),\
                                   &(src),\
                                   DI_VAR_SIZE(dest), \
                                   flag_OF));
//-
#define DI_END_UPDATE_O_FLAGS_SUB(dest, src) \
    DI_CHECK(DianaProcessor_SetCOA_Sub(pDianaContext, \
                                   pCallContext,\
                                   &oldDestValue,\
                                   &(dest),\
                                   &(src),\
                                   DI_VAR_SIZE(dest), \
                                   flag_OF));


#define DI_SIGN_EXTEND(variable, newSize) \
    DI_CHECK(DianaProcessor_SignExtend(&variable, DI_VAR_SIZE(variable), newSize));

#endif