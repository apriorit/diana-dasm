#ifndef DIANA_STREAMS
#define DIANA_STREAMS

#include "diana_core.h"

typedef struct _dianaMemoryStream
{
    DianaReadStream parent;
    void * pBuffer;
    size_t bufferSize;
    size_t curSize;
}DianaMemoryStream;

void Diana_InitMemoryStream(DianaMemoryStream * pStream,
                            void * pBuffer,
                            size_t bufferSize
                            );  // this

int Diana_ParseCmdOnBuffer(int iMode,
                           void * pBuffer,
                           size_t size,
                           DianaBaseGenObject_type * pInitialLine,  // IN
                           DianaParserResult * pResult,  //OUT
                           size_t * sizeRead);    // OUT

int Diana_ParseCmdOnBuffer_testmode(int iMode,
                                    void * pBuffer,
                                    size_t size,
                                    DianaBaseGenObject_type * pInitialLine,  // IN
                                    DianaParserResult * pResult,  //OUT
                                    size_t * sizeRead);    // OUT

int DianaMemoryStream_RandomRead(void * pThis, 
                                   OPERAND_SIZE offset,
                                   void * pBuffer, 
                                   int iBufferSize, 
                                   OPERAND_SIZE * readBytes);


#endif