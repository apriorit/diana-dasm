#include "diana_debugger.h"

#include "iostream"

extern "C"
{
#include "diana_win32_core_init.h"
#include "diana_processor_commands.h"
#include "diana_processor_core_impl.h"
#include "diana_core_gen_tags.h"
}


static 
int Diana_Call_sysenter(struct _dianaContext * pDianaContext,
                        DianaProcessor * pCallContext)
{
    return DI_ERROR_NOT_USED_BY_CORE;
}


static 
int Diana_Call_int(struct _dianaContext * pDianaContext,
                        DianaProcessor * pCallContext)
{
    return DI_ERROR_NOT_USED_BY_CORE;
}

static 
int Diana_Call_wait(struct _dianaContext * pDianaContext,
                        DianaProcessor * pCallContext)
{
    return DI_ERROR_NOT_USED_BY_CORE;
}


static 
int Diana_Call_fnstcw(struct _dianaContext * pDianaContext,
                        DianaProcessor * pCallContext)
{
    return DI_ERROR_NOT_USED_BY_CORE;
}

static 
int Diana_Call_fnclex(struct _dianaContext * pDianaContext,
                        DianaProcessor * pCallContext)
{
    return DI_ERROR_NOT_USED_BY_CORE;
}
static 
int Diana_Call_cpuid(struct _dianaContext * pDianaContext,
                        DianaProcessor * pCallContext)
{
    return DI_ERROR_NOT_USED_BY_CORE;
}

static 
int Diana_Call_movapd(struct _dianaContext * pDianaContext,
                        DianaProcessor * pCallContext)
{
    return DI_ERROR_NOT_USED_BY_CORE;
}

static 
int Diana_Call_movdqa(struct _dianaContext * pDianaContext,
                        DianaProcessor * pCallContext)
{
    return DI_ERROR_NOT_USED_BY_CORE;
}

static 
int Diana_Call_pxor(struct _dianaContext * pDianaContext,
                        DianaProcessor * pCallContext)
{
    return DI_ERROR_NOT_USED_BY_CORE;
}

static
void DianaWin32Processor_OnGroup(DianaGroupInfo * p)
{
    if (0){}
    DI_PROC_REGISTER_COMMAND(sysenter)
    DI_PROC_REGISTER_COMMAND(int)
    DI_PROC_REGISTER_COMMAND(wait)
    DI_PROC_REGISTER_COMMAND(fnstcw)
    DI_PROC_REGISTER_COMMAND(fnclex)
    DI_PROC_REGISTER_COMMAND(cpuid)
    DI_PROC_REGISTER_COMMAND(movapd)
    DI_PROC_REGISTER_COMMAND(movdqa)
    DI_PROC_REGISTER_COMMAND(pxor)
}

static void DianaWin32Processor_LinkCommands()
{
    DianaGroupInfo * p = Diana_GetGroups();
    long i = 0, count = Diana_GetGroupsCount();

    for(; i < count; ++i)
    {
        DianaWin32Processor_OnGroup(p);
        ++p;
    }
}
int wmain(int argc, wchar_t * argv[])
{
    if (argc != 2)
    {
        std::cout<<"Usage: exe_file\n";
        return ERROR_INVALID_FUNCTION;
    }

    Diana_Init();
    DianaProcessor_GlobalInit();

    DianaWin32Processor_LinkCommands();


    try
    {
        diana::CDianaDebugger dbg(argv[1]);
        dbg.Start();
    }
    catch(diana::Win32Exception & e)
    {
        std::cout<<e.what()<<". Error: "<<e.m_error<<"\n";
    }
    catch(std::exception & e)
    {
        std::cout<<e.what()<<"\n";
    }
    return NO_ERROR;
}