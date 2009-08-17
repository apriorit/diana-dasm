#include "diana_streams.h"

static int MemoryStream_Read(void * pThis, void * pBuffer, int iBufferSize, int * readed)
{
    DianaMemoryStream * pStream = pThis;
    size_t sizeToGive = pStream->bufferSize - pStream->curSize;
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
    *sizeRead = stream.curSize - context.iReadedSize;
    return iRes;
}
