#ifndef DIANA_CORE_H
#define DIANA_CORE_H

#include "memory.h"

// <gen
#include "diana_gen.h"
// >gen

typedef enum 
{
 reg_none,
 reg_AL,    reg_CL,    reg_DL,    reg_BL,    reg_AH,    reg_CH,    reg_DH,    reg_BH,
 reg_AX,    reg_CX,    reg_DX,    reg_BX,    reg_SP,    reg_BP,    reg_SI,    reg_DI,
 reg_EAX,   reg_ECX,   reg_EDX,   reg_EBX,   reg_ESP,   reg_EBP,   reg_ESI,   reg_EDI,
 reg_ES,    reg_CS,    reg_SS,    reg_DS,    reg_FS,    reg_GS,
 reg_CR0,   reg_CR1,   reg_CR2,   reg_CR3,   reg_CR4,   reg_CR5,   reg_CR6,   reg_CR7,
 reg_DR0,   reg_DR1,   reg_DR2,   reg_DR3,   reg_DR4,   reg_DR5,   reg_DR6,   reg_DR7,
 reg_TR0,   reg_TR1,   reg_TR2,   reg_TR3,   reg_TR4,   reg_TR5,   reg_TR6,   reg_TR7
}DianaUnifiedRegister;

// index fields
#define DI_CHAR           unsigned char
#define DI_CHAR_NULL      ((unsigned char)(-1))

#define DI_FULL_CHAR           unsigned int
#define DI_FULL_CHAR_NULL           ((unsigned int)(-1))
#define DI_MAX_OPERANDS_COUNT   (3)
#define DI_MAX_OPCODE_COUNT   (4)
#define DI_CACHE_SIZE (DI_MAX_OPCODE_COUNT+1) //+extension byte


// common
void Diana_DispatchSIB(unsigned char sib, int * pSS, int *pIndex, int *pBase);
DI_CHAR Diana_GetReg(unsigned char postbyte);
DI_CHAR Diana_GetRm(unsigned char postbyte);
DI_CHAR Diana_GetMod(unsigned char postbyte);

// parser structures:
typedef struct _dianaCmdKey
{
    void * keyLineOrCmdInfo;
    DI_CHAR options;
    DI_CHAR extension;
    DI_CHAR chOpcode;

}DianaCmdKey;

// extension additional values:
#define DIANA_OPT_HAS_RESULT       0x01


typedef struct _dianaCmdKeyLine
{
    int iKeysCount;
    DianaCmdKey key[1];
}DianaCmdKeyLine;


// result
typedef struct _dianaOperandInfo
{
    DianaOperands_type m_type;
    DI_CHAR m_size;
    DI_CHAR m_size2;

    DI_CHAR m_value;
    DianaSreg_type m_sreg_type;

}DianaOperandInfo;

typedef struct _Diana_LinkedAdditionalGroupInfo
{
    int isCall;
    int isJump;
    int canChangeCSIP;
}Diana_LinkedAdditionalGroupInfo;

typedef struct _dianaGroupInfo
{
    long m_lGroupId;
    const char * m_pName;
    Diana_LinkedAdditionalGroupInfo * m_pLinkedInfo;
}DianaGroupInfo;

typedef void (*Diana_PrefixFnc)(struct _dianaContext * pContext);

typedef struct _dianaCmdInfo
{
    long m_lGroupId;
    DI_CHAR m_bFullPostbyteUsed;
    DI_CHAR m_iCSIPExtensionSizeInBytes;
    DI_CHAR m_iImmediateOperandSizeInBytes;
    DI_CHAR m_iImmediateOperandSizeInBytes2;
    DI_CHAR m_iRegisterCodeAsOpcodePart;
    DI_CHAR m_bHas32BitAnalog;
    DI_CHAR m_extension;

    DI_CHAR m_operandCount;
    DI_CHAR m_bIsTruePrefix;
    Diana_PrefixFnc m_linkedPrefixFnc;
    DianaOperandInfo m_operands[1];
}DianaCmdInfo;

