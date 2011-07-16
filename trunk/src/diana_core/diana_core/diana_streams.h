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
                           DianaCmdKeyLine * pInitialLine,  // IN
                           DianaParserResult * pResult,  //OUT
                           size_t * sizeRead);    // OUT


// random stream
#define DI_STREAM_FLAG_LOCK       1

typedef int (* DianaRandomRead_fnc)(void * pThis, 
                                    OPERAND_SIZE selector,
                                    OPERAND_SIZE offset,
                                    void * pBuffer, 
                                    OPERAND_SIZE iBufferSize, 
                                    OPERAND_SIZE * readed,
                                    int flags,
                                    DianaUnifiedRegister segReg);
typedef int (* DianaRandomWrite_fnc)(void * pThis, 
                                     OPERAND_SIZE selector,
                                     OPERAND_SIZE offset,
                                     void * pBuffer, 
                                     OPERAND_SIZE iBufferSize, 
                                     OPERAND_SIZE * wrote,
                                     int flags,
                                     DianaUnifiedRegister segReg);


typedef struct _dianaRandomReadWriteStream
{
    DianaRandomRead_fnc pReadFnc;
    DianaRandomWrite_fnc pWriteFnc;
}DianaRandomReadWriteStream;

typedef struct _dianaRandomMemoryStream
{
    DianaRandomReadWriteStream parent;
    void * pBuffer;
    OPERAND_SIZE bufferSize;
    OPERAND_SIZE segmentSize;
}DianaRandomMemoryStream;

void Diana_InitRandomMemoryStream(DianaRandomMemoryStream * pStream,
                                  void * pBuffer,
                                  OPERAND_SIZE bufferSize,
                                  OPERAND_SIZE segmentSize
                                  );  
#endif