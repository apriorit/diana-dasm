#include "test_push.h"
extern "C"
{
#include "diana_streams.h"
#include "diana_gen.h"
}

#include "test_common.h"
#include "string.h"

unsigned char nop[1] = {0x90};
unsigned char push[] = {0x50};            //         push        eax  
unsigned char push1[] = {0x51};           //         push        ecx  
unsigned char push2[] = {0x52};           //         push        edx  
unsigned char push3[] = {0x6A, 0x01};     //         push        1    
unsigned char push4[] = {0xFF, 0x75, 0xFC};     //         push        ttt    
unsigned char push5[]= {0xFF, 0x74, 0x71, 0x05};//      push        dword ptr [ecx+esi*2+5] 
unsigned char push6[]= {0xFF, 0x34, 0x75, 0x05, 0x00, 0x00, 0x00}; // push        dword ptr [esi*2+5] 
unsigned char push7[]= {0xFF, 0xB4, 0x71, 0xF6, 0x00, 0x00, 0x00}; // push        dword ptr [ecx+esi*2+0F6h] 
unsigned char push8[]= {0x0E};//         PUSH CS       2        Push CS
unsigned char push9[]= {0x16};//         PUSH SS       2        Push SS
unsigned char push10[]= {0x1E};//         PUSH DS       2        Push DS
unsigned char push11[]= {0x06};//         PUSH ES       2        Push ES
unsigned char push12[]= {0x0F, 0xA0};//   PUSH FS       2        Push FS
unsigned char push13[]= {0x0F, 0xA8};//   PUSH GS       2        Push GS
unsigned char push14[]= {0x68,0x08,0xaf,0x98,0xbf};//   push    offset win32k!`string'+0x168 (bf98af08)


int test_push()
{
    DianaGroupInfo * pGroupInfo=0;
    DianaParserResult result;
    size_t read;
    //unsigned char nop[1] = {0x90};
    int iRes = 0;

    //unsigned char push[] = {0x50};            //         push        eax  
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,push, sizeof(push), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "push")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EAX);
    }

    //unsigned char push1[] = {0x51};           //         push        ecx  
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,push1, sizeof(push1), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "push")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_ECX);
    }

    //unsigned char push2[] = {0x52};           //         push        edx  
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,push2, sizeof(push2), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "push")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT (result.linkedOperands[0].type == diana_register);
        TEST_ASSERT (result.linkedOperands[0].value.recognizedRegister == reg_EDX);
    }

    //unsigned char push3[] = {0x6A, 0x01};     //         push        1    
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,push3, sizeof(push3), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "push")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT (result.linkedOperands[0].type == diana_imm);
        TEST_ASSERT (result.linkedOperands[0].value.imm == 1);
    }
        
    // unsigned char push4[] = {0xFF, 0x75, 0xFC};     //         push        ttt    
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,push4, sizeof(push4), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "push")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT (result.linkedOperands[0].type == diana_index);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.reg == reg_EBP);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.dispValue == -4);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.dispSize == 0x1);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.indexed_reg == reg_none);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.index == 0);
    }

    //unsigned char push5[]= {0xFF, 0x74, 0x71, 0x05};//      push        dword ptr [ecx+esi*2+5] 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,push5, sizeof(push5), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "push")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT (result.linkedOperands[0].type == diana_index);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.reg == reg_ECX);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.dispValue == 0x5);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.dispSize == 0x1);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.indexed_reg == reg_ESI);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.index == 2);
    }

    //unsigned char push6[]= {0xFF, 0x34, 0x75, 0x05, 0x00, 0x00, 0x00}; // push        dword ptr [esi*2+5] 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,push6, sizeof(push6), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "push")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT (result.linkedOperands[0].type == diana_index);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.reg == reg_none);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.dispValue == 0x5);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.dispSize == 0x4);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.indexed_reg == reg_ESI);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.index == 2);
    }

    //unsigned char push7[]= {0xFF, 0xB4, 0x71, 0xF6, 0x00, 0x00, 0x00}; // push        dword ptr [ecx+esi*2+0F6h]
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,push7, sizeof(push7), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "push")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT (result.linkedOperands[0].type == diana_index);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.reg == reg_ECX);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.dispValue == 0xf6);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.dispSize == 0x4);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.indexed_reg == reg_ESI);
        TEST_ASSERT (result.linkedOperands[0].value.rmIndex.index == 2);
    }
 
    //unsigned char push8[]= {0x0E};//         PUSH CS       2        Push CS
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,push8, sizeof(push8), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "push")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_CS);
   }

    //unsigned char push9[]= {0x16};//         PUSH SS       2        Push SS
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,push9, sizeof(push9), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "push")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_SS);
   }
    //unsigned char push10[]= {0x1E};//         PUSH DS       2        Push DS
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,push10, sizeof(push10), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "push")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_DS);
   }
   //unsigned char push11[]= {0x06};//         PUSH ES       2        Push ES
   iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,push11, sizeof(push11), Diana_GetRootLine(), &result, &read);
   TEST_ASSERT_IF(!iRes)
   {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "push")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_ES);
   }
   //unsigned char push12[]= {0x0F, 0xA0};//   PUSH FS       2        Push FS
   iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,push12, sizeof(push12), Diana_GetRootLine(), &result, &read);
   TEST_ASSERT_IF(!iRes)
   {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "push")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_FS);
   }
   //unsigned char push13[]= {0x0F, 0xA8};//   PUSH GS       2        Push GS
   iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,push13, sizeof(push13), Diana_GetRootLine(), &result, &read);
   TEST_ASSERT_IF(!iRes)
   {
        TEST_ASSERT(result.iLinkedOpCount==1);
        TEST_ASSERT(result.pInfo->m_operandCount==1);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "push")==0);
		TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_GS);
   }

   //unsigned char push14[]= {0x68,0x08,0xaf,0x98,0xbf};//   push    offset win32k!`string'+0x168 (bf98af08)
   iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,push14, sizeof(push14), Diana_GetRootLine(), &result, &read);
   TEST_ASSERT_IF(!iRes)
   {
       TEST_ASSERT(result.iLinkedOpCount==1);
       TEST_ASSERT(result.pInfo->m_operandCount==1);
       TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
       TEST_ASSERT(strcmp(pGroupInfo->m_pName, "push")==0);
	   TEST_ASSERT(DI_FLAG_CMD_PRIVILEGED != (result.pInfo->m_flags & DI_FLAG_CMD_PRIVILEGED));
       TEST_ASSERT (result.linkedOperands[0].type == diana_imm);
       TEST_ASSERT (result.linkedOperands[0].value.imm == 0xbf98af08);
   }
   return 0;
}