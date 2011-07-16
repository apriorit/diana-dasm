#include "test_processor_m.h"
#include "test_common.h"
#include "test_processor_impl.h"
#include "vector"

void test_processor_movs()
{
    unsigned char code[1024] = 
    {
        0xF3, 0xA4                      //rep movs    byte ptr es:[edi],byte ptr [esi] 
    }; 

    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_ESI(100);
    SET_REG_EDI(200);
    SET_REG_ECX(7);

    memcpy(code+100, "hello!", 7);
    memset(code+200, '*', 7);
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    TEST_ASSERT(memcmp(code+200, "hello!", 7) == 0);
}

void test_processor_m()
{
    test_processor_movs();
}