#include "test_add.h"
extern "C"
{
#include "diana_streams.h"
#include "diana_gen.h"
}

#include "test_common.h"
#include "string.h"

static unsigned char add[] = {0x83, 0xC0, 0x08}; //          add         eax,8 
static unsigned char add1[] = {0x83, 0xC4, 0x10};//          add         esp,10h 
static unsigned char code33[] = {0x2e, 0x67, 0xf0, 0x48, 0x81, 0x84, 0x80, 0x23, 0xdf, 0x06, 0x7e, 0x89, 0xab, 0xcd, 0xef};

int test_add()
{
    DianaGroupInfo * pGroupInfo=0;
    DianaParserResult result;
    size_t read;
    int 

    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,add, sizeof(add), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "add")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EAX);
        TEST_ASSERT(result.linkedOperands[1].type == diana_imm);
        TEST_ASSERT(result.linkedOperands[1].value.imm == 8);
    }
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,add1, sizeof(add1), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "add")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_ESP);
        TEST_ASSERT(result.linkedOperands[1].type == diana_imm);
        TEST_ASSERT(result.linkedOperands[1].value.imm == 0x10);
    }

    // lock add qword cs:[eax + 4 * eax + 07e06df23h], 0efcdab89h
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE64,code33, sizeof(code33), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "add")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_index);
        TEST_ASSERT(result.linkedOperands[0].usedSize == 8);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.seg_reg == reg_CS);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.reg == reg_EAX);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.indexed_reg == reg_EAX);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.index == 4);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispSize == 4);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispValue== 0x07e06df23);

        TEST_ASSERT(result.linkedOperands[1].type == diana_imm);
        TEST_ASSERT(result.linkedOperands[1].usedSize == 4);
        TEST_ASSERT(result.linkedOperands[1].value.imm == 0x0efcdab89);
    }
    return 0;
}