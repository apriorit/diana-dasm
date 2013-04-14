#include "diana_pe.h"
#include "string.h"
#include "stdlib.h"

static
void Diana_PeFile_impl_CommonInit(Diana_PeFile_impl  * pImpl,
                                  int mode,
                                  OPERAND_SIZE sizeOfFile,
                                  OPERAND_SIZE sizeOfImpl,
                                  int optionalHeaderSize,
                                  OPERAND_SIZE loadedBase)
{
    pImpl->dianaMode = mode;
    pImpl->sizeOfFile = sizeOfFile;
    pImpl->sizeOfImpl = sizeOfImpl;
    pImpl->optionalHeaderSize = optionalHeaderSize;
    pImpl->loadedBase = loadedBase;
}
static
void Diana_InitCustomFields(Diana_PeFile_impl * pImpl,
                            DI_UINT32  addressOfEntryPoint)
{
    pImpl->addressOfEntryPoint = addressOfEntryPoint;
}

static 
int DI_Init386(Diana_PeFile_impl * pImpl_in,
               DianaReadStream * pStream)
{
    Diana_PeFile32_impl * pImpl = (Diana_PeFile32_impl * )pImpl_in;
    DIANA_IMAGE_OPTIONAL_HEADER32 * pOpt =  (DIANA_IMAGE_OPTIONAL_HEADER32 * )(pImpl + 1);

    DI_CHECK(DianaExactRead(pStream, pOpt, sizeof(DIANA_IMAGE_OPTIONAL_HEADER32)));
    pImpl->pOptionalHeader = pOpt;
    Diana_InitCustomFields(pImpl_in, pOpt->AddressOfEntryPoint);
    return DI_SUCCESS;
}
static 
int DI_InitAmd64(Diana_PeFile_impl * pImpl_in,
                 DianaReadStream * pStream)
{
    Diana_PeFile64_impl * pImpl = (Diana_PeFile64_impl * )pImpl_in;
    DIANA_IMAGE_OPTIONAL_HEADER64 * pOpt =  (DIANA_IMAGE_OPTIONAL_HEADER64 * )(pImpl + 1);

    DI_CHECK(DianaExactRead(pStream, pOpt, sizeof(DIANA_IMAGE_OPTIONAL_HEADER64)));
    pImpl->pOptionalHeader = pOpt;
    Diana_InitCustomFields(pImpl_in, pOpt->AddressOfEntryPoint);
    return DI_SUCCESS;
}

static
int Diana_VerifyDosHeader(DIANA_IMAGE_DOS_HEADER * pDosHeader,
                         OPERAND_SIZE sizeOfFile)
{
    if (sizeOfFile && sizeOfFile <= sizeof(DIANA_IMAGE_DOS_HEADER))
        return DI_INVALID_INPUT;

    if (pDosHeader->e_magic[0] != 'M' ||
        pDosHeader->e_magic[1] != 'Z')
    {
        return DI_INVALID_INPUT;
    }

    if (sizeOfFile)
    {
        if (pDosHeader->e_lfanew >= sizeOfFile)
        {
            return DI_INVALID_INPUT;
        }

        if (sizeof(DIANA_IMAGE_NT_HEADERS) >= sizeOfFile - pDosHeader->e_lfanew)
        {
            return DI_INVALID_INPUT;
        }   
    }
    return DI_SUCCESS;
}

static
int ReadOptionalHeader(Diana_PeFile_impl * pImpl,
                       DianaReadStream * pStream,
                       int * pDisasmMode,
                       int * pOptionalHeaderSize)
{
    if (strncmp(pImpl->ntHeaders.Signature, "PE", 3))
    {
        return DI_INVALID_INPUT;
    }
    switch( pImpl->ntHeaders.FileHeader.Machine )
    {
    case DIANA_IMAGE_FILE_MACHINE_I386:
        *pDisasmMode = DIANA_MODE32;
        *pOptionalHeaderSize = sizeof(DIANA_IMAGE_OPTIONAL_HEADER32);
        return DI_Init386(pImpl, pStream);

    case DIANA_IMAGE_FILE_MACHINE_AMD64:
        *pDisasmMode = DIANA_MODE64;
        *pOptionalHeaderSize = sizeof(DIANA_IMAGE_OPTIONAL_HEADER64);
        return DI_InitAmd64(pImpl, pStream);
    }
    return DI_UNSUPPORTED_COMMAND;
}

static
size_t Diana_GetMaxSize(size_t size1, size_t size2)
{
    if (size1 > size2)
        return size1;
    return size2;
}

