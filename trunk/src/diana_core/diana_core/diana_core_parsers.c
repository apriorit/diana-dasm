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
    {
        key->extension = Diana_GetReg(*(pAddress+1));
        key->rmExtension = Diana_GetRm(*(pAddress+1));
        key->modExtension = Diana_GetMod(*(pAddress+1));
    }
    else
    {
        key->extension = DI_CHAR_NULL;
        key->rmExtension = DI_CHAR_NULL;
        key->modExtension = DI_CHAR_NULL;
    }

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

static int logical_xor(int bValue1, int bValue2)
{
    int val1 = bValue1 ? 1 : 0;
    int val2 = bValue2 ? 1 : 0;
    return val1 ^ val2;
}

static void CompareKeys(DianaCmdKey * pUsedKey, 
                        DianaCmdKey * p, 
                        DianaCmdKey ** ppSecondaryResult,
                        DianaCmdKey ** ppResult)
{
    if (p->extension == DI_CHAR_NULL)
    {
        *ppSecondaryResult = p;
    }
    else
    {
        if (p->extension == pUsedKey->extension)
        {
            if (p->options & DIANA_OPT_RM_EXTENSION)
            {
                if (logical_xor(p->rmExtension & DI_VALUE_FLAG_CMD_REVERSE,
                               ((DI_VALUE_FLAG_CMD_MASK & p->rmExtension) == pUsedKey->rmExtension)))
                    *ppResult = p;
            }
            else
            if (p->options & DIANA_OPT_USES_MOD_EXTENSION)
            {
                if (logical_xor(p->modExtension & DI_VALUE_FLAG_CMD_REVERSE, 
                                ((DI_VALUE_FLAG_CMD_MASK & p->modExtension) == pUsedKey->modExtension)))
                    *ppResult = p;
            }
            else
                *ppResult = p;
        }
    }
}

static DianaCmdKey *FindCmdKeyImpl(int left,
                                   int right,
                                   DianaCmdKeyLine *pLine, 
                                   const DianaCmdKey *pKey,
                                   DianaCmdKeyLine **pNextLine,
                                   int bCleanOpcode)
{
	int foundClean = 0;
	int nextLeft = 0;
	int nextRight = pLine->iKeysCount;
	DianaCmdKey usedKey = *pKey;
	unsigned char cleanOpcode = usedKey.chOpcode & ~7;
	*pNextLine = 0;

	if (bCleanOpcode)
	{
		usedKey.chOpcode &= ~7;
	}
	while(left < right)
	{
		int mid = ( left + right ) / 2;
		DianaCmdKey *p = pLine->key + mid;

		if (bCleanOpcode == 0 && foundClean == 0 && p->chOpcode == cleanOpcode ) {
			foundClean = 1;
			nextLeft = left;
			nextRight = right;
		}
		if (p->chOpcode == usedKey.chOpcode)
		{
			// opcode found
			// analyze it
			int i = 0;
			DianaCmdKey *pResult = 0;
			DianaCmdKey *pSecondaryResult = 0;
			DianaCmdKey *pBegin = p;

			--pBegin;
			while(pBegin >= pLine->key) {
				if (pBegin->chOpcode != usedKey.chOpcode) {
					break;
				}
				--pBegin;
			}
			++pBegin;

			p = pBegin;
			i = (int)(p - pLine->key);
			pResult = 0;

			for( ; (i < pLine->iKeysCount) && (p->chOpcode == usedKey.chOpcode); ++i, ++p) {
				// check extension
				if (!(p->options & DIANA_OPT_HAS_RESULT)) {
					*pNextLine = (DianaCmdKeyLine * )p->keyLineOrCmdInfo;
					pSecondaryResult = p;
				}

				if (bCleanOpcode) {
					DianaCmdInfo * pCmdInfo = (DianaCmdInfo * )p->keyLineOrCmdInfo;
					DI_CHAR reg = pCmdInfo->m_iRegisterCodeAsOpcodePart;
					if ((reg != DI_CHAR_NULL) || (pCmdInfo->m_flags & DI_FLAG_CMD_FPU_I)) {
						pResult = p;
					}
				} else {
					// not clean opcode
					CompareKeys(&usedKey, p, &pSecondaryResult, &pResult);
				}
			}

			if (!pResult)
				pResult = pSecondaryResult;

			return pResult;
		}

		if (Diana_IsLess(&usedKey, p))
			right = mid;
		else 
			left = mid + 1;
	}

	if( bCleanOpcode == 0 ) {
		return FindCmdKeyImpl( nextLeft, nextRight, pLine, pKey, pNextLine, 1 );
	}

	return 0;
}

static DianaCmdKey * FindCmdKey(DianaCmdKeyLine * pLine, 
                                const DianaCmdKey * pKey,
                                DianaCmdKeyLine ** pNextLine)
{
    return FindCmdKeyImpl(0,
                          pLine->iKeysCount,
                          pLine, 
                          pKey,
                          pNextLine,
                          0);
}

