#include "test_processor_s.h"
#include "test_common.h"
#include "test_processor_impl.h"
#include "vector"

void test_processor_set()
{
    // set al
    unsigned char code[] = {0x0F, 0x97, 0xC0,
                            0x0F, 0x97, 0xC0};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    OPERAND_SIZE eax = GET_REG_EAX;
    TEST_ASSERT(eax == 0x1);
    TEST_ASSERT(GET_REG_RIP == 3);
    
    SET_FLAG_ZF;
    res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    eax = GET_REG_EAX;
    TEST_ASSERT(eax == 0x0);
    TEST_ASSERT(GET_REG_RIP == 6);

}

void test_processor_sahf()
{
    // sahf
    unsigned char code[] = {0x9E};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_AH(0xFF);
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    TEST_ASSERT(GET_REG_RIP == 1);
}

void test_processor_sal()
{
    // shl         eax,1
    unsigned char code[] = {0xD1, 0xE0, 0xD1, 0xE0};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_EAX(0xBFFFFFFF);
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    TEST_ASSERT(GET_REG_RIP == 2);

    OPERAND_SIZE rax = GET_REG_RAX;
    TEST_ASSERT(rax == 0x7FFFFFFE);
    TEST_ASSERT(GET_FLAG_CF);
    TEST_ASSERT(GET_FLAG_OF);

    res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    rax = GET_REG_RAX;
    TEST_ASSERT(rax == 0xFFFFFFFC);
    TEST_ASSERT(!GET_FLAG_CF);
    TEST_ASSERT(GET_FLAG_OF);
    TEST_ASSERT(GET_FLAG_SF);
    TEST_ASSERT(GET_FLAG_PF);
}

void test_processor_sar()
{
    // sar         eax,1
    unsigned char code[] = {0xD1, 0xF8, 0xD1, 0xF8};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_EAX(0x80000001);
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    TEST_ASSERT(GET_REG_RIP == 2);

    OPERAND_SIZE rax = GET_REG_RAX;
    TEST_ASSERT(rax == 0xC0000000);
    TEST_ASSERT(GET_FLAG_CF);
    TEST_ASSERT(GET_FLAG_SF);

    res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    rax = GET_REG_RAX;
    TEST_ASSERT(rax == 0xE0000000);
    TEST_ASSERT(!GET_FLAG_CF);
    TEST_ASSERT(!GET_FLAG_OF);
    TEST_ASSERT(GET_FLAG_SF);
    TEST_ASSERT(GET_FLAG_PF);
}

void test_processor_sar2()
{
    // sar         eax,5 
    unsigned char code[] = {0xC1, 0xF8, 0x05};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_EAX(0x00000020);
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    TEST_ASSERT(GET_REG_RIP == 3);

    OPERAND_SIZE rax = GET_REG_RAX;
    TEST_ASSERT(rax == 0x1);
}


void test_processor_shld()
{
    // shld        ebx,eax,1 
    unsigned char code[] = {0x0F, 0xA4, 0xC3, 0x01};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_EBX(0x80000001);
    SET_REG_EAX(0xC0000000);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    TEST_ASSERT(GET_REG_RIP == 4);

    OPERAND_SIZE rbx = GET_REG_RBX;
    TEST_ASSERT(rbx == 0x00000003);
    TEST_ASSERT(GET_FLAG_CF);
    TEST_ASSERT(GET_FLAG_OF);

    OPERAND_SIZE rax = GET_REG_RAX;
    TEST_ASSERT(rax == 0xC0000000);
}

void test_processor_shld2()
{
    // shld        ebx,eax,1 
    unsigned char code[] = {0x0F, 0xA4, 0xC3, 0x1f};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_EBX(0x80000001);
    SET_REG_EAX(0xC0000000);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    TEST_ASSERT(GET_REG_RIP == 4);

    OPERAND_SIZE rbx = GET_REG_RBX;
    TEST_ASSERT(rbx == 0xE0000000);
    TEST_ASSERT(!GET_FLAG_CF);
    TEST_ASSERT(!GET_FLAG_OF);
    TEST_ASSERT(GET_FLAG_PF);

    OPERAND_SIZE rax = GET_REG_RAX;
    TEST_ASSERT(rax == 0xC0000000);
}


void test_processor_shr()
{
    // shr         eax,1
    unsigned char code[] = {0xD1, 0xE8};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_EAX(0x80000001);
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    TEST_ASSERT(GET_REG_RIP == 2);

    OPERAND_SIZE rax = GET_REG_RAX;
    TEST_ASSERT(rax == 0x40000000);
    TEST_ASSERT(GET_FLAG_CF);
    TEST_ASSERT(GET_FLAG_OF);
    TEST_ASSERT(GET_FLAG_PF);
}


