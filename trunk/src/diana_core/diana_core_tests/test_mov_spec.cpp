#include "test_mov_spec.h"
extern "C"
{
#include "diana_streams.h"
#include "diana_gen.h"
}

#include "test_common.h"
#include "string.h"


// IDA says:
static unsigned char mov[] = {0x0F, 0x20, 0xC0};        // mov     eax, cr0
static unsigned char mov1[] = {0x0F, 0x20, 0xFF&~7|1};  // mov     ecx, cr7
static unsigned char mov2[] = {0x0F, 0x22, 0xFF&~7|1};  // mov     cr7, ecx
static unsigned char mov3[] = {0x0F, 0x21, 0xFF&~7|1};  // mov     dr7, ecx
static unsigned char mov4[] = {0x0F, 0x24, 0xFF&~7|1};  // mov     ecx, tr7

static unsigned char mov5[] = {0x48, 0x63, 0x4C, 0x24, 0x04};  // movsxd  rcx, [rsp+4]
static unsigned char mov6[] = {0x66, 0x63, 0x68, 0x4c};  // movsxd  rbp,[rax+0x4c]
static unsigned char mov7[] = {0xf, 0x22, 0xf9};         // mov cr7, rcx


void test_mov_spec()
{
    DianaGroupInfo * pGroupInfo=0;
    DianaParserResult result;
    size_t read;
    //static unsigned char mov[] = {0x0F, 0x20, 0xC0};  // MOV EAX,CR0
    int 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov, sizeof(mov), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
///		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED == (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EAX);
        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_CR0);
    }

    //static unsigned char mov1[] = {0x0F, 0x20, 0xFF&~7|1};  // mov     ecx, cr7
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov1, sizeof(mov1), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
///		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED == (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_ECX);
        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_CR7);
    }

    //static unsigned char mov2[] = {0x0F, 0x22, 0xFF&~7|1};  // mov     cr7, ecx
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov2, sizeof(mov2), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
///		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED == (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_CR7);
        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_ECX);
    }
    //static unsigned char mov3[] = {0x0F, 0x21, 0xFF&~7|1};  // mov     ecx, dr7
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov3, sizeof(mov3), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
///		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED == (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_ECX);
        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_DR7);
    }
    //static unsigned char mov4[] = {0x0F, 0x24, 0xFF&~7|1};  // mov     ecx, tr7
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov4, sizeof(mov4), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
///		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED == (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_ECX);
        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_TR7);
    }

    //static unsigned char mov5[] = {0x48, 0x63, 0x4C, 0x24, 0x04};  // movsxd  rcx, [rsp+428h+var_424]
    iRes = Diana_ParseCmdOnBuffer(DIANA_MODE64,mov5, sizeof(mov5), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "movsxd")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.iFullCmdSize == sizeof(mov5));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_RCX);
        TEST_ASSERT(result.linkedOperands[1].type == diana_index);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.seg_reg == reg_DS);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_RSP);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_none);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 0x00);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0x01);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == 0x0000000000000004);
    }

    // check arpl below, it has the same opcode, ooooo how I love you guys from intel :)
    // just skip prefix
    // 63 4C 24 04      arpl        word ptr [esp+4],cx 
    iRes = Diana_ParseCmdOnBuffer(DIANA_MODE32, mov5+1, sizeof(mov5)-1, Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "arpl")==0);
        TEST_ASSERT(result.iFullCmdSize == sizeof(mov5)-1);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED == (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_index);
		TEST_ASSERT(result.linkedOperands[0].value.rmIndex.seg_reg == reg_DS);
		TEST_ASSERT(result.linkedOperands[0].value.rmIndex.reg == reg_ESP);
		TEST_ASSERT(result.linkedOperands[0].value.rmIndex.indexed_reg == reg_none);
		TEST_ASSERT(result.linkedOperands[0].value.rmIndex.index == 0x00);
		TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispSize == 0x01);
		TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispValue == 0x0000000000000004);
        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_CX);
    }

    // very nice command, back to 64
    iRes = Diana_ParseCmdOnBuffer(DIANA_MODE64, mov6, sizeof(mov6), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "movsxd")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.iFullCmdSize == sizeof(mov6));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_RBP);
        TEST_ASSERT(result.linkedOperands[1].type == diana_index);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.seg_reg == reg_DS);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_RAX);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_none);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 0x00);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0x01);
		TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == 0x04C);
    }

    //static unsigned char mov7[] = {0xf, 0x22, 0xf9};         // mov cr7, rcx
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE64,mov7, sizeof(mov7), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
///		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED == (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_CR7);
        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_RCX);
    }
}