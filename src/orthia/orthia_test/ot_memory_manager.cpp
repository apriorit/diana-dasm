#include "test_common.h"
#include "orthia_module_manager.h"
#include "orthia_interfaces.h"
#pragma warning(disable:4996)
static void test_mm1()
{
    std::vector<wchar_t> buf(1024);
    GetTempPath((DWORD)buf.size(), &buf.front());
    wcscat(&buf.front(), L"\\orthia_test\\");

    orthia::CModuleManager manager;
    manager.Reinit(&buf.front());

    void * pFile = GetModuleHandle(0);
    orthia::CMemoryReader reader((orthia::Address_type)pFile);
    manager.ReloadModule((orthia::Address_type)pFile, &reader, true);

    std::vector<orthia::CommonReferenceInfo> references;
    manager.QueryReferencesToInstruction((orthia::Address_type)&test_mm1, &references);
    TEST_ASSERT(!references.empty());
}

void test_memory_manager()
{
    DIANA_TEST(test_mm1())
}