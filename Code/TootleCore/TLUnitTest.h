/*------------------------------------------------------
	
	interface to unit tests

-------------------------------------------------------*/
#pragma once

#include "TLTypes.h"
#if defined(TL_USER_GRAHAM)
	#define TL_ENABLE_UNITTEST
#endif

//	not supported on IPod so undef if it's been enabled
#if defined(TL_ENABLE_UNITTEST) && defined(TL_TARGET_IPOD)
	#undef TL_ENABLE_UNITTEST
#endif



#if defined(TL_ENABLE_UNITTEST)
	#include <UnitTest++/src/UnitTest++.h>
#else
	//	for platforms that don't support unit tests, provide stub macros
	#define TEST(TestName)		void TLUnitTest_##TestName()
	#define CHECK(x)
#endif




namespace TLUnitTest
{
	bool		RunAllTests(int& Result);	//	run unit tests. returns true/false as to whether it is supported
}

