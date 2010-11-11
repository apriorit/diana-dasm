#include "diana_core.h"
#include "search.h"
#include "string.h"
#include "diana_operands.h"
#include "diana_stream_proxy.h"
#include "diana_core_utils.h"
#include "diana_core_parsers.h"


static Diana_LinkedAdditionalGroupInfo g_infoForCalls = {1, 0, 1};
static Diana_LinkedAdditionalGroupInfo g_infoForJmps = {0, 1, 1};


typedef struct _dianaProxyReadStream
{
    DianaReadStream m_parent;
    DianaReadStream * m_pOriginalStream;
    int m_iReadSize;
}DianaProxyReadStream;

static int ProxyDianaReadFnc(void * pThisIn, void * pBuffer, int iBufferSize, int * readed)
{
    *readed = 0;
    {
        DianaProxyReadStream * pThis = pThisIn;
        int iRes = pThis->m_pOriginalStream->pReadFnc(pThis->m_pOriginalStream, pBuffer, iBufferSize, readed);
        if (!iRes)
        {
            pThis->m_iReadSize += *readed;
        }
        return iRes;
    }
}


int Diana_ParseCmd(DianaContext * pContext, //IN
                   DianaCmdKeyLine * pInitialLine,  // IN
                   DianaReadStream * readStream,    // IN
                   DianaParserResult * pResult)    // OUT
{
    int iRes = 0;
    int iPreviousCacheSize = pContext->cacheSize;
    DianaProxyReadStream proxy;
    DianaParseParams parseParams;

    proxy.m_iReadSize = 0;
    proxy.m_pOriginalStream = readStream;
    proxy.m_parent.pReadFnc = ProxyDianaReadFnc;

    parseParams.pContext = pContext;
    parseParams.pInitialLine = pInitialLine;
    parseParams.pResult = pResult;
    parseParams.readStream = &proxy.m_parent;

    iRes = Diana_ParseCmdEx(&parseParams);

    pResult->iPrefix = pContext->iPrefix;
    pResult->iRexPrefix = pContext->iRexPrefix;

    pResult->iFullCmdSize = iPreviousCacheSize + proxy.m_iReadSize - pContext->cacheSize;
    return iRes;
}


DianaGroupInfo * Diana_GetGroupInfo(long lId)
{
    DianaGroupInfo * pGroups = Diana_GetGroups();
    if (lId > Diana_GetGroupsCount())
        return 0;
    if (lId <1)
        return 0;
    return pGroups + lId - 1;
}

void Diana_FatalBreak()
{
#ifndef _M_X64
    __asm int 3
#else
    __debugbreak();
#endif
}
void Diana_InitContext(DianaContext * pThis, int Mode)
{
    memset(pThis, 0, sizeof(*pThis));

    pThis->cacheIt = DI_CACHE_RESERVED;
    if (Mode == DIANA_MODE64)
    {
        pThis->iAMD64Mode = 1;
        pThis->iMainMode_opsize = DIANA_MODE32;
        pThis->iCurrentCmd_opsize = DIANA_MODE32;

        pThis->iMainMode_addressSize = DIANA_MODE64;
        pThis->iCurrentCmd_addressSize = DIANA_MODE64;
    }
    else
    {
        pThis->iMainMode_opsize = Mode;
        pThis->iCurrentCmd_opsize = Mode;

        pThis->iMainMode_addressSize = Mode;
        pThis->iCurrentCmd_addressSize = Mode;
    }

    pThis->mainMode_sreg = reg_DS;
    pThis->currentCmd_sreg = reg_DS;
}

// debug
static void Diana_Nope(struct _dianaContext * pContext) 
{  
    Diana_FatalBreak();;
}
// normal
static void Diana_Normal(struct _dianaContext * pContext) 
{  
    pContext->lastPrefixBeforeRex = DI_FULL_CHAR_NULL;
    pContext->prefixesCount = 0;
    pContext->iCurrentCmd_opsize = pContext->iMainMode_opsize;
    pContext->currentCmd_sreg = pContext->mainMode_sreg;
    pContext->iCurrentCmd_addressSize = pContext->iMainMode_addressSize;
    pContext->iPrefix = 0;
    pContext->iSizePrefixes = 0;
    pContext->iRexPrefix = 0;
    pContext->iRexPrefixValue = 0;
}

