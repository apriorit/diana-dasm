#include "test_analyze.h"
extern "C"
{
#include "diana_analyze.h"
}


struct TestStream:public DianaAnalyzeObserver
{
    OPERAND_SIZE m_base;
    OPERAND_SIZE m_current;
    OPERAND_SIZE m_size;

    TestStream(OPERAND_SIZE base,
               OPERAND_SIZE current,
               OPERAND_SIZE size)
         : 
            m_base(base), 
            m_current(current),
            m_size(size)
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

    if (pStream->m_current+iBufferSize > pStream->m_size)
    {
        iBufferSize = (int)(pStream->m_size - pStream->m_current);
        if (iBufferSize <=0)
            return DI_END;
    }
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
	pbInvalidPointer;

    TestStream * pStream = (TestStream * )pThis;
	pStream;
    *pRelativeOffset = address;
    return DI_SUCCESS;
}
 
static
int DianaAddSuspectedDataAddress(void * pThis, 
                                 OPERAND_SIZE address)
{
	address;

	TestStream * pStream = (TestStream * )pThis;
	pStream;
    return DI_SUCCESS;
}

struct IntructionInfo
{
    int offset;
    char * pXrefsTo;
    char * pXrefsFrom;
};

void VerifyREF(const char * pTestRefs, 
               const Diana_List * pRefs,
               int index)
{
    bool bEnd = false;
    const char * pLast = pTestRefs;
    Diana_XRef * pCurXRef = 0;
    Diana_SubXRef * pSubRef = (Diana_SubXRef * )pRefs->m_pFirst;
    
    if (pTestRefs)
    {
        for(int u = 0; !bEnd; ++u)
        {
            pCurXRef = Diana_CastXREF(&pSubRef->m_instructionEntry, index);

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
            
            TEST_ASSERT(pCurXRef->m_subrefs[index].m_pInstruction->m_offset == value);

            pSubRef = (Diana_SubXRef *)pSubRef->m_instructionEntry.m_pNext;
            pLast = pTestRefs + u + 1;
        }
    }
    else
    {
        while(pSubRef)
        {
            pCurXRef = Diana_CastXREF(&pSubRef->m_instructionEntry, index);

            TEST_ASSERT(pCurXRef->m_flags & DI_XREF_INVALID);

            pSubRef = (Diana_SubXRef *)pSubRef->m_instructionEntry.m_pNext;
        }
    }
    
}

void test_analyzer1()
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
                            , 0x48, 0xb8, 50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00                   //    mov rax, 1  :39
                            , 0xC3       // ret :49
                            , 0x33, 0xd2 // xor :50
                            , 0xc5                   //    db c5  :52 
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
                         {39,              0,                           "16;33"},
                         {49,              0,                           0}
    };

    const int instructionsCount = sizeof(instructions)/sizeof(instructions[0]);

    size_t minOffset = 0;
    size_t maxOffset = sizeof(code);
    size_t start = 0;

    TestStream stream((OPERAND_SIZE)code, start, sizeof(code));
    DianaAnalyzeObserver_Init(&stream, 
                              DianaRead, 
                              DianaAnalyzeMoveTo,
                              DianaConvertAddressToRelative,
                              DianaAddSuspectedDataAddress);
    Diana_InstructionsOwner owner;

    TEST_ASSERT(DI_SUCCESS == Diana_InstructionsOwner_Init(&owner, maxOffset- minOffset));

    TEST_ASSERT(DI_SUCCESS == Diana_AnalyzeCode(&owner,
                                                &stream,
                                                DIANA_MODE64,
                                                start,
                                                maxOffset));

    TEST_ASSERT(owner.m_actualSize == instructionsCount);
    // verify found instructions
    for(int i = 0; i < instructionsCount; ++i)
    {
        const IntructionInfo * pInfo = instructions + i;
        Diana_Instruction * pInstruction = owner.m_ppPresenceVec[pInfo->offset];
        TEST_ASSERT(pInstruction);
        TEST_ASSERT(pInstruction->m_offset == pInfo->offset);

        VerifyREF(pInfo->pXrefsFrom, &pInstruction->m_refsFrom, 0);
        VerifyREF(pInfo->pXrefsTo, &pInstruction->m_refsTo, 1);
    }

    Diana_InstructionsOwner_Free(&owner);
}


void test_analyzer2()
{
    unsigned char code[] = {  0xbe, 23, 0x00, 0x00, 0x00 //    mov     esi,14 :0
                            , 0xbe, 24, 0x00, 0x00, 0x00 //    mov     esi,15 :5
                            , 0xbe, 25, 0x00, 0x00, 0x00 //    mov     esi,16 :10
                            , 0x75, 0x5                  //    jne            :15
                            , 0xbe, 26, 0x00, 0x00, 0x00 //    mov     esi,17 :17
                            , 0xC3                       //    ret            :22
                            , 0xbe, 55, 0x00, 0x00, 0x00 //    mov     esi,55 :23
                            , 0xc5                       //    db c5          :28
                            , 0xc5                       //    db c5          :29
                            , 0xc5                       //    db c5          :30
                            , 0xc5                       //    db c5          :31
                            , 0xc5                       //    db c5          :32
                            , 0xc5                       //    db c5          :33
                            , 0xc5                       //    db c5          :34
                            };

    IntructionInfo instructions[] =   
                        {{0,               0,                           0},
                         {5,               0,                           0},
                         {10,               0,                        0},
                         {15,              "22",                        0},
                         {17,              0,                        0},
                         {22,              0,                        "15"},
    };

    const int instructionsCount = sizeof(instructions)/sizeof(instructions[0]);

    size_t minOffset = 0;
    size_t maxOffset = sizeof(code);
    size_t start = 0;

    TestStream stream((OPERAND_SIZE)code, start, sizeof(code));
    DianaAnalyzeObserver_Init(&stream, 
                              DianaRead, 
                              DianaAnalyzeMoveTo,
                              DianaConvertAddressToRelative,
                              DianaAddSuspectedDataAddress);
    Diana_InstructionsOwner owner;

    TEST_ASSERT(DI_SUCCESS == Diana_InstructionsOwner_Init(&owner, maxOffset- minOffset));

    TEST_ASSERT(DI_SUCCESS == Diana_AnalyzeCode(&owner,
                                                &stream,
                                                DIANA_MODE64,
                                                start,
                                                maxOffset));

    TEST_ASSERT(owner.m_actualSize == instructionsCount);
    // verify found instructions
    for(int i = 0; i < instructionsCount; ++i)
    {
        const IntructionInfo * pInfo = instructions + i;
        Diana_Instruction * pInstruction = owner.m_ppPresenceVec[pInfo->offset];
        TEST_ASSERT(pInstruction);
        TEST_ASSERT(pInstruction->m_offset == pInfo->offset);

        VerifyREF(pInfo->pXrefsFrom, &pInstruction->m_refsFrom, 0);
        VerifyREF(pInfo->pXrefsTo, &pInstruction->m_refsTo, 1);
    }

    Diana_InstructionsOwner_Free(&owner);
}

void test_analyze()
{
    test_analyzer1();
   // test_analyzer2();
}