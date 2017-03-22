#include "test_processor_m_xmm.h"
#include "test_common.h"
#include "test_processor_impl.h"

static void test_processor_movups()
{
    // movups xmm1,xmm0
    unsigned char code[] = {0x0f, 0x10, 0xc8};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE32);
    DianaProcessor * pCallContext = proc.GetSelf();

    DianaRegisterXMM_type xmm0 = {0};
    xmm0.u64[0] = 0x0102030405060708ULL;
    xmm0.u64[1] = 0x090a0b0c0d0e0f10ULL;
    SET_REG_XMM0(xmm0);
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    DianaRegisterXMM_type xmm1 = GET_REG_XMM1;
    TEST_ASSERT(0x0102030405060708ULL == xmm0.u64[0]);
    TEST_ASSERT(0x090a0b0c0d0e0f10ULL == xmm0.u64[1]);
    TEST_ASSERT(0x0102030405060708ULL == xmm1.u64[0]);
    TEST_ASSERT(0x090a0b0c0d0e0f10ULL == xmm1.u64[1]);
}

static void test_processor_movups_2()
{
    // movups [eax],xmm0
    unsigned char code[] = {0x0f, 0x11, 0x40, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE32);
    DianaProcessor * pCallContext = proc.GetSelf();

    DianaRegisterXMM_type xmm0 = {0};
    xmm0.u64[0] = 0x0102030405060708ULL;
    xmm0.u64[1] = 0x090a0b0c0d0e0f10ULL;
    SET_REG_XMM0(xmm0);
    SET_REG_RAX(4);
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    TEST_ASSERT(0 == memcmp(code + 4, xmm0.u8, 16));
}

static void test_processor_movups_3()
{
    // movups xmm0,[eax]
    unsigned char code[] = {0x0f, 0x10, 0x40, 0x00,
                            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                            0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE32);
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_RAX(4);
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    DianaRegisterXMM_type xmm0 = GET_REG_XMM0;
    TEST_ASSERT(0 == memcmp(xmm0.u8, code + 4, 16));
}

static void test_processor_movaps()
{
    // movaps xmm1,xmm0
    unsigned char code[] = {0x0f, 0x28, 0xc8};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE32);
    DianaProcessor * pCallContext = proc.GetSelf();

    DianaRegisterXMM_type xmm0 = {0};
    xmm0.u64[0] = 0x0102030405060708ULL;
    xmm0.u64[1] = 0x090a0b0c0d0e0f10ULL;
    SET_REG_XMM0( xmm0 );
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    DianaRegisterXMM_type xmm1 = GET_REG_XMM1;
    TEST_ASSERT(0 == memcmp(xmm1.u8, xmm0.u8, 16));
}

static void test_processor_movaps_2()
{
    // movaps [eax],xmm0
    unsigned char code[] = {0x0f, 0x29, 0x40, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE32);
    DianaProcessor * pCallContext = proc.GetSelf();

    DianaRegisterXMM_type xmm0 = {0};
    xmm0.u64[0] = 0x0102030405060708ULL;
    xmm0.u64[1] = 0x090a0b0c0d0e0f10ULL;
    SET_REG_XMM0(xmm0);
    SET_REG_RAX(4);
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_GP);

    TEST_ASSERT(0 != memcmp(code + 4, xmm0.u8, 16));
}

static void test_processor_movaps_3()
{
    // movaps xmm0,[eax]
    unsigned char code[] = {0x0f, 0x28, 0x40, 0x00,
                            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                            0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE32);
    DianaProcessor * pCallContext = proc.GetSelf();

    SET_REG_RAX(4);
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_GP);
}

static void test_processor_movss()
{
    // movss xmm0,xmm1
    unsigned char code[] = {0xf3, 0x0f, 0x10, 0xc1};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE32);
    DianaProcessor * pCallContext = proc.GetSelf();

    DianaRegisterXMM_type xmm0 = {0};
    xmm0.u64[0] = 0x1111111111111111ULL;
    xmm0.u64[1] = 0x1111111111111111ULL;
    SET_REG_XMM0(xmm0);
    DianaRegisterXMM_type xmm1 = {0};
    xmm1.u64[0] = 0x2222222222222222ULL;
    xmm1.u64[1] = 0x2222222222222222ULL;
    SET_REG_XMM1(xmm1);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    xmm0 = GET_REG_XMM0;
    TEST_ASSERT(0x1111111122222222ULL == xmm0.u64[0]);
    TEST_ASSERT(0x1111111111111111ULL == xmm0.u64[1]);
}

