#ifndef DIANA_PROCESSOR_CMD_INTERNAL_H
#define DIANA_PROCESSOR_CMD_INTERNAL_H

int diana_internal_pop(DianaProcessor * pCallContext,
                       OPERAND_SIZE * pValue);
int diana_internal_push(DianaProcessor * pCallContext,
                        OPERAND_SIZE * pValue);
int Diana_Call_jcc(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext,
                    int opCount,
                    int relOp);

int Di_PrepareSIDI_regs(DianaProcessor * pCallContext,
                        DianaRmIndex * pSrcIndex, 
                        DianaRmIndex * pDestIndex);

void Di_UpdateSIDI(DianaProcessor * pCallContext,
                   OPERAND_SIZE * pSrcRegAddress, 
                   OPERAND_SIZE * pDestRegAddress);
#endif