extern "C"
{
#include "diana_win32_core.h"
}

void test_streams();
void test_processor1();
void test_processor2();
void test_processor3();
void test_patchers();

#include "iostream"

int main()
{
    Diana_Init();
    DianaProcessor_GlobalInit();
    DianaWin32Processor_GlobalInit();

    test_patchers();
    test_streams();
    test_processor1();
    test_processor2();
    test_processor3();
    return 0;
}