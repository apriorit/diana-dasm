#include "orthia_diana_module.h"
#include "orthia_memory_cache.h"
#include <iomanip>
#include "orthia_sha1.h"
extern "C"
{
#include "diana_pe_analyzer.h"
}
#include "diana_core_cpp.h"

namespace orthia
{

int DianaAnalyzeMoveTo(void * pThis, 
                       OPERAND_SIZE offset);
int DianaRead(void * pThis, 
              void * pBuffer, 
              int iBufferSize, 
              int * readed);

int DianaEnvironment_ConvertAddressToRelative(void * pThis, 
                                              OPERAND_SIZE address,              
                                              OPERAND_SIZE * pRelativeOffset,
                                              int * pbInvalidPointer);

int DianaEnvironment_AnalyzeJumpAddress(void * pThis, 
                                       OPERAND_SIZE address,
                                       int flags,
                                       OPERAND_SIZE * pRelativeOffset,
                                       DianaAnalyzeAddressResult_type * pResult);

struct DianaMemoryStream:public DianaMovableReadStream
{
    IMemoryReader * m_pMemoryReader;
    OPERAND_SIZE m_currentOffset;
    OPERAND_SIZE m_moduleSize;

    DianaMemoryStream(Address_type currentOffset, 
                      IMemoryReader * pMemoryReader,
                      OPERAND_SIZE moduleSize)
         : 
            m_pMemoryReader(pMemoryReader),
            m_currentOffset(currentOffset),
            m_moduleSize(moduleSize)
    {

        DianaMovableReadStream_Init(this,
                                    DianaRead,
                                    DianaAnalyzeMoveTo);
    }
};
static
int DianaAnalyzeMoveTo(void * pThis, 
                       OPERAND_SIZE offset)
{
    DianaMemoryStream * pStream = (DianaMemoryStream * )pThis;

    if (pStream->m_moduleSize)
    {
        if (offset > pStream->m_moduleSize)
            return DI_END_OF_STREAM;
    }
    pStream->m_currentOffset = offset;
    return DI_SUCCESS;
}
static
int DianaRead(void * pThis, 
              void * pBuffer, 
              int iBufferSize, 
              int * bytesRead)
{
    try
    {
        Address_type bytesRead2 = 0;
        DianaMemoryStream * pStream = (DianaMemoryStream * )pThis;
        pStream->m_pMemoryReader->Read(pStream->m_currentOffset, 
                                    iBufferSize,
                                    pBuffer,
                                    &bytesRead2);
        *bytesRead = (int)bytesRead2;
        pStream->m_currentOffset += (Address_type)*bytesRead;
        return DI_SUCCESS;
    }
    catch(std::exception & ex)
    {
        &ex;
        return DI_ERROR;
    }
}

struct DianaEnvironment:public DianaAnalyzeObserver
{
    Address_type m_moduleStart;
    Address_type m_moduleSize;
    DianaMemoryStream m_stream;

