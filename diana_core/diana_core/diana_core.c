#include "diana_core.h"
#include "search.h"
#include "string.h"
#include "diana_operands.h"
#include "diana_stream_proxy.h"

static
DianaUnifiedRegister g_regs[3][8] = 
{
    {    reg_AL,    reg_CL,    reg_DL,    reg_BL,    reg_AH,    reg_CH,    reg_DH,    reg_BH},
    {    reg_AX,    reg_CX,    reg_DX,    reg_BX,    reg_SP,    reg_BP,    reg_SI,    reg_DI},
    {    reg_EAX,   reg_ECX,   reg_EDX,   reg_EBX,   reg_ESP,   reg_EBP,   reg_ESI,   reg_EDI},
};

int DianaRecognizeCommonReg(DI_CHAR iOpSize,
                            DI_CHAR regId, 
                            DianaUnifiedRegister * pOut)
{
    int iIndex = 0;
    switch (iOpSize)
    {
    default:
        return DI_ERROR;

    case 4:
        iIndex = 2;
        break;
    case 2:
        iIndex = 1;
        break;
    case 1: 
        iIndex = 0;
    };

    if (regId < sizeof(g_regs[0])/sizeof(g_regs[0][0]))
    {
        *pOut = g_regs[iIndex][regId];
        return DI_SUCCESS;
    }
    return DI_ERROR;

}


void Diana_DispatchSIB(unsigned char sib, int * pSS, int *pIndex, int *pBase)
{
    *pSS = (sib&0xC0)>>6;
    *pIndex = (sib & 0x38) >> 3;
    *pBase = sib & 7;
}
DI_CHAR Diana_GetReg(unsigned char postbyte)
{
    return (postbyte & 0x38) >> 3;
}
DI_CHAR Diana_GetRm(unsigned char postbyte)
{
    return postbyte & 7;
}
DI_CHAR Diana_GetMod(unsigned char postbyte)
{
    return (postbyte & 0xC0) >> 6;
}

static void GetKeyByAddress(const unsigned char * pAddress, int iLostSize, DianaCmdKey * key)
{
    key->keyLineOrCmdInfo = 0;
    key->options = 0;

    if (iLostSize)
        key->extension = Diana_GetReg(*(pAddress+1));
    else
        key->extension = 0;

    key->chOpcode = *pAddress;
}

int Diana_IsLess(const DianaCmdKey * key1, const DianaCmdKey * key2)
{
    if (key1->chOpcode < key2->chOpcode)
        return 1;
    if (key1->chOpcode > key2->chOpcode)
        return 0;
    if (key1->extension == DI_CHAR_NULL)
        return 1;
    if (key2->extension == DI_CHAR_NULL)
        return 0;

    return key1->extension < key2->extension;
}

static DianaCmdKey * FindCmdKey(DianaCmdKeyLine * pLine, const DianaCmdKey * key)
{
    int left  = 0;
    int right = pLine->iKeysCount;
    DI_CHAR cleanKeyOpcode = key->chOpcode&~7;
    while(left < right)
    {
        int mid = (left + right)/2;
        DianaCmdKey * p = pLine->key + mid;
        if (p->chOpcode == key->chOpcode)
        {
            if (p->extension == DI_CHAR_NULL)
                return p; 
            if (p->extension == key->extension)
                return p;
        }
        else
        if ((p->chOpcode >= cleanKeyOpcode) && (p->chOpcode < cleanKeyOpcode + 8))
        {
            if (p->options & DIANA_OPT_HAS_RESULT)
            {
                DI_CHAR reg= ((DianaCmdInfo * )p->keyLineOrCmdInfo)->m_iRegisterCodeAsOpcodePart;
                if (reg != DI_CHAR_NULL && reg)
                    return p;
            }
        }
        
        if (Diana_IsLess(key, p))
            right = mid;
        else 
            left = mid+1;
    }
    return 0;
}

