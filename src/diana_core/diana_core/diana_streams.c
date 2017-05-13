#include "diana_streams.h"

static int MemoryStream_Read(void * pThis, void * pBuffer, int iBufferSize, int * readBytes)
{
    DianaMemoryStream * pStream = pThis;
    int sizeToGive = (int)(pStream->bufferSize - pStream->curSize);
    if (sizeToGive > iBufferSize)
        sizeToGive = iBufferSize;

    DIANA_MEMCPY(pBuffer,(char*)pStream->pBuffer+pStream->curSize, sizeToGive);
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

    DIANA_MEMCPY(pBuffer,(char*)pStream->pBuffer+offset, sizeToGive);
    *readBytes = sizeToGive;
    return 0;
}
int DianaMemoryStream_RandomReadMoveTo(void * pThis, 
                                       OPERAND_SIZE offset)
{
    DianaMemoryStream * pStream = (DianaMemoryStream * )pThis;
    pStream->curSize = offset;
    return DI_SUCCESS;
}
int DianaMemoryStream_RandomWrite_V(void * pThis, 
                                   OPERAND_SIZE offset,
                                   void * pBuffer, 
                                   int iBufferSize, 
                                   OPERAND_SIZE * readBytes,
                                   int flags)
{
    DianaMemoryStream * pStream = (DianaMemoryStream * )pThis;
    OPERAND_SIZE sizeToGive = 0;
    
    if (flags & DIANA_ANALYZE_RANDOM_READ_ABSOLUTE)
    {
        return DI_END_OF_STREAM;
    }
    if (offset >= pStream->bufferSize)
    {
        return DI_END_OF_STREAM;
    }

    sizeToGive = (int)(pStream->bufferSize - offset);
    if (sizeToGive > iBufferSize)
        sizeToGive = iBufferSize;

    DIANA_MEMCPY((char*)pStream->pBuffer+offset, pBuffer, sizeToGive);
    *readBytes = sizeToGive;
    return 0;
}
                                  
int DianaMemoryStream_RandomRead_V(void * pThis, 
                                   OPERAND_SIZE offset,
                                   void * pBuffer, 
                                   int iBufferSize, 
                                   OPERAND_SIZE * readBytes,
                                   int flags)
{
    DianaMemoryStream * pStream = (DianaMemoryStream * )pThis;
    OPERAND_SIZE sizeToGive = 0;
    
    if (flags & DIANA_ANALYZE_RANDOM_READ_ABSOLUTE)
    {
        return DI_END_OF_STREAM;
    }
    if (offset >= pStream->bufferSize)
    {
        return DI_END_OF_STREAM;
    }

    sizeToGive = (int)(pStream->bufferSize - offset);
    if (sizeToGive > iBufferSize)
        sizeToGive = iBufferSize;

    DIANA_MEMCPY(pBuffer,(char*)pStream->pBuffer+offset, sizeToGive);
    *readBytes = sizeToGive;
    return 0;
}

void Diana_InitMemoryStreamEx(DianaMemoryStream * pStream,
                              void * pBuffer,
                              DIANA_SIZE_T bufferSize,
                              int bWritable)
{
    DianaAnalyzeRandomWrite_fnc pWriteFunction = 0;
    if (bWritable)
    {
        pWriteFunction = DianaMemoryStream_RandomWrite_V;
    }
    DianaReadWriteRandomStream_Init(&pStream->parent,
                                    MemoryStream_Read,
                                    DianaMemoryStream_RandomReadMoveTo,
                                    DianaMemoryStream_RandomRead_V,
                                    pWriteFunction);
    pStream->pBuffer = pBuffer;
    pStream->bufferSize = bufferSize;
    pStream->curSize = 0;
}
                              
void Diana_InitMemoryStream(DianaMemoryStream * pStream,
                            void * pBuffer,
                            DIANA_SIZE_T bufferSize)
{
    Diana_InitMemoryStreamEx(pStream, pBuffer, bufferSize, 0);
}


int Diana_ParseCmdOnBuffer(int iMode,
                           void * pBuffer,
                           DIANA_SIZE_T size,
                           DianaBaseGenObject_type * pInitialLine,  // IN
                           DianaParserResult * pResult,  //OUT
                           DIANA_SIZE_T * sizeRead)    // OUT
{
    DianaMemoryStream stream;
    DianaContext context;
    int iRes;

    Diana_InitContext(&context, iMode);
    Diana_InitMemoryStream(&stream, pBuffer, size);
    iRes = Diana_ParseCmd(&context, 
                          pInitialLine, 
                          &stream.parent.parent.parent,  
                          pResult);
    *sizeRead = stream.curSize - context.cacheSize;
    return iRes;
}


int Diana_ParseCmdOnBuffer_testmode(int iMode,
                                    void * pBuffer,
                                    DIANA_SIZE_T size,
                                    DianaBaseGenObject_type * pInitialLine,  // IN
                                    DianaParserResult * pResult,  //OUT
                                    DIANA_SIZE_T * sizeRead)    // OUT
{
    DianaMemoryStream stream;
    DianaContext context;
    int iRes;

    Diana_InitContextWithTestMode(&context, iMode);
    Diana_InitMemoryStream(&stream, pBuffer, size);
    iRes = Diana_ParseCmd(&context, 
                          pInitialLine, 
                          &stream.parent.parent.parent,  
                          pResult);
    *sizeRead = stream.curSize - context.cacheSize;
    return iRes;
}