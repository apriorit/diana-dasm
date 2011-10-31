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

    TEST_ASSERT(pCallContext->m_flags.l.value == 0x246);
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


static void test_processor_and64()
{
	unsigned char buff[] = {0x25, 0xf0, 0xf0, 0xf0, 0xf0};
	//and eax,0f0f0f0f0h

	CTestProcessor proc(buff, sizeof(buff), 0, DIANA_MODE64);
	DianaProcessor * pCallContext = proc.GetSelf();

	SET_REG_RAX(0xFFFFFFFFFFFFFFFFULL);
	int res = proc.ExecOnce();
	TEST_ASSERT(res == DI_SUCCESS);

	TEST_ASSERT(GET_REG_RAX == 0x00000000F0F0F0F0ULL);
}


static void test_processor_and64_2()
{
	unsigned char buff[] = {0x66, 0x25, 0xf0, 0xf0, 0xf0, 0xf0};
	//and ax,0f0f0h

	CTestProcessor proc(buff, sizeof(buff), 0, DIANA_MODE64);
	DianaProcessor * pCallContext = proc.GetSelf();

	SET_REG_RAX(0xFFFFFFFFFFFFFFFFULL);
	int res = proc.ExecOnce();
	TEST_ASSERT(res == DI_SUCCESS);
	TEST_ASSERT(GET_REG_RIP == 0x0004);

	TEST_ASSERT(GET_REG_RAX == 0xFFFFFFFFFFFFF0F0ULL);
}

void test_processor_a()
{
    test_processor_and();
    test_processor_and2();
    test_processor_adc();
	test_processor_and64();
	test_processor_and64_2();
}