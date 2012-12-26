#include "diana_processor_cmd_m_xmm.h"
#include "diana_proc_gen.h"
#include "diana_gen.h"
#include "diana_processor_cmd_internal_xmm.h"

int Diana_Call_movups(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext)
{
    DI_CHECK(diana_mov_xmm(pDianaContext,
                           pCallContext));
    DI_PROC_END
}

int Diana_Call_movupd(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext)
{
    DI_CHECK(diana_mov_xmm(pDianaContext,
                           pCallContext));
    DI_PROC_END
}

int Diana_Call_movdqu(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext)
{
    DI_CHECK(diana_mov_xmm(pDianaContext,
                           pCallContext));
    DI_PROC_END
}

int Diana_Call_movaps(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext)
{
    DI_CHECK(diana_mov_xmm(pDianaContext,
                           pCallContext));
    DI_PROC_END
}

int Diana_Call_movapd(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext)
{
    DI_CHECK(diana_mov_xmm(pDianaContext,
                           pCallContext));
    DI_PROC_END
}

int Diana_Call_movdqa(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext)
{
    DI_CHECK(diana_mov_xmm(pDianaContext,
                           pCallContext));
    DI_PROC_END
}

int Diana_Call_movntps(struct _dianaContext * pDianaContext,
                       DianaProcessor * pCallContext)
{
    DI_CHECK(diana_mov_xmm(pDianaContext,
                           pCallContext));
    DI_PROC_END
}

int Diana_Call_movntpd(struct _dianaContext * pDianaContext,
                       DianaProcessor * pCallContext)
{
    DI_CHECK(diana_mov_xmm(pDianaContext,
                           pCallContext));
    DI_PROC_END
}

int Diana_Call_movntdq(struct _dianaContext * pDianaContext,
                       DianaProcessor * pCallContext)
{
    DI_CHECK(diana_mov_xmm(pDianaContext,
                           pCallContext));
    DI_PROC_END
}

int Diana_Call_movss(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext)
{
    DI_DEF_LOCAL_XMM(dest)
    DI_DEF_LOCAL_XMM(src)

    DI_MEM_GET_DEST_XMM(dest)
    DI_MEM_GET_SRC_XMM(src)

    dest.u32[0] = src.u32[0];
    if (diana_register == pCallContext->m_result.linkedOperands[0].type &&
        diana_register == pCallContext->m_result.linkedOperands[1].type)
    {
        // when the source and destination operands are both XMM registers
        // DEST[31:0] <- SRC[31:0]
    }
    else
    if (diana_index == pCallContext->m_result.linkedOperands[0].type &&
        diana_register == pCallContext->m_result.linkedOperands[1].type)
    {
        // when the source operand is an XMM register and the destination is memory
        // DEST[31:0] <- SRC[31:0]
        dest_size = 4;
    }
    else
    if (diana_register == pCallContext->m_result.linkedOperands[0].type &&
        diana_index == pCallContext->m_result.linkedOperands[1].type)
    {
        // when the source operand is memory and the destination is an XMM register
        // DEST[31:0] <- SRC[31:0]
        // DEST[127:32] <- 0
        dest.u32[1] = 0;
        dest.u64[1] = 0;
    }
    else
        Diana_FatalBreak();

    DI_MEM_SET_DEST_XMM(dest)
    DI_PROC_END
}

int Diana_Call_movlpd(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext)
{
    DI_CHECK(diana_mov_xmm8(pDianaContext,
                            pCallContext));
    DI_PROC_END
}

int Diana_Call_movlps(struct _dianaContext * pDianaContext,
                      DianaProcessor * pCallContext)
{
    DI_CHECK(diana_mov_xmm8(pDianaContext,
                            pCallContext));
    DI_PROC_END
}

int Diana_Call_movsd(struct _dianaContext * pDianaContext,
                     DianaProcessor * pCallContext)
{
    DI_DEF_LOCAL_XMM(dest)
    DI_DEF_LOCAL_XMM(src)

    DI_MEM_GET_DEST_XMM(dest)
    DI_MEM_GET_SRC_XMM(src)

    dest.u64[0] = src.u64[0];
    if (diana_register == pCallContext->m_result.linkedOperands[0].type &&
        diana_register == pCallContext->m_result.linkedOperands[1].type)
    {
        // when the source and destination operands are both XMM registers
        // DEST[63:0] <- SRC[63:0]
    }
    else
    if (diana_index == pCallContext->m_result.linkedOperands[0].type &&
        diana_register == pCallContext->m_result.linkedOperands[1].type)
    {
        // when the source operand is an XMM register and the destination is memory
        // DEST[63:0] <- SRC[63:0]
        dest_size = 8;
    }
    else
    if (diana_register == pCallContext->m_result.linkedOperands[0].type &&
        diana_index == pCallContext->m_result.linkedOperands[1].type)
    {
        // when the source operand is memory and the destination is an XMM register
        // DEST[63:0] <- SRC[63:0]
        // DEST[127:63] <- 0
        dest.u64[1] = 0;
    }
    else
        Diana_FatalBreak();

    DI_MEM_SET_DEST_XMM(dest)
    DI_PROC_END
}

int Diana_Call_movddup(struct _dianaContext * pDianaContext,
                       DianaProcessor * pCallContext)
{
    DI_DEF_LOCAL_XMM(dest)
    DI_DEF_LOCAL_XMM(src)

    DI_MEM_GET_DEST_XMM(dest)
    src_size = 8;
    DI_MEM_GET_SRC_XMM(src)

    dest.u64[0] = src.u64[0];
    dest.u64[1] = src.u64[0];

    DI_MEM_SET_DEST_XMM(dest)
    DI_PROC_END
}