static void test_processor_movss_2()
{
    // movss xmm0,[8] ; rip
    unsigned char code[] = {0xf3, 0x0f, 0x10, 0x05, 0x00, 0x00, 0x00, 0x00,
                            0x11, 0x22, 0x33, 0x44, 0x22, 0x22, 0x22, 0x22,
                            0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
    DianaProcessor * pCallContext = proc.GetSelf();

    DianaRegisterXMM_type xmm0 = {0};
    xmm0.u64[0] = 0x1111111111111111ULL;
    xmm0.u64[1] = 0x1111111111111111ULL;
    SET_REG_XMM0(xmm0);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    xmm0 = GET_REG_XMM0;
    TEST_ASSERT(0x0000000044332211ULL == xmm0.u64[0]);
    TEST_ASSERT(0x0000000000000000ULL == xmm0.u64[1]);
}

static void test_processor_movss_3()
{
    // movss [8],xmm0 ; rip
    unsigned char code[] = {0xf3, 0x0f, 0x11, 0x05, 0x00, 0x00, 0x00, 0x00,
                            0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
                            0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
    DianaProcessor * pCallContext = proc.GetSelf();

    DianaRegisterXMM_type xmm0 = {0};
    xmm0.u64[0] = 0x2222222222222222ULL;
    xmm0.u64[1] = 0x3333333333333333ULL;
    SET_REG_XMM0(xmm0);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    TEST_ASSERT(0x22 == code[8]);
    TEST_ASSERT(0x22 == code[9]);
    TEST_ASSERT(0x22 == code[10]);
    TEST_ASSERT(0x22 == code[11]);
    TEST_ASSERT(0x11 == code[12]);
}

static void test_processor_movlps()
{
    // movlps xmm0,[7] ; rip
    unsigned char code[] = {0x0f, 0x12, 0x05, 0x00, 0x00, 0x00, 0x00,
                            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                            0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
    DianaProcessor * pCallContext = proc.GetSelf();

    DianaRegisterXMM_type xmm0 = {0};
    xmm0.u64[0] = 0x2222222222222222ULL;
    xmm0.u64[1] = 0x3333333333333333ULL;
    SET_REG_XMM0(xmm0);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    xmm0 = GET_REG_XMM0;
    TEST_ASSERT(0x0807060504030201ULL == xmm0.u64[0]);
    TEST_ASSERT(0x3333333333333333ULL == xmm0.u64[1]);
}

static void test_processor_movlps_2()
{
    // movlps [7],xmm0 ; rip
    unsigned char code[] = {0x0f, 0x13, 0x05, 0x00, 0x00, 0x00, 0x00,
                            0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
                            0x33, 0x44, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
    DianaProcessor * pCallContext = proc.GetSelf();

    DianaRegisterXMM_type xmm0 = {0};
    xmm0.u64[0] = 0x0102030405060708ULL;
    xmm0.u64[1] = 0x2233445566778899ULL;
    SET_REG_XMM0(xmm0);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    TEST_ASSERT(0x08 == code[7]);
    TEST_ASSERT(0x07 == code[8]);
    TEST_ASSERT(0x06 == code[9]);
    TEST_ASSERT(0x05 == code[10]);
    TEST_ASSERT(0x04 == code[11]);
    TEST_ASSERT(0x03 == code[12]);
    TEST_ASSERT(0x02 == code[13]);
    TEST_ASSERT(0x01 == code[14]);
    TEST_ASSERT(0x33 == code[15]);
    TEST_ASSERT(0x44 == code[16]);
}

static void test_processor_movsd()
{
    // movsd xmm1,xmm0
    unsigned char code[] = {0xf2, 0x0f, 0x10, 0xc8};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE32);
    DianaProcessor * pCallContext = proc.GetSelf();

    DianaRegisterXMM_type xmm0 = {0};
    xmm0.u64[0] = 0x1111111111111111ULL;
    xmm0.u64[1] = 0x1111111111111111ULL;
    SET_REG_XMM0(xmm0);
    DianaRegisterXMM_type xmm1 = {0};
    xmm1.u64[0] = 0x2222222222222222ULL;
    xmm1.u64[1] = 0x2222222222222222ULL;
    SET_REG_XMM1(xmm1);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    xmm1 = GET_REG_XMM1;
    TEST_ASSERT(0x1111111111111111ULL == xmm0.u64[0]);
    TEST_ASSERT(0x1111111111111111ULL == xmm0.u64[1]);
    TEST_ASSERT(0x1111111111111111ULL == xmm1.u64[0]);
    TEST_ASSERT(0x2222222222222222ULL == xmm1.u64[1]);
}

static void test_processor_movsd_2()
{
    // movsd xmm0,[8] ;rip
    unsigned char code[] = {0xF2, 0x0F, 0x10, 0x05, 0x00, 0x00, 0x00, 0x00,
                            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                            0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
    DianaProcessor * pCallContext = proc.GetSelf();

    DianaRegisterXMM_type xmm0 = {0};
    xmm0.u64[0] = 0x1111111111111111ULL;
    xmm0.u64[1] = 0x1111111111111111ULL;
    SET_REG_XMM0(xmm0);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    xmm0 = GET_REG_XMM0;
    TEST_ASSERT(0x2726252423222120ULL == xmm0.u64[0]);
    TEST_ASSERT(0x0000000000000000ULL == xmm0.u64[1]);
}

static void test_processor_movsd_3()
{
    // movsd [8],xmm0 ;rip
    unsigned char code[] = {0xF2, 0x0F, 0x11, 0x05, 0x00, 0x00, 0x00, 0x00,
                            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                            0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
    DianaProcessor * pCallContext = proc.GetSelf();

    DianaRegisterXMM_type xmm0 = {0};
    xmm0.u64[0] = 0x1111111111111111ULL;
    xmm0.u64[1] = 0x1111111111111111ULL;
    SET_REG_XMM0(xmm0);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    TEST_ASSERT(0x11 == code[8]);
    TEST_ASSERT(0x11 == code[9]);
    TEST_ASSERT(0x11 == code[10]);
    TEST_ASSERT(0x11 == code[11]);
    TEST_ASSERT(0x11 == code[12]);
    TEST_ASSERT(0x11 == code[13]);
    TEST_ASSERT(0x11 == code[14]);
    TEST_ASSERT(0x11 == code[15]);
    TEST_ASSERT(0x28 == code[16]);
}

static void test_processor_movddup()
{
    // movddup xmm1,xmm0
    unsigned char code[] = {0xf2, 0x0f, 0x12, 0xc8};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE32);
    DianaProcessor * pCallContext = proc.GetSelf();

    DianaRegisterXMM_type xmm0 = {0};
    xmm0.u64[0] = 0x0123456789abcdefULL;
    xmm0.u64[1] = 0xfedcba9876543210ULL;
    SET_REG_XMM0(xmm0);

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    xmm0 = GET_REG_XMM0;
    TEST_ASSERT(0x0123456789abcdefULL == xmm0.u64[0]);
    TEST_ASSERT(0xfedcba9876543210ULL == xmm0.u64[1]);
    DianaRegisterXMM_type xmm1 = GET_REG_XMM1;
    TEST_ASSERT(0x0123456789abcdefULL == xmm1.u64[0]);
    TEST_ASSERT(0x0123456789abcdefULL == xmm1.u64[1]);
}

static void test_processor_movddup_2()
{
    // movddup xmm0,[8] ; rip
    unsigned char code[] = {0xf2, 0x0f, 0x12, 0x05, 0x00, 0x00, 0x00, 0x00,
                            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
    DianaProcessor * pCallContext = proc.GetSelf();

    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    DianaRegisterXMM_type xmm0 = GET_REG_XMM0;
    TEST_ASSERT(0x0807060504030201ULL == xmm0.u64[0]);
    TEST_ASSERT(0x0807060504030201ULL == xmm0.u64[1]);
}


static void test_processor_pxor()
{
    // pxor    xmm0,xmm0
    unsigned char code[] = {0x66, 0x0f, 0xef, 0xc0};

    CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE32);
    DianaProcessor * pCallContext = proc.GetSelf();

    DianaRegisterXMM_type xmm0 = {0};
    xmm0.u64[0] = 0x1111111111111111ULL;
    xmm0.u64[1] = 0x1111111111111111ULL;

    SET_REG_XMM0(xmm0);
    int res = proc.ExecOnce();
    TEST_ASSERT(res == DI_SUCCESS);

    xmm0 = GET_REG_XMM0;
    TEST_ASSERT(xmm0.u64[0] == 0);
    TEST_ASSERT(xmm0.u64[1] == 0);
}

void test_processor_m_xmm()
{
    DIANA_TEST(test_processor_movups());
    DIANA_TEST(test_processor_movups_2());
    DIANA_TEST(test_processor_movups_3());

    DIANA_TEST(test_processor_movaps());
    DIANA_TEST(test_processor_movaps_2());
    DIANA_TEST(test_processor_movaps_3());

    DIANA_TEST(test_processor_movss());
    DIANA_TEST(test_processor_movss_2());
    DIANA_TEST(test_processor_movss_3());

    DIANA_TEST(test_processor_movlps());
    DIANA_TEST(test_processor_movlps_2());

    DIANA_TEST(test_processor_movsd());
    DIANA_TEST(test_processor_movsd_2());
    DIANA_TEST(test_processor_movsd_3());

    DIANA_TEST(test_processor_movddup());
    DIANA_TEST(test_processor_movddup_2());

    DIANA_TEST(test_processor_pxor());
}
