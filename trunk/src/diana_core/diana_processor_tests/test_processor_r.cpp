#include "test_processor_r.h"
#include "test_common.h"
#include "test_processor_impl.h"
#include "vector"

void test_processor_rcl()
{
    // rcl eax, cl
    unsigned char code[] = {0xD3, 0xD0};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_RAX(0xA5A5A5A5);
    SET_REG_CL(5);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    OPERAND_SIZE eax = GET_REG_EAX;
    TEST_ASSERT(eax == 0xb4b4b4aa);    
    TEST_ASSERT(GET_REG_RIP == 2);

    TEST_ASSERT(!GET_FLAG_CF);
    TEST_ASSERT(!GET_FLAG_OF);
}

void test_processor_rcl2()
{
    // rcl eax, cl
    unsigned char code[] = {0xD3, 0xD0};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_RAX(0xA5A5A5A5);
    SET_REG_CL(1);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    OPERAND_SIZE eax = GET_REG_EAX;
    TEST_ASSERT(eax == 0x4B4B4B4A);
    TEST_ASSERT(GET_REG_RIP == 2);

    TEST_ASSERT(GET_FLAG_CF);
    TEST_ASSERT(GET_FLAG_OF);
}


void test_processor_rcl3()
{
    // rcl eax, cl
    unsigned char code[] = {0xD3, 0xD0};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_RAX(0xA5A5A5A5);
    SET_FLAG_CF;
    SET_REG_CL(2);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    OPERAND_SIZE eax = GET_REG_EAX;
    TEST_ASSERT(eax == 0x96969697);
    TEST_ASSERT(GET_REG_RIP == 2);

    TEST_ASSERT(!GET_FLAG_CF);
    TEST_ASSERT(!GET_FLAG_OF);
}

void test_processor_rcr()
{
    // rcr eax, 1
    unsigned char code[] = {0xD1, 0xD8};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_RAX(0xA5A5A5A5);
    SET_FLAG_CF;

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    OPERAND_SIZE eax = GET_REG_EAX;
    TEST_ASSERT(eax == 0xD2D2D2D2);
    TEST_ASSERT(GET_REG_RIP == 2);

    TEST_ASSERT(GET_FLAG_CF);
    TEST_ASSERT(!GET_FLAG_OF);
}


void test_processor_rol()
{
    // rol eax, 1
    unsigned char code[] = {0xD1, 0xC0};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_RAX(0xA5A5A5A4);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    OPERAND_SIZE eax = GET_REG_EAX;
    TEST_ASSERT(eax == 0x4B4B4B49);
    TEST_ASSERT(GET_REG_RIP == 2);

    TEST_ASSERT(GET_FLAG_CF);
    TEST_ASSERT(GET_FLAG_OF);
}

   
void test_processor_ror()
{
    // ror eax, 1
    unsigned char code[] = {0xD1, 0xC8};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_RAX(0xA5A5A5A5);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    OPERAND_SIZE eax = GET_REG_EAX;
    TEST_ASSERT(eax == 0xD2D2D2D2);
    TEST_ASSERT(GET_REG_RIP == 2);

    TEST_ASSERT(GET_FLAG_CF);
    TEST_ASSERT(!GET_FLAG_OF);
}

void test_processor_ror2()
{
    // ror eax, 1
    unsigned char code[] = {0xD1, 0xC8};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_RAX(0x75A5A5A5);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    OPERAND_SIZE eax = GET_REG_EAX;
    TEST_ASSERT(eax == 0xBAD2D2D2);
    TEST_ASSERT(GET_REG_RIP == 2);

    TEST_ASSERT(GET_FLAG_CF);
    TEST_ASSERT(GET_FLAG_OF);
}

void test_processor_r()
{
    test_processor_rcl();
    test_processor_rcl2();
    test_processor_rcl3();
    
    test_processor_rcr();
    test_processor_rol();

    test_processor_ror();
    test_processor_ror2();
    
}