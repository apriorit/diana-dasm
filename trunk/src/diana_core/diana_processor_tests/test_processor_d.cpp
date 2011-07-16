#include "test_processor_d.h"
#include "test_common.h"
#include "test_processor_impl.h"
#include "vector"

static void test_processor_div()
{
    // div         edx:eax,ecx 
    unsigned char code[] = {0xF7, 0xF1};      
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_EDX(2);
    SET_REG_EAX(1);
    SET_REG_ECX(0x10);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_RAX == 0x20000000);
    TEST_ASSERT(GET_REG_RDX == 0x1);
    TEST_ASSERT(GET_REG_RCX == 0x10);
}

static void test_processor_div2()
{
    // div         dx:ax, cx 
    unsigned char code[] = {0x66, 0xF7, 0xF1};      
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_EDX(2);
    SET_REG_EAX(1);
    SET_REG_ECX(0x10);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_RAX == 0x00002000);
    TEST_ASSERT(GET_REG_RDX == 0x1);
    TEST_ASSERT(GET_REG_RCX == 0x10);
}



static void test_processor_div3()
{
    // div         ax, cl 
    unsigned char code[] = {0xF6, 0xF1}; 
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_EAX(0x123);
    SET_REG_ECX(0x10);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_RAX == 0x312);

}


static void test_processor_div_fail()
{
    // div         edx:eax,ecx 
    unsigned char code[] = {0xF7, 0xF1};      
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_EDX(0xFF);
    SET_REG_EAX(1);
    SET_REG_ECX(0x10);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_DIVISION_OVERFLOW);
    
}

void test_processor_d()
{
    test_processor_div();
    test_processor_div2();
    test_processor_div3();
    test_processor_div_fail();
}