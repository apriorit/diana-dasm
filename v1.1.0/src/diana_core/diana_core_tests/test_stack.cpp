#include "test_stack.h"

extern "C"
{
#include "diana_stack.h"
}


static 
void test_stack1()
{
    int temp = 0, temp2 = 0;
    Diana_Stack stack;

    // init
    TEST_ASSERT(!Diana_Stack_Init(&stack, sizeof(int)*2, sizeof(int)));

    for(int i=0; i<2; ++i)
    {
        TEST_ASSERT(stack.m_count == 0);

        // push
        temp = 1;
        TEST_ASSERT(!Diana_Stack_Push(&stack,
                                      &temp));
        TEST_ASSERT(stack.m_count == 1);

        // push
        temp = 2;
        TEST_ASSERT(!Diana_Stack_Push(&stack,
                                      &temp));
        TEST_ASSERT(stack.m_count == 2);

        // push
        temp = 3;
        TEST_ASSERT(!Diana_Stack_Push(&stack,
                                      &temp));
        TEST_ASSERT(stack.m_count == 3);

        // pop
        TEST_ASSERT(!Diana_Stack_Pop(&stack,
                                     &temp2));
        TEST_ASSERT(temp2 == 3);
        TEST_ASSERT(stack.m_count == 2);

        // pop
        TEST_ASSERT(!Diana_Stack_Pop(&stack,
                                     &temp2));
        TEST_ASSERT(temp2 == 2);
        TEST_ASSERT(stack.m_count == 1);

        // pop
        TEST_ASSERT(!Diana_Stack_Pop(&stack,
                        &temp2
                        ));
        TEST_ASSERT(temp2 == 1);
        TEST_ASSERT(stack.m_count == 0);

        // empty pop
        TEST_ASSERT(Diana_Stack_Pop(&stack,
                                    &temp2));

    }
    Diana_Stack_Free(&stack);
}

void test_stack()
{
    DIANA_TEST(test_stack1());
}