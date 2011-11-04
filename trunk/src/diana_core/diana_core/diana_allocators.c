#include "diana_allocators.h"
#include "stdlib.h"

static void * MAllocator_Alloc(void * pThis, size_t size)
{
	return malloc(size);
}
static void MAllocator_Dealloc(void * pThis, void * pBuffer)
{
	free(pBuffer);
}
static int MAllocator_Patch(void * pThis, void * pDest, const void * pSource, size_t size)
{
    memcpy(pDest, pSource, size);
    return DI_SUCCESS;
}
void Diana_InitMAllocator(DianaMAllocator * pThis)
{
    Diana_Allocator  * pAll = &pThis->m_parent;
    Diana_AllocatorInit(pAll, 
                        MAllocator_Alloc, 
                        MAllocator_Dealloc,
                        MAllocator_Patch);
}