/*------------------------------------------------------
	
	interface to unit tests

-------------------------------------------------------*/
#include "TLUnitTest.h"
#include "TLDebug.h"
#include "TString.h"


//	this forces us to build unittest++ lib in the same mode as TootleCore...
#if defined(_DEBUG)
	#pragma comment(lib,"../../../Tootle/Code/LibDebug/UnitTest++.lib")
#else
	#pragma comment(lib,"../../../Tootle/Code/LibRelease/UnitTest++.lib")
#endif


//------------------------------------------------------
//	execute the tests
//------------------------------------------------------
int TLUnitTest::RunAllTests()
{
#if defined(TL_ENABLE_UNITTEST)
	return UnitTest::RunAllTests();
#else
	TTempString StdOut("Unit tests not supported in this build");
	TLDebug::Print( StdOut );
	TLDebug::FlushBuffer();
	return 0;
#endif
}

//------------------------------------------------------
//	if this doesn't compile, we haven't included UnitTest++ correctly
//------------------------------------------------------
TEST(BasicUnitTest)
{
    CHECK(true);
}
