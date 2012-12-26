#ifndef DIANA_PROCESSOR_CMD_M_XMM_H
#define DIANA_PROCESSOR_CMD_M_XMM_H

#include "diana_processor_core_impl_xmm.h"

int Diana_Call_movups(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext);

int Diana_Call_movupd(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext);

int Diana_Call_movdqu(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext);

int Diana_Call_movaps(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext);

int Diana_Call_movapd(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext);

int Diana_Call_movdqa(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext);

int Diana_Call_movntps(struct _dianaContext * pDianaContext,
                       DianaProcessor * pCallContext);

int Diana_Call_movntpd(struct _dianaContext * pDianaContext,
                       DianaProcessor * pCallContext);

int Diana_Call_movntdq(struct _dianaContext * pDianaContext,
                       DianaProcessor * pCallContext);

int Diana_Call_movss(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext);

int Diana_Call_movlpd(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext);

int Diana_Call_movlps(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext);

int Diana_Call_movsd(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext);

int Diana_Call_movddup(struct _dianaContext * pDianaContext,
                       DianaProcessor * pCallContext);

#endif