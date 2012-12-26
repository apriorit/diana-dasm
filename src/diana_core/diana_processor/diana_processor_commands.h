#ifndef DIANA_PROCESSOR_COMMANDS_H
#define DIANA_PROCESSOR_COMMANDS_H

#define DI_PROC_REGISTER_COMMAND(Name) \
    else if (strcmp(p->m_pName, #Name)==0) \
    {\
    DIANA_SET_PROCESSOR_TAG(p, Diana_Call_ ## Name);\
    }
//---

void DianaProcessor_LinkCommands();

#endif
