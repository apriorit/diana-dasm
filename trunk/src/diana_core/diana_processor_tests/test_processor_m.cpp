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

void test_processor_movsxd64()
{
    unsigned char code[9] = 
    {
        0x48, 0x63, 0x4C, 0x24, 0x04, 1, 2, 3, 0x80        //movsxd  rcx, [rsp+4]
    }; 

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_RSP(1);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    TEST_ASSERT(GET_REG_RCX == 0xFFFFFFFF80030201ULL);
}


void test_processor_movsxd64_2()
{
    unsigned char code[9] = 
    {
        0x63, 0x4C, 0x24, 0x04, 1, 2, 3, 0x80        //movsxd  rcx, [rsp+4]
    }; 

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
    DianaProcessor * pCallContext = proc.GetSelf();

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    TEST_ASSERT(GET_REG_RCX == 0xFFFFFFFF80030201ULL);
}


void test_processor_mov64()
{
    unsigned char code[] = {0x48, 0xb8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; 
    // 48b8ffffffffffffffff mov rax,0FFFFFFFFFFFFFFFFh

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
    DianaProcessor * pCallContext = proc.GetSelf();

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    TEST_ASSERT(GET_REG_RAX == 0xFFFFFFFFFFFFFFFFULL);
}


void test_processor_mov64_2()
{
    unsigned char code[] = {0x48, 0xc7, 0xc0, 0xff, 0xff, 0xff, 0xff}; 
    // 48c7c0ffffffff      rax,0FFFFFFFFFFFFFFFFh

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
    DianaProcessor * pCallContext = proc.GetSelf();

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    TEST_ASSERT(GET_REG_RAX == 0xFFFFFFFFFFFFFFFFULL);
}

void test_processor_mov64_3()
{
    unsigned char code[] = {0xc7, 0xc0, 0x55, 0x55, 0x55, 0x55}; 
    // mov eax,-1

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_RAX(0xFFFFFFFFFFFFFFFFULL);
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    TEST_ASSERT(GET_REG_RAX == 0x0000000055555555ULL);
}


void test_processor_mov64_4()
{
	unsigned char code[] = {0xb8, 0x55, 0x55, 0x55, 0x55}; 
	// mov eax,-1

	CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
	DianaProcessor * pCallContext = proc.GetSelf();

	SET_REG_RAX(0xFFFFFFFFFFFFFFFFULL);
	int res = proc.ExecOnce();
	TEST_ASSERT(res == DI_SUCCESS);

	TEST_ASSERT(GET_REG_RAX == 0x0000000055555555ULL);
}

void test_processor_m()
{
    test_processor_movs();
    test_processor_movsxd64();
    test_processor_movsxd64_2();
    test_processor_mov64();
    test_processor_mov64_2();
    test_processor_mov64_3();
	test_processor_mov64_4();
}