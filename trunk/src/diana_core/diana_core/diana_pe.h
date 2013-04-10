#ifndef DIANA_PE_H
#define DIANA_PE_H

#include "diana_pe_defs.h"
#include "diana_streams.h"
#include "diana_analyze.h"

typedef struct _diana_PeFile_impl
{
    // basic
    DIANA_IMAGE_DOS_HEADER dosHeader;
    DIANA_IMAGE_NT_HEADERS ntHeaders;
    DIANA_IMAGE_SECTION_HEADER * pCapturedSections;
    int capturedSectionCount;

    // advanced
    int dianaMode;
    OPERAND_SIZE sizeOfFile;
    OPERAND_SIZE sizeOfImpl;
    int optionalHeaderSize;
}Diana_PeFile_impl;

typedef struct _diana_PeFile32_impl
{
    Diana_PeFile_impl parent;
    DIANA_IMAGE_OPTIONAL_HEADER32 * pOptionalHeader;
}
Diana_PeFile32_impl;

typedef struct _diana_PeFile64_impl
{
    Diana_PeFile_impl parent;
    DIANA_IMAGE_OPTIONAL_HEADER64 * pOptionalHeader;
}
Diana_PeFile64_impl;


typedef struct _diana_PeFile
{
    Diana_PeFile_impl  * pImpl;
}
Diana_PeFile;

int Diana_InitPeFile(/* out */ Diana_PeFile * pPeFile,
                      /* in */ DianaMovableReadStream * pStream,
                      /* in, optional */ OPERAND_SIZE sizeOfFile);

void Diana_FreePeFile(Diana_PeFile * pPeFile);
#endif