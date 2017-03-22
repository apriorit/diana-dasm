#ifndef DIANA_STREAMS
#define DIANA_STREAMS

#include "diana_core.h"

typedef struct _dianaMemoryStream
{
    DianaReadStream parent;
    void * pBuffer;
    DIANA_SIZE_T bufferSize;
    DIANA_SIZE_T curSize;
}DianaMemoryStream;

void Diana_InitMemoryStream(DianaMemoryStream * pStream,
                            void * pBuffer,
                            DIANA_SIZE_T bufferSize
                            );  // this

int Diana_ParseCmdOnBuffer(int iMode,
                           void * pBuffer,
                           DIANA_SIZE_T size,
                           DianaBaseGenObject_type * pInitialLine,  // IN
                           DianaParserResult * pResult,  //OUT
                           DIANA_SIZE_T * sizeRead);    // OUT

int Diana_ParseCmdOnBuffer_testmode(int iMode,
                                    void * pBuffer,
                                    DIANA_SIZE_T size,
                                    DianaBaseGenObject_type * pInitialLine,  // IN
                                    DianaParserResult * pResult,  //OUT
                                    DIANA_SIZE_T * sizeRead);    // OUT

int DianaMemoryStream_RandomRead(void * pThis, 
                                   OPERAND_SIZE offset,
                                   void * pBuffer, 
                                   int iBufferSize, 
                                   OPERAND_SIZE * readBytes);


#endif