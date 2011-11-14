#include "test_mov2.h"
extern "C"
{
#include "diana_streams.h"
#include "diana_gen.h"
}

#include "test_common.h"
#include "string.h"

static unsigned char mov[] = {0x66, 0x8C, 0xCC};          // mov         sp,cs 
static unsigned char mov1[] = {0x66, 0x8C, 0xD3};         // mov         bx,ss 
static unsigned char mov2[] = {0x3E, 0xA2, 0x34, 0x12, 0x00, 0x00}; // mov         byte ptr ds:[00001234h],al 
static unsigned char mov3[] = {0x36, 0xA3, 0x34, 0x12, 0x00, 0x00}; // mov         dword ptr ss:[00001234h],eax 
static unsigned char mov4[] = {0x3E, 0xA0, 0x34, 0x12, 0x00, 0x00}; // mov         al,byte ptr ds:[00001234h] 
static unsigned char mov5[] = {0x36, 0xA1, 0x34, 0x12, 0x00, 0x00}; // mov         eax,dword ptr ss:[00001234h] 
static unsigned char mov6[] = {0x2E, 0xC6, 0x02, 0x34};         // mov         byte ptr cs:[edx],34h 
static unsigned char mov7[] = {0x3E, 0xC6, 0x44, 0xBE, 0x01, 0x34}; // mov         byte ptr ds:[esi+edi*4+1],34h 
static unsigned char mov8[] = {0x8C, 0xC8};                // mov         eax,cs 
static unsigned char mov9[] = {0x0f, 0xb7, 0x00};// movzx eax, [word eax]
static unsigned char mov10[] = {0x67, 0x45, 0xf, 0x43, 0x18}; // cmovnb r11d dword ptr ds:[r8d]
static unsigned char mov11[] = {0x8E, 0xD6}; //    mov ss,si 

void test_mov2()
{
    DianaGroupInfo * pGroupInfo=0;
    DianaParserResult result;
    size_t read;
    //static unsigned char mov[] = {0x66, 0x8C, 0xCC};          // mov         sp,cs
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

    //static unsigned char mov1[] = {0x66, 0x8C, 0xD3};         // mov         bx,ss
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov1, sizeof(mov1), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_BX);
        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_SS);
    }

    //static unsigned char mov2[] = {0x3E, 0xA2, 0x34, 0x12, 0x00, 0x00}; // mov         byte ptr ds:[00001234h],al 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov2, sizeof(mov2), Diana_GetRootLine(), &result, &read);
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

    //static unsigned char mov3[] = {0x36, 0xA3, 0x34, 0x12, 0x00, 0x00}; // mov         dword ptr ss:[00001234h],eax 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov3, sizeof(mov3), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.seg_reg == reg_SS)
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.reg == reg_none);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispValue == 0x1234);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispSize == 0x4);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.indexed_reg == reg_none);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.index == 0);
        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_EAX);
    }

    //static unsigned char mov4[] = {0x3E, 0xA0, 0x34, 0x12, 0x00, 0x00}; // mov         al,byte ptr ds:[00001234h] 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov4, sizeof(mov4), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_AL);
        TEST_ASSERT(result.linkedOperands[1].type == diana_index);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.seg_reg == reg_DS)
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_none);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == 0x1234);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0x4);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_none);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 0);
    }

    //static unsigned char mov5[] = {0x36, 0xA1, 0x34, 0x12, 0x00, 0x00}; // mov         eax,dword ptr ss:[00001234h] 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov5, sizeof(mov5), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EAX);    
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.seg_reg == reg_SS)
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_none);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == 0x1234);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0x4);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_none);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 0);
    }


    //static unsigned char mov6[] = {0x2E, 0xC6, 0x02, 0x34};         // mov         byte ptr cs:[edx],34h 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov6, sizeof(mov6), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.seg_reg == reg_CS)
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.reg == reg_EDX);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispValue == 0);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispSize == 0);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.indexed_reg == reg_none);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.index == 0);
        TEST_ASSERT(result.linkedOperands[1].type == diana_imm);
        TEST_ASSERT(result.linkedOperands[1].value.imm == 0x34);
    }

    //static unsigned char mov7[] = {0x3E, 0xC6, 0x44, 0xBE, 0x01, 0x34}; // mov         byte ptr ds:[esi+edi*4+1],34h 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov7, sizeof(mov7), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.seg_reg == reg_DS)
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.reg == reg_ESI);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispValue == 1);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispSize == 1);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.indexed_reg == reg_EDI);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.index == 4);
        TEST_ASSERT(result.linkedOperands[1].type == diana_imm);
        TEST_ASSERT(result.linkedOperands[1].value.imm == 0x34);
    }

    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov8, sizeof(mov8), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "mov")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EAX);    
        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_CS);

    }

    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov9, sizeof(mov9), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "movzx")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_EAX);    
        TEST_ASSERT(result.linkedOperands[0].usedSize == 4);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.seg_reg == reg_DS)
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_EAX);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == 0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_none);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 0);
        TEST_ASSERT(result.linkedOperands[1].usedSize == 2);
    }
    //static unsigned char mov10[] = {0x67, 0x45, 0xf, 0x43, 0x18}; // cmovnb r11d dword ptr ds:[r8d]
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE64,mov10, sizeof(mov10), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "cmovae")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_R11D);    
        TEST_ASSERT(result.linkedOperands[0].usedSize == 4);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.seg_reg == reg_DS)
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.reg == reg_R8D);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispValue == 0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.dispSize == 0);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.indexed_reg == reg_none);
        TEST_ASSERT(result.linkedOperands[1].value.rmIndex.index == 0);
        TEST_ASSERT(result.linkedOperands[1].usedSize == 4);
    }

    //static unsigned char mov11[] = {0x8E, 0xD6}; //    mov ss,si 
    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,mov11, sizeof(mov11), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount = 2)

        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_SS);
        TEST_ASSERT(result.linkedOperands[0].usedSize == 2);

        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_SI);
        TEST_ASSERT(result.linkedOperands[1].usedSize == 2);    
    }
}