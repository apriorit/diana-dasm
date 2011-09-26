#include "test_processor_i.h"
#include "test_common.h"
#include "test_processor_impl.h"
#include "vector"

static void test_processor_idiv()
{
    unsigned char code[] = {0xF7, 0xF9};      
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_EDX(2);
    SET_REG_EAX(1);
    SET_REG_ECX(-10);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_RAX == 0xCCCCCCCD);
    TEST_ASSERT(GET_REG_RDX == 0x00000003);
    TEST_ASSERT(GET_REG_RCX == 0xFFFFFFF6);
}


static void test_processor_imul()
{
    unsigned char code[] = {0xF6, 0xEB}; //imul        bl      
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_BL((DI_CHAR)-2);
    SET_REG_AL((DI_CHAR)-128);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_RAX == 0x100);
    TEST_ASSERT(GET_FLAG_CF);
    TEST_ASSERT(GET_FLAG_OF);
}

static void test_processor_imul2()
{
    unsigned char code[] = {0xF6, 0xEB}; //imul        bl      
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_BL((DI_CHAR)2);
    SET_REG_AL((DI_CHAR)-128);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_RAX == 0xFF00);
    TEST_ASSERT(GET_FLAG_CF);
    TEST_ASSERT(GET_FLAG_OF);
}

static void test_processor_imul3()
{
    unsigned char code[] = {0xF6, 0xEB}; //imul        bl      
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_BL((DI_CHAR)1);
    SET_REG_AL((DI_CHAR)-128);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_RAX == 0xFF80);
    TEST_ASSERT(GET_FLAG_CF);
    TEST_ASSERT(GET_FLAG_OF);
}

static void test_processor_imul4()
{
    unsigned char code[] = {0xF6, 0xEB}; //imul        bl      
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_BL((DI_CHAR)-1);
    SET_REG_AL((DI_CHAR)127);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_RAX == 0xFF81);
    TEST_ASSERT(!GET_FLAG_CF);
    TEST_ASSERT(!GET_FLAG_OF);
}

static void test_processor_imul5()
{
    unsigned char code[] = {0x6B,0xC2,0x02}; // imul        eax,edx,2 
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_EDX(0x7FFFFFFF);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_RAX == 0xFFFFFFFe);
    TEST_ASSERT(GET_FLAG_CF);
    TEST_ASSERT(GET_FLAG_OF);
}

static void test_processor_imul6()
{
    unsigned char code[] = {0x6B,0xC2,0x02}; // imul        eax,edx,2 
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_EDX(0xFFFFFFFF);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_RAX == 0xFFFFFFFe);
    TEST_ASSERT(!GET_FLAG_CF);
    TEST_ASSERT(!GET_FLAG_OF);
}

static void test_processor_imul7()
{
    unsigned char code[] = {0x0F, 0xAF, 0xC2}; // imul        eax,edx
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_EAX(-23455);
    SET_REG_EDX(5636);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_RAX == 0xF81EE784);
    TEST_ASSERT(!GET_FLAG_CF);
    TEST_ASSERT(!GET_FLAG_OF);
}

static void test_processor_imul8()
{
    unsigned char code[] = {0xf7, 0xea}; // imul        edx
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_EAX(0x40000000);
    SET_REG_EDX(0x00000130);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_RAX == 0x00000000);
    TEST_ASSERT(GET_REG_RDX == 0x0000004c);
}

static void test_processor_imul9()
{
    unsigned char code[] = {0x66, 0x6B, 0xC2, 0xFF}; // imul        ax,dx,-1
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_EDX(0x2);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
        
    TEST_ASSERT(GET_REG_RAX == 65534);
    TEST_ASSERT(!GET_FLAG_CF);
    TEST_ASSERT(!GET_FLAG_OF);
}

static void test_processor_imul10()
{
    unsigned char code[] = {0x66, 0x6B, 0xC2, 0xD3}; // imul        ax,dx,0FFD3h
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_EDX(0x17);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
        
    TEST_ASSERT(GET_REG_RAX == 0xfbf5);
    TEST_ASSERT(!GET_FLAG_CF);
    TEST_ASSERT(!GET_FLAG_OF);
}

static void test_processor_inc()
{
    unsigned char code[] = {0xF0, 0xFF, 0x42, 0x04, 0xff,0xff,0xff,0xff}; // lock inc    dword ptr [edx+4] 
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_EDX(0);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_FLAG_ZF);
}

void test_processor_i()
{
    test_processor_idiv();
    test_processor_imul();
    test_processor_imul2();
    test_processor_imul3();
    test_processor_imul4();
    test_processor_imul5();
    test_processor_imul6();
    test_processor_imul7();
    test_processor_imul8();
    test_processor_imul9();
    test_processor_imul10();
    test_processor_inc();
}