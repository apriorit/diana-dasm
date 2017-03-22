#include "test_new.h"
extern "C"
{
#include "diana_streams.h"
#include "diana_gen.h"
}

#include "test_common.h"
#include "string.h"

void test_sal1()
{
    DianaGroupInfo * pGroupInfo=0;
    DianaParserResult result;
    size_t read;
    
    int iRes = 0;

    static unsigned char mov0[] = {0xd1, 0xe6}; //    shl esi,1
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, mov0, sizeof(mov0), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "shl")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
    }

}

static void test_r32_64()
{
    DianaGroupInfo * pGroupInfo=0;
    DianaParserResult result;
    size_t read;
    
    int iRes = 0;

    static unsigned char bswap0[] = {0x0F, 0xC9}; //    bswap ecx
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, bswap0, sizeof(bswap0), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "bswap")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].usedSize == 4);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_ECX);
    }
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE64, bswap0, sizeof(bswap0), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount ==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "bswap")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].usedSize == 4);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_ECX);
    }
    
    static unsigned char bswap1[] = {0x48, 0x0f, 0xc9}; //    bswap rcx
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE64, bswap1, sizeof(bswap1), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "bswap")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].usedSize == 8);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_RCX);
    }
    
    static unsigned char cvttsd2si[] = {0xf2, 0x48, 0x0f, 0x2c, 0}; //    cvtsd2si rdx,dword ptr [rax]
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE64, cvttsd2si, sizeof(cvttsd2si), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "cvttsd2si")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].usedSize == 8);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_RAX);
        TEST_ASSERT(result.linkedOperands[1].type == diana_index);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.seg_reg == reg_DS);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_RAX);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_none);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 0x00);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0x0);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == 0x0);
    }

    static unsigned char cvttsd2si2[] = {0xf2, 0x0f, 0x2c, 0}; //    cvttsd2si eax, QWORD ptr [rax]
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE64, cvttsd2si2, sizeof(cvttsd2si2), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "cvttsd2si")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].usedSize == 4);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EAX);
        TEST_ASSERT(result.linkedOperands[1].type == diana_index);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.seg_reg == reg_DS);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_RAX);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_none);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 0);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == 0);
    }
    
}

void test_nop_pause()
{
    DianaGroupInfo * pGroupInfo=0;
    DianaParserResult result;
    size_t read;
    
    int iRes = 0;

    static unsigned char pause[] = {0xF3, 0x90}; //    PAUSE 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, pause, sizeof(pause), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==0);
        TEST_ASSERT(result.pInfo->m_operandCount==0);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "pause")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
    }

    static unsigned char nop[] = {0x90}; //    nop
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, nop, sizeof(nop), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==0);
        TEST_ASSERT(result.pInfo->m_operandCount==0);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "nop")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
    }
}

void test_nop()
{
	DianaParserResult result;
	size_t read;
	int iRes = 0;

	static unsigned char nop[] = {0x66, 0x90}; // NOP
	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, nop, sizeof(nop), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==2);
		TEST_ASSERT(result.iLinkedOpCount==0);
		TEST_ASSERT(result.pInfo->m_operandCount==0);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "nop")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
	}

	static unsigned char nop2[] = {0x66, 0x90}; // NOP
	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, nop2, sizeof(nop2), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==2);
		TEST_ASSERT(result.iLinkedOpCount==0);
		TEST_ASSERT(result.pInfo->m_operandCount==0);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "nop")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
	}
}

