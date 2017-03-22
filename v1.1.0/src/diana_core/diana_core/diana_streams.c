#include "diana_streams.h"

static int MemoryStream_Read(void * pThis, void * pBuffer, int iBufferSize, int * readBytes)
{
    DianaMemoryStream * pStream = pThis;
    int sizeToGive = (int)(pStream->bufferSize - pStream->curSize);
    if (sizeToGive > iBufferSize)
        sizeToGive = iBufferSize;

    memcpy(pBuffer,(char*)pStream->pBuffer+pStream->curSize, sizeToGive);
    pStream->curSize += sizeToGive;
    *readBytes = sizeToGive;
    return 0;
}

int DianaMemoryStream_RandomRead(void * pThis, 
                                   OPERAND_SIZE offset,
                                   void * pBuffer, 
                                   int iBufferSize, 
                                   OPERAND_SIZE * readBytes)
{
    DianaMemoryStream * pStream = pThis;
    int sizeToGive = 0;
    
    if (offset >= pStream->bufferSize)
    {
        return DI_END_OF_STREAM;
    }

    sizeToGive = (int)(pStream->bufferSize - offset);
    if (sizeToGive > iBufferSize)
        sizeToGive = iBufferSize;

    memcpy(pBuffer,(char*)pStream->pBuffer+offset, sizeToGive);
    *readBytes = sizeToGive;
    return 0;
}

void Diana_InitMemoryStream(DianaMemoryStream * pStream,
                            void * pBuffer,
                            size_t bufferSize)
{
    pStream->pBuffer = pBuffer;
    pStream->bufferSize = bufferSize;
    pStream->curSize = 0;
    pStream->parent.pReadFnc = MemoryStream_Read;
}


int Diana_ParseCmdOnBuffer(int iMode,
                           void * pBuffer,
                           size_t size,
                           DianaBaseGenObject_type * pInitialLine,  // IN
                           DianaParserResult * pResult,  //OUT
                           size_t * sizeRead)    // OUT
{
    DianaMemoryStream stream;
    DianaContext context;
    int iRes;

    Diana_InitContext(&context, iMode);
    Diana_InitMemoryStream(&stream, pBuffer, size);
    iRes = Diana_ParseCmd(&context, pInitialLine, &stream.parent,  pResult);
    *sizeRead = stream.curSize - context.cacheSize;
    return iRes;
}


int Diana_ParseCmdOnBuffer_testmode(int iMode,
                                    void * pBuffer,
                                    size_t size,
                                    DianaBaseGenObject_type * pInitialLine,  // IN
                                    DianaParserResult * pResult,  //OUT
                                    size_t * sizeRead)    // OUT
{
    DianaMemoryStream stream;
    DianaContext context;
    int iRes;

    Diana_InitContextWithTestMode(&context, iMode);
    Diana_InitMemoryStream(&stream, pBuffer, size);
    iRes = Diana_ParseCmd(&context, pInitialLine, &stream.parent,  pResult);
    *sizeRead = stream.curSize - context.cacheSize;
    return iRes;
}