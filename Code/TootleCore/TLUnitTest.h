/*------------------------------------------------------
	
	interface to unit tests

-------------------------------------------------------*/
#pragma once

#if defined(TL_TARGET_PC)
	#define TL_ENABLE_UNITTEST
#endif

#if defined(TL_ENABLE_UNITTEST)
	#include "TLTypes.h"
	#include <UnitTest++/src/UnitTest++.h>
#else
	//	for platforms that don't support unit tests, provide stub macros
	#define TEST(TestName)		void TestName()
	#define CHECK(x)
#endif




namespace TLUnitTest
{
	int		RunAllTests();	//	run unit tests
}

