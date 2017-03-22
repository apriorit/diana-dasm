#include "diana_core_cpp.h"
void test_utils();
void test_memory_manager();

int main()
{
    Diana_Init();
    test_memory_manager();
    test_utils();
    return 0;
}