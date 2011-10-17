#include "diana_core_parsers.h"
#include "diana_core_utils.h"
#include "diana_stream_proxy.h"
#include "string.h"
#include "diana_operands.h"

static void GetKeyByAddress(const unsigned char * pAddress, int iLostSize, DianaCmdKey * key)
{
    key->keyLineOrCmdInfo = 0;
    key->options = 0;

    if (iLostSize)
        key->extension = Diana_GetReg(*(pAddress+1));
    else
        key->extension = DI_CHAR_NULL;

    key->chOpcode = *pAddress;
}

static int Diana_IsLess(const DianaCmdKey * key1, const DianaCmdKey * key2)
{
    if (key1->chOpcode < key2->chOpcode)
        return 1;
    if (key1->chOpcode > key2->chOpcode)
        return 0;
    return 0;
}


static DianaCmdKey * FindCmdKeyImpl(DianaCmdKeyLine * pLine, 
                                    const DianaCmdKey * pKey,
                                    DianaCmdKeyLine ** pNextLine,
                                    int bCleanOpcode)
{
    int left  = 0;
    int right = pLine->iKeysCount;
    DianaCmdKey usedKey = *pKey;
    *pNextLine = 0;

    
    if (bCleanOpcode)
    {
        usedKey.chOpcode &= ~7;
    }
    while(left < right)
    {
        int mid = (left + right)/2;
        DianaCmdKey * p = pLine->key + mid;
        if (p->chOpcode == usedKey.chOpcode)
        {
            // opcode found
            // analyze it
            int i =0;
            DianaCmdKey * pResult = 0;
            DianaCmdKey * pSecondaryResult = 0;
            DianaCmdKey * pBegin = p;

            --pBegin;
            while(pBegin >= pLine->key)
            {
                if (pBegin->chOpcode != usedKey.chOpcode)
                {
                    break;
                }
                --pBegin;
            }
            ++pBegin;
            
            p = pBegin;
            i = (int)(p - pLine->key);
            pResult = 0;
            
            for(; (i < pLine->iKeysCount) && (p->chOpcode == usedKey.chOpcode); ++i, ++p)
            {
                if (bCleanOpcode)
                {
                    DianaCmdInfo * pCmdInfo = (DianaCmdInfo * )p->keyLineOrCmdInfo;
                    DI_CHAR reg= pCmdInfo->m_iRegisterCodeAsOpcodePart;
                    if ((reg != DI_CHAR_NULL) || (pCmdInfo->m_flags & DI_FLAG_CMD_FPU_I))
                    {
                        pResult = p;
                    }
                }
                else
                {
                    if (p->options & DIANA_OPT_HAS_RESULT)
                    {
                        {
                            if (p->extension == DI_CHAR_NULL)
                            {
                                pSecondaryResult = p;
                            }
                            else
                            {
                                if (p->extension == usedKey.extension)
                                {
                                    pResult = p;
                                }
                            }
                        }
                    }
                    else
                    {
                        *pNextLine = (DianaCmdKeyLine * )p->keyLineOrCmdInfo;
                        pSecondaryResult = p;
                    }
                }
                if (p->chOpcode != usedKey.chOpcode)
                    break;
             }
 
             if (!pResult)
                    pResult = pSecondaryResult;

             return pResult;
        }
        
        if (Diana_IsLess(&usedKey, p))
            right = mid;
        else 
            left = mid+1;
    }
    return 0;
}

static DianaCmdKey * FindCmdKey(DianaCmdKeyLine * pLine, 
                                const DianaCmdKey * pKey,
                                DianaCmdKeyLine ** pNextLine)
{
    DianaCmdKey * pResult = FindCmdKeyImpl(pLine, 
                                           pKey,
                                           pNextLine,
                                           0);
    if (!pResult)
    {
        pResult = FindCmdKeyImpl(pLine, 
                                 pKey,
                                 pNextLine,
                                 1);
    }
    return pResult;
}


