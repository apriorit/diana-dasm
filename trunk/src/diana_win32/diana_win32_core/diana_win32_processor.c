#include "diana_win32_processor.h"


// {64018619-F561-41b1-90FF-2EA1BB57CF4C}
static const DIANA_UUID g_win32ProcessorId = 
{ 0x64018619, 0xf561, 0x41b1, { 0x90, 0xff, 0x2e, 0xa1, 0xbb, 0x57, 0xcf, 0x4c } };

DianaWin32Processor * DianaWin32Processor_Cast(DianaProcessor* pThis)
{
    if (DianaBase_Match(&pThis->m_base, &g_win32ProcessorId))
        return (DianaWin32Processor * )pThis;
    return 0;
}

static
int DianaWin32Processor_InitImpl(DianaWin32Processor * pThis,
                                 DianaRandomReadWriteStream * pMemoryStream)
{
    memset(pThis, 0, sizeof(DianaWin32Processor));

    DianaWin32Stream_Init(&pThis->m_memoryStream);

    // memory allocator
    Diana_InitMAllocator(&pThis->m_memAllocator);
       

    // create heap
    pThis->m_heap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 
                               0,
                               0);
    if (!pThis->m_heap)
        return DI_WIN32_ERROR;

    // processor
    if (pMemoryStream)
    {
        DI_CHECK(DianaProcessor_Init(&pThis->m_processor, 
                                    pMemoryStream,
                                    &pThis->m_memAllocator.m_parent,
                                    DIANA_MODE32));
    }
    else
    {
        DI_CHECK(DianaProcessor_Init(&pThis->m_processor, 
                                    &pThis->m_memoryStream.m_parent,
                                    &pThis->m_memAllocator.m_parent,
                                    DIANA_MODE32));
    }

    DianaBase_Init(&pThis->m_processor.m_base, &g_win32ProcessorId);
    return DI_SUCCESS;
}

int DianaWin32Processor_InitEx(DianaWin32Processor * pThis,
                               DianaRandomReadWriteStream * pMemoryStream)
{
    int result = DianaWin32Processor_InitImpl(pThis, pMemoryStream);
    if (result != DI_SUCCESS)
    {
        if (pThis->m_heap)
        {
            HeapDestroy(pThis->m_heap);
        }
    }
    return result;
}
int DianaWin32Processor_Init(DianaWin32Processor * pThis)
{
    return DianaWin32Processor_InitEx(pThis, 0);
}

void DianaWin32Processor_Free(DianaWin32Processor * pThis)
{
    HeapDestroy(pThis->m_heap);
    DianaProcessor_Free(&pThis->m_processor);
}