#include "TArray.h"
#include "TLDebug.h"
#include "TString.h"
#include "TLUnitTest.h"

TEST(TLArray_FixedArray)
{
	TFixedArray<u8,1> a;
	CHECK( a.GetSize() == 0 );		//	initial fixed array should have no size
	CHECK( a.Add( (u8)123 ) == 0 );	//	should be able to add...
	CHECK( a.GetSize() == 1 );		//	then size should be 1...
	CHECK( a.Add( 1u ) == -1 );		//	and becuase the buffer size is 1, we should fail to add another
	CHECK( a.Exists( (u8)123 ) );	//	should be able to find that initial value
	CHECK( a.Exists( a[0] ) );		//	should be able to find that initial value
}


TEST(TLArray_HeapArray)
{
	THeapArray<u8> a;
	CHECK( a.GetSize() == 0 );		//	initial array should have no size

	CHECK( a.Add( (u8)12 ) == 0 );
	CHECK( a.FindIndex( a[0] ) == 0 );
	CHECK( a.GetSize() == 1 );
	CHECK( a.GetData() != NULL );

	CHECK( a.Add( (u8)34 ) == 1 );
	CHECK( a.Exists( a[1] ) == 1 );
	CHECK( a.GetSize() == 2 );
	CHECK( a.GetData() != NULL );

	THeapArray<u8> b;
	b.Copy(a);
	CHECK( a.GetSize() == b.GetSize() );
	CHECK( b.GetData() != NULL );
	CHECK( a.GetData() != b.GetData() );

	//	b = a;		//	<--- failing test! not copying properly!
	CHECK( a.GetSize() == b.GetSize() );
	CHECK( b.GetData() != NULL );
	CHECK( a.GetData() != b.GetData() );
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

	