    DianaEnvironment(Address_type moduleStart, IMemoryReader * pMemoryReader)
        :
            m_stream(0, pMemoryReader, 0),
            m_moduleStart(moduleStart),
            m_moduleSize(0)
    {
        DianaAnalyzeObserver_Init(this, 
                                  &m_stream, 
                                  DianaEnvironment_AnalyzeJumpAddress); 
    }
};

static 
int DianaEnvironment_ConvertAddressToRelative(void * pThis, 
                                              OPERAND_SIZE address,               
                                              OPERAND_SIZE * pRelativeOffset,
                                              int * pbInvalidPointer)
{
    DianaEnvironment * pEnv = (DianaEnvironment * )pThis;
    *pbInvalidPointer = 1;
    if (address < pEnv->m_moduleStart)
        return DI_SUCCESS;

    if (address > pEnv->m_moduleSize)
        return DI_SUCCESS;

    *pbInvalidPointer = 0;
    *pRelativeOffset = address - pEnv->m_moduleStart; 
    return DI_SUCCESS;
}

static 
int DianaEnvironment_AnalyzeJumpAddress(void * pThis, 
                                       OPERAND_SIZE address,
                                       int flags,
                                       OPERAND_SIZE * pRelativeOffset,
                                       DianaAnalyzeAddressResult_type * pResult)
{
    DianaEnvironment * pEnv = (DianaEnvironment * )pThis;
    *pResult = diaJumpNormal;
    *pRelativeOffset = address;

    if (!pEnv->m_moduleSize)
    {
        Diana_FatalBreak();
    }
    if (flags&DIANA_ANALYZE_ABSOLUTE_ADDRESS)
    {
        int invalidPointer = 0;
        DI_CHECK(DianaEnvironment_ConvertAddressToRelative(pThis, 
                                                           address, 
                                                           pRelativeOffset,
                                                           &invalidPointer));
        if (invalidPointer)
        {
            *pResult = diaJumpExternal;
        }
    }
    if (address > pEnv->m_moduleSize)
    {
        *pResult = diaJumpExternal;
    }
    return DI_SUCCESS;
}

class CDianaModuleImpl
{
    Diana_PeFile m_peFile;
    CMemoryCache m_cache;
    DianaEnvironment m_env;
    diana::Guard<diana::PeFile> m_peFileGuard;
    Diana_InstructionsOwner m_owner;
    diana::Guard<diana::InstructionsOwner> m_instructionsOwnerGuard;
public:
    CDianaModuleImpl(Address_type offset,
                     IMemoryReader * pMemoryReader)
    :
        m_cache(pMemoryReader),
        m_env(offset, &m_cache)
    {
        DI_CHECK_CPP(DianaPeFile_Init(&m_peFile, &m_env.m_stream, 0, offset));
        m_peFileGuard.reset(&m_peFile);
        m_env.m_moduleSize = m_peFile.pImpl->sizeOfFile;
        m_env.m_stream.m_moduleSize = m_peFile.pImpl->sizeOfFile;
    }

    void Analyze()
    {
        m_cache.Init(0, m_env.m_moduleSize);
        DI_CHECK_CPP(Diana_PE_AnalyzePE(&m_peFile, &m_env, &m_owner));
        m_instructionsOwnerGuard.reset(&m_owner);
    }
    const Diana_PeFile * GetPeFile() const { return &m_peFile; }
};
CDianaModule::CDianaModule()
    :
        m_pMemoryReader(0),
        m_offset(0)
{
}

CDianaModule::~CDianaModule()
{
}

void CDianaModule::Init(Address_type offset,
                        IMemoryReader * pMemoryReader)
{
    m_offset = offset;
    m_pMemoryReader = pMemoryReader;
    m_impl.reset(new CDianaModuleImpl(offset, pMemoryReader));

    // calc unique hash
    uint8_t hash[SHA1_HASH_SIZE];
    memset(hash, 0, sizeof(hash));
    SHA1Context shaContext;      
    SHA1Reset(&shaContext);
    SHA1Input2(&shaContext, &m_impl->GetPeFile()->pImpl->dosHeader);
    SHA1Input2(&shaContext, &m_impl->GetPeFile()->pImpl->ntHeaders);
    SHA1Input2(&shaContext, m_impl->GetPeFile()->pImpl->pCapturedSections, m_impl->GetPeFile()->pImpl->capturedSectionCount);
    SHA1Result(&shaContext, hash);
    m_uniqueName = orthia::ToHexString(hash);
}

void CDianaModule::Analyze()
{
    m_impl->Analyze();
}
std::wstring CDianaModule::GetUniqueName() const
{
    if (m_uniqueName.empty())
        throw std::runtime_error("Internal error: m_uniqueName is empty");
    return m_uniqueName;
}

std::wstring CDianaModule::GetName() const
{
    std::wstringstream str;
    std::hex(str);
    str<<std::setfill(L'0') << std::setw(m_impl->GetPeFile()->pImpl->dianaMode*2) <<m_offset;
    return str.str();
}

}