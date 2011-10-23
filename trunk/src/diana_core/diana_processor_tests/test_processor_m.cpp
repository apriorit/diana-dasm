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


void test_processor_mov()
{
	unsigned char code[] = {0xb8, 0x55, 0x55, 0x55, 0x55}; 
	// mov eax,-1

	CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE32);
	DianaProcessor * pCallContext = proc.GetSelf();

	SET_REG_RAX(0xFFFFFFFFFFFFFFFFULL);
	int res = proc.ExecOnce();
	TEST_ASSERT(res == DI_SUCCESS);

	TEST_ASSERT(GET_REG_RAX == 0xFFFFFFFF55555555ULL);
}


static void test_processor_mul64()
{
	unsigned char code[] = {0x48, 0xf7, 0xe1};
	// mul rcx

	CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
	DianaProcessor * pCallContext = proc.GetSelf();

	SET_REG_RDX(0);
	SET_REG_RAX(0x1234001d1234001dULL);
	SET_REG_RCX(45);

	int res = proc.ExecOnce();
	TEST_ASSERT(res == DI_SUCCESS);

	TEST_ASSERT(GET_REG_RAX == 0x3324051C33240519ULL);
	TEST_ASSERT(GET_REG_RDX == 0x0000000000000003ULL);
}


static void test_processor_mul64_2()
{
	unsigned char code[] = {0x48, 0xf7, 0xe3};
	// mul rbx

	CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
	DianaProcessor * pCallContext = proc.GetSelf();

	SET_REG_RDX(0);
	SET_REG_RAX(23);
	SET_REG_RBX(-45);

	int res = proc.ExecOnce();
	TEST_ASSERT(res == DI_SUCCESS);

	TEST_ASSERT(GET_REG_RAX == 0xFFFFFFFFFFFFFBF5ULL);
	TEST_ASSERT(GET_REG_RDX == 0x0000000000000016ULL);
}


static void test_processor_mul64_3()
{
	unsigned char code[] = {0x48, 0xf7, 0xe6};
	// mul rsi

	CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
	DianaProcessor * pCallContext = proc.GetSelf();

	SET_REG_RDX(0);
	SET_REG_RAX(0x8000000000000000ULL);
	SET_REG_RSI(0x8000000000000000ULL);

	int res = proc.ExecOnce();
	TEST_ASSERT(res == DI_SUCCESS);

	TEST_ASSERT(GET_REG_RAX == 0x0000000000000000ULL);
	TEST_ASSERT(GET_REG_RDX == 0x4000000000000000ULL);
}


static void test_processor_mul64_4()
{
	unsigned char code[] = {0x49, 0xf7, 0xe6};
	// mul r14

	CTestProcessor proc(code, sizeof(code), 0, DIANA_MODE64);
	DianaProcessor * pCallContext = proc.GetSelf();

	SET_REG_RDX(0);
	SET_REG_RAX(0x100000000);
	SET_REG_R14(0x100000000);

	int res = proc.ExecOnce();
	TEST_ASSERT(res == DI_SUCCESS);

	TEST_ASSERT(GET_REG_RAX == 0x0000000000000000ULL);
	TEST_ASSERT(GET_REG_RDX == 0x0000000000000001ULL);
}

