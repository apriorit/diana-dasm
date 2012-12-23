#ifndef DIANA_PROCESSOR_CORE_H
#define DIANA_PROCESSOR_CORE_H


#include "diana_processor_core_impl.h"
#include "diana_core.h"


// call it after Diana_Init
void DianaProcessor_GlobalInit();

int DianaProcessor_Init(DianaProcessor * pThis, 
                        DianaRandomReadWriteStream * pMemoryStream,
                        Diana_Allocator * pAllocator,
                        int mode);

void DianaProcessor_Free(DianaProcessor * pThis);

int DianaProcessor_ExecOnce(DianaProcessor * pThis);

void DianaProcessor_ClearCache(DianaProcessor * pThis);

#endif