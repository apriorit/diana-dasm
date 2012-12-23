#include "diana_stack.h"
#include "stdlib.h"

int Diana_Stack_Init(Diana_Stack * pStack,
                     int minBlockSize,
                     int dataSize)
{
    memset(pStack, 0, sizeof(*pStack));
    Diana_InitList(&pStack->m_blockList);
    pStack->m_minBlockSize = minBlockSize;
    pStack->m_dataSize = dataSize;
    return DI_SUCCESS;
}

static int deleter(Diana_ListNode * pNode,
                   void * pContext,
                   int * pbDone)
{
    Diana_StackBlock * pBlock = (Diana_StackBlock*)pNode;
    free(pBlock);

	pContext;
	pbDone;

    return DI_SUCCESS;
}

void Diana_Stack_Free(Diana_Stack * pStack)
{
    Diana_ListForEach(&pStack->m_blockList, deleter, 0);
}

int Diana_Stack_Push(Diana_Stack * pStack,
                     const void * pData)
{
    char * pDataPlace = 0;
    if (pStack->m_pCurrentBlock)
    {
		#pragma warning( suppress : 4127 ) // conditional expression is constant
        while(1)
        {
            if (pStack->m_minBlockSize - 
                pStack->m_pCurrentBlock->m_curSizeInBytes >= pStack->m_dataSize)
            {
                pDataPlace = (char*)pStack->m_pCurrentBlock + sizeof(Diana_StackBlock) + pStack->m_pCurrentBlock->m_curSizeInBytes;
                memcpy(pDataPlace, pData, pStack->m_dataSize);
                
                pStack->m_pCurrentBlock->m_curSizeInBytes += pStack->m_dataSize;
                ++pStack->m_count;
                return DI_SUCCESS;
            }
            if (pStack->m_pCurrentBlock->m_entry.m_pNext)
            {
                pStack->m_pCurrentBlock = (Diana_StackBlock*)pStack->m_pCurrentBlock->m_entry.m_pNext;
                continue;
            }     
            break;
        }        
    }
    // need insert
    {
        Diana_StackBlock * pBlock = malloc(sizeof(Diana_StackBlock)+pStack->m_minBlockSize);
        if (!pBlock)
            return DI_OUT_OF_MEMORY;

        memset(pBlock, 0, sizeof(*pBlock));

        pDataPlace = (char*)pBlock + sizeof(Diana_StackBlock);
        memcpy(pDataPlace, pData, pStack->m_dataSize);
        pBlock->m_curSizeInBytes = pStack->m_dataSize;

        Diana_PushBack(&pStack->m_blockList, &pBlock->m_entry);
        pStack->m_pCurrentBlock = pBlock;
        ++pStack->m_count;
    }
    return DI_SUCCESS;
}
int Diana_Stack_Pop(Diana_Stack * pStack,
                    void * pData)
{
    char * pDataPlace = Diana_Stack_GetTopPtr(pStack);
    if (!pDataPlace)
        return DI_ERROR;

    memcpy(pData, pDataPlace, pStack->m_dataSize);
    pStack->m_pCurrentBlock->m_curSizeInBytes -= pStack->m_dataSize;
    --pStack->m_count;
    return DI_SUCCESS;

}
void * Diana_Stack_GetTopPtr(Diana_Stack * pStack)
{
    char * pDataPlace = 0;
    if (!pStack->m_pCurrentBlock)
        return 0;
    if (!pStack->m_count)
        return 0;

    if (pStack->m_pCurrentBlock->m_curSizeInBytes < pStack->m_dataSize)
    {
        pStack->m_pCurrentBlock = (Diana_StackBlock*)pStack->m_pCurrentBlock->m_entry.m_pPrev;
    }
    if (!pStack->m_pCurrentBlock)
    {
        return 0;
    }

    pDataPlace = (char*)pStack->m_pCurrentBlock + sizeof(Diana_StackBlock) + pStack->m_pCurrentBlock->m_curSizeInBytes - pStack->m_dataSize;
    return pDataPlace;
}
