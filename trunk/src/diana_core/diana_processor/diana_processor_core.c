#include "diana_processor_core.h"
#include "diana_proc_gen.h"
#include "diana_core_gen_tags.h"
#include "diana_processor_commands.h"

void DianaProcessor_GlobalInit()
{
    DianaProcessor_LinkCommands();
    DianaProcessor_ProcImplInit();
}

static int ProcessorReadStream(void * pThis, 
                               void * pBuffer, 
                               int iBufferSize, 
                               int * readed)
{
    DianaReadStream * pStream = pThis;
    DianaProcessor * pCallContext = 0;
    int res = 0;

    OPERAND_SIZE readedOp = 0;

    pCallContext= (DianaProcessor *)((char*)pStream - (size_t)&pCallContext->m_readStream);
    
    res = DianaProcessor_ReadMemory(pCallContext, 
                                    GET_REG_CS,
                                    pCallContext->m_tempRIP, 
                                    pBuffer, 
                                    iBufferSize, 
                                    &readedOp,
                                    0,
                                    reg_CS);
    if (res != DI_SUCCESS)
        return res;

    *readed = (int)readedOp;
    pCallContext->m_tempRIP += readedOp;
    return res;
}

// {A0E078BD-9C42-40b1-A59E-69900E121D95}
static const DIANA_UUID g_ProcessorId = 
{ 0xa0e078bd, 0x9c42, 0x40b1, { 0xa5, 0x9e, 0x69, 0x90, 0xe, 0x12, 0x1d, 0x95 } };

int DianaProcessor_Init(DianaProcessor * pThis, 
                        DianaRandomReadWriteStream * pMemoryStream,
                        Diana_Allocator * pAllocator,
                        int mode)
{
    memset(pThis, 0, sizeof(DianaProcessor)); 
    memset(pThis->m_registers, -1, sizeof(pThis->m_registers)); 

    DianaBase_Init(&pThis->m_base, &g_ProcessorId);

    pThis->m_readStream.pReadFnc = ProcessorReadStream;
    Diana_InitContext(&pThis->m_context, mode);

    pThis->m_pAllocator = pAllocator;
    pThis->m_pMemoryStream = pMemoryStream;

    {
        int iResult = DianaProcessor_InitProcessorImpl(pThis);
        if (iResult != DI_SUCCESS)
        {
            DianaProcessor_Free(pThis);
            return iResult;
        }
        return 0;
    }
}

void DianaProcessor_Free(DianaProcessor * pThis)
{
    pThis->m_pAllocator->m_free( pThis->m_pAllocator, pThis->m_pRegistersVector );
}

static
int Call(DianaProcessorCommand_type pCommand,
         DianaProcessor * pCallContext)
{
    int res = pCommand(&pCallContext->m_context, pCallContext);

    // clean resources
    // remove flags
    pCallContext->m_stateFlags &= ~pCallContext->m_stateFlagsToRemove;
    pCallContext->m_stateFlagsToRemove = 0;

    return res;
}

static
int CallWithRep(DianaProcessorCommand_type pCommand,
                DianaProcessor * pCallContext,
                int bRepn)

{
    DianaUnifiedRegister usedReg = 0;
    switch(pCallContext->m_context.iCurrentCmd_addressSize)
    {
    case DIANA_MODE64:
        usedReg = reg_RCX;
        break;
    case DIANA_MODE32:
        usedReg = reg_ECX;
        break;
    case DIANA_MODE16:
        usedReg = reg_CX;
        break;
    default:
        Diana_DebugFatalBreak();
        return DI_ERROR;
    }
        
    while(1)
    {
        if (!DianaProcessor_GetValue(pCallContext, DianaProcessor_QueryReg(pCallContext, usedReg)))
            return DI_SUCCESS;

        DI_CHECK(Call(pCommand, pCallContext));
        if (pCallContext->m_stateFlags & DI_PROC_STATE_RIP_CHANGED)
            break;

        SET_REG_RCX(GET_REG_RCX - 1);

        if (bRepn)
        {
            if (GET_FLAG_ZF)
                return DI_SUCCESS;
        }
        else
        {
            // if DI_PROC_STATE_CMD_USES_NORMAL_REP flag is not set 
            if (!(pCallContext->m_stateFlags & DI_PROC_STATE_CMD_USES_NORMAL_REP))
            {
                if (!GET_FLAG_ZF)
                    return DI_SUCCESS;
            }
        }
    }
    return DI_SUCCESS;
}

void DianaProcessor_ClearCache(DianaProcessor * pThis)
{
    pThis->m_stateFlags = 0;
    Diana_ClearCache(&pThis->m_context);
}

int DianaProcessor_ExecOnce(DianaProcessor * pThis)
{
    // parse next command
    DianaProcessor * pCallContext = pThis;
    OPERAND_SIZE rip = GET_REG_RIP;
    int res = 0;
    int i =0;
    DianaProcessorCommand_type pCommand = 0;
    
    for(i = 0 ; i < pCallContext->m_firePointsCount; ++i)
    {
        DianaProcessorFirePoint  * pPoint = pCallContext->m_firePoints + i;
        if (pPoint->address == rip)
            pPoint->action(pPoint, pThis);
    }

    rip = GET_REG_RIP;
    if (!(pThis->m_stateFlags & DI_PROC_STATE_TEMP_RIP_IS_VALID))
    {
        // first run
        pThis->m_tempRIP = rip;

        pThis->m_stateFlags |= DI_PROC_STATE_TEMP_RIP_IS_VALID;
    }

    pThis->m_stateFlags &= ~DI_PROC_STATE_RIP_CHANGED;

    res = Diana_ParseCmd(&pThis->m_context, 
                         Diana_GetRootLine(),  // IN
                         &pThis->m_readStream,    // IN
                         &pThis->m_result);

    if (res != DI_SUCCESS)
        return res;
    
    // query context 
    pCommand = DIANA_QUERY_PROCESSOR_TAG(pThis->m_result.pInfo->m_pGroupInfo);
    if (!pCommand)
    {
        // command is not supported
        Diana_DebugFatalBreak();
        return DI_UNSUPPORTED_COMMAND;
    }

    // execute command
    switch(pThis->m_result.iPrefix)
    {
    case DI_PREFIX_REP:
        res = CallWithRep(pCommand, pCallContext, 0);
        break;
    case DI_PREFIX_REPN:
        res = CallWithRep(pCommand, pCallContext, 1);
        break;
    default:
        res = Call(pCommand, pCallContext);
    }

    // clear DI_PROC_STATE_CMD_USES_NORMAL_REP flag
    pCallContext->m_stateFlags &= ~DI_PROC_STATE_CMD_USES_NORMAL_REP;
           
    if (pThis->m_stateFlags & DI_PROC_STATE_RIP_CHANGED)
    {
        // RIP is changed by the command
        // temp rip is not valid right now
        pThis->m_stateFlags &= ~DI_PROC_STATE_TEMP_RIP_IS_VALID;
        pThis->m_stateFlags &= ~DI_PROC_STATE_RIP_CHANGED;
        // clear cache
        Diana_ClearCache(&pThis->m_context);

        if (res != DI_SUCCESS)
            return res;
    }
    else
    {
        if (res != DI_SUCCESS)
            return res;

        // shift RIP, usual command
        rip += pThis->m_result.iFullCmdSize;

        // update it
        DianaProcessor_SetValue(pCallContext, 
                                reg_none,  // skip check
                                DianaProcessor_QueryReg(pCallContext, reg_RIP), 
                                rip);
    }
    return DI_SUCCESS;
}
