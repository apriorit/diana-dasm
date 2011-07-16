#include "diana_allocators.h"
#include "stdlib.h"

static void * MAllocator_Alloc(void * pThis, int iBufferSize)
{
    return malloc(iBufferSize);
}
static void MAllocator_Dealloc(void * pThis, void * pBuffer)
{
   free(pBuffer);
}

void Diana_InitMAllocator(DianaMAllocator * pThis)
{
    pThis->m_parent.m_alloc = MAllocator_Alloc;
    pThis->m_parent.m_dealloc = MAllocator_Dealloc;
}