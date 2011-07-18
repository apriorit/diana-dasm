#include "test_analyze.h"
extern "C"
{
#include "diana_analyze.h"
}

void test_stack()
{
    int temp = 0, temp2 = 0;
    Diana_Stack stack;

    // init
    TEST_ASSERT(!Diana_Stack_Init(&stack, sizeof(int)*2, sizeof(int)));

    for(int i =0; i<2; ++i)
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
                        &temp2
                        ));
        TEST_ASSERT(temp2 == 3);
        TEST_ASSERT(stack.m_count == 2);

        // pop
        TEST_ASSERT(!Diana_Stack_Pop(&stack,
                        &temp2
                        ));
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
                        &temp2
                        ));

    }
    Diana_Stack_Free(&stack);
}


struct TestStream:public DianaAnalyzeReadStream
{
    OPERAND_SIZE m_base;
    OPERAND_SIZE m_current;

    TestStream(OPERAND_SIZE base,
               OPERAND_SIZE current)
         : 
            m_base(base), m_current(current)
    {
    }
};

static
int DianaRead(void * pThis, 
              void * pBuffer, 
              int iBufferSize, 
              int * readed)
{
    TestStream * pStream = (TestStream * )pThis;
    size_t data = (size_t)(pStream->m_base + pStream->m_current);
    memcpy(pBuffer, (void*)data, iBufferSize);
    *readed = iBufferSize;
    pStream->m_current += iBufferSize;
    return DI_SUCCESS;
}

static
int DianaAnalyzeMoveTo(void * pThis, 
                       OPERAND_SIZE offset)
{
    TestStream * pStream = (TestStream * )pThis;
    pStream->m_current = offset;
    return DI_SUCCESS;
}

void test_analyzer()
{
    unsigned char code[] = { 0x48, 0x8b, 0xc4 //mov     rax,rsp
                            , 0x45, 0x33, 0xf6//    xor     r14d,r14d};
                            , 0xe8, 0x10, 0x00, 0x00, 0x00//    call proc1
                            , 0xe8, 0x01, 0x00, 0x00, 0x00//    call proc2
                            , 0xc3                        //    ret
                            , 0x66, 0x44, 0x0f, 0x7f, 0xa1, 0xc0, 0x00, 0x00, 0x00//    movdqa oword ptr [rcx+0xc0],xmm12
                            , 0xc3                                               //    ret
                            , 0x48, 0x89, 0x71, 0x20 //    mov     [rcx+0x20],rsi
                            , 0xc3                   //    ret
                            };

    size_t minOffset = 0;
    size_t maxOffset = sizeof(code);
    size_t start = 0;

    TestStream stream((OPERAND_SIZE)code, start);
    DianaAnalyzeReadStream_Init(&stream, DianaRead, DianaAnalyzeMoveTo);
    Diana_InstructionsOwner owner;

    TEST_ASSERT(DI_SUCCESS ==Diana_InstructionsOwner_Init(&owner, maxOffset- minOffset));

    TEST_ASSERT(DI_SUCCESS == Diana_AnalyzeCode(&owner,
                      &stream,
                      DIANA_MODE64,
                      start,
                      maxOffset));

    Diana_InstructionsOwner_Free(&owner);
}

void test_analyze()
{
    test_stack();
    test_analyzer();
}