void test_nops()
{
	// this form GAS, the GNU Assembler
	// nopl (%[re]ax)
	static unsigned char alt_3[] = {0x0f,0x1f,0x00};
	// nopl 0(%[re]ax)
	static unsigned char alt_4[] = {0x0f,0x1f,0x40,0x00};
	// nopl 0(%[re]ax,%[re]ax,1)
	static unsigned char alt_5[] = {0x0f,0x1f,0x44,0x00,0x00};
	// nopw 0(%[re]ax,%[re]ax,1)
	static unsigned char alt_6[] = {0x66,0x0f,0x1f,0x44,0x00,0x00};
	// nopl 0L(%[re]ax)
	static unsigned char alt_7[] = {0x0f,0x1f,0x80,0x00,0x00,0x00,0x00};
	// nopl 0L(%[re]ax,%[re]ax,1)
	static unsigned char alt_8[] = {0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00};
	// nopw 0L(%[re]ax,%[re]ax,1)
	static unsigned char alt_9[] = {0x66,0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00};
	// nopw %cs:0L(%[re]ax,%[re]ax,1)
	static unsigned char alt_10[] = {0x66,0x2e,0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00};
	// nopw %cs:0L(%[re]ax,%[re]ax,1)
	static unsigned char alt_long_11[] = {0x66,0x66,0x2e,0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00};
	// nopw %cs:0L(%[re]ax,%[re]ax,1)
	static unsigned char alt_long_12[] = {0x66,0x66,0x66,0x2e,0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00};
	// nopw %cs:0L(%[re]ax,%[re]ax,1)
	static unsigned char alt_long_13[] = {0x66,0x66,0x66,0x66,0x2e,0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00};
	// nopw %cs:0L(%[re]ax,%[re]ax,1)
	static unsigned char alt_long_14[] = {0x66,0x66,0x66,0x66,0x66,0x2e,0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00};
	// nopw %cs:0L(%[re]ax,%[re]ax,1)
	static unsigned char alt_long_15[] = {0x66,0x66,0x66,0x66,0x66,0x66,0x2e,0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00};
	// nopl 0(%[re]ax,%[re]ax,1)
	static unsigned char alt_short_11[] = {0x0f,0x1f,0x44,0x00,0x00};
	// nopl 0L(%[re]ax)
	static unsigned char alt_short_14[] = {0x0f,0x1f,0x80,0x00,0x00,0x00,0x00};

	DianaParserResult result;
	size_t read;
	int iRes = 0;

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, alt_3, sizeof(alt_3), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==3);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, alt_4, sizeof(alt_4), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==4);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, alt_5, sizeof(alt_5), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==5);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, alt_6, sizeof(alt_6), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==6);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, alt_7, sizeof(alt_7), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==7);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, alt_8, sizeof(alt_8), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==8);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, alt_9, sizeof(alt_9), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==9);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, alt_10, sizeof(alt_10), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==10);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, alt_long_11, sizeof(alt_long_11), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==11);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, alt_long_12, sizeof(alt_long_12), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==12);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, alt_long_13, sizeof(alt_long_13), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==13);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, alt_long_14, sizeof(alt_long_14), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==14);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, alt_long_15, sizeof(alt_long_15), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==15);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, alt_short_11, sizeof(alt_short_11), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==5);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, alt_short_14, sizeof(alt_short_14), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==7);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	// this form Intel manual
	static unsigned char intel1[] = {0x0f,0x1f,0x00};
	static unsigned char intel2[] = {0x0f,0x1f,0x40,0x00};
	static unsigned char intel3[] = {0x0f,0x1f,0x44,0x00,0x00};
	static unsigned char intel4[] = {0x66,0x0f,0x1f,0x44,0x00,0x00};
	static unsigned char intel5[] = {0x0f,0x1f,0x80,0x00,0x00,0x00,0x00};
	static unsigned char intel6[] = {0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00};
	static unsigned char intel7[] = {0x66,0x0f,0x1f,0x84,0x00,0x00,0x00,0x00,0x00};

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, intel1, sizeof(intel1), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==3);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, intel2, sizeof(intel2), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==4);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, intel3, sizeof(intel3), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==5);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, intel4, sizeof(intel4), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==6);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, intel5, sizeof(intel5), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==7);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, intel6, sizeof(intel6), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==8);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, intel7, sizeof(intel7), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==9);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "hint_nop")==0);
	}

}

void test_ret()
{
	DianaParserResult result;
	size_t read;
	int iRes = 0;

	// ret 0
	unsigned char code[] = {0xC2, 0x00, 0x00};
	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, code, sizeof(code), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==3);
		TEST_ASSERT(result.iLinkedOpCount==1);
		TEST_ASSERT(strcmp(result.pInfo->m_pGroupInfo->m_pName, "ret")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
	}

}

void test_nop_toobig()
{
	DianaParserResult result;
	size_t read;
	int iRes = 0;

	static unsigned char nop[] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x90}; // NOP
	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, nop, sizeof(nop), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==15);
		TEST_ASSERT(result.pInfo->m_pGroupInfo->m_commandId==diana_cmd_nop);
	}
	static unsigned char nop2[] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x90}; // NOP
	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, nop2, sizeof(nop2), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT(iRes)
}

void test_new()
{
    DIANA_TEST(test_nop_pause());
    DIANA_TEST(test_nop());
    DIANA_TEST(test_nops());
    DIANA_TEST(test_r32_64());
    DIANA_TEST(test_sal1());
    DIANA_TEST(test_ret());
    DIANA_TEST(test_nop_toobig());
}