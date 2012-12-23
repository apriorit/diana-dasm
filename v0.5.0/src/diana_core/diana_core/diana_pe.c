#include "diana_pe.h"
#include "string.h"
#include "stdlib.h"

static
void Diana_PeFile_impl_Init(Diana_PeFile_impl  * pImpl,
                            int mode)
{
    pImpl->dianaMode = mode;
}

static 
int DI_Init386(Diana_PeFile * pPeFile,
               Diana_PeFile_impl * pPeImpl)
{
    Diana_PeFile32_impl * pImpl = 0;
    DIANA_IMAGE_OPTIONAL_HEADER32 * pOpt =  (DIANA_IMAGE_OPTIONAL_HEADER32 * )(pPeImpl->pNtHeaders+1);

    if (( unsigned int )((char *)pOpt + sizeof(DIANA_IMAGE_OPTIONAL_HEADER32) - (char*)pPeImpl->pFileStart) > pPeImpl->sizeOfFile)
        return DI_INVALID_INPUT; 

    pImpl = malloc(sizeof(Diana_PeFile32_impl));
    DI_CHECK_ALLOC(pImpl);
    pPeFile->pImpl = &pImpl->parent;
    Diana_PeFile_impl_Init(pPeFile->pImpl, DIANA_MODE32);

    pImpl->pOptinalHeader = pOpt;

    pImpl->parent = *pPeImpl;
    return DI_SUCCESS;
}
static 
int DI_InitAmd64(Diana_PeFile * pPeFile,
                 Diana_PeFile_impl * pPeImpl)
{
    Diana_PeFile64_impl * pImpl = 0;
    DIANA_IMAGE_OPTIONAL_HEADER64 * pOpt =  (DIANA_IMAGE_OPTIONAL_HEADER64 * )(pPeImpl->pNtHeaders+1);

    if (( unsigned int )((char *)pOpt + sizeof(DIANA_IMAGE_OPTIONAL_HEADER64) - (char*)pPeImpl->pFileStart) > pPeImpl->sizeOfFile)
        return DI_INVALID_INPUT; 

    pImpl = malloc(sizeof(Diana_PeFile64_impl));
    DI_CHECK_ALLOC(pImpl);
    pPeFile->pImpl = &pImpl->parent;
    Diana_PeFile_impl_Init(pPeFile->pImpl, DIANA_MODE64);

    pImpl->pOptinalHeader = pOpt;

    pImpl->parent = *pPeImpl;
    return DI_SUCCESS;
}


int Diana_InitPeFile(Diana_PeFile * pPeFile,
                     void * pFileBegin,
                     OPERAND_SIZE sizeOfFile)
{
    Diana_PeFile_impl peImpl;

    memset(&peImpl, 0, sizeof(peImpl));
    peImpl.pDOS = pFileBegin;
    peImpl.pFileStart = pFileBegin;
    peImpl.sizeOfFile = sizeOfFile;

    if (sizeOfFile <= sizeof(DIANA_IMAGE_DOS_HEADER))
        return DI_INVALID_INPUT;

    if (peImpl.pDOS->e_magic[0] != 'M' ||
        peImpl.pDOS->e_magic[1] != 'Z')
    {
        return DI_INVALID_INPUT;
    }

    if (peImpl.pDOS->e_lfanew >= sizeOfFile)
    {
        return DI_INVALID_INPUT;
    }

    if (sizeof(DIANA_IMAGE_NT_HEADERS) >= sizeOfFile - peImpl.pDOS->e_lfanew)
    {
        return DI_INVALID_INPUT;
    }
        
    peImpl.pNtHeaders = (DIANA_IMAGE_NT_HEADERS *)((char*)pFileBegin + peImpl.pDOS->e_lfanew);
    if (strncmp(peImpl.pNtHeaders->Signature, "PE", 3))
    {
        return DI_INVALID_INPUT;
    }

    memset(pPeFile, 0, sizeof(*pPeFile));

    peImpl.pFirstSection = (DIANA_IMAGE_SECTION_HEADER *)
                ((char *)peImpl.pNtHeaders + 
                         peImpl.pNtHeaders->FileHeader.SizeOfOptionalHeader);

    switch( peImpl.pNtHeaders->FileHeader.Machine )
    {
    case DIANA_IMAGE_FILE_MACHINE_I386:
        return DI_Init386(pPeFile,
                          &peImpl);

    case DIANA_IMAGE_FILE_MACHINE_AMD64:
        return DI_InitAmd64(pPeFile,
                            &peImpl);
    }
    return DI_UNSUPPORTED_COMMAND;
}

void Diana_FreePeFile(Diana_PeFile * pPeFile)
{
    if (pPeFile->pImpl)
        free(pPeFile->pImpl);
}
