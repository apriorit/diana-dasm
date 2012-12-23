#ifndef DIANA_PE_H
#define DIANA_PE_H

#include "diana_pe_defs.h"

typedef struct _diana_PeFile_impl
{
    // basic
    void * pFileStart;
    OPERAND_SIZE sizeOfFile;
    DIANA_IMAGE_NT_HEADERS * pNtHeaders;
    DIANA_IMAGE_SECTION_HEADER * pFirstSection;
    DIANA_IMAGE_DOS_HEADER * pDOS;

    // advanced
    int dianaMode;
}Diana_PeFile_impl;

typedef struct _diana_PeFile32_impl
{
    Diana_PeFile_impl parent;
    DIANA_IMAGE_OPTIONAL_HEADER32 * pOptinalHeader;
}
Diana_PeFile32_impl;

typedef struct _diana_PeFile64_impl
{
    Diana_PeFile_impl parent;
    DIANA_IMAGE_OPTIONAL_HEADER64 * pOptinalHeader;
}
Diana_PeFile64_impl;



typedef struct _diana_PeFile
{
    Diana_PeFile_impl  * pImpl;
}
Diana_PeFile;

int Diana_InitPeFile(Diana_PeFile * pPeFile,
                     void * pFileBegin,
                     OPERAND_SIZE sizeOfFile);

void Diana_FreePeFile(Diana_PeFile * pPeFile);

#endif