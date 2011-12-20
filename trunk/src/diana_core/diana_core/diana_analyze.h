#ifndef DIANA_ANALYZE_H
#define DIANA_ANALYZE_H

#include "diana_stack.h"


#define DI_INSTRUCTION_CAN_CHANGE_CSIP    0x1
#define DI_INSTRUCTION_IS_RET             0x2
#define DI_INSTRUCTION_IS_LOADING         0x4
#define DI_INSTRUCTION_USES_RIP           0x8
#define DI_INSTRUCTION_USES_UNKNOWN_RIP   0x10
#define DI_INSTRUCTION_INVALID            0x20
#define DI_INSTRUCTION_ROOT               0x40

#define DI_ROUTE_QUESTIONABLE             0x1

#define DI_XREF_INVALID                   0x1
typedef struct _Diana_Instruction
{
    // used only in contructror time
    Diana_ListNode m_routeEntry;

    OPERAND_SIZE m_offset;
    Diana_List m_refsTo;
    Diana_List m_refsFrom;
    int m_flags;
}Diana_Instruction;

typedef struct _Diana_SubXRef
{
    Diana_ListNode m_instructionEntry; // must be first
    Diana_Instruction * m_pInstruction;
}Diana_SubXRef;


typedef struct _Diana_XRef
{
    Diana_SubXRef m_subrefs[2];  // from -> to
    int m_flags;
}Diana_XRef;

typedef int (* DianaAnalyzeMoveTo_fnc)(void * pThis, OPERAND_SIZE offset);
typedef int (* ConvertAddressToRelative_fnc)(void * pThis, 
                                             OPERAND_SIZE address,
                                             OPERAND_SIZE * pRelativeOffset,
                                             int * pbInvalidPointer);
typedef int (* AddSuspectedDataAddress_fnc)(void * pThis, 
                                            OPERAND_SIZE address);
typedef struct _dianaAnalyzeObserver
{
    DianaReadStream m_stream;
    DianaAnalyzeMoveTo_fnc m_pMoveTo;
    ConvertAddressToRelative_fnc m_pConvertAddress;
    AddSuspectedDataAddress_fnc m_pSuspectedDataAddress;
}DianaAnalyzeObserver;

void DianaAnalyzeObserver_Init(DianaAnalyzeObserver * pThis,
                               DianaRead_fnc pReadFnc,
                               DianaAnalyzeMoveTo_fnc pMoveFnc,
                               ConvertAddressToRelative_fnc pConvertAddress,
                               AddSuspectedDataAddress_fnc pSuspectedDataAddress);

void Diana_Instruction_Init(Diana_Instruction * pInstruction,
                            OPERAND_SIZE m_offset,
                            int flags);

typedef struct _Diana_InstructionsOwnerBase
{
    int c;
}Diana_InstructionsOwnerBase;


typedef struct _Diana_InstructionsOwner
{
    Diana_InstructionsOwnerBase m_base;
    Diana_Stack m_xrefs;
    Diana_Instruction ** m_ppPresenceVec;
    Diana_Instruction * m_pInstructionsVec;
    OPERAND_SIZE m_maxSize;
    OPERAND_SIZE m_usedSize;
    OPERAND_SIZE m_actualSize;
    int m_stackInited;
}Diana_InstructionsOwner;

typedef struct _diana_RouteInfo
{
    Diana_List instructions;
    OPERAND_SIZE  startOffset;
    long flags;
}Diana_RouteInfo;

int Diana_InstructionsOwner_Init(Diana_InstructionsOwner * pOwner,
                                 OPERAND_SIZE maxOffsetSize);
void Diana_InstructionsOwner_Free(Diana_InstructionsOwner * pOwner);
Diana_Instruction * Diana_InstructionsOwner_AllocateInstruction(Diana_InstructionsOwner * pOwner);

int Diana_AnalyzeCode(Diana_InstructionsOwner * pOwner,
                      DianaAnalyzeObserver * pObserver,
                      int mode,
                      OPERAND_SIZE initialOffset,
                      OPERAND_SIZE maxOffset);

Diana_XRef * Diana_CastXREF(Diana_ListNode * pNode,
                            int index);

#endif