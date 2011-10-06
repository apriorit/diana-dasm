#include "test_mov_as.h"
extern "C"
{
#include "diana_streams.h"
#include "diana_gen.h"
}

#include "test_common.h"
#include "string.h"


//no changes:
static unsigned char mov[] = {0x67, 0x66, 0x8C, 0xCC};          // mov         sp,cs
static unsigned char mov1[] = {0x67, 0x3E, 0xA2, 0x34, 0x12, 0x00, 0x00}; // mov         byte ptr ds:[00001234h],al 

//changes:
static unsigned char mov2[] = {0x67, 0x2E, 0xC6, 0x02, 0x34};         // mov         byte ptr cs:[bp+si],34h 
static unsigned char mov3[] = {0x67, 0x3E, 0xC6, 0x44, 0xBE, 0x01}; // mov         byte ptr ds:[si-42h],1 

void test_mov_as()
{
    DianaGroupInfo * pGroupInfo=0;
    DianaParserResult result;
    size_t read;
    //static unsigned char mov[] = {0x67, 0x66, 0x8C, 0xCC};          // mov         sp,cs
    int 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov, sizeof(mov), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_SP);
        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_CS);
    }

    //static unsigned char mov1[] = {0x67, 0x3E, 0xA2, 0x34, 0x12, 0x00, 0x00}; // mov         byte ptr ds:[00001234h],al 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov1, sizeof(mov1), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_index);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.seg_reg == reg_DS)
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.reg == reg_none);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispValue == 0x1234);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispSize == 0x4);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.indexed_reg == reg_none);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.index == 0);
        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_AL);
    }

    //static unsigned char mov2[] = {0x67, 0x2E, 0xC6, 0x02, 0x34};         // mov         byte ptr cs:[bp+si],34h 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov2, sizeof(mov2), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.seg_reg == reg_CS)
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.reg == reg_SI);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispValue == 0);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispSize == 0);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.indexed_reg == reg_BP);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.index == 1);
        TEST_ASSERT(result.linkedOperands[1].type == diana_imm);
        TEST_ASSERT(result.linkedOperands[1].value.imm == 0x34);
    }

    //static unsigned char mov3[] = {0x67, 0x3E, 0xC6, 0x44, 0xBE, 0x01}; // mov         byte ptr ds:[si-42h],1 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov3, sizeof(mov3), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.seg_reg == reg_DS)
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.reg == reg_none);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispValue == -66);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispSize == 1);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.indexed_reg == reg_SI);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.index == 1);
        TEST_ASSERT(result.linkedOperands[1].type == diana_imm);
        TEST_ASSERT(result.linkedOperands[1].value.imm == 0x1);
    }

}