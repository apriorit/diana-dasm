#include "diana_processor_cmd_n.h"
#include "diana_proc_gen.h"
#include "diana_gen.h"
#include "diana_core_gen_tags.h"
#include "diana_processor_cmd_internal.h"

int Diana_Call_neg(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    //DEST := DEST + SRC;
    DI_DEF_LOCALS(src, dest);
        
    DI_MEM_GET_DEST(src);

    dest = 0;

    DI_START_UPDATE_O_FLAGS(dest);

    dest = dest - src;

    DI_END_UPDATE_O_FLAGS_SUB(dest, src);

    DI_UPDATE_FLAGS_PSZ(DI_MEM_SET_DEST(dest));

    CLEAR_FLAG_CF;
    if (src)
    {
        SET_FLAG_CF;
    }   
    DI_PROC_END
}

int Diana_Call_not(struct _dianaContext * pDianaContext,
                    DianaProcessor * pCallContext)
{
    //DEST := SRC
    DI_DEF_LOCAL(dest);
    
    DI_MEM_GET_DEST(dest)

    dest = ~dest;

    DI_MEM_SET_DEST(dest);
    DI_PROC_END
}