extern "C"
{
#include "diana_streams.h"
#include "diana_gen.h"
#include "diana_processor_core.h"
}
#include "test_processor.h"
#include "test_processor_flags.h"
#include "test_processor_call.h"
#include "test_processor_c.h"
#include "test_processor_d.h"
#include "test_processor_i.h"
#include "test_processor_j.h"
#include "test_processor_l.h"
#include "test_processor_m.h"
#include "test_processor_r.h"
#include "test_processor_s.h"
#include "test_processor_a.h"
#include "test_processor_x.h"

void di_gen();


unsigned char buf[] = {0x0F, 0xAC, 0xd0, 0x18};
unsigned char buf2[] = {0x0F, 0xAC, 0xd0, 0x2};

int main()
{
    Diana_Init();
    DianaProcessor_GlobalInit();

    test_processor_a();
    test_processor_s();
    test_processor_r();
    test_processor_m();
    test_processor_l();
    test_processor_j();
    test_processor_i();
    test_processor_d();
    test_processor_c();
    test_processor_x();
    test_processor_call();
    test_processor();
    test_processor_flags();
    return 0;
}
