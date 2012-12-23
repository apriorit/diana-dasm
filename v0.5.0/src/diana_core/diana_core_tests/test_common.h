#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <iostream>
extern "C"
{
#include "diana_core.h"
}
#include "string.h"
#if _DEBUG
#define TEST_ASSERT(X)  if (!(X))   {   std::cout<<"[ERROR] \""<<#X<<"\" failed in \""<<__FILE__<<"\" at line "<<__LINE__<<"\n";   Diana_FatalBreak();  }
#else
#define TEST_ASSERT(X)  if (!(X))   {   std::cout<<"[ERROR] \""<<#X<<"\" failed in \""<<__FILE__<<"\" at line "<<__LINE__<<"\n";   }
#endif
#define TEST_ASSERT_IF(X)  TEST_ASSERT(X) else

int Diana_ParseCmdOnBuffer_test(int iMode,
                           void * pBuffer,
                           size_t size,
                           DianaCmdKeyLine * pInitialLine,  // IN
                           DianaParserResult * pResult,  //OUT
                           size_t * sizeRead);    // OUT

struct TestEntry_Name
{
    const char * pCmdName;
    int iCmdSize;
	int iPrivileged;
};

#endif