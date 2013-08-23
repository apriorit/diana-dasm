#include "test_common.h"
#include "orthia_module_manager.h"
#include "orthia_interfaces.h"

// wcscat
#pragma warning(disable:4996)

struct OT_TestEnv
{
    orthia::CModuleManager manager;
    orthia::CMemoryReader reader;

    OT_TestEnv()
    {
        std::vector<wchar_t> buf(1024);
        GetTempPath((DWORD)buf.size(), &buf.front());
        wcscat(&buf.front(), L"\\orthia_test");
        CreateDirectory(&buf.front(), 0);
        wcscat(&buf.front(), L"\\test.db");
        manager.Reinit(&buf.front(), true);
    }
};
static void test_mm1()
{
    OT_TestEnv testEnv;
    
    void * pFile = GetModuleHandle(0);
    testEnv.manager.ReloadModule((orthia::Address_type)pFile, &testEnv.reader, true, L"test");

    std::vector<orthia::CommonReferenceInfo> references;
    testEnv.manager.QueryReferencesToInstruction((orthia::Address_type)&test_mm1, &references);
    TEST_ASSERT(!references.empty());

    std::vector<orthia::CommonModuleInfo> modules;
    testEnv.manager.QueryLoadedModules(&modules);
    TEST_ASSERT(modules.size() == 1);
    TEST_ASSERT(modules[0].address == (orthia::Address_type)pFile);

    testEnv.manager.UnloadModule((orthia::Address_type)pFile);
    testEnv.manager.QueryLoadedModules(&modules);
    TEST_ASSERT(modules.size() == 0);

    testEnv.manager.QueryReferencesToInstruction((orthia::Address_type)&test_mm1, &references);
    TEST_ASSERT(references.empty());
}

static void test_performance1()
{
    OT_TestEnv testEnv;
    orthia::CDll dll(L"shell32.dll");
    testEnv.manager.ReloadModule((orthia::Address_type)dll.GetBase(), &testEnv.reader, true, L"test");
}

void test_memory_manager()
{
    DIANA_TEST(test_mm1())
    DIANA_TEST(test_performance1())
}