static
int Diana_InitPeFileImpl(Diana_PeFile * pPeFile,
                         DianaMovableReadStream * pStream,
                         OPERAND_SIZE sizeOfFile,
                         OPERAND_SIZE loadedBase)
{
    int mode = 0;
    int sizeOfImpl = 0;
    int optionalHeaderSize = 0;
    Diana_PeFile_impl * pImpl= 0;
    DIANA_IMAGE_SECTION_HEADER * pSectionHeader = 0;
    
    memset(pPeFile, 0, sizeof(*pPeFile));
    // allocate impl header
    sizeOfImpl = (int)Diana_GetMaxSize(sizeof(Diana_PeFile64_impl), sizeof(Diana_PeFile32_impl)) + 
                 (int)Diana_GetMaxSize(sizeof(DIANA_IMAGE_OPTIONAL_HEADER64), sizeof(DIANA_IMAGE_OPTIONAL_HEADER32));
    pImpl = malloc(sizeOfImpl);
    DI_CHECK_ALLOC(pImpl);
    pPeFile->pImpl = pImpl;
    memset(pImpl, 0, sizeof(*pImpl));

    // read dos header
    DI_CHECK(pStream->pMoveTo(pStream, 0));
    DI_CHECK(DianaExactRead(&pStream->parent, &pImpl->dosHeader, sizeof(DIANA_IMAGE_DOS_HEADER)));
    DI_CHECK(Diana_VerifyDosHeader(&pImpl->dosHeader, sizeOfFile));

    // read nt header
    DI_CHECK(pStream->pMoveTo(pStream, pImpl->dosHeader.e_lfanew));
    DI_CHECK(DianaExactRead(&pStream->parent, &pImpl->ntHeaders, sizeof(DIANA_IMAGE_NT_HEADERS)));

    // read optional header
    DI_CHECK(ReadOptionalHeader(pImpl, &pStream->parent, &mode, &optionalHeaderSize));
    Diana_PeFile_impl_CommonInit(pImpl, mode, sizeOfFile, sizeOfImpl, optionalHeaderSize, loadedBase);

    // init sections
    {
        int sectionOffset = pImpl->dosHeader.e_lfanew + sizeof(DIANA_IMAGE_NT_HEADERS) + optionalHeaderSize;
        DI_CHECK(pStream->pMoveTo(pStream, sectionOffset));
    
        pSectionHeader = malloc(pImpl->ntHeaders.FileHeader.NumberOfSections * sizeof(DIANA_IMAGE_SECTION_HEADER));
        DI_CHECK_ALLOC(pSectionHeader);
        pImpl->pCapturedSections = pSectionHeader;

        DI_CHECK(DianaExactRead(&pStream->parent, pSectionHeader, pImpl->ntHeaders.FileHeader.NumberOfSections * sizeof(DIANA_IMAGE_SECTION_HEADER)));
        pImpl->capturedSectionCount = pImpl->ntHeaders.FileHeader.NumberOfSections;
    }
    // init file size
    if  (!sizeOfFile)
    {
        // calculate it
        OPERAND_SIZE size = 0;
        int i;
        for(i = 0; i < pImpl->capturedSectionCount; ++i)
        {
            OPERAND_SIZE currentSectionMax = 0;
            DIANA_IMAGE_SECTION_HEADER * pHeader = pImpl->pCapturedSections + i;
            currentSectionMax = pHeader->VirtualAddress + pHeader->Misc.VirtualSize;
            if (currentSectionMax > size)
            {
                size = currentSectionMax;
            }
        }
        pImpl->sizeOfFile = size;
    }
    return DI_SUCCESS;
}

int DianaPeFile_Init(Diana_PeFile * pPeFile,
                     DianaMovableReadStream * pStream,
                     OPERAND_SIZE sizeOfFile,
                     OPERAND_SIZE loadedBase)
{
    int status = Diana_InitPeFileImpl(pPeFile, pStream, sizeOfFile, loadedBase);
    if (status)
    {
        DianaPeFile_Free(pPeFile);
    }
    return status;
}

void DianaPeFile_Free(Diana_PeFile * pPeFile)
{
    if (pPeFile->pImpl)
    {
        if (pPeFile->pImpl->pCapturedSections)
        {
            free(pPeFile->pImpl->pCapturedSections);
            pPeFile->pImpl->pCapturedSections = 0;
        }
        free(pPeFile->pImpl);
        pPeFile->pImpl = 0;
    }
}

