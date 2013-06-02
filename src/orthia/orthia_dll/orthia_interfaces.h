#ifndef ORTHIA_INTERFACES_H
#define ORTHIA_INTERFACES_H

#include "orthia_utils.h"

namespace orthia
{

struct IMemoryReader
{
    virtual ~IMemoryReader(){}
    virtual void Read(Address_type offset, 
                      Address_type bytesToRead,
                      void * pBuffer,
                      Address_type * pBytesRead)=0;
};

}

#endif