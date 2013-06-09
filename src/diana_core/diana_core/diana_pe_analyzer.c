#include "diana_pe_analyzer.h"
#include "stdlib.h"

int DianaAnalyzeObserverOverMemory_ConvertAddressToRelative(void * pThis, 
                                                            OPERAND_SIZE address,
                                                            OPERAND_SIZE * pRelativeOffset,
                                                            int * pbInvalidPointer);

static
int ScanPage(Diana_PeFile * pPeFile,
                      DianaAnalyzeObserver * pObserver,
                      Diana_InstructionsOwner * pOwner,
                      void * pPage,
                      int pageSize)
{
    char * p = pPage;
    char * pEnd = p + pageSize - sizeof(size_t)+1;
    if (pageSize < sizeof(size_t))
        return DI_SUCCESS;

    for(++p; p < pEnd; ++p)
    {
        size_t * pAddress = (size_t *)p;
        OPERAND_SIZE relativeAddress = 0;
        DianaAnalyzeAddressResult_type result = diaJumpInvalid;
        DI_CHECK(pObserver->m_pAnalyzeAddress(pObserver, 
                                              *pAddress, 
                                              DIANA_ANALYZE_ABSOLUTE_ADDRESS,
                                              &relativeAddress, 
                                              &result));
        if (result == diaJumpNormal)
        {
            DI_CHECK(Diana_AnalyzeCode(pOwner,
                                        pObserver,
                                        pPeFile->pImpl->dianaMode,
                                        relativeAddress,
                                        pPeFile->pImpl->sizeOfFile));
        }
    }
    return DI_SUCCESS;
}
static
int Diana_AnalyzeData(Diana_PeFile * pPeFile,
                      DianaAnalyzeObserver * pObserver,
                      Diana_InstructionsOwner * pOwner,
                      void * pPage,
                      int pageSize)
{       
    DIANA_IMAGE_SECTION_HEADER * pCurrentCapturedSection = pPeFile->pImpl->pCapturedSections;
    DIANA_IMAGE_SECTION_HEADER * pCapturedSectionsEnd = pPeFile->pImpl->pCapturedSections + pPeFile->pImpl->capturedSectionCount;
    int readBytes = 0;
    size_t pageLastPointer = 0;
    for(;
        pCurrentCapturedSection != pCapturedSectionsEnd;
        ++pCurrentCapturedSection)
    {
        int result = 0;
        OPERAND_SIZE allReadBytes = 0;
        DI_CHECK(pObserver->m_pStream->pMoveTo(pObserver->m_pStream, pCurrentCapturedSection->VirtualAddress));
        for(;;)
        {
            readBytes = 0;
            result = pObserver->m_pStream->parent.pReadFnc(pObserver->m_pStream, (char*)pPage+sizeof(size_t), pageSize, &readBytes);
            if (result || !readBytes)
                break;
            allReadBytes += readBytes;
            *(size_t*)pPage = pageLastPointer;
            DI_CHECK(ScanPage(pPeFile,
                              pObserver,
                              pOwner,
                              (char*)pPage,
                              readBytes + sizeof(size_t)));
            if (readBytes >= sizeof(size_t))
            {
                pageLastPointer = *(size_t*)((char*)pPage + readBytes);
            }
            if (allReadBytes >= pCurrentCapturedSection->Misc.VirtualSize)
                break;
        }
    }
    return DI_SUCCESS;
}

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
    {
        int pageSize = 0x1000;
        void * pPage = malloc(pageSize + sizeof(OPERAND_SIZE));
        int status = 0;
        DI_CHECK_ALLOC(pPage);
        status = Diana_AnalyzeData(pPeFile, pObserver, pOwner, pPage, pageSize);
        free(pPage);
    }

    // scan exports
    // scan imports
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

int DianaAnalyzeObserverOverMemory_AnalyzeJumpAddress(void * pThis, 
                                                       OPERAND_SIZE address,
                                                       int flags,
                                                       OPERAND_SIZE * pRelativeOffset,
                                                       DianaAnalyzeAddressResult_type * pResult)
{
    DianaAnalyzeObserverOverMemory * pObserver = pThis;
    DianaMovableReadStreamOverMemory * pStream = &pObserver->stream;
    *pResult = diaJumpNormal;
    *pRelativeOffset = address;

    if (flags&DIANA_ANALYZE_ABSOLUTE_ADDRESS)
    { 
        if (address >= (OPERAND_SIZE)pStream->memoryStream.pBuffer &&
            address <= (OPERAND_SIZE)pStream->memoryStream.pBuffer + pStream->memoryStream.bufferSize)
        {
            *pRelativeOffset = address - (OPERAND_SIZE)pStream->memoryStream.pBuffer;
            *pResult = diaJumpNormal;
            return DI_SUCCESS;
        }
        *pResult = diaJumpExternal;
        return DI_SUCCESS;
    }
        
    if (address >= pStream->memoryStream.bufferSize)
    {
        *pResult = diaJumpExternal;
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