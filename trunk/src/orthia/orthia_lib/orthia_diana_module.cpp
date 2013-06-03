#include "orthia_diana_module.h"
#include "orthia_memory_cache.h"
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
                                        DianaAnalyzeJumpFlags_type * pFlags);

struct DianaMemoryStream:public DianaMovableReadStream
{
    IMemoryReader * m_pMemoryReader;
    OPERAND_SIZE m_currentOffset;

    DianaMemoryStream(Address_type currentOffset, IMemoryReader * pMemoryReader)
         : 
            m_pMemoryReader(pMemoryReader),
            m_currentOffset(currentOffset)
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
            m_stream(moduleStart, pMemoryReader),
            m_moduleStart(moduleStart),
            m_moduleSize(0)
    {
        DianaAnalyzeObserver_Init(this, 
                                  &m_stream, 
                                  DianaEnvironment_ConvertAddressToRelative, 
                                  DianaEnvironment_AnalyzeJumpAddress); 
    }
};

static 
int DianaEnvironment_ConvertAddressToRelative(void * pThis, 
                                              OPERAND_SIZE address,               
                                              OPERAND_SIZE * pRelativeOffset,
                                              int * pbInvalidPointer)
{
    *pbInvalidPointer = 0;
    *pRelativeOffset = address;
    return DI_SUCCESS;
}

static 
int DianaEnvironment_AnalyzeJumpAddress(void * pThis, 
                                        OPERAND_SIZE address,
                                        DianaAnalyzeJumpFlags_type * pFlags)
{
    DianaEnvironment * pEnv = (DianaEnvironment * )pThis;
    *pFlags = diaJumpNormal;

    if (!pEnv->m_moduleSize)
    {
        Diana_FatalBreak();
    }
    if (address < pEnv->m_moduleStart || address > (pEnv->m_moduleStart + pEnv->m_moduleSize))
    {
        *pFlags = diaJumpExternal;
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
        m_cache.Init(offset, m_env.m_moduleSize);
        DI_CHECK_CPP(Diana_PE_AnalyzePE(&m_peFile, &m_env, &m_owner));
        m_instructionsOwnerGuard.reset(&m_owner);
    }
};
CDianaModule::CDianaModule()
    :
        m_pMemoryReader(0)
{
}

CDianaModule::~CDianaModule()
{
}

void CDianaModule::Init(Address_type offset,
                        IMemoryReader * pMemoryReader)
{
    m_pMemoryReader = pMemoryReader;
    m_impl.reset(new CDianaModuleImpl(offset, pMemoryReader));
    m_name = L"test";
    m_uniqueName = L"{62F8BBF9-79AC-481d-8FB4-46AB8CC10CD0}";
}

std::wstring CDianaModule::GetUniqueName() const
{
    if (m_uniqueName.empty())
        throw std::runtime_error("Internal error: m_uniqueName is empty");
    return m_uniqueName;
}

std::wstring CDianaModule::GetName() const
{
    if (m_name.empty())
        throw std::runtime_error("Internal error: m_name is empty");
    return m_name;
}

}