void test_processor_shrd()
{
    // shrd         ebx, eax,1
    unsigned char code[] = {0x0F, 0xAC, 0xC3, 0x01,
                            0x0F, 0xAC, 0xC3, 0x01,
                            0x0F, 0xAC, 0xC3, 0x01};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_EBX(0x80000001);
    SET_REG_EAX(0xC0000000);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    TEST_ASSERT(GET_REG_RIP == 4);

    OPERAND_SIZE rbx = GET_REG_RBX;
    TEST_ASSERT(rbx == 0x40000000);
    TEST_ASSERT(GET_FLAG_CF);
    TEST_ASSERT(GET_FLAG_OF);

    OPERAND_SIZE rax = GET_REG_RAX;
    TEST_ASSERT(rax == 0xC0000000);
    
    // stage 2
    SET_REG_EBX(0x80000000);
    res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    TEST_ASSERT(GET_REG_RIP == 8);

    rbx = GET_REG_RBX;
    TEST_ASSERT(rbx == 0x40000000);
    TEST_ASSERT(!GET_FLAG_CF);
    TEST_ASSERT(GET_FLAG_OF);

    // stage 3
    SET_REG_EBX(0x40000001);
    res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    TEST_ASSERT(GET_REG_RIP == 0xc);

    rbx = GET_REG_RBX;
    TEST_ASSERT(rbx == 0x20000000);
    TEST_ASSERT(GET_FLAG_CF);
    TEST_ASSERT(GET_FLAG_PF);
    TEST_ASSERT(!GET_FLAG_OF);
}

void test_processor_shrd2()
{
    // shrd         ebx, eax,1
    unsigned char code[] = {0x0F, 0xAC, 0xd0, 0x18};
    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();
    
    SET_REG_EDX(0x000c0585);
    SET_REG_EAX(0x65e00000);
    int res = proc.ExecOnce();
    OPERAND_SIZE rax = GET_REG_RAX;
    TEST_ASSERT(rax == 0x0C058565);
    TEST_ASSERT(GET_FLAG_CF);
}

void test_processor_scas()
{
    // repz scasb   al, byte ptr es:[edi] 
    unsigned char code[] = {0xF2, 0xAE, 1, 2};

    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    proc.GetSelf()->m_flags.value = 0;
    SET_FLAG_IF;
    DianaProcessor_SetFlag(pCallContext,  flag_1 );

    SET_REG_EAX(2);
    SET_REG_ECX(6);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_EDI == 4);
    TEST_ASSERT(GET_REG_ESI == 0);
    TEST_ASSERT(GET_REG_ECX == 2);

    TEST_ASSERT(proc.GetSelf()->m_flags.value == 0x00000246);
}


void test_processor_scas2()
{
    // repe scas   word ptr es:[edi] 
    unsigned char code[] = {0xF3, 0x66, 0xAF, 0, 0, 1, 2};

    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    proc.GetSelf()->m_flags.value = 0;
    SET_FLAG_IF;
    DianaProcessor_SetFlag(pCallContext,  flag_1 );

    SET_REG_EDI(3);
    SET_REG_EAX(0);
    SET_REG_ECX(6);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_EDI == 7);
    TEST_ASSERT(GET_REG_ESI == 0);
    TEST_ASSERT(GET_REG_ECX == 4);

    TEST_ASSERT(proc.GetSelf()->m_flags.value == 0x00000297);
}

void test_processor_stos()
{
    // rep stos 
    unsigned char code[] = {0xF3, 0xAA, 0, 0, 0};
    unsigned char etalon[] = {0xFF, 0xFF, 0xFF, 0xFF, 0};

    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_EAX(0xFF);
    SET_REG_ECX(4);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_ECX == 0);

    TEST_ASSERT(memcmp(code, etalon, sizeof(etalon)) == 0);
}


void test_processor_stos2()
{
    // rep stos 
    unsigned char code[] = {0xF3, 0xAB, 0, 0, 0};
    unsigned char etalon[] = {0x67, 0x45, 0x23, 0x01, 0};

    CTestProcessor proc(code, sizeof(code));
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_EAX(0x01234567);
    SET_REG_ECX(1);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);
    
    TEST_ASSERT(GET_REG_ECX == 0);

    TEST_ASSERT(memcmp(code, etalon, sizeof(etalon)) == 0);
}


void test_processor_s()
{
    test_processor_set();
    test_processor_sahf();
    test_processor_sal();
    test_processor_sar();
    test_processor_sar2();
    test_processor_shld();
    test_processor_shld2();
    test_processor_shr();
    test_processor_shrd2();
    test_processor_shrd();
    test_processor_scas();
    test_processor_scas2();
    test_processor_stos();
    test_processor_stos2();
}