int Diana_InsertPrefix(DianaContext * pContext, 
                       DI_CHAR prefix,
                       Diana_PrefixFnc linkedPrefixFnc,
                       int isRex
                       )
{
    DI_FULL_CHAR i = 0;
    for(; i<  pContext->prefixesCount; ++i)
    {
        if (pContext->prefixes[i].prefix == prefix)
            return DI_SUCCESS;
    }

    if (pContext->prefixesCount >= DI_MAX_PREFIXES_COUNT) 
        return DI_ERROR;
    
    pContext->prefixes[pContext->prefixesCount].prefix = prefix;
    pContext->prefixes[pContext->prefixesCount].linkedPrefixFnc = linkedPrefixFnc;
    ++pContext->prefixesCount;

    if (!isRex)
        pContext->lastPrefixBeforeRex = i;

    return DI_SUCCESS;
}


static int TryMatchKey(DianaCmdKey * pFoundKey, DianaParseParams * pParseParams)
{
    DI_CHAR flagMask = 0;
    DianaCmdInfo * pInfo = (DianaCmdInfo * )pFoundKey->keyLineOrCmdInfo;
    
    flagMask = pInfo->m_flags & (DI_FLAG_CMD_AMD64 | DI_FLAG_CMD_I386);
    if (!flagMask)
        return 1;

    if (flagMask & DI_FLAG_CMD_AMD64 &&
        pParseParams->pContext->iMainMode_addressSize == DIANA_MODE64)
    {
        return 1;
    }

    if (flagMask & DI_FLAG_CMD_I386 &&
        pParseParams->pContext->iMainMode_addressSize == DIANA_MODE32)
    {
        return 1;
    }
    return 0;
}

static DianaCmdKey * ScanLeft(DianaCmdKeyLine * pLine, 
                              DianaCmdKey * pFoundKey, 
                              DianaParseParams * pParseParams)
{
    --pFoundKey;
    while(pFoundKey > pLine->key)
    {
        if (TryMatchKey(pFoundKey, pParseParams))
            return pFoundKey;
        --pFoundKey;
    }
    return 0;
}

static DianaCmdKey * ScanRight(DianaCmdKeyLine * pLine, 
                              DianaCmdKey * pFoundKey, 
                              DianaParseParams * pParseParams)
{
    size_t number = pFoundKey - pLine->key;
    
    ++pFoundKey;
    ++number;
    
    for( ; number < (size_t)pLine->iKeysCount; ++number)
    {
        if (TryMatchKey(pFoundKey, pParseParams))
            return pFoundKey;
    }
    return 0;
}

static
int Diana_ParseCmdImpl(DianaParseParams * pParseParams, // IN
                       DianaStreamProxy * pOutProxy, // OUT
                       int * pOutIt)    // OUT
{
    DianaCmdKeyLine * pLine = pParseParams->pInitialLine;

    int readed = 0;
    int iResult = 0;
    int it = pParseParams->pContext->cacheIt;
    DianaCmdKey * pFoundKeyWithResult = 0;
    int resultIt = 0;

    do
    {
        {
            DianaCmdKey * pFoundKey = 0;
            DianaCmdKey key;
            DianaCmdKeyLine * pNextLine = 0;
            
            GetKeyByAddress(pParseParams->pContext->cache + it, 
                            pParseParams->pContext->cacheIt + pParseParams->pContext->cacheSize - it - 1, 
                            &key);
            pFoundKey = FindCmdKey(pLine, &key, &pNextLine);
            if (!pFoundKey)
            {
                if (!pFoundKeyWithResult)
                    return DI_ERROR;

                // use previos solution
                pFoundKey = pFoundKeyWithResult;
                it = resultIt;
                pNextLine = 0;
            }

            if (pFoundKey->options & DIANA_OPT_HAS_RESULT)
            {
                int iResult = 0;

                if (pNextLine && it+1 <= pParseParams->pContext->cacheSize)
                {
                    // try to get more
                    pLine = pNextLine;
                    pFoundKeyWithResult = pFoundKey;
                    resultIt = it;
                    ++it;
                    continue;
                }

                // found data, check processor mode
                if (!TryMatchKey(pFoundKey, pParseParams))
                {
                    // not matched totally, scan left or right
                    pFoundKey = ScanLeft(pLine, pFoundKey, pParseParams);
                    if (!pFoundKey)
                    {
                        pFoundKey = ScanRight(pLine, pFoundKey, pParseParams);
                    }
                    if (!pFoundKey)
                    {
                        return DI_ERROR;
                    }
                }

                Diana_InitStreamProxy(pOutProxy, 
                                      pParseParams->readStream,
                                      pParseParams->pContext->cache+it, 
                                      pParseParams->pContext->cacheIt + pParseParams->pContext->cacheSize - it);
                *pOutIt = it;

                pParseParams->pResult->pInfo = (DianaCmdInfo * )pFoundKey->keyLineOrCmdInfo;
                return DI_SUCCESS;
            } 
            else
            {
                pLine = (DianaCmdKeyLine *)pFoundKey->keyLineOrCmdInfo;
            }
        }

        ++it;

        if (it-pParseParams->pContext->cacheIt >= pParseParams->pContext->cacheSize)
        {
            return DI_ERROR;
        }
    }
    while(1);
}

