extern "C"
{
#include "diana_win32_core.h"
}

void test_streams();
void test_processor1();
void test_processor2();

#include "iostream"

extern void __cdecl test_function();
int main()
{
    //std::cout<<"hello, world!";
    std::cout<<" ";//hello, world!";
    test_function();
    Diana_Init();
    DianaProcessor_GlobalInit();
    DianaWin32Processor_GlobalInit();

    test_streams();
    test_processor1();
    test_processor2();
    return 0;
}