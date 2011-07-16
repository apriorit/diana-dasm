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

void test_processor_l()
{
    test_processor_lea();
}