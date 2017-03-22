extern "C"
{
#include "diana_pe_analyzer.h"

}
#include "test_common.h"
#include "windows.h"

static void pe_analyze_test1()
{
    void * pFile = GetModuleHandle(0);
    Diana_InstructionsOwner owner;
    TEST_ASSERT(Diana_PE_AnalyzePEInMemory(pFile,
                                           0,
                                           &owner) == DI_SUCCESS);
}
void pe_analyze_test()
{
    DIANA_TEST(pe_analyze_test1());
}