#include "test_cmp.h"
extern "C"
{
#include "diana_streams.h"
#include "diana_gen.h"
}

#include "test_common.h"
#include "string.h"

unsigned char cmp[] = {0x3B, 0xF4};       //          cmp         esi,esp 

int test_cmp()
{
    DianaGroupInfo * pGroupInfo=0;
    DianaParserResult result;
    size_t read;
    int 

    iRes = Diana_ParseCmdOnBuffer_test(DIANA_MODE32,cmp, sizeof(cmp), Diana_GetRootLine(), &result, &read);
    TEST_ASSERT_IF(!iRes)
    {
        TEST_ASSERT(result.iLinkedOpCount==2);
        TEST_ASSERT(result.pInfo->m_operandCount ==2);
        TEST_ASSERT(pGroupInfo = Diana_GetGroupInfo(result.pInfo->m_lGroupId));
        TEST_ASSERT(strcmp(pGroupInfo->m_pName, "cmp")==0);
        TEST_ASSERT(result.linkedOperands[0].type == diana_register);
        TEST_ASSERT(result.linkedOperands[0].value.recognizedRegister == reg_ESI);
        TEST_ASSERT(result.linkedOperands[1].type == diana_register);
        TEST_ASSERT(result.linkedOperands[1].value.recognizedRegister == reg_ESP);
    }

    return 0;
}