typedef struct _dianaRmIndex
{
    DianaUnifiedRegister seg_reg;
    DianaUnifiedRegister reg;
    DianaUnifiedRegister indexed_reg;
    DI_CHAR index;
    DI_CHAR dispSize;
    long dispValue;
}DianaRmIndex;

typedef struct _dianaCallPtr
{
    unsigned long m_segment;
    unsigned long m_address;
    unsigned long m_segment_size;
    unsigned long m_address_size;
}DianaCallPtr;

typedef struct _dianaRel
{
    unsigned long m_size;
    long m_value;
}DianaRel;

typedef struct _dianaMemory
{
    DianaRmIndex m_index;
}DianaMemory;

typedef enum {diana_none, diana_register, diana_index, diana_imm, diana_call_ptr, diana_rel, diana_memory, diana_reserved_reg}DianaValueType;

typedef union _dianaOperandValue
{
    // register
    DianaUnifiedRegister recognizedRegister;
    // r/m
    DianaRmIndex rmIndex;
    // immediate
    unsigned long imm;
    // call ptr
    DianaCallPtr ptr;
    // rel
    DianaRel rel;
    // memory
    DianaMemory memory;

}DianaOperandValue;

typedef struct _dianaLinkedOperand
{
    DianaOperandInfo * pInfo;

    DianaValueType type;    

    DianaOperandValue value;
    // offset
    int iOffset;

    int usedSize;
    int usedAddressSize;
}DianaLinkedOperand;

typedef struct _dianaParserResult
{
    DianaCmdInfo * pInfo;
    DianaLinkedOperand linkedOperands[DI_MAX_OPERANDS_COUNT];
    int iLinkedOpCount;
    int iPrefix;
}DianaParserResult;

// Callback
#define DI_END ((int)1)
#define DI_ERROR ((int)-1)
#define DI_NOT_SUPPORTED ((int)-2)
#define DI_SUCCESS ((int)0)

typedef int (* DianaRead_fnc)(void * pThis, void * pBuffer, int iBufferSize, int * readed);

typedef struct _dianaReadStream
{
    DianaRead_fnc pReadFnc;
}DianaReadStream;


#define DI_NO_PREFIX     0
#define DI_PREFIX_LOCK   1
#define DI_PREFIX_REP    2
#define DI_PREFIX_REPN   3

#define DIANA_MODE32    4
#define DIANA_MODE16    2
typedef struct _dianaContext
{
    int iMainMode_opsize;    // 4 or 2
    int iCurrentCmd_opsize;  // 4 or 2

    int iMainMode_addressSize;    // 4 or 2
    int iCurrentCmd_addressSize;  // 4 or 2

    DianaUnifiedRegister mainMode_sreg;    
    DianaUnifiedRegister currentCmd_sreg;  

    int iPrefix;
    unsigned char readed[DI_CACHE_SIZE];
    int iReadedSize;
}DianaContext;

void Diana_InitContext(DianaContext * pThis, int Mode);

int Diana_ParseCmd(DianaContext * pContext, // IN
                   DianaCmdKeyLine * pInitialLine,  // IN
                   DianaReadStream * readStream,    // IN
                   DianaParserResult * pResult);    // OUT

int DianaRecognizeCommonReg(DI_CHAR iOpSize,
                            DI_CHAR regId, 
                            DianaUnifiedRegister * pOut);

typedef int (*Diana_ReadIndexStructure_type)(DI_CHAR iOpSize,
                               unsigned char postbyte,
                               DianaReadStream * pStream, 
                               DianaOperandValue * pValue,
                               DianaValueType * pType);

DianaGroupInfo * Diana_GetGroupInfo(long lId);

void Diana_Init();

typedef void * (*Diana_Alloc_type)(void * pThis, size_t size);
typedef void (*Diana_Free_type)(void * pThis, void * memory);
typedef int (*Diana_Patcher_type)(void * pThis, void * pDest, const void * pSource, size_t size);

typedef struct _Diana_Allocator
{
    Diana_Alloc_type m_alloc;
    Diana_Free_type m_free;
    Diana_Patcher_type m_patch;
}Diana_Allocator;

#endif