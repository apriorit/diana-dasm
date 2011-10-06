#include "test_processor_j.h"
#include "test_common.h"
#include "test_processor_impl.h"
#include "vector"

static void test_processor_jmp()
{
    // jmp $+5
    unsigned char code[] = {0xE9, 0x00, 0x00, 00, 0x00};      
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_RIP == 5);
}

static void test_processor_jmp2()
{
    // jmp eax
    unsigned char code[] = {0xFF, 0xE0};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_EAX(1);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_RIP == 1)
}


static void test_processor_jmp3()
{
    // jmp         dword ptr es:[0Ah] 
    unsigned char code[] = {0x26, 0xFF, 0x25, 0x0A, 0x00, 0x00, 0x00};  

    const int segSize = 0x1000;
    std::vector<unsigned char> memory(0x2000);
    memcpy(&memory.front(), code, sizeof(code));

    CTestProcessor proc(&memory.front(), memory.size(), segSize);
    DianaProcessor * pCallContext = proc.GetSelf();

    OPERAND_SIZE newIP = 111;
    int res = DianaProcessor_SetMemValue(pCallContext, 
                               0,
                               0x100A,
                               4,
                               &newIP,
                               0,
                               reg_DS);
    TEST_ASSERT(res == DI_SUCCESS);

    SET_REG_ES(1);

    // exec call
    res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    OPERAND_SIZE rip = GET_REG_RIP;
    TEST_ASSERT(rip == newIP);
}


static void test_processor_jmp4()
{
    // jle         cc (10CBD4Fh) 
    unsigned char code[] = {0x7E, 0x2F};

    const OPERAND_SIZE newIP = 0x31;

    const int segSize = 0x1000;
    std::vector<unsigned char> memory(0x2000);
    memcpy(&memory.front(), code, sizeof(code));

    CTestProcessor proc(&memory.front(), memory.size(), segSize);
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_FLAG_SF;

    SET_REG_ES(1);

    // exec call
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    OPERAND_SIZE rip = GET_REG_RIP;
    TEST_ASSERT(rip == newIP);
}


void test_processor_j()
{
    test_processor_jmp();
    test_processor_jmp2();
    test_processor_jmp3();
    test_processor_jmp4();
}