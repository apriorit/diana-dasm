#ifndef DIANA_ANALYZE_H
#define DIANA_ANALYZE_H

#include "diana_stack.h"


#define DI_INSTRUCTION_CAN_CHANGE_CSIP    0x1
#define DI_INSTRUCTION_IS_RET             0x2
#define DI_INSTRUCTION_IS_LOADING         0x4

typedef struct _Diana_Instruction
{
    OPERAND_SIZE m_offset;
    Diana_List m_refsTo;
    Diana_List m_refsFrom;
    int m_flags;
}Diana_Instruction;

typedef struct _Diana_XRef
{
    Diana_ListNode m_instructionEntry;
    Diana_Instruction * m_pInstruction;
}Diana_XRef;

typedef int (* DianaAnalyzeMoveTo_fnc)(void * pThis, OPERAND_SIZE offset);

typedef struct _dianaAnalyzeReadStream
{
    DianaReadStream m_parent;
    DianaAnalyzeMoveTo_fnc m_pMoveTo;
}DianaAnalyzeReadStream;

void DianaAnalyzeReadStream_Init(DianaAnalyzeReadStream * pThis,
                                 DianaRead_fnc pReadFnc,
                                 DianaAnalyzeMoveTo_fnc pMoveFnc);

void Diana_Instruction_Init(Diana_Instruction * pInstruction,
                            OPERAND_SIZE m_offset,
                            int flags);

typedef struct _Diana_InstructionsOwner
{
    Diana_Stack m_xrefs;
    Diana_Instruction ** m_ppPresenceVec;
    Diana_Instruction * m_pInstructionsVec;
    OPERAND_SIZE m_maxSize;
    OPERAND_SIZE m_usedSize;
    int m_stackInited;
}Diana_InstructionsOwner;

int Diana_InstructionsOwner_Init(Diana_InstructionsOwner * pOwner,
                                 OPERAND_SIZE maxOffsetSize);
void Diana_InstructionsOwner_Free(Diana_InstructionsOwner * pOwner);
Diana_Instruction * Diana_InstructionsOwner_AllocateInstruction(Diana_InstructionsOwner * pOwner);

int Diana_AnalyzeCode(Diana_InstructionsOwner * pOwner,
                      DianaAnalyzeReadStream * pStream,
                      int mode,
                      OPERAND_SIZE initialOffset,
                      OPERAND_SIZE maxOffset);

#endif