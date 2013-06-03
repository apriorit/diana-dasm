#ifndef ORTHIA_MEMORY_CACHE_H
#define ORTHIA_MEMORY_CACHE_H

#include "orthia_interfaces.h"

namespace orthia
{

class CMemoryCache:public IMemoryReader
{
    typedef std::vector<char> RegionData_type;
    RegionData_type m_regionData;
    Address_type m_regionAddress;
    Address_type m_regionSize;

    IMemoryReader * m_pReader;
public:
    CMemoryCache(IMemoryReader * pReader);

    void Init(Address_type regionAddress,
              std::vector<char> * pData);

    void Init(Address_type regionAddress,
              Address_type size);

    virtual void Read(Address_type offset, 
                      Address_type bytesToRead,
                      void * pBuffer,
                      Address_type * pBytesRead);

};

}

#endif