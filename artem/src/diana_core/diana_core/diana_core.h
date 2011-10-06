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
 reg_TR0,   reg_TR1,   reg_TR2,   reg_TR3,   reg_TR4,   reg_TR5,   reg_TR6,   reg_TR7,
 reg_IP,


 // x64 part
 reg_RAX,   reg_RCX,   reg_RDX,   reg_RBX,   reg_RSP,   reg_RBP,   reg_RSI,   reg_RDI,
 reg_SIL,   reg_DIL,   reg_BPL,   reg_SPL,
 reg_R8,    reg_R9,    reg_R10,   reg_R11,   reg_R12,   reg_R13,   reg_R14,   reg_R15,
 reg_R8D,   reg_R9D,   reg_R10D,  reg_R11D,  reg_R12D,  reg_R13D,  reg_R14D,  reg_R15D,
 reg_R8W,   reg_R9W,   reg_R10W,  reg_R11W,  reg_R12W,  reg_R13W,  reg_R14W,  reg_R15W,
 reg_R8B,   reg_R9B,   reg_R10B,  reg_R11B,  reg_R12B,  reg_R13B,  reg_R14B,  reg_R15B,
 reg_RIP,

 reg_MM0, reg_MM1, reg_MM2, reg_MM3, reg_MM4, reg_MM5, reg_MM6, reg_MM7,
 reg_MM8, reg_MM9, reg_MM10, reg_MM11, reg_MM12, reg_MM13, reg_MM14, reg_MM15,

 reg_XMM0, reg_XMM1, reg_XMM2, reg_XMM3, reg_XMM4, reg_XMM5, reg_XMM6, reg_XMM7,
 reg_XMM8, reg_XMM9, reg_XMM10, reg_XMM11, reg_XMM12, reg_XMM13, reg_XMM14, reg_XMM15,

 reg_fpu_ST0, reg_fpu_ST1, reg_fpu_ST2, reg_fpu_ST3, reg_fpu_ST4, reg_fpu_ST5, reg_fpu_ST6, reg_fpu_ST7,

 count_of_DianaUnifiedRegister

}DianaUnifiedRegister;

// flags
#define DI_FLAG_CMD_SUPPORTS_IMM64          1
#define DI_FLAG_CMD_AMD_DEFAULT_OPSIZE_64   2
#define DI_FLAG_CMD_AMD_INVALID             4
#define DI_FLAG_CMD_FPU_I                   8
#define DI_FLAG_CMD_IGNORE_REX_PREFIX      16
#define DI_FLAG_CMD_I386                   32
#define DI_FLAG_CMD_AMD64                  64
#define DI_FLAG_CMD_AMD64_SIGN_EXTENDS    128


// index fields
#define DI_UINT16         unsigned short
#define DI_INT16          short
#define DI_INT32          int
#define DI_INT64          long long
#define DI_UINT32         unsigned int
#define DI_UINT64         unsigned long long
#define DI_CHAR           unsigned char
#define DI_SIGNED_CHAR    char
#define DI_CHAR_NULL      ((unsigned char)(-1))

#define OPERAND_SIZE  unsigned long long
#define OPERAND_SIZE_SIGNED  long long

#define DI_FULL_CHAR           unsigned int
#define DI_FULL_CHAR_NULL           ((unsigned int)(-1))
#define DI_MAX_OPERANDS_COUNT   (4)
#define DI_MAX_OPCODE_COUNT   (4)

#define DI_CACHE_RESERVED      2
#define DI_CACHE_SIZE (DI_MAX_OPCODE_COUNT + 1+ DI_CACHE_RESERVED)

#define DI_MAX_PREFIXES_COUNT  16


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

#define DIANA_GT_CAN_CHANGE_RIP                      0x1
#define DIANA_GT_IS_CALL                             0x2
#define DIANA_GT_IS_JUMP                             0x4
#define DIANA_GT_CAN_GO_TO_THE_NEXT_INSTRUCTION      0x8
#define DIANA_GT_RET                                 0x10

typedef struct _Diana_LinkedAdditionalGroupInfo
{
    int flags;
    int relArgrumentNumber;
}Diana_LinkedAdditionalGroupInfo;

typedef struct _dianaGroupInfo
{
    long m_lGroupId;
    const char * m_pName;
    Diana_LinkedAdditionalGroupInfo * m_pLinkedInfo;
    void * m_pTag;
}DianaGroupInfo;

typedef void (*Diana_PrefixFnc)(struct _dianaContext * pContext);

typedef void (*Diana_CallFnc)(struct _dianaContext * pDianaContext,
                              void * pCallContext);

// WARNING! This structure SHOULD EXACT MATCH DianaCmdInfo1 from diana_gen.c 
typedef struct _dianaCmdInfo
{
    DI_INT32 m_lGroupId;
    DI_CHAR m_flags;
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
    DianaGroupInfo * m_pGroupInfo;
    DianaOperandInfo m_operands[1];
}DianaCmdInfo;

typedef struct _dianaRmIndex
{
    DianaUnifiedRegister seg_reg;
    DianaUnifiedRegister reg;
    DianaUnifiedRegister indexed_reg;
    DI_CHAR index;
    DI_CHAR dispSize;
    DI_INT64 dispValue;
}DianaRmIndex;

typedef struct _dianaCallPtr
{
    DI_UINT64 m_segment;
    DI_UINT64 m_address;
    unsigned long m_segment_size;
    unsigned long m_address_size;
}DianaCallPtr;

