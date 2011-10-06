#ifndef DIANA_LIST_H
#define DIANA_LIST_H

#include "diana_core.h"

typedef struct _Diana_ListNode
{
    struct _Diana_ListNode * m_pNext;
    struct _Diana_ListNode * m_pPrev;
}Diana_ListNode;

typedef struct _Diana_List
{
    Diana_ListNode * m_pFirst;
    Diana_ListNode * m_pLast;
    OPERAND_SIZE m_size;
}Diana_List;


typedef int (*DianaListObserver_type)(Diana_ListNode * pNode,
                                      void * pContext,
                                      int * pbDone);
int Diana_ListForEach(Diana_List * pList,
                   DianaListObserver_type pObserver,
                   void * pContext);


void Diana_InitList(Diana_List * pList);

void Diana_PushBack(Diana_List * pList, 
                    Diana_ListNode * pNode);
void Diana_PushFront(Diana_List * pList,
                     Diana_ListNode * pNode);

void Diana_InsertAfter(Diana_List * pList,
                       Diana_ListNode * pNode,
                       Diana_ListNode * pNodeToInsert);

void Diana_InsertBefore(Diana_List * pList,
                       Diana_ListNode * pNode,
                       Diana_ListNode * pNodeToInsert);

void Diana_EraseNode(Diana_List * pList,
                     Diana_ListNode * pNode);

#endif