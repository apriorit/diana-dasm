extern "C"
{
#include "diana_patchers/diana_ultimate_patcher.h"
#include "diana_streams.h"
}

#include "test_common.h"
#include "string.h"
#include "vector"
#include "test_processor_impl.h"

static int DianaHook_Alloc_test(void * pThis, 
                                 OPERAND_SIZE size, 
                                 OPERAND_SIZE * pAddress,
                                 const OPERAND_SIZE * pHintAddress,
                                 int flags);
static void DianaHook_Free_test(void * pThis, 
                                 const OPERAND_SIZE * pAddress);

class CTestMemoryProvider
{
    CTestMemoryProvider(const CTestMemoryProvider&);
    CTestMemoryProvider & operator = (const CTestMemoryProvider&);
public:
    DianaMemoryStream m_memoryStream;
    DianaHook_Allocator m_allocator;
    DianaHook_TargetMemoryProvider  m_impl;
    std::vector<char> m_heap;

    CTestMemoryProvider(const void * pBuffer,
                        DIANA_SIZE_T size)
    {
        if (size == 0)
            throw std::runtime_error("Invalid size");

        m_heap.assign((char*)pBuffer, (char*)pBuffer+size);
        DianaHook_Allocator_Init(&m_allocator, DianaHook_Alloc_test, DianaHook_Free_test);
        Diana_InitMemoryStreamEx(&m_memoryStream, &m_heap.front(), m_heap.size(), 1);
        DianaHook_TargetMemoryProvider_Init(&m_impl, &m_memoryStream.parent, &m_allocator);
    }
    DianaHook_TargetMemoryProvider  * GetTargetMemoryProvider()
    {
        return &m_impl;
    }
    OPERAND_SIZE Alloc(OPERAND_SIZE size)
    {
        OPERAND_SIZE result = m_heap.size();
        m_heap.resize(m_heap.size() + size);
        m_memoryStream.bufferSize = m_heap.size();

        m_memoryStream.pBuffer = &m_heap.front();
        m_memoryStream.bufferSize = m_heap.size();
        return result;
    }

};
int DianaHook_Alloc_test(void * pThis, 
                                 OPERAND_SIZE size, 
                                 OPERAND_SIZE * pAddress,
                                 const OPERAND_SIZE * pHintAddress,
                                 int flags)
{
    CTestMemoryProvider * pProvider = DIANA_CONTAINING_RECORD(pThis, CTestMemoryProvider, m_allocator);
    *pAddress = pProvider->Alloc(size);
    return DI_SUCCESS;
}
void DianaHook_Free_test(void * pThis, 
                                 const OPERAND_SIZE * pAddress)
{
    CTestMemoryProvider * pProvider = DIANA_CONTAINING_RECORD(pThis, CTestMemoryProvider, m_allocator);
}

static void test_patchers1()
{
    unsigned char data[] = 
    {
        // function to hook
        0x55,              // push        ebp  
        0x8B, 0xEC,        // mov         ebp,esp 
        0x5D,              // pop         ebp  
        0xC3,              // ret       

        // handler
        0x55,              // push        ebp  
        0x8B, 0xEC,        // mov         ebp,esp 
        0x5D,              // pop         ebp  
        0xC2, 0x0C, 0x00   // ret   8
    };
    CTestMemoryProvider testMemoryProvider(data, 
                                           sizeof(data));

    TEST_ASSERT(0 == DianaHook_PatchStream(testMemoryProvider.GetTargetMemoryProvider(),
                                           DIANA_MODE32,
                                           0,
                                           5,
                                           0));

    // allocate ESP
    OPERAND_SIZE espEnd = testMemoryProvider.m_heap.size();
    OPERAND_SIZE espStart = ((espEnd&(~0xF))+8)+1024;
    testMemoryProvider.m_heap.resize(espStart+8);

    const OPERAND_SIZE eofRIP = 0x12345678;
    *(OPERAND_SIZE *)(&testMemoryProvider.m_heap.front() + espStart) = eofRIP;

    unsigned char * pBuffer = (unsigned char *)&testMemoryProvider.m_heap.front();
    size_t bufferSize = testMemoryProvider.m_heap.size();

    CTestProcessor processor(pBuffer, bufferSize);
    DianaProcessor * pCallContext = processor.GetSelf();

    SET_REG_RSP(espStart);
    
    for(int i = 0; i < 100; ++i)
    {
        OPERAND_SIZE rip = GET_REG_RIP;

        if (rip == eofRIP)
            break;

        OPERAND_SIZE esp = GET_REG_RSP;
        int res = processor.ExecOnce();
        TEST_ASSERT(!res);
    }
}
void test_patchers()
{
    DIANA_TEST(test_patchers1());
}

