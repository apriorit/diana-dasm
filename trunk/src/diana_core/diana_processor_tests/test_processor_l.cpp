#include "test_processor_l.h"
#include "test_common.h"
#include "test_processor_impl.h"
#include "vector"

void test_processor_lea()
{
	// lea         ebx,[esp+ebx*2] 
    unsigned char code[] = {0x8D, 0x1C, 0x5C};

    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_ESP(1);
    SET_REG_EBX(6);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_EBX == 13);
}


void test_processor_lods()
{
	// lods    byte ptr [esi] 
    unsigned char code[] = {0xAC};

    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_ESI == 1);
    TEST_ASSERT(GET_REG_EAX == 0xAC);
}

void test_processor_lods2()
{
	// lods        word ptr [esi] 
    unsigned char code[] = {0x66, 0xAD};

    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    TEST_ASSERT(GET_REG_ESI == 2);
	TEST_ASSERT(GET_REG_EAX == 0xAD66);
}

void test_processor_l()
{
    test_processor_lea();
    test_processor_lods();
    test_processor_lods2();
}
