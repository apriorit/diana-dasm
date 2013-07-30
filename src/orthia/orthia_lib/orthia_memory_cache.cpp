#include "orthia_memory_cache.h"

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
                        Address_type size)
{
    if (!size)
        return;

    if (size >= SIZE_MAX)
        throw std::runtime_error("Internal error: size too big");
    m_regionData.resize((size_t)size);
    
    m_regionAddress = regionAddress;
    Address_type bytesRead = 0;
    m_pReader->Read(m_regionAddress, size, &m_regionData.front(), &bytesRead);
    m_regionData.resize((size_t)bytesRead);
    m_regionSize = bytesRead;
}
    
void CMemoryCache::Read(Address_type offset, 
                        Address_type bytesToRead,
                        void * pBuffer,
                        Address_type * pBytesRead)
{
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
        if (end <= m_regionAddress)
        {
            // 1
            m_pReader->Read(offset, bytesToRead, pBuffer, pBytesRead);
            return;
        }
        Address_type firstChunkSize = m_regionAddress - offset;
        m_pReader->Read(offset, firstChunkSize, pBuffer, pBytesRead);
        if (*pBytesRead != firstChunkSize)
            return;
        Address_type sizeToRead2 = m_regionSize;
        if (end <= m_regionAddress + m_regionSize)
        {
            // 2
            sizeToRead2 = end - m_regionAddress;
        }
        memcpy((char*)pBuffer + *pBytesRead, &m_regionData.front(), (size_t)sizeToRead2);
        *pBytesRead += sizeToRead2;
        // check the tail
        if (*pBytesRead > bytesToRead)
        {
            __debugbreak();
        }
        if (*pBytesRead == bytesToRead)
            return;

        Address_type tailReadBytes = 0;
        m_pReader->Read(offset + *pBytesRead, 
                        bytesToRead - *pBytesRead, 
                        (char*)pBuffer + *pBytesRead, 
                        &tailReadBytes);        
        *pBytesRead += tailReadBytes;
        return;
    }
    // 4, 5, 6
    Address_type regionEnd = m_regionAddress + m_regionSize;
    if (offset >= regionEnd)
    {
        // 6
        m_pReader->Read(offset, bytesToRead, pBuffer, pBytesRead);
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
    
    m_pReader->Read(regionEnd, 
                    bytesToRead-sizeToRead, 
                    (char*)pBuffer+sizeToRead, 
                    pBytesRead);
    *pBytesRead += sizeToRead;
}


}