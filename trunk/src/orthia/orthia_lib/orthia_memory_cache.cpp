#include "orthia_memory_cache.h"
#include "diana_pe.h"

namespace orthia
{

CMemoryCache::CMemoryCache(IMemoryReader * pReader,
                           Address_type regionAddress)
    :
        m_pReader(pReader),
        m_regionAddress(regionAddress),
        m_regionSize(0)
{
}
    

void CMemoryCache::Init(Address_type regionAddress,
                          Address_type size,
                          DIANA_IMAGE_SECTION_HEADER * pCapturedSections,
                          int capturedSectionCount)
{
    if (!size)
        return;

    if (size >= SIZE_MAX)
        throw std::runtime_error("Internal error: size too big");
    m_regionData.resize((size_t)size);
    
    m_regionAddress = regionAddress;
    m_regionSize = m_regionData.size();
    for(int i = 0; i < capturedSectionCount; ++i)
    {
        DIANA_IMAGE_SECTION_HEADER & section = pCapturedSections[i];
        if (section.Misc.VirtualSize > m_regionData.size())
            continue;
        if (section.VirtualAddress > m_regionData.size() - section.Misc.VirtualSize)
            continue;
        Address_type bytesRead = 0;
        m_pReader->Read(m_regionAddress + section.VirtualAddress, 
                        section.Misc.VirtualSize, 
                        section.VirtualAddress + &m_regionData.front(), 
                        &bytesRead);
    }
}
    
void CMemoryCache::Read(Address_type offset, 
                        Address_type bytesToRead,
                        void * pBuffer,
                        Address_type * pBytesRead)
{
    *pBytesRead = 0;
    // it gets relative addresses!!!!
    offset += m_regionAddress;
    if (!m_regionSize)
    {
        m_pReader->Read(offset, bytesToRead, pBuffer, pBytesRead);
        return;
    }

    Address_type end = offset + bytesToRead;

    // 1
    //A: [-]
    //B:    [--------]
    // 2
    //A: [----]
    //B:    [--------]
    // 3
    //A: [----------------]
    //B:    [--------]
    // 4
    //A:      [---]
    //B:    [--------]
    // 5
    //A:        [-------]
    //B:    [--------]
    // 6
    //A:               [---]
    //B:    [--------]

    if (offset < m_regionAddress)
    {
        // 1,2,3
        return;
    }
    // 4, 5, 6
    Address_type regionEnd = m_regionAddress + m_regionSize;
    if (offset >= regionEnd)
    {
        // 6
        return;
    }
    if (end < regionEnd)
    {
        // 4
        memcpy(pBuffer, 
               &m_regionData.front() + offset - m_regionAddress,
               (size_t)bytesToRead);
        *pBytesRead = bytesToRead;
        return;
    }
    // 5
    Address_type skipSize = offset - m_regionAddress;
    Address_type sizeToRead = m_regionSize - skipSize;
    memcpy(pBuffer, 
           &m_regionData.front() + skipSize,
           (size_t)sizeToRead);    
    
    *pBytesRead += sizeToRead;
}


}