int Diana_InsertPrefix(DianaContext * pContext, 
                       DI_CHAR prefix,
                       Diana_PrefixFnc linkedPrefixFnc,
                       int isRex
                       )
{
    DI_FULL_CHAR i = 0;

	if (pContext->prefixesCount >= DI_MAX_PREFIXES_COUNT) 
		return DI_ERROR;

    for(; i < pContext->prefixesCount; ++i)
    {
        if (pContext->prefixes[i].prefix == prefix)
            return DI_SUCCESS;
    }

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

static int KeyIsAcceptable(DianaCmdKey * p, 
                           DI_CHAR lastData)
{
    DianaCmdKey * pRes1 = 0, * pRes2 = 0;
    DianaCmdKey key;
   
    GetKeyByAddress(&lastData, 
                    0, 
                    &key);
    if (key.chOpcode != p->chOpcode)
        return 0;

    CompareKeys(p, &key, &pRes1, &pRes2);
    if (pRes1 || pRes2)
        return 1;
    return 0;
}
static DianaCmdKey * ScanLeft(DianaCmdKeyLine * pLine, 
                              DianaCmdKey * pFoundKey, 
                              DianaParseParams * pParseParams,
                              DI_CHAR lastData)
{
    DianaCmdKey * p = pFoundKey;
    --p;
    while(p > pLine->key)
    {
        if (!KeyIsAcceptable(p, lastData))
        {
            break;
        }

        if (TryMatchKey(p, pParseParams))
            return p;
        --p;
    }
    return 0;
}

static DianaCmdKey * ScanRight(DianaCmdKeyLine * pLine, 
                               DianaCmdKey * pFoundKey, 
                               DianaParseParams * pParseParams,
                               DI_CHAR lastData)
{
    size_t number = pFoundKey - pLine->key;
    DianaCmdKey * p = pFoundKey;
    
    ++p;
    ++number;
    
    for( ; number < (size_t)pLine->iKeysCount; ++number)
    {
        if (!KeyIsAcceptable(p, lastData))
        {
            break;
        }

        if (TryMatchKey(p, pParseParams))
            return p;
    }
    return 0;
}

static
int Diana_ParseCmdImpl(DianaParseParams * pParseParams, // IN
                       DianaStreamProxy * pOutProxy, // OUT
                       int * pOutIt)    // OUT
{
    DianaCmdKeyLine * pLine = pParseParams->pInitialLine;

    //int readed = 0;
    //int iResult = 0;
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

                // use previous solution
                pFoundKey = pFoundKeyWithResult;
                it = resultIt;
                pNextLine = 0;
            }

            if (pFoundKey->options & DIANA_OPT_HAS_RESULT)
            {
                //int iResult = 0;

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
                    pFoundKey = ScanLeft(pLine, 
                                         pFoundKey, 
                                         pParseParams,
                                         pParseParams->pContext->cache[it]);
                    if (!pFoundKey)
                    {
                        pFoundKey = ScanRight(pLine, 
                                              pFoundKey, 
                                              pParseParams,
                                              pParseParams->pContext->cache[it]);
                    }
                    if (!pFoundKey)
                    {
                        return DI_UNSUPPORTED_COMMAND;
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
                pLine = (DianaCmdKeyLine * )pFoundKey->keyLineOrCmdInfo;
            }
        }

        ++it;

        if (it-pParseParams->pContext->cacheIt >= pParseParams->pContext->cacheSize)
        {
            return DI_ERROR;
        }
    }
	#pragma warning( suppress : 4127 ) // conditional expression is constant
    while(1);
}

static void ApplyPrefixes(DianaContext * pContext,
                          unsigned int iToExclude)
{
    DI_FULL_CHAR i = 0;
    for( ; i < pContext->prefixesCount; ++i)
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

        if (pParseParams->pContext->iSizePrefixes & DIANA_INVALID_STATE)
        {
            return DI_ERROR;
        }

        if (oldPrefixFound)
        {
            pParseParams->pContext->iRexPrefix = 0;
            pParseParams->pContext->iRexPrefixValue = 0;
        }

        pParseParams->pContext->cacheIt = newIt;
        iResult = Diana_LinkOperands(pParseParams->pContext, 
                                     pParseParams->pResult, 
                                     &cmdProxy.parent);
        
        pParseParams->pContext->cacheSize = cmdProxy.tail_size;
        pParseParams->pContext->cacheIt += oldTailSize - cmdProxy.tail_size;

        if (pParseParams->pContext->cacheIt < DI_CACHE_RESERVED)
        {
            Diana_FatalBreak();
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
    DI_CHAR data = 0;
    int dataValid = 1;
    int iBytesCounter = 0;
    int iOriginalCacheSize = 0;
    int nopFlags = 0;
    pParseParams->pContext->prefixesCount = 0;
    
    Diana_ResetPrefixes(pParseParams->pContext);
    memset(pParseParams->pResult, 0, sizeof(DianaParserResult));

    // check prefixes
    for(;;++iBytesCounter)
    {
		if( iBytesCounter > DI_MAX_PREFIXES_COUNT )
			return DI_ERROR;
        iResult = Diana_ReadCache(pParseParams->pContext, 
                                  pParseParams->readStream, 
                                  0);
        if (iResult)
            return iResult;

        data = Diana_CacheEatOne(pParseParams->pContext);

        if (!iBytesCounter || !nopFlags)
        {
            // check nop
            switch(data)
            {
            case 0x90:
                pParseParams->pResult->iFullCmdSize = 1;
                pParseParams->pResult->iLinkedOpCount = 0;
                pParseParams->pResult->pInfo = Diana_GetNopInfo();
                return DI_SUCCESS;
            };
        }

        // parse prefixes
        nopFlags |= Di_ProcessCustomPrefix(data, 
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
