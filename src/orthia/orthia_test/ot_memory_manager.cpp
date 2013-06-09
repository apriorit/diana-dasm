#include "test_common.h"
#include "orthia_module_manager.h"
#include "orthia_interfaces.h"

static void test_mm1()
{
    std::vector<wchar_t> buf(1024);
    GetTempPath((DWORD)buf.size(), &buf.front());

    orthia::CModuleManager manager;
    manager.Reinit(&buf.front());

    void * pFile = GetModuleHandle(0);
    orthia::CMemoryReader reader((orthia::Address_type)pFile);
    manager.ReloadModule((orthia::Address_type)pFile, &reader);
}

void test_memory_manager()
{
    DIANA_TEST(test_mm1())
}