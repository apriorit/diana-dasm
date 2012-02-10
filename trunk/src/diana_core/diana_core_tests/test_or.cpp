#include "test_or.h"
extern "C"
{
#include "diana_streams.h"
#include "diana_gen.h"
}

#include "test_common.h"
#include "string.h"


static unsigned char or[]= {0x0C, 1};  // AL,imm8        2         OR immediate byte to AL
static unsigned char or1[]= {0x0D, 1,0,0,0};  // EAX,imm32      2         OR immediate dword to EAX
static unsigned char or2[]= {0x80, 0x08, 0x01};  // r/m8,imm8      2/7       OR immediate byte to r/m byte


void test_or()
{
    DianaGroupInfo * pGroupInfo=0;
    DianaParserResult result;
    size_t read;
    //static unsigned char or[]= {0x0C, 1};  // AL,imm8        2         OR immediate byte to AL
    int 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,or, sizeof(or), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "or")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_AL);
        TEST_ASSERT(result.linkedOperands[1].type == diana_imm);
        TEST_ASSERT(result.linkedOperands[1].value.imm == 1);
    }
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,or1, sizeof(or1), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "or")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EAX);
        TEST_ASSERT(result.linkedOperands[1].type == diana_imm);
        TEST_ASSERT(result.linkedOperands[1].value.imm == 1);
    }
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,or2, sizeof(or2), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "or")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_index);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.reg == reg_EAX);
        TEST_ASSERT(result.linkedOperands[1].type == diana_imm);
        TEST_ASSERT(result.linkedOperands[1].value.imm == 1);
    }
    
}