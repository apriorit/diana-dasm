#include "diana_win32_core_init.h"
#include "diana_processor_commands.h"
#include "diana_processor_core_impl.h"
#include "string.h"
#include "diana_core_gen_tags.h"


int Diana_Call_sysenter(struct _dianaContext * pDianaContext,
                        DianaProcessor * pCallContext);


static
void DianaWin32Processor_OnGroup(DianaGroupInfo * p)
{
    if (0){}
    DI_PROC_REGISTER_COMMAND(sysenter)
}

void DianaWin32Processor_LinkCommands()
{
    DianaGroupInfo * p = Diana_GetGroups();
    long i = 0, count = Diana_GetGroupsCount();

    for(; i < count; ++i)
    {
        DianaWin32Processor_OnGroup(p);
        ++p;
    }
}

void DianaWin32Processor_GlobalInit()
{
    DianaWin32Processor_LinkCommands();
}