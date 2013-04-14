#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <iostream>
extern "C"
{
#include "diana_core.h"
}
#include "string.h"
struct DianaTestError:public std::runtime_error
{
    DianaTestError()
        :
            std::runtime_error("DianaTestError")
    {
    }
    virtual ~DianaTestError() {}
};

#if _DEBUG
#define TEST_ASSERT_IMPL(X, Y)  if (!(X))   {   std::cout<<"[ERROR] \""<<#X<<"\" failed in \""<<__FILE__<<"\" at line "<<__LINE__<<"\n";   Diana_FatalBreak();  Y; }
#else
#define TEST_ASSERT_IMPL(X, Y)  if (!(X))   {   std::cout<<"[ERROR] \""<<#X<<"\" failed in \""<<__FILE__<<"\" at line "<<__LINE__<<"\n";  Y; }
#endif

#define DIANA_TEST(X) try{ X; }catch(DianaTestError & ){ std::cout<<"Test failed: "<<#X<<"\n\n"; }catch(const std::exception & e) {std::cout<<"Test failed: "<<#X<<": "<<e.what()<<"\n\n"; }
#define TEST_ASSERT(X)  TEST_ASSERT_IMPL(X, throw DianaTestError());
#define TEST_ASSERT_IF(X)  TEST_ASSERT_IMPL(X, ;) else

int Diana_ParseCmdOnBuffer_test(int iMode,
                           void * pBuffer,
                           size_t size,
                           DianaBaseGenObject_type * pInitialLine,  // IN
                           DianaParserResult * pResult,  //OUT
                           size_t * sizeRead);    // OUT

struct TestEntry_Name
{
    const char * pCmdName;
    int iCmdSize;
	int iPrivileged;
};

#endif