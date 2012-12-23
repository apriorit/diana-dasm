#include "test_lea.h"
extern "C"
{
#include "diana_streams.h"
#include "diana_gen.h"
}

#include "test_common.h"
#include "string.h"

static unsigned char lea[] = {0x8D, 0x01};                          // lea         eax,[ecx] 
static unsigned char lea1[] = {0x8D, 0xBD, 0x40, 0xFF, 0xFF, 0xFF};  // lea         edi,[ebp-0C0h] 
static unsigned char lea2[] = {0x67, 0x8D, 0x01};                         // lea         eax,[bx+di] 
static unsigned char lea3[] = {0x67, 0x8D, 0xBD, 0x40, 0xFF};             // lea         edi,[di+0000h] 
static unsigned char lea4[] = {0x66, 0x8D, 0x01};                         // lea         ax,[ecx] 
static unsigned char lea5[] = {0x66, 0x8D, 0xBD, 0x40, 0xFF, 0xFF, 0xFF}; // lea         di,[ebp-0C0h] 
static unsigned char lea6[] = {0x67, 0x66, 0x8D, 0x01};             // lea     ax,[bx+di] 
static unsigned char lea7[] = {0x67, 0x66, 0x8D, 0xBD, 0x40, 0xFF}; // lea         di,[di+0000h] 
static unsigned char lea8[] = {0x67, 0x45, 0x8d, 0x31};             // lea     r14d,[r9d]

void test_lea()
{
    DianaGroupInfo * pGroupInfo=0;
    DianaParserResult result;
    size_t read;
    //static unsigned char lea[] = {0x8D, 0x01};                          // lea         eax,[ecx] 
    int 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,lea, sizeof(lea), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "lea")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EAX);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_ECX);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == 0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_none);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 0);
    }

    // static unsigned char lea1[] = {0x8D, 0xBD, 0x40, 0xFF, 0xFF, 0xFF};  // lea         edi,[ebp-0C0h] 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,lea1, sizeof(lea1), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "lea")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EDI);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_EBP);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == -0xC0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0x4);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_none);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 0);
    }

    //static unsigned char lea2[] = {0x67, 0x8D, 0x01};                         // lea         eax,[bx+di] 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,lea2, sizeof(lea2), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "lea")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EAX);
        TEST_ASSERT(result.linkedOperands[1].type == diana_index);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_DI);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == 0x00);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0x0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_BX);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 1);
    }

    //static unsigned char lea3[] = {0x67, 0x8D, 0xBD, 0x40, 0xFF};             // lea         edi,[di+0000h] 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,lea3, sizeof(lea3), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "lea")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EDI);
        TEST_ASSERT(result.linkedOperands[1].type == diana_index);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_none);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == -0xc0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0x2);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_DI);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 1);
    }

    //static unsigned char lea4[] = {0x66, 0x8D, 0x01};                         // lea         ax,[ecx] 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,lea4, sizeof(lea4), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "lea")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_AX);
        TEST_ASSERT(result.linkedOperands[1].type == diana_index);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_ECX);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == 0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_none);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 0);
    }

    //static unsigned char lea5[] = {0x66, 0x8D, 0xBD, 0x40, 0xFF, 0xFF, 0xFF}; // lea         di,[ebp-0C0h] 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,lea5, sizeof(lea5), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "lea")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_DI);
        TEST_ASSERT(result.linkedOperands[1].type == diana_index);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_EBP);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == -0xC0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0x4);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_none);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 0);
    }

    //static unsigned char lea6[] = {0x67, 0x66, 0x8D, 0x01};             // lea     ax,[bx+di] 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,lea6, sizeof(lea6), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "lea")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_AX);
        TEST_ASSERT(result.linkedOperands[1].type == diana_index);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_DI);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == 0x00);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0x0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_BX);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 1);
    }

    //static unsigned char lea7[] = {0x67, 0x66, 0x8D, 0xBD, 0x40, 0xFF}; // lea         di,[di+0000h] 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,lea7, sizeof(lea7), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "lea")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_DI);
        TEST_ASSERT(result.linkedOperands[1].type == diana_index);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_none);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == -0xc0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0x2);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_DI);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 1);
    }

    //static unsigned char lea8[] = {0x67, 0x45, 0x8d, 0x31};             // lea     r14d,[r9d]
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE64,lea8, sizeof(lea8), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "lea")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_R14D);
        TEST_ASSERT(result.linkedOperands[0].usedSize == 4);
        TEST_ASSERT(result.linkedOperands[1].type == diana_index);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_R9D);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == 0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_none);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 0);
        TEST_ASSERT(result.linkedOperands[1].usedSize == 4);
    }
}