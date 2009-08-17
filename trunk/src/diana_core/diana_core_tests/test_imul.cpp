#include "test_imul.h"
extern "C"
{
#include "diana_streams.h"
#include "diana_gen.h"
}

#include "test_common.h"
#include "string.h"

unsigned char imul[] = {0x0F, 0xAF, 0xFE};                         // imul        edi,esi 
unsigned char imul1[] = {0x6B, 0x7D, 0xF8, 0x0F};                  // imul        edi,dword ptr [c],0Fh 
unsigned char imul2[] = {0x69, 0x7D, 0xF8, 0x78, 0x56, 0x34, 0x12};// imul        edi,dword ptr [c],12345678h 

int test_imul()
{
    DianaGroupInfo * pGroupInfo=0;
    DianaParserResult result;
    size_t read;
    int 

    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,imul, sizeof(imul), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "imul")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EDI);
        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_ESI);
    }
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,imul1, sizeof(imul1), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==3);
        TEST_ASSERT(result.pInfo->m_operandCount ==3);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "imul")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EDI);
        TEST_ASSERT(result.linkedOperands[1].type == diana_index);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_EBP);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_none);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 1);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == -8);
        TEST_ASSERT(result.linkedOperands[2].type == diana_imm);
        TEST_ASSERT(result.linkedOperands[2].value.imm == 0xF);
    }
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,imul2, sizeof(imul2), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==3);
        TEST_ASSERT(result.pInfo->m_operandCount ==3);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "imul")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EDI);
        TEST_ASSERT(result.linkedOperands[1].type == diana_index);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_EBP);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_none);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 1);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == -8);
        TEST_ASSERT(result.linkedOperands[2].type == diana_imm);
        TEST_ASSERT(result.linkedOperands[2].value.imm == 0x12345678);
    }

    return 0;
}