int Diana_ParseCmdImpl(DianaContext * pContext, //IN
                   DianaCmdKeyLine * pInitialLine,  // IN
                   DianaReadStream * readStream,    // IN
                   DianaParserResult * pResult)    // OUT
{
    DianaCmdKeyLine * pLine = pInitialLine;

    int readed = 0;
    int iResult = 0;
    int it= 0;

    memset(pResult, 0, sizeof(DianaParserResult));

    if (pContext->iReadedSize!= DI_CACHE_SIZE)
    {
        /// first read 
        iResult = readStream->pReadFnc(readStream, 
                                       pContext->readed+pContext->iReadedSize, 
                                       DI_CACHE_SIZE-pContext->iReadedSize, 
                                       &readed);
        if (iResult != DI_SUCCESS)
            return iResult;
    
        pContext->iReadedSize += readed;
    }

    if (pContext->iReadedSize == 0)
        return DI_END;

    do
    {
        {
            DianaCmdKey * pFoundKey = 0;
            DianaCmdKey key;
            
            GetKeyByAddress(pContext->readed+it, pContext->iReadedSize - it - 1, &key);

            pFoundKey = FindCmdKey(pLine, &key);
            if (!pFoundKey)
                return DI_ERROR;

            if (pFoundKey->options & DIANA_OPT_HAS_RESULT)
            {
                DianaStreamProxy proxy;
                int iResult = 0;
                Diana_InitStreamProxy(&proxy, 
                                      readStream,
                                      pContext->readed+it, 
                                      pContext->iReadedSize-it);
                // fill result
                pResult->iLinkedOpCount = 0;
                pResult->pInfo = (DianaCmdInfo * )pFoundKey->keyLineOrCmdInfo;
                iResult = Diana_LinkOperands( pContext, pResult, &proxy.parent);
                
                if (proxy.tail_size)
                {
                    memmove(pContext->readed, proxy.pBuffer + proxy.begin, proxy.tail_size);
                }
                pContext->iReadedSize = proxy.tail_size;
                return iResult;
            } 
            else
            {
                pLine = (DianaCmdKeyLine *)pFoundKey->keyLineOrCmdInfo;
            }
        }

        ++it;

        if (it >= pContext->iReadedSize)
        {
            return DI_ERROR;
        }
    }
    while(1);
}

