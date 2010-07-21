#include "TArray.h"
#include "TLDebug.h"
#include "TString.h"
#include "TLUnitTest.h"

namespace TLArray
{
	namespace TLTest
	{
		template<typename TYPE>
		bool	Debug_Compare(TArray<TYPE>& a,TArray<TYPE>& b);

		template<typename TYPE>
		bool	Debug_Verify(TArray<TYPE>& a);

		template<typename TYPE>
		bool	Debug_Alloc(TArray<TYPE>& a);
	}
}


//---------------------------------------------------
//	test to make sure these two arrays match
//---------------------------------------------------
template<typename TYPE>
bool TLArray::TLTest::Debug_Compare(TArray<TYPE>& a,TArray<TYPE>& b)
{
	bool Result = true;
	
	//	make sure sizes match
	if ( a.GetSize() != b.GetSize() )
	{
		TDebugString Debug_String;
		Debug_String << "Array sizes don't match, " << a.GetSize() << " Vs " << b.GetSize();
		TLDebug_Print( Debug_String );
		Result = false;
	}
	
	//	make sure the comparisons work
	if ( !(a==b) )
	{
		TLDebug_Print("Array's don't match with == operator");
		Result = false;
	}

	if ( a!=b )
	{
		TLDebug_Print("Array's wrongly mis-matched with != operator");
		Result = false;
	}

	//	if they're the same, the comparisons should never say one is less than another
	if ( a < b )
	{
		TLDebug_Print("Matching arrays should not return true for < operator (a < b)");
		Result = false;
	}

	if ( b < a )
	{
		TLDebug_Print("Matching arrays should not return true for < operator (b < a)");
		Result = false;
	}

	//	data pointers should not match as they are copies and should not point at the same data
	if ( &a != &b && a.GetData() != NULL )
	{
		if ( a.GetData() == b.GetData() )
		{
			TLDebug_Print("Two different array instances should not have matching data addresses");
			Result = false;
		}
	}
			
	return Result;
}

//---------------------------------------------------
//	test to make sure certain artifacts of arrays work
//---------------------------------------------------
template<typename TYPE>
bool TLArray::TLTest::Debug_Verify(TArray<TYPE>& a)
{
	bool Result = true;
	
	//	if no size, data should always return NULL
	if ( a.GetSize() == 0 && a.GetData() != NULL )
	{
		TLDebug_Print("Array size is zero, but data is not NULL");
		Result = false;
	}
	else if ( a.GetSize() > 0 && a.GetData() == NULL )
	{
		TLDebug_Print("Array is not empty, but data is NULL");
		Result = false;
	}
	
	//	check the find stuff
	for ( u32 i=0;	i<a.GetSize();	i++ )
	{
		const TYPE& Element = a[i];
		s32 FoundIndex = a.FindIndex( Element );
		
		//	failed to find element
		if ( FoundIndex == -1 )
		{
			TDebugString Debug_String;
			Debug_String << "Failed to find index of element " << i;
			TLDebug_Print( Debug_String );
			Result = false;
		}
		else if ( FoundIndex != i )
		{
			TDebugString Debug_String;
			Debug_String << "Wrongly found index of element " << i << " at location " << FoundIndex;
			TLDebug_Print( Debug_String );
			Result = false;
		}
	}
	
	return Result;
}


//---------------------------------------------------
//	test of allocation of array
//---------------------------------------------------
template<typename TYPE>
bool TLArray::TLTest::Debug_Alloc(TArray<TYPE>& a)
{
	bool Result = true;

	//	try allocating max items
	u32 MaxAlloc = a.GetMaxAllocSize();
	if ( !a.SetAllocSize( MaxAlloc ) )
	{
		TDebugString Debug_String;
		Debug_String << "Failed to set array size to the MaxAllocSize of " << MaxAlloc;
		TLDebug_Print( Debug_String );
		Result = false;
	}
	else
	{
		//	make sure that size is reported right
		u32 AllocSize = a.GetAllocSize();
		if ( AllocSize != MaxAlloc )
		{
			TDebugString Debug_String;
			Debug_String << "Alloc size is wrong. Is " << AllocSize << ", should be the MaxAllocSize " << MaxAlloc;
			TLDebug_Print( Debug_String );
			Result = false;
		}
	}
	
	//	empty
	a.Empty();
	if ( a.GetSize() != 0 )
	{
		TLDebug_Print("Array Empty failed");
		Result = false;
	}

	return Result;
}


TEST(TLArray_FixedArray)
{
	TFixedArray<u8,1> a;
	CHECK( TLArray::TLTest::Debug_Verify( a ) );
	CHECK( a.GetSize() == 0 );		//	initial fixed array should have no size
	CHECK( a.Add( (u8)123 ) == 0 );	//	should be able to add...
	CHECK( a.GetSize() == 1 );		//	then size should be 1...
	CHECK( a.Add( 1u ) == -1 );		//	and becuase the buffer size is 1, we should fail to add another
	CHECK( a.Exists( (u8)123 ) );	//	should be able to find that initial value
	CHECK( a.Exists( a[0] ) );		//	should be able to find that initial value
	
	CHECK( TLArray::TLTest::Debug_Verify( a ) );
	CHECK( TLArray::TLTest::Debug_Alloc( a ) );
}


TEST(TLArray_HeapArray)
{
	THeapArray<u8> a;
	CHECK( TLArray::TLTest::Debug_Verify( a ) );
	CHECK( a.GetSize() == 0 );		//	initial array should have no size

	CHECK( a.Add( (u8)12 ) == 0 );
	CHECK( a.FindIndex( a[0] ) == 0 );
	CHECK( a.GetSize() == 1 );
	CHECK( a.GetData() != NULL );

	CHECK( a.Add( (u8)34 ) == 1 );
	CHECK( a.Exists( a[1] ) == 1 );
	CHECK( a.GetSize() == 2 );
	CHECK( a.GetData() != NULL );
	CHECK( TLArray::TLTest::Debug_Verify( a ) );

	THeapArray<u8> b;
	CHECK( TLArray::TLTest::Debug_Verify( b ) );

	//	check the "copy" func
	b.Copy(a);
	CHECK( TLArray::TLTest::Debug_Compare( a, b ) );
	CHECK( TLArray::TLTest::Debug_Compare( a, a ) );
	CHECK( TLArray::TLTest::Debug_Compare( b, b ) );

	//	check the = operator does the same as the Copy()
	b = a;
	CHECK( TLArray::TLTest::Debug_Compare( a, b ) );
/*
	THeapArray<u8> c( a );
	CHECK( TLArray::TLTest::Debug_Compare( a, c ) );
	
	TFixedArray<u8,100> d( a );
	CHECK( TLArray::TLTest::Debug_Compare( a, d ) );
	*/
}


Bool TLArray::Debug::Break(const char* pErrorString,const char* pSourceFunction)
{
#if defined(_DEBUG)
	if ( !TLDebug::IsEnabled() )
		return FALSE;
	
	TTempString String( pErrorString );
	return TLDebug::Break( String, pSourceFunction );
#else
	return FALSE;
#endif
}

	
void TLArray::Debug::Print(const char* pErrorString,const char* pSourceFunction)
{
#if defined(_DEBUG)
	if ( !TLDebug::IsEnabled() )
		return;
	
	//	gr: print the source function in array debug to quickly track down source of warnings
	TTempString String( pErrorString );
	String.Appendf(" (%s)", pSourceFunction );
	TLDebug::Print( String, pSourceFunction );
#endif
}

	