static void ApplyPrefixes(DianaContext * pContext,
                          int iToExclude)
{
    DI_FULL_CHAR i = 0;
    for(; i<  pContext->prefixesCount; ++i)
    {
        if (i != iToExclude)
        {
            pContext->prefixes[i].linkedPrefixFnc(pContext);
        }
    }
}

int TryMatch(DianaParseParams * pParseParams,
             int dataValid,
             DI_CHAR data,
             int iOriginalCacheSize,
             DI_FULL_CHAR iCurrentPrefix)
{
    DianaStreamProxy cmdProxy;
    int iResult = 0;
    int newIt = 0;
    int oldPrefixFound = 0;

    if (iCurrentPrefix == DI_FULL_CHAR_NULL)
    {
        if (dataValid)
        {
            pParseParams->pContext->cache[1] = data;
            pParseParams->pContext->cacheIt = 1;
            pParseParams->pContext->cacheSize = iOriginalCacheSize + 1;        
        }
        else
        {
            pParseParams->pContext->cacheIt = DI_CACHE_RESERVED;
            pParseParams->pContext->cacheSize = iOriginalCacheSize;
        }
    }
    else
    {
        if (dataValid)
        {
            pParseParams->pContext->cache[0] = pParseParams->pContext->prefixes[iCurrentPrefix].prefix;
            pParseParams->pContext->cache[1] = data;
            pParseParams->pContext->cacheIt = 0;
            pParseParams->pContext->cacheSize = iOriginalCacheSize + 2;
        }
        else
        {
            pParseParams->pContext->cache[1] = pParseParams->pContext->prefixes[iCurrentPrefix].prefix;
            pParseParams->pContext->cacheIt = 1;
            pParseParams->pContext->cacheSize = iOriginalCacheSize + 1;
        }
    }

    // analyze first byte 
    if (iCurrentPrefix == DI_FULL_CHAR_NULL)
    {
        // [REX][OLDPREFIX][CMD] case
        Di_ProcessCustomPrefix(pParseParams->pContext->cache[pParseParams->pContext->cacheIt],
                                &oldPrefixFound,
                                pParseParams->pContext,
                                0);
    }
                      

    
    newIt = 0;
    iResult = Diana_ParseCmdImpl(pParseParams,
                                 &cmdProxy,
                                 &newIt);
    if (!iResult)
    {
        // something found
        int oldTailSize = cmdProxy.tail_size;
        ApplyPrefixes(pParseParams->pContext, 
                      iCurrentPrefix); // exclude iCurrentPrefix

        if (oldPrefixFound)
        {
            pParseParams->pContext->iRexPrefix = 0;
            pParseParams->pContext->iRexPrefixValue = 0;
        }

        pParseParams->pContext->cacheIt = newIt;
        iResult = Diana_LinkOperands( pParseParams->pContext, 
                                      pParseParams->pResult, 
                                      &cmdProxy.parent);
        
        pParseParams->pContext->cacheSize = cmdProxy.tail_size;
        pParseParams->pContext->cacheIt += oldTailSize - cmdProxy.tail_size;

        if (pParseParams->pContext->cacheIt < DI_CACHE_RESERVED)
        {
            Diana_FatalBreak();;
        }
        return iResult;
    }

    pParseParams->pContext->cacheIt = DI_CACHE_RESERVED;
    pParseParams->pContext->cacheSize = iOriginalCacheSize;
    return iResult;
}

