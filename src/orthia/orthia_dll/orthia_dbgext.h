#ifndef ORTHIA_DBGEXT_H
#define ORTHIA_DBGEXT_H

#define KDEXT_64BIT
#include "windows.h"
#include "imagehlp.h"
#include "wdbgexts.h"
#include <dbgeng.h>

#define EXT_EXPORT CPPMOD _declspec(dllexport)

#define SIGN_EXTEND(_x_) (ULONG64)(LONG)(_x_)

#define READ_VALUE(node, address, return_0) \
{\
    ULONG bytes = 0;\
    ReadMemory(SIGN_EXTEND(address), \
               &node, \
               sizeof(node), \
               &bytes);\
    if (bytes != sizeof(node))\
    {\
        dprintf("Inaccessible memory %I64lx\n", address);\
        return_0;\
    }\
}

#define INIT_API()                             \
    HRESULT Status;                            \
    if ((Status = ExtQuery(Client)) != S_OK) return Status;

#define EXIT_API     ExtRelease

#define EXT_RELEASE(Unk) \
    ((Unk) != NULL ? ((Unk)->Release(), (Unk) = NULL) : NULL)

ULONG DbgExt_GetTargetMachine();
bool DbgExt_GetNameByOffset(ULONG64 offset,
                            PSTR nameBuffer,
                            ULONG nameBufferSize,
                            PULONG pResultNameSize,
                            ULONG64 * pDisplacement);

void
ExtRelease(void);

HRESULT
ExtQuery(PDEBUG_CLIENT4 Client);

#endif