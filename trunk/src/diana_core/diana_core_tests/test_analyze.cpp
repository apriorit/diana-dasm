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


struct TestStream:public DianaAnalyzeObserver
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
static 
int DianaConvertAddressToRelative(void * pThis, 
                             OPERAND_SIZE address,
                             OPERAND_SIZE * pRelativeOffset,
                             int * pbInvalidPointer)
{
    TestStream * pStream = (TestStream * )pThis;
    *pRelativeOffset = 0;
    if (address < pStream->m_base)
    {
        *pbInvalidPointer = 1;
    }
    else
    {
        *pRelativeOffset = address - pStream->m_base;
    }
    return DI_SUCCESS;
}
 
static
int DianaAddSuspectedDataAddress(void * pThis, 
                                 OPERAND_SIZE address)
{
    TestStream * pStream = (TestStream * )pThis;
    return DI_SUCCESS;
}

struct IntructionInfo
{
    int offset;
    char * pXrefsFrom;
    char * pXrefsTo;
};

void VerifyREF(const char * pTestRefs, const Diana_List * pRefs)
{
    if (!pTestRefs)
    {
        TEST_ASSERT(pRefs->m_size == 0);
    }
    else
    {
        bool bEnd = false;
        const char * pLast = pTestRefs;
        Diana_XRef * pCurXRef = (Diana_XRef *)pRefs->m_pFirst;
        int pos = 0;
        for(int u = 0; !bEnd; ++u)
        {
            char ch = pTestRefs[u];
            switch(ch)
            {
            default:
                continue;
            case 0:
                bEnd = true;
            case ';':;
            }
            int value = atoi(pLast);
            
            TEST_ASSERT(pCurXRef);
            if (!pCurXRef)
                return;
            TEST_ASSERT(pCurXRef->m_pInstruction->m_offset == value);
            pCurXRef = (Diana_XRef *)pCurXRef->m_instructionEntry.m_pNext;
            pLast = pTestRefs + u + 1;
        }
    }
}

void test_analyzer()
{
    unsigned char code[] = { 0x48, 0x8b, 0xc4 //mov     rax,rsp :0
                            , 0x45, 0x33, 0xf6//    xor     r14d,r14d   :3
                            , 0xe8, 0x12, 0x00, 0x00, 0x00//    call proc1   :6
                            , 0xe8, 0x03, 0x00, 0x00, 0x00//    call proc2   :11
                            , 0x75, 0x15                  //    jne :16
                            , 0xc3                        //    ret :18
                            , 0x66, 0x44, 0x0f, 0x7f, 0xa1, 0xc0, 0x00, 0x00, 0x00//    movdqa oword ptr [rcx+0xc0],xmm12 :19
                            , 0xc3                                               //    ret :28
                            , 0x48, 0x89, 0x71, 0x20 //    mov     [rcx+0x20],rsi :29
                            , 0xe9, 0x01, 0x00, 0x00, 0x00 // jmp to next :33
                            , 0x00                   //    db 0  :38 // should never have come here
                            , 0xc3                   //    ret   :39
                            , 0x00                   //    db 0  :40 // should never have come here
                            };

    IntructionInfo instructions[] =   
                        {{0,               0,                           0},
                         {3,               0,                           0},
                         {6,               "29",                        0},
                         {11,              "19",                        0},
                         {16,              "39",                        0},
                         {18,              0,                           0},
                         {19,              0,                           "11"},
                         {28,              0,                           0},
                         {29,              0,                           "6"},
                         {33,              "39",                        0},
                         {39,              0,                           "16;33"}
    };

    const int instructionsCount = sizeof(instructions)/sizeof(instructions[0]);

    size_t minOffset = 0;
    size_t maxOffset = sizeof(code);
    size_t start = 0;

    TestStream stream((OPERAND_SIZE)code, start);
    DianaAnalyzeObserver_Init(&stream, 
                                DianaRead, 
                                DianaAnalyzeMoveTo,
                                DianaConvertAddressToRelative,
                                DianaAddSuspectedDataAddress);
    Diana_InstructionsOwner owner;

    TEST_ASSERT(DI_SUCCESS ==Diana_InstructionsOwner_Init(&owner, maxOffset- minOffset));

    TEST_ASSERT(DI_SUCCESS == Diana_AnalyzeCode(&owner,
                      &stream,
                      DIANA_MODE64,
                      start,
                      maxOffset));

    TEST_ASSERT(owner.m_usedSize == instructionsCount);
    // verify found instructions
    for(int i = 0; i < instructionsCount; ++i)
    {
        const IntructionInfo * pInfo = instructions + i;
        Diana_Instruction * pInstruction = owner.m_ppPresenceVec[pInfo->offset];
        TEST_ASSERT(pInstruction);
        TEST_ASSERT(pInstruction->m_offset == pInfo->offset);

        VerifyREF(pInfo->pXrefsFrom, &pInstruction->m_refsFrom);
        VerifyREF(pInfo->pXrefsTo, &pInstruction->m_refsTo);
    }

    Diana_InstructionsOwner_Free(&owner);
}

void test_analyze()
{
    test_stack();
    test_analyzer();
}