typedef struct _dianaRel
{
    int       m_size;
    DI_UINT64 m_value;
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
    DI_UINT64 imm;
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
    DI_CHAR iRexPrefix;
    int iFullCmdSize;
}DianaParserResult;

// Callback
#define DI_END                      ((int)1)
#define DI_ERROR                    ((int)-1)
#define DI_OUT_OF_MEMORY            ((int)-2)
#define DI_DIVISION_BY_ZERO         ((int)-3)
#define DI_UNSUPPORTED_COMMAND      ((int)-4)
#define DI_DIVISION_OVERFLOW        ((int)-5)
#define DI_INVALID_CONFIGURATION    ((int)-6)
#define DI_WIN32_ERROR              ((int)-7)
#define DI_ERROR_NOT_USED_BY_CORE   ((int)-8)
#define DI_INVALID_INPUT            ((int)-9)

#define DI_SUCCESS ((int)0)

#define DI_CHECK_ALLOC(x) { if(!(x)) return DI_OUT_OF_MEMORY; }
#define DI_CHECK(x) { int di____code = (x); if (di____code != DI_SUCCESS) { return di____code; } }

typedef int (* DianaRead_fnc)(void * pThis, 
                              void * pBuffer, 
                              int iBufferSize, 
                              int * readed);
typedef int (* DianaWrite_fnc)(void * pThis, 
                               void * pBuffer, 
                               int iBufferSize, 
                               int * wrote);

typedef struct _dianaReadStream
{
    DianaRead_fnc pReadFnc;
}DianaReadStream;


// Allocators
#define DI_REX_PREFIX_START    0x40
#define DI_REX_PREFIX_END      0x4F
#define DI_REX_HAS_FLAG_W(X)          (X&0x8)
#define DI_REX_HAS_FLAG_R(X)          (X&0x4)
#define DI_REX_HAS_FLAG_X(X)          (X&0x2)
#define DI_REX_HAS_FLAG_B(X)          (X&0x1)

#define DI_NO_PREFIX     0
#define DI_PREFIX_LOCK   1
#define DI_PREFIX_REP    2
#define DI_PREFIX_REPN   3

#define DIANA_MODE64    8
#define DIANA_MODE32    4
#define DIANA_MODE16    2

#define DIANA_HAS_ADDRESS_SIZE_PREFIX   1
#define DIANA_HAS_OPERAND_SIZE_PREFIX   2
#define DIANA_INVALID_STATE             4

typedef struct _dianaPrefixInfo
{
    Diana_PrefixFnc linkedPrefixFnc;
    DI_CHAR prefix;
}DianaPrefixInfo;

typedef struct _dianaContext
{
    int iMainMode_opsize;    // 4 or 2
    int iCurrentCmd_opsize;  // 4 or 2

    int iMainMode_addressSize;    // 4 or 2
    int iCurrentCmd_addressSize;  // 4 or 2

    DianaUnifiedRegister mainMode_sreg;    
    DianaUnifiedRegister currentCmd_sreg;  

    int iPrefix;
    int iSizePrefixes;

    int iAMD64Mode;
    DI_CHAR iRexPrefix;
    DI_CHAR iRexPrefixValue;

    unsigned char cache[DI_CACHE_SIZE];
    int cacheSize;
    int cacheIt;
    
    DI_FULL_CHAR lastPrefixBeforeRex;
    DianaPrefixInfo prefixes[DI_MAX_PREFIXES_COUNT];
    DI_FULL_CHAR prefixesCount;
}DianaContext;

void Diana_FatalBreak();
void Diana_DebugFatalBreak();

void Diana_InitContext(DianaContext * pThis, int Mode);

void Diana_ClearCache(DianaContext * pThis);

int Diana_ParseCmd(DianaContext * pContext, // IN
                   DianaCmdKeyLine * pInitialLine,  // IN
                   DianaReadStream * readStream,    // IN
                   DianaParserResult * pResult);    // OUT

int DianaRecognizeCommonReg(DI_CHAR iOpSize,
                            DI_CHAR regId, 
                            DianaUnifiedRegister * pOut);
int DianaRecognizeMMX(DI_CHAR regId, 
                         DianaUnifiedRegister * pOut);
int DianaRecognizeXMM(DI_CHAR regId, 
                     DianaUnifiedRegister * pOut);

typedef int (*Diana_ReadIndexStructure_type)(DianaContext * pContext,
                                             DianaLinkedOperand * pInfo,
                                             DI_CHAR iOpSize,
                                             unsigned char postbyte,
                                             DianaReadStream * pStream, 
                                             DianaOperandValue * pValue,
                                             DianaValueType * pType);

DianaGroupInfo * Diana_GetGroupInfo(long lId);

void Diana_Init();
void Diana_ResetPrefixes(DianaContext * pContext);

typedef void * ( *Diana_Alloc_type)(void * pThis, size_t size);
typedef void ( *Diana_Free_type)(void * pThis, void * memory);
typedef int ( *Diana_Patcher_type)(void * pThis, void * pDest, const void * pSource, size_t size);

typedef struct _Diana_Allocator
{
    Diana_Alloc_type m_alloc;
    Diana_Free_type m_free;
    Diana_Patcher_type m_patch;
}Diana_Allocator;

void Diana_AllocatorInit(Diana_Allocator * pAllocator,
                         Diana_Alloc_type alloc,    
                         Diana_Free_type free,
                         Diana_Patcher_type patch);


void Diana_CacheEatOneSafe(DianaContext * pContext);


#define DIANA_CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (char*)(address) - \
                                                  (size_t)(&((type *)0)->field)))
#endif