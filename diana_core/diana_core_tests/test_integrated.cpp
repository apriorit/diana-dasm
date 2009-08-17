#include "test_integrated.h"

extern "C"
{
#include "diana_streams.h"
#include "diana_gen.h"
}

#include "test_common.h"
#include "string.h"
/* TEST 1 
8195e9d7 f00fb116         lock    cmpxchg [esi],edx
81881dc5 c1e902           shr     ecx,0x2
81881db0 f3a5             rep     movsd
*/
void test_integrated()
{
    unsigned char buf[] = {0xf0, 0x0f, 0xb1, 0x16,
                           0xc1, 0xe9, 0x02,
                           0xf3, 0xa5};

    DianaParserResult result;
    DianaGroupInfo * pGroupInfo = 0;
    size_t cmdSize = 0;
    int iRes = 0;
        
    DianaMemoryStream stream;
    DianaContext context;

    Diana_InitContext(&context, DIANA_MODE32);
    Diana_InitMemoryStream(&stream, buf, sizeof(buf));

    // 1
    iRes = Diana_ParseCmd(&context, Diana_GetRootLine(), &stream.parent,  &result);
    cmdSize = stream.curSize - context.iReadedSize;

    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "cmpxchg")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_index);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.seg_reg == reg_DS);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.reg == reg_ESI);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.indexed_reg == reg_none);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.index == 0);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispSize == 0);
        TEST_ASSERT(result.linkedOperands[0].value.rmIndex.dispValue == 0);

        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_EDX);

        TEST_ASSERT(result.iPrefix == DI_PREFIX_LOCK);
   }

    // 2
    iRes = Diana_ParseCmd(&context, Diana_GetRootLine(), &stream.parent,  &result);
    cmdSize = stream.curSize - context.iReadedSize;

    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "sal")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_ECX);
        TEST_ASSERT(result.linkedOperands[1].type == diana_imm);
        TEST_ASSERT(result.linkedOperands[1].value.imm == 2);
        TEST_ASSERT(result.iPrefix == 0);
   }

   // 3
   iRes = Diana_ParseCmd(&context, Diana_GetRootLine(), &stream.parent,  &result);
   cmdSize = stream.curSize - context.iReadedSize;

   TEST_ASSERT_IF(!iRes)
   {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "movs")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_reserved_reg);
        TEST_ASSERT(result.linkedOperands[1].type == diana_reserved_reg);
        TEST_ASSERT(result.iPrefix == DI_PREFIX_REP);
   }

   iRes = Diana_ParseCmd(&context, Diana_GetRootLine(), &stream.parent,  &result);
   TEST_ASSERT(iRes == DI_END);
}