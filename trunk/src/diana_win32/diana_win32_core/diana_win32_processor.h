#ifndef DIANA_WIN32_PROCESSOR_H
#define DIANA_WIN32_PROCESSOR_H

#include "diana_win32_core.h"
#include "diana_allocators.h"
#include "diana_win32_streams.h"
#include "diana_uids.h"
#include "windows.h"

typedef struct _dianaWin32Processor
{
    DianaProcessor m_processor; // MUST BE FIRST!
    DianaWin32Stream m_memoryStream;
    DianaMAllocator m_memAllocator;
    HANDLE m_heap;
}
DianaWin32Processor;

int DianaWin32Processor_InitEx(DianaWin32Processor * pThis, DianaRandomReadWriteStream * pMemoryStream);
int DianaWin32Processor_Init(DianaWin32Processor * pThis);
void DianaWin32Processor_Free(DianaWin32Processor * pThis);

DianaWin32Processor * DianaWin32Processor_Cast(DianaProcessor* pThis);

#endif