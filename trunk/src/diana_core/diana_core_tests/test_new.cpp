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
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "shl")==0);
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
        TEST_ASSERT(result.pInfo->m_operandCount ==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "bswap")==0);
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
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].usedSize == 4);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_ECX);
    }

    
    static unsigned char bswap1[] = {0x48, 0x0f, 0xc9}; //    bswap rcx
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE64, bswap1, sizeof(bswap1), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount ==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "bswap")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].usedSize == 8);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_RCX);
    }
    
    static unsigned char cvttsd2si[] = {0xf2, 0x48, 0x0f, 0x2c, 0}; //    cvtsd2si rdx,dword ptr [rax]
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE64, cvttsd2si, sizeof(cvttsd2si), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "cvttsd2si")==0);
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
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "cvttsd2si")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].usedSize == 4);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EAX);

        TEST_ASSERT(result.linkedOperands[1].type == diana_index);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.seg_reg == reg_DS);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_RAX);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_none);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 0x00);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0x0);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == 0x0);
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
        TEST_ASSERT(result.pInfo->m_operandCount ==0);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "pause")==0);
    }

    static unsigned char nop[] = {0x90}; //    nop
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, nop, sizeof(nop), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==0);
        TEST_ASSERT(result.pInfo->m_operandCount ==0);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "nop")==0);
    }
}

void test_nop()
{
	DianaGroupInfo * pGroupInfo=0;
	DianaParserResult result;
	size_t read;

	int iRes = 0;
	static unsigned char nop[] = {0x90}; // NOP
	iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, nop, sizeof(nop), Diana_GetRootLine(), &result, &read);
	TEST_ASSERT_IF(!iRes)
	{
		TEST_ASSERT(result.iFullCmdSize==1);
		TEST_ASSERT(result.iLinkedOpCount==0);
		TEST_ASSERT(result.pInfo->m_operandCount==0);
		TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
		TEST_ASSERT(strcmp(pGroupInfo->m_pName, "nop")==0);
	}

	//static unsigned char nop2[] = {0x66,0x90}; // NOP
}

void test_new()
{
    test_nop_pause();
	test_nop();
    test_r32_64();
    test_sal1();
}