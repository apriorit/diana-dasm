#include "diana_streams.h"

static int MemoryStream_Read(void * pThis, void * pBuffer, int iBufferSize, int * readed)
{
    DianaMemoryStream * pStream = pThis;
    int sizeToGive = (int)(pStream->bufferSize - pStream->curSize);
    if (sizeToGive > iBufferSize)
        sizeToGive = iBufferSize;

    memcpy(pBuffer,(char*)pStream->pBuffer+pStream->curSize, sizeToGive);
    pStream->curSize += sizeToGive;
    *readed = sizeToGive;
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
                           DianaCmdKeyLine * pInitialLine,  // IN
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

static int MemoryRandomStream_ReadWrite(void * pThis, 
                                        OPERAND_SIZE selector,
                                        OPERAND_SIZE offset_in,
                                        void * pBuffer, 
                                        OPERAND_SIZE iBufferSize, 
                                        OPERAND_SIZE * readed,
                                        int bWrite,
                                        int flags)
{
    DianaRandomMemoryStream * pStream = pThis;
    OPERAND_SIZE sizeToGive = 0;
    OPERAND_SIZE segmentOffset = 0;
    OPERAND_SIZE offset = 0;

    segmentOffset = selector*pStream->segmentSize;
    offset = segmentOffset + offset_in;

    *readed = 0;
    if (offset > pStream->bufferSize)
        return DI_ERROR;

    sizeToGive = pStream->bufferSize - offset;
    if (sizeToGive > iBufferSize)
        sizeToGive = iBufferSize;

    if (bWrite)
    {
        memcpy((char*)pStream->pBuffer+offset,
               pBuffer,
               (size_t)sizeToGive);
    }
    else
    {
        memcpy(pBuffer,
            (char*)pStream->pBuffer+offset, 
            (size_t)sizeToGive);
    }

    *readed = sizeToGive;
    return DI_SUCCESS;
}

static int MemoryRandomStream_Read(void * pThis, 
                                   OPERAND_SIZE selector,
                                   OPERAND_SIZE offset,
                                   void * pBuffer, 
                                   OPERAND_SIZE iBufferSize, 
                                   OPERAND_SIZE * readed,
                                   int flags,
                                   DianaUnifiedRegister segReg)
{
    &segReg;
    return MemoryRandomStream_ReadWrite(pThis, 
                                        selector,
                                        offset,
                                        pBuffer, 
                                        iBufferSize, 
                                        readed,
                                        0,
                                        flags);
}

static int MemoryRandomStream_Write(void * pThis, 
                                    OPERAND_SIZE selector,
                                    OPERAND_SIZE offset,
                                    void * pBuffer, 
                                    OPERAND_SIZE iBufferSize, 
                                    OPERAND_SIZE * readed,
                                    int flags,
                                    DianaUnifiedRegister segReg)
 {
    &segReg;
    return MemoryRandomStream_ReadWrite(pThis, 
                                        selector,
                                        offset,
                                        pBuffer, 
                                        iBufferSize, 
                                        readed,
                                        1,
                                        flags);
}

void Diana_InitRandomMemoryStream(DianaRandomMemoryStream * pStream,
                                  void * pBuffer,
                                  OPERAND_SIZE bufferSize,
                                  OPERAND_SIZE segmentSize
                                  )
{
    pStream->pBuffer = pBuffer;
    pStream->bufferSize = bufferSize;
    pStream->segmentSize = segmentSize;
    pStream->parent.pReadFnc = MemoryRandomStream_Read;
    pStream->parent.pWriteFnc = MemoryRandomStream_Write;
}
