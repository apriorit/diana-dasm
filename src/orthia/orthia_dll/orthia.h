#ifndef ORTHIA_H
#define ORTHIA_H


#define KDEXT_64BIT

#include "windows.h"
#include "imagehlp.h"
#include "wdbgexts.h"


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


#define ORTHIA_TRY try {
#define ORTHIA_CATCH } catch(const std::exception & e) { dprintf("Error: %s\n", e.what()); } 

namespace orthia
{
void InitLib();
}


#endif 