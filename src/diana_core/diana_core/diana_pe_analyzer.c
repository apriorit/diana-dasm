#include "diana_pe_analyzer.h"

static 
int Diana_PE_AnalyzePEImpl(Diana_PeFile * pPeFile,
                           DianaAnalyzeObserver * pObserver,
                           Diana_InstructionsOwner * pOwner)
{
    // scan from entry point
    if (pPeFile->pImpl->addressOfEntryPoint)
    {
        DI_CHECK(Diana_AnalyzeCode(pOwner,
                                   pObserver, 
                                   pPeFile->pImpl->dianaMode, 
                                   pPeFile->pImpl->addressOfEntryPoint, 
                                   pPeFile->pImpl->sizeOfFile));
    }
    // scan exports
    // scan imports
    // scan section pointers
    return DI_SUCCESS;
}
int Diana_PE_AnalyzePE(Diana_PeFile * pPeFile,
                       DianaAnalyzeObserver * pObserver,
                       Diana_InstructionsOwner * pOwner)
{
    int status = 0;
    DI_CHECK(Diana_InstructionsOwner_Init(pOwner, pPeFile->pImpl->sizeOfFile));
    status = Diana_PE_AnalyzePEImpl(pPeFile,
                                        pObserver,
                                        pOwner);
    if (status)
    {
        Diana_InstructionsOwner_Free(pOwner);
    }
    return status;
}


typedef struct _DianaMovableReadStreamOverMemory
{
    DianaMovableReadStream stream;
    DianaMemoryStream memoryStream;
}DianaMovableReadStreamOverMemory;

typedef struct _DianaAnalyzeObserverOverMemory
{
    DianaAnalyzeObserver parent;
    DianaMovableReadStreamOverMemory stream;
}DianaAnalyzeObserverOverMemory;


int DianaMovableReadStreamOverMemory_MoveTo(void * pThis, OPERAND_SIZE offset)
{
    DianaMovableReadStreamOverMemory * pStream = pThis;
    if (offset >= pStream->memoryStream.bufferSize)
        return DI_END_OF_STREAM;
    pStream->memoryStream.curSize = offset;
    return DI_SUCCESS;
}
int DianaMovableReadStreamOverMemory_Read(void * pThis,
                                            void * pBuffer,
                                            int iBufferSize,
                                            int * readBytes)
{
    DianaMovableReadStreamOverMemory * pStream = pThis;
    return pStream->memoryStream.parent.pReadFnc(&pStream->memoryStream.parent, pBuffer, iBufferSize, readBytes);
}

void DianaMovableReadStreamOverMemory_Init(DianaMovableReadStreamOverMemory * pThis,
                                            void * pBuffer,
                                            OPERAND_SIZE bufferSize)
{
    DianaMovableReadStream_Init(&pThis->stream, DianaMovableReadStreamOverMemory_Read, DianaMovableReadStreamOverMemory_MoveTo);
    Diana_InitMemoryStream(&pThis->memoryStream, pBuffer, bufferSize);
}
int DianaAnalyzeObserverOverMemory_ConvertAddressToRelative(void * pThis, 
                                                            OPERAND_SIZE address,
                                                            OPERAND_SIZE * pRelativeOffset,
                                                            int * pbInvalidPointer)
{
    DianaAnalyzeObserverOverMemory * pObserver = pThis;
    DianaMovableReadStreamOverMemory * pStream = &pObserver->stream;

    *pbInvalidPointer = 1;
    if (address >= (OPERAND_SIZE)pStream->memoryStream.pBuffer &&
        address <= (OPERAND_SIZE)pStream->memoryStream.pBuffer + pStream->memoryStream.bufferSize)
    {
        *pbInvalidPointer = 0;
        *pRelativeOffset = address - (OPERAND_SIZE)pStream->memoryStream.pBuffer;
    }
    return DI_SUCCESS;
}
int DianaAnalyzeObserverOverMemory_AddSuspectedDataAddress(void * pThis, 
                                                          OPERAND_SIZE address)
{
    &pThis;
    &address;
    return DI_SUCCESS;
}

int DianaAnalyzeObserverOverMemory_AnalyzeJumpAddress(void * pThis, 
                                                      OPERAND_SIZE address,
                                                      DianaAnalyzeJumpFlags_type * pFlags)
{
    DianaAnalyzeObserverOverMemory * pObserver = pThis;
    DianaMovableReadStreamOverMemory * pStream = &pObserver->stream;
    *pFlags = diaJumpExternal;

    if (address >= (OPERAND_SIZE)pStream->memoryStream.pBuffer &&
        address <= (OPERAND_SIZE)pStream->memoryStream.pBuffer + pStream->memoryStream.bufferSize)
    {
        *pFlags = diaJumpNormal;
    }
    return DI_SUCCESS;
}

void DianaAnalyzeObserverOverMemory_Init(DianaAnalyzeObserverOverMemory * pThis,
                                         void * pPeFile,
                                         OPERAND_SIZE fileSize)
{
    DianaMovableReadStreamOverMemory_Init(&pThis->stream,
                                          pPeFile,
                                          fileSize);
    DianaAnalyzeObserver_Init(&pThis->parent,
                              &pThis->stream.stream,
                              DianaAnalyzeObserverOverMemory_ConvertAddressToRelative,
                              DianaAnalyzeObserverOverMemory_AddSuspectedDataAddress,
                              DianaAnalyzeObserverOverMemory_AnalyzeJumpAddress
                              );
}

int Diana_PE_AnalyzePEInMemory(void * pPeFile,
                                OPERAND_SIZE fileSize,
                                Diana_InstructionsOwner * pOwner)
{
    OPERAND_SIZE fileSizeToPass = fileSize;
    int actualizeFileSize = 0;
    int result = 0;
    Diana_PeFile peFile;
    DianaAnalyzeObserverOverMemory analyzeObserver;

    if (!fileSize)
    {
        fileSize = 0x10000;
        actualizeFileSize = 1;
        fileSizeToPass = 0;
    }
    DianaAnalyzeObserverOverMemory_Init(&analyzeObserver,
                                        pPeFile,
                                        fileSize);

    DI_CHECK(DianaPeFile_Init(&peFile, &analyzeObserver.stream.stream, fileSizeToPass, (OPERAND_SIZE)pPeFile));
    if (actualizeFileSize)
    {
        analyzeObserver.stream.memoryStream.bufferSize = peFile.pImpl->sizeOfFile;
    }
    result = Diana_PE_AnalyzePE(&peFile, &analyzeObserver.parent, pOwner);
    DianaPeFile_Free(&peFile);
    return result;
}