static int InitJmps(DianaGroupInfo * pGroupInfo)
{
    // CALLS
    if (!strcmp(pGroupInfo->m_pName,"call"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForCalls;
        return 1;
    } else

    // LOOPS
    if (!strcmp(pGroupInfo->m_pName,"loop"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    } else
    if (!strcmp(pGroupInfo->m_pName,"loope"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    } else
    if (!strcmp(pGroupInfo->m_pName,"loopn"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    } else

    // JMPS
    if (!strcmp(pGroupInfo->m_pName,"jmp"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    } else
    if (!strcmp(pGroupInfo->m_pName,"ja"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    }else
    if (!strcmp(pGroupInfo->m_pName,"jae"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    }else
    if (!strcmp(pGroupInfo->m_pName,"jb"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    }else
    if (!strcmp(pGroupInfo->m_pName,"je"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    }else
    if (!strcmp(pGroupInfo->m_pName,"jecxz"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    }else
    if (!strcmp(pGroupInfo->m_pName,"jg"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    }else
    if (!strcmp(pGroupInfo->m_pName,"jge"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    }else
    if (!strcmp(pGroupInfo->m_pName,"jl"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    }else
    if (!strcmp(pGroupInfo->m_pName,"jle"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    }else
    if (!strcmp(pGroupInfo->m_pName,"jne"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    }else
    if (!strcmp(pGroupInfo->m_pName,"jno"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    }else
    if (!strcmp(pGroupInfo->m_pName,"jnp"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    }else
    if (!strcmp(pGroupInfo->m_pName,"jns"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    }
    else
    if (!strcmp(pGroupInfo->m_pName,"jo"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    }
    else
    if (!strcmp(pGroupInfo->m_pName,"jp"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    }
    else
    if (!strcmp(pGroupInfo->m_pName,"js"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForJmps;
        return 1;
    }
    return 0;
}

void Diana_InitLine(DianaCmdKeyLine * pRoot)
{
    int i = 0;
    for(i = 0; i<pRoot->iKeysCount; ++i)
    {
        if (pRoot->key[i].options & DIANA_OPT_HAS_RESULT)
        {
            DianaCmdInfo * pInfo = (DianaCmdInfo * )pRoot->key[i].keyLineOrCmdInfo;
            DianaGroupInfo * pGroupInfo = 0;

            if (pInfo->m_bIsTruePrefix)
                pInfo->m_linkedPrefixFnc = Diana_Nope;
            else
                pInfo->m_linkedPrefixFnc = Diana_Normal;

            
            pGroupInfo = Diana_GetGroupInfo(pInfo->m_lGroupId);
            
            InitJmps(pGroupInfo);
            


            //--------------------------------------
            // AMD 64 ADDITIONAL INITIALIZATION
            //--------------------------------------
            // 1) DI_FLAG_CMD_SUPPORTS_IMM64
            // push 0x68
            // mov  0xB8
            if (pRoot == Diana_GetRootLine())
            {
                switch (pRoot->key[i].chOpcode)
                {
                case 0x68:
                case 0xB8:
                    pInfo->m_flags |= DI_FLAG_CMD_SUPPORTS_IMM64;
                }
            }

            // 2) DI_FLAG_CMD_AMD_DEFAULT_OPSIZE_64
            // a) all who can change CS/IP should have def operand size 64
            if (pGroupInfo->m_pLinkedInfo)
            {
                if (pGroupInfo->m_pLinkedInfo->canChangeCSIP)
                {
                    pInfo->m_flags |= DI_FLAG_CMD_AMD_DEFAULT_OPSIZE_64;
                }
            }
            // b) pop and push should have def operand size 64
            if (!strcmp(pGroupInfo->m_pName,"pop"))
            {
                pInfo->m_flags |= DI_FLAG_CMD_AMD_DEFAULT_OPSIZE_64;
            } else
            if (!strcmp(pGroupInfo->m_pName,"popf"))
            {
                pInfo->m_flags |= DI_FLAG_CMD_AMD_DEFAULT_OPSIZE_64;
            } else
            if (!strcmp(pGroupInfo->m_pName,"push"))
            {
                pInfo->m_flags |= DI_FLAG_CMD_AMD_DEFAULT_OPSIZE_64;
            } else
            if (!strcmp(pGroupInfo->m_pName,"pushf"))
            {
                pInfo->m_flags |= DI_FLAG_CMD_AMD_DEFAULT_OPSIZE_64;
            }
        }
        else
        {
            DianaCmdKeyLine * pLine = (DianaCmdKeyLine * )pRoot->key[i].keyLineOrCmdInfo;
            Diana_InitLine(pLine);
        }
    }
}

void Diana_ResetPrefixes(DianaContext * pContext)
{
    Diana_Normal(pContext);
}
void Diana_Init()
{
    // init prefixes
    DianaCmdKeyLine *  pRoot = Diana_GetRootLine();
    Diana_InitLine(pRoot);

    Diana_InitUtils();
}