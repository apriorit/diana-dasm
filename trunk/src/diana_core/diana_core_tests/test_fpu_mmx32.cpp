#include "test_fpu_mmx32.h"
extern "C"
{
#include "diana_streams.h"
#include "diana_gen.h"
}

#include "test_common.h"
#include "string.h"


void test_fpu_mmx32()
{
    int iRes = 0;
    DianaGroupInfo * pGroupInfo=0;
    DianaParserResult result;
    size_t read;

    static unsigned char mov00[] = {0x0f, 0xae, 0xc0};//           fxsave  eax
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, mov00, sizeof(mov00), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount ==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "fxsave")==0);

        TEST_ASSERT(result.linkedOperands[0].usedSize == 4);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EAX);
    } 

    static unsigned char mov01[] = {0xf3, 0x36, 0x66, 0x44};//           rep     inc sp
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, mov01, sizeof(mov01), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iRexPrefix == 0);
        TEST_ASSERT(result.iPrefix == DI_PREFIX_REP);
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount ==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "inc")==0);

        TEST_ASSERT(result.linkedOperands[0].usedSize == 2);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_SP);
    } 

    static unsigned char mov02[] = {0x26, 0x66, 0x0f, 0x66, 0x44, 0x0f, 0x66};//           pcmpgtd xmm0,oword ptr es:[edi+ecx+0x66]
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, mov02, sizeof(mov02), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "pcmpgtd")==0);

        TEST_ASSERT(result.linkedOperands[0].usedSize == 16);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_XMM0);

        TEST_ASSERT(result.linkedOperands[1].type == diana_index);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.seg_reg == reg_ES);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_EDI);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_ECX);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 1);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 1);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue== 0x66);
    } 

    static unsigned char mov03[] = {0x67, 0xf3, 0x0f, 0x6f, 0xf1};       // movdqu  xmm6,xmm1  
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, mov03, sizeof(mov03), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "movdqu")==0);

        TEST_ASSERT(result.linkedOperands[0].usedSize == 16);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_XMM6);

        TEST_ASSERT(result.linkedOperands[1].usedSize == 16);
        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_XMM1);
    }

    static unsigned char mov04[] = {0xde, 0xe7};  // fsubrp  st(7),st 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, mov04, sizeof(mov04), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "fsubrp")==0);

        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_fpu_ST7);

        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_fpu_ST0);
    }

    static unsigned char mov05[] = {0x67, 0xd8, 0x2f};  //    fsubr   dword ptr [bx]
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, mov05, sizeof(mov05), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount ==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "fsubr")==0);

        TEST_ASSERT(result.linkedOperands[0].usedSize == 4);
        TEST_ASSERT(result.linkedOperands[0].type == diana_index);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.seg_reg == reg_DS);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.reg == reg_none);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.indexed_reg == reg_BX);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.index == 1);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispSize == 0);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispValue== 0);
    }

    static unsigned char data[] = {0xdf, 0xe0};  //    fstsw ax
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32, data, sizeof(data), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount ==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "fnstsw")==0);

        TEST_ASSERT(result.linkedOperands[0].usedSize == 2);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister = reg_AX);
    }

  
}