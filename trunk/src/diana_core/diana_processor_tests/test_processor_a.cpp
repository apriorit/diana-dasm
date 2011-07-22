#include "test_processor_a.h"
#include "test_common.h"
#include "test_processor_impl.h"

static void test_processor_and()
{
    unsigned char buff[] = {0x24, 0x01}; //and         al,1 
    
    CTestProcessor proc(buff, sizeof(buff));
    DianaProcessor * pCallContext = proc.GetSelf();

    pCallContext->m_flags.l.value = 0x256;

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    TEST_ASSERT(pCallContext->m_flags.l.value == 0x256);
}


static void test_processor_and2()
{
    unsigned char buff[] = {0x83, 0x66, 0x00, 0x00,//and  [esi], 0
                            0,0,0,0
    }; 
    
    CTestProcessor proc(buff, sizeof(buff));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_ESI(4);
    pCallContext->m_flags.l.value = 0x206;

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    TEST_ASSERT(pCallContext->m_flags.l.value == 0x246);
}


static void test_processor_adc()
{
    unsigned char buff[] = {0x83, 0xd3, 0xff};           //adc     ebx,0xffffffff
    
    CTestProcessor proc(buff, sizeof(buff));
    DianaProcessor * pCallContext = proc.GetSelf();

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    TEST_ASSERT(GET_REG_EBX == 0xffffffff);
}
void test_processor_a()
{
    test_processor_and();
    test_processor_and2();
    test_processor_adc();
}