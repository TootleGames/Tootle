/*------------------------------------------------------
	
	interface to unit tests

-------------------------------------------------------*/
#include "TLUnitTest.h"
#include "TLDebug.h"
#include "TString.h"



//	this forces us to build unittest++ lib in the same mode as TootleCore...
#if defined(TL_ENABLE_UNITTEST) && defined(_DEBUG)
	#pragma comment(lib,"../../../Tootle/Code/LibDebug/UnitTest++.lib")
#elif defined(TL_ENABLE_UNITTEST)
	#pragma comment(lib,"../../../Tootle/Code/LibRelease/UnitTest++.lib")
#endif


//------------------------------------------------------
//	execute the tests
//------------------------------------------------------
bool TLUnitTest::RunAllTests(int& Result)
{
#if defined(TL_ENABLE_UNITTEST)
	Result = UnitTest::RunAllTests();
	TLDebug::FlushBuffer();
	return true;
#else
	//	gr: this ins't done yet but debug::print should still print out to STDOUT so we can see
	//		test information in the compile phase
	TTempString StdOut("Unit tests not supported in this build");
	TLDebug::Print( StdOut );
	TLDebug::FlushBuffer();
	return false;
#endif
}

//------------------------------------------------------
//	if this doesn't compile, we haven't included UnitTest++ correctly or there is something wrong with our macros
//------------------------------------------------------
TEST(BasicUnitTest)
{
    CHECK(true);
}