int Diana_ParseCmdEx(DianaParseParams * pParseParams)    // OUT
{
    int iResult = 0;
    int prefixFound = 0;
    DI_FULL_CHAR i = 0;
    DI_CHAR data = 0;
    int dataValid = 1;
    int iOriginalCacheSize = 0;
    pParseParams->pContext->prefixesCount = 0;
    
    Diana_ResetPrefixes(pParseParams->pContext);
    memset(pParseParams->pResult, 0, sizeof(DianaParserResult));

    // check prefixes
    while(1)
    {
        iResult = Diana_ReadCache(pParseParams->pContext, 
                                  pParseParams->readStream, 
                                  0);
        if (iResult)
            return iResult;

        data = Diana_CacheEatOne(pParseParams->pContext);

        // parse prefixes
        Di_ProcessCustomPrefix(data, 
                               &prefixFound,
                               pParseParams->pContext, 
                               pParseParams->pResult);
    
        if (!prefixFound)
            break;

        iResult = Diana_InsertPrefix(pParseParams->pContext, 
                                     data,
                                     pParseParams->pResult->pInfo->m_linkedPrefixFnc,
                                     0);
        if (iResult)
        {
            return iResult;
        }
    }

     // check rex prefix
    if (pParseParams->pContext->iAMD64Mode)
    {
        iResult = Di_ProcessRexPrefix(data, 
                                      &prefixFound, 
                                      pParseParams->pContext, 
                                      pParseParams->pResult);
        if (iResult)
            return iResult;

        if (prefixFound)
        {
            iResult = Diana_InsertPrefix(pParseParams->pContext,
                                        data,
                                        pParseParams->pResult->pInfo->m_linkedPrefixFnc,
                                        1);
            if (iResult)
            {
                return iResult;
            }
            dataValid = 0;
        }
    }

    iResult = Diana_ReadCache(pParseParams->pContext, 
                              pParseParams->readStream, 
                              1);
    if (iResult && !dataValid)
        return iResult;


    pParseParams->pResult->pInfo = 0;

    // parse cmd 
    iOriginalCacheSize = pParseParams->pContext->cacheSize;


    //    WARNING!
    //    now parser implemented as in distorm [http://code.google.com/p/distorm/wiki/x86_x64_Machine_Code]
    //    i.e. by using "mandatory prefix"
    //    intel documentation say nothing about "mandatory prefix" 
    //    also windbg can unparse something like this
    //       fffff800`028c2f53 66f3440f7f8990000000 rep  movdqa oword ptr [rcx+0x90],xmm9
    //    this instruction windbg interprets as 660f7f8990000000
    //    but distorm and diana interpreting it as  f30f7f8990000000
    //    because nobody knows real priority of prefixes
    //    for(i=0; i <  (DI_FULL_CHAR)pParseParams->pContext->prefixesCount; ++i)


    if (pParseParams->pContext->lastPrefixBeforeRex != DI_FULL_CHAR_NULL)
    {
        iResult = TryMatch(pParseParams,
                           dataValid,
                           data,
                           iOriginalCacheSize,
                           pParseParams->pContext->lastPrefixBeforeRex);

        if (!iResult)
            return iResult;
    }

    // prefixes not matched as commands, try to match command
    iResult = TryMatch(pParseParams,
                       dataValid,
                       data,
                       iOriginalCacheSize,
                       DI_FULL_CHAR_NULL);

    if (iResult && !dataValid)
    {
        if (pParseParams->pContext->cacheSize)
            Diana_CacheEatOne(pParseParams->pContext);
    }
    return iResult;
}
