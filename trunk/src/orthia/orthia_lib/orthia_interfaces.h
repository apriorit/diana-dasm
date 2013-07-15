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

class CMemoryReader:public IMemoryReader
{
    Address_type m_base;
public:
    CMemoryReader(Address_type base)
        :
            m_base(base)
    {
    }
    virtual void Read(Address_type offset, 
                      Address_type bytesToRead,
                      void * pBuffer,
                      Address_type * pBytesRead)
    {
        void * realAddress = (void *)(m_base + offset);
        memcpy(pBuffer, realAddress, (size_t)bytesToRead);
        *pBytesRead = (size_t)bytesToRead;
    }
};

struct CommonReferenceInfo
{
    Address_type address;
    bool external;
    CommonReferenceInfo(Address_type address_in,
                       bool external_in)
      :
        address(address_in),
        external(external_in)
    {
    }
};

// module info
struct CommonModuleInfo
{
    Address_type address;
    std::wstring name;
    CommonModuleInfo()
    {
    }
    CommonModuleInfo(Address_type address_in,
                     const std::wstring & name_in)
                     :
        address(address_in),
        name(name_in)
    {
    }
};

}

#endif