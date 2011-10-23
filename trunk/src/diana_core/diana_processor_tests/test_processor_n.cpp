#include "test_processor_n.h"
#include "test_common.h"
#include "test_processor_impl.h"
#include "vector"

static void test_processor_neg()
{
    unsigned char code[] = {0xf7, 0xdb}; // neg         ebx

    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_EBX(0xFF);

    int res = proc.ExecOnce();
    
    TEST_ASSERT(res == DI_SUCCESS);
    TEST_ASSERT(GET_REG_RBX == 0xFFFFFF01);
    TEST_ASSERT(pCallContext->m_flags.l.l.value == 0x00000293);
}

static void test_processor_neg2()
{
    unsigned char code[] = {0xf7, 0xdb}; // neg         ebx

    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_EBX(0x80000000);

    int res = proc.ExecOnce();
    
    TEST_ASSERT(res == DI_SUCCESS);
    TEST_ASSERT(GET_REG_RBX == 0x80000000);
    TEST_ASSERT(pCallContext->m_flags.l.l.value == 0x00000A87);
}


void test_processor_n()
{
    test_processor_neg();
    test_processor_neg2();
}