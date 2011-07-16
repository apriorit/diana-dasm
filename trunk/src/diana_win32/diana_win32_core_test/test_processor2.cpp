#include "test_win32_common.h"

void __cdecl test_function_impl()
{
     std::cout<<"hello, world!";
}

static char g_stack[1024*1024];
static void * g_oldESP = 0;

void __cdecl test_function()
{
    test_function_impl();
}
//
//    void * newESP = g_stack + sizeof(g_stack);
//    __asm xor eax, eax
//    __asm xor ebx, ebx
//    __asm xor ecx, ecx
//    __asm xor edx, edx
//    __asm xor edi, edi
//    __asm xor esi, esi
//
//    __asm push ebp
//    __asm mov g_oldESP, esp
//
//    __asm mov esp, newESP
//    __asm mov ebp, esp
//
//    __asm call test_function_impl
//
//    __asm mov esp, g_oldESP
//    __asm pop ebp
//}


void test_processor2()
{
//    test_function();
    DianaWin32Processor proc;

    TEST_ASSERT(DianaWin32Processor_Init(&proc)==DI_SUCCESS);
    
    DianaProcessor * pCallContext = &proc.m_processor;

    DianaProcessor_LoadLiveContext32(pCallContext);

    DI_JUMP_TO_RIP((OPERAND_SIZE)(void*)test_function);
    SET_REG_RSP((OPERAND_SIZE)g_stack + sizeof(g_stack));
  
    OPERAND_SIZE exitOp = 0x87654321;
    diana_internal_push(pCallContext, &exitOp);

    std::vector<State> states;
    for(int i = 0; ; ++i)
    {
        OPERAND_SIZE rip = GET_REG_RIP;
        if (rip == exitOp)
            break;

        states.push_back(State(pCallContext));

        int res = DianaProcessor_ExecOnce(pCallContext);
        TEST_ASSERT(res == DI_SUCCESS);
        if (res != DI_SUCCESS)
        {
            OPERAND_SIZE failedRip  = GET_REG_RIP;
            break;
        }
    }
}
