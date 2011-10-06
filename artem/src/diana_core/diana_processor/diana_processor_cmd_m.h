#ifndef DIANA_PROCESSOR_CMD_M_H
#define DIANA_PROCESSOR_CMD_M_H

#include "diana_processor_core_impl.h"

int Diana_Call_mov(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext);

int Diana_Call_movs(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext);

int Diana_Call_movsx(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext);

int Diana_Call_movsxd(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext);

int Diana_Call_movzx(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext);

int Diana_Call_mul(struct _dianaContext * pDianaContext,
                   DianaProcessor * pCallContext);

int Di_CheckZeroExtends(DianaProcessor * pCallContext,
                        OPERAND_SIZE * pDest,
                        int src_size,
                        int * dest_size);
#endif