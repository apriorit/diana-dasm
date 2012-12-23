#include "diana_stream_proxy.h"

static int StreamProxy_Read(void * pThis, void * pBuffer, int iBufferSize, int * readed)
{
    DianaStreamProxy * pStream = (DianaStreamProxy * )pThis;
    if (pStream->tail_size)
    {
        if (pStream->tail_size >= iBufferSize)
        {
            memcpy(pBuffer, pStream->pBuffer + pStream->begin, iBufferSize);
            pStream->tail_size -= iBufferSize;
            pStream->begin+=iBufferSize;
            *readed = iBufferSize;
            return 0;
        }
        else
        {
            int iRes = 0;
            memcpy(pBuffer, pStream->pBuffer + pStream->begin, pStream->tail_size);

            // read from stream
            iRes = 
            pStream->pUsedStream->pReadFnc(pStream->pUsedStream, 
                                           (char*)pBuffer + pStream->tail_size,
                                           iBufferSize - pStream->tail_size,
                                           readed);
            *readed += pStream->tail_size;
            pStream->tail_size = 0;
            return iRes;
        }
    }
    // direct mode
    return pStream->pUsedStream->pReadFnc(pStream->pUsedStream, 
                                          pBuffer,
                                          iBufferSize,
                                          readed);

}

void Diana_InitStreamProxy(DianaStreamProxy * pStream, 
                           DianaReadStream * pUsedStream,
                           unsigned char * pBuffer, 
                           int readed)
{
    pStream->pUsedStream = pUsedStream;
    pStream->tail_size = readed; 
    pStream->begin = 0;
    pStream->parent.pReadFnc = StreamProxy_Read;
    pStream->pBuffer = pBuffer;
}