/*static void test_processor_mul_byte( int m = DIANA_MODE32 )
{
	int i;
	unsigned char code[] = {0xf6, 0x00};
	unsigned char second[] = {0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7};
	DianaUnifiedRegister regs[] = { reg_AL, reg_CL, reg_DL, reg_BL, reg_AH, reg_CH, reg_DH, reg_BH };

	for( i = 0; i < sizeof( second ); i++) {
		DI_CHAR n1 = rand();
		DI_CHAR n2;
		if( i == 0 )
			n2 = n1;
		else
			n2 = rand();
		DI_UINT16 n1n2 = n1 * n2;
		code[ 1 ] = second[ i ];
		CTestProcessor proc( code, sizeof( code ), 0, m );
		DianaProcessor * pCallContext = proc.GetSelf();
		SET_REG_RAX( 0 );
		DianaProcessor_SetValue(pCallContext, reg_AL, DianaProcessor_QueryReg(pCallContext, reg_AL ), n1 );
		DianaProcessor_SetValue(pCallContext, regs[ i ], DianaProcessor_QueryReg(pCallContext, regs[ i ] ), n2 );

		int res = proc.ExecOnce();
		TEST_ASSERT(res == DI_SUCCESS);

		TEST_ASSERT( n1n2 == ( DI_UINT16 )GET_REG_RAX );
	}
}

static void test_processor_mul_word( int m = DIANA_MODE32 )
{
	int i;
	unsigned char code[] = {0x66, 0xf7, 0x00};
	unsigned char second[] = {0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7};
	DianaUnifiedRegister regs[] = { reg_AX, reg_CX, reg_DX, reg_BX, reg_SP, reg_BP, reg_SI, reg_DI };

	for( i = 0; i < sizeof( second ); i++) {
		DI_UINT16 n1 = rand();
		DI_UINT16 n2;
		if( i == 0 )
			n2 = n1;
		else
			n2 = rand();
		DI_UINT32 n1n2 = n1 * n2;
		code[ 2 ] = second[ i ];
		CTestProcessor proc( code, sizeof( code ), 0, m );
		DianaProcessor * pCallContext = proc.GetSelf();
		SET_REG_RAX( 0 );
		SET_REG_RDX( 0 );
		DianaProcessor_SetValue(pCallContext, reg_AX, DianaProcessor_QueryReg(pCallContext, reg_AX ), n1 );
		DianaProcessor_SetValue(pCallContext, regs[ i ], DianaProcessor_QueryReg(pCallContext, regs[ i ] ), n2 );

		int res = proc.ExecOnce();
		TEST_ASSERT(res == DI_SUCCESS);

		TEST_ASSERT( ( n1n2 & 0xFFFF ) == ( DI_UINT16 )GET_REG_RAX );
		TEST_ASSERT( ( n1n2 & 0xFFFF0000 ) >> 16 == ( DI_UINT16 )GET_REG_RDX );
	}
}

static void test_processor_mul_dword( int m = DIANA_MODE32 )
{
	int i;
	unsigned char code[] = {0xf7, 0x00};
	unsigned char second[] = {0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7};
	DianaUnifiedRegister regs[] = { reg_EAX, reg_ECX, reg_EDX, reg_EBX, reg_ESP, reg_EBP, reg_ESI, reg_EDI };

	for( i = 0; i < sizeof( second ); i++) {
		DI_UINT32 n1 = rand();
		DI_UINT32 n2;
		n1 = ( n1 << 16 ) | n1;
		if( i == 0 )
			n2 = n1;
		else {
			n2 = rand();
			n2 = ( n2 << 16 ) | n2;
		}
		DI_UINT64 n1n2 = ( DI_UINT64 )n1 * ( DI_UINT64 )n2;
		code[ 1 ] = second[ i ];
		CTestProcessor proc( code, sizeof( code ), 0, m );
		DianaProcessor * pCallContext = proc.GetSelf();
		SET_REG_RAX( 0 );
		SET_REG_RDX( 0 );
		DianaProcessor_SetValue(pCallContext, reg_EAX, DianaProcessor_QueryReg(pCallContext, reg_EAX ), n1 );
		DianaProcessor_SetValue(pCallContext, regs[ i ], DianaProcessor_QueryReg(pCallContext, regs[ i ] ), n2 );

		int res = proc.ExecOnce();
		TEST_ASSERT(res == DI_SUCCESS);

		TEST_ASSERT( ( n1n2 & 0xFFFFFFFF ) == ( DI_UINT32 )GET_REG_RAX );
		TEST_ASSERT( ( n1n2 & 0xFFFFFFFF00000000 ) >> 32 == ( DI_UINT32 )GET_REG_RDX );
	}
}*/

void test_processor_m()
{
    test_processor_movs();
    test_processor_movsxd64();
    test_processor_movsxd64_2();
    test_processor_mov64();
    test_processor_mov64_2();
    test_processor_mov64_3();
	test_processor_mov64_4();
	test_processor_mov();

	test_processor_mul64();
	test_processor_mul64_2();
	test_processor_mul64_3();
	test_processor_mul64_4();

	/*test_processor_mul_byte();
	test_processor_mul_byte( DIANA_MODE64 );
	test_processor_mul_word();
	test_processor_mul_word( DIANA_MODE64 );
	test_processor_mul_dword();
	test_processor_mul_dword( DIANA_MODE64 );*/
}