int Diana_ParseCmd(DianaContext * pContext, //IN
                   DianaCmdKeyLine * pInitialLine,  // IN
                   DianaReadStream * readStream,    // IN
                   DianaParserResult * pResult)    // OUT
{
    while(1)
    {
        int iRes = Diana_ParseCmdImpl(pContext, 
                                      pInitialLine,  
                                      readStream,    
                                      pResult);
        if (iRes != DI_SUCCESS)
            return iRes;

        // save old prefix as result
        pResult->iPrefix = pContext->iPrefix;

        // prefix
        pResult->pInfo->m_linkedPrefixFnc( pContext );

        if (!pResult->pInfo->m_bIsTruePrefix)
            return iRes;
    }
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

void Diana_InitContext(DianaContext * pThis, int Mode)
{
    memset(pThis, 0, sizeof(*pThis));

    pThis->iMainMode_opsize = Mode;   
    pThis->iCurrentCmd_opsize = Mode; 

    pThis->iMainMode_addressSize = Mode;    
    pThis->iCurrentCmd_addressSize = Mode;  

    pThis->mainMode_sreg = reg_DS;
    pThis->currentCmd_sreg = reg_DS;
}

// debug
static void Diana_Nope(struct _dianaContext * pContext) 
{  
    __asm int 3;
}
// normal
static void Diana_Normal(struct _dianaContext * pContext) 
{  
    pContext->iCurrentCmd_opsize = pContext->iMainMode_opsize;
    pContext->currentCmd_sreg = pContext->mainMode_sreg;
    pContext->iCurrentCmd_addressSize = pContext->iMainMode_addressSize;
    pContext->iPrefix = 0;
}

static void Diana_Lock(struct _dianaContext * pContext) 
{  
    pContext->iPrefix = DI_PREFIX_LOCK;
}
static void Diana_Rep(struct _dianaContext * pContext) 
{  
    pContext->iPrefix = DI_PREFIX_REP;
}
static void Diana_Repn(struct _dianaContext * pContext) 
{  
    pContext->iPrefix = DI_PREFIX_REPN;
}

// sreg:
static void Diana_CS(struct _dianaContext * pContext) 
{  
    pContext->currentCmd_sreg = reg_CS;
}
static void Diana_SS(struct _dianaContext * pContext) 
{  
    pContext->currentCmd_sreg = reg_SS;
}
static void Diana_DS(struct _dianaContext * pContext) 
{  
    pContext->currentCmd_sreg = reg_DS;
}
static void Diana_ES(struct _dianaContext * pContext) 
{  
    pContext->currentCmd_sreg = reg_ES;
}
static void Diana_FS(struct _dianaContext * pContext) 
{  
    pContext->currentCmd_sreg = reg_FS;
}
static void Diana_GS(struct _dianaContext * pContext) 
{  
    pContext->currentCmd_sreg = reg_GS;
}
// operand size override
static void Diana_OpSize(struct _dianaContext * pContext) 
{  
    pContext->iCurrentCmd_opsize = 6 - pContext->iMainMode_opsize;
}
static void Diana_AddrSize(struct _dianaContext * pContext) 
{  
    pContext->iCurrentCmd_addressSize = 6 - pContext->iMainMode_addressSize;
}

static Diana_LinkedAdditionalGroupInfo g_infoForCalls = {1, 0, 1};
static Diana_LinkedAdditionalGroupInfo g_infoForJmps = {0, 1, 1};


static int InitJmps(DianaGroupInfo * pGroupInfo)
{
    if (!strcmp(pGroupInfo->m_pName,"call"))
    {
        pGroupInfo->m_pLinkedInfo = &g_infoForCalls;
        return 1;
    } else
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
            
            if (!InitJmps(pGroupInfo))
            {
                // general
                if (!strcmp(pGroupInfo->m_pName,"lock"))
                {
                    pInfo->m_linkedPrefixFnc = Diana_Lock;
                } else
                if (!strcmp(pGroupInfo->m_pName,"rep"))
                {
                    pInfo->m_linkedPrefixFnc = Diana_Rep;
                } else
                if (!strcmp(pGroupInfo->m_pName,"repn"))
                {
                    pInfo->m_linkedPrefixFnc = Diana_Repn;
                } else

                // modifiers
                if (!strcmp(pGroupInfo->m_pName,"prefix_operandsize"))
                {
                    pInfo->m_linkedPrefixFnc = Diana_OpSize;
                } else
                if (!strcmp(pGroupInfo->m_pName,"prefix_addresssize"))
                {
                    pInfo->m_linkedPrefixFnc = Diana_AddrSize;
                } else
                if (!strcmp(pGroupInfo->m_pName,"prefix_cs"))
                {
                    pInfo->m_linkedPrefixFnc = Diana_CS;
                } else
                if (!strcmp(pGroupInfo->m_pName,"prefix_ss"))
                {
                    pInfo->m_linkedPrefixFnc = Diana_SS;
                } else
                if (!strcmp(pGroupInfo->m_pName,"prefix_ds"))
                {
                    pInfo->m_linkedPrefixFnc = Diana_DS;
                } else
                if (!strcmp(pGroupInfo->m_pName,"prefix_es"))
                {
                    pInfo->m_linkedPrefixFnc = Diana_ES;
                } else
                if (!strcmp(pGroupInfo->m_pName,"prefix_fs"))
                {
                    pInfo->m_linkedPrefixFnc = Diana_FS;
                } else
                if (!strcmp(pGroupInfo->m_pName,"prefix_gs"))
                {
                    pInfo->m_linkedPrefixFnc = Diana_GS;
                } 
            }
        }
        else
        {
            DianaCmdKeyLine * pLine = (DianaCmdKeyLine * )pRoot->key[i].keyLineOrCmdInfo;
            Diana_InitLine(pLine);
        }
    }
}
void Diana_Init()
{
    // init prefixes
    DianaCmdKeyLine *  pRoot = Diana_GetRootLine();
    Diana_InitLine(pRoot);
  
}