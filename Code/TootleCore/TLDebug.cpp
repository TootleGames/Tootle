#include "TLDebug.h"
#include "TString.h"
#include "TLMaths.h"

//	include the right header for float checks
#ifdef __GNUG__
	#include <cmath>
#else
	#include <float.h>
#endif


//-------------------------------------------------------
//	forward declarations
//-------------------------------------------------------

namespace TLDebug
{
	const char* g_pLastBreakSourceFunction = NULL;

#ifdef _DEBUG
	Bool		g_IsEnabled = TRUE;
#else
	Bool		g_IsEnabled = FALSE;
#endif	
	Bool		g_IsBreaking = FALSE;
}


///////////////////////////////////////////////////////////////////////



//-------------------------------------------------------
//	note last break place in the code
//-------------------------------------------------------
void TLDebug::SetLastBreak(const char* pSourceFunction)
{
	g_pLastBreakSourceFunction = pSourceFunction;
}


//-------------------------------------------------------
//	get our last break position appended to this string
//-------------------------------------------------------
void TLDebug::GetLastBreak(TString& String)
{
	if ( !g_pLastBreakSourceFunction )
	{
		String.Append("???");
	}
	else
	{
		String.Append( g_pLastBreakSourceFunction );
	}
}


//-------------------------------------------------------
//	halt! return TRUE to ignore the error and continue
//-------------------------------------------------------
Bool TLDebug::Break(const TString& String,const char* pSourceFunction)
{
	if ( !IsEnabled() )
		return FALSE;

	//	allow continue if we're in the middle of a break
	if ( g_IsBreaking )
		return TRUE;

	//	breaking
	g_IsBreaking = TRUE;

	//	update last break location
	SetLastBreak( pSourceFunction );

	//	do platform debug break
	Bool BreakResult = Platform::Break(String);

	//	finished breaking
	g_IsBreaking = FALSE;

	return BreakResult;
}


//-------------------------------------------------------
//	print to console
//-------------------------------------------------------
void TLDebug::Print(const TString& String,const char* pSourceFunction)
{
	if ( !IsEnabled() )
		return;

	SetLastBreak( pSourceFunction );

	if ( !String.GetData() || !String.GetLength() )
		return;

	Platform::PrintToBuffer( String );
}



//-------------------------------------------------------
//	check & assert if float is invalid
//-------------------------------------------------------
Bool TLDebug::CheckFloat(const float& Value,const char* pSourceFunction)
{
	if ( !IsEnabled() )
		return TRUE;

	//	gr: todo: turn this into a TLMaths::IsNan() or a TLMaths::CheckFloat()
#if defined(__GNUG__)
	if ( std::isnan( Value ) )
#else
	if ( _isnan( Value ) )
#endif
	{
		TTempString BreakString;
		BreakString.Appendf("float %3.3f is NaN", Value );
		return Break( BreakString );
	}
	
	
#if defined(__GNUG__)
	if ( !std::isfinite( Value ) )
#else
	if ( !_finite( Value ) )
#endif
	{
		TTempString BreakString;
		BreakString.Appendf("float %3.3f is an infinate float", Value );
		return Break( BreakString );
	}

	return TRUE;
}



//-------------------------------------------------------
//	check floats in a matrix
//-------------------------------------------------------
Bool TLDebug::CheckFloat(const TLMaths::TMatrix& Value,const char* pSourceFunction)
{
	if ( !IsEnabled() )
		return TRUE;

	for ( u32 i=0;	i<4;	i++ )
	{
		if ( !CheckFloatType( Value.GetCol(i), pSourceFunction ) )
			return FALSE;
	}

	return TRUE;
}


//-------------------------------------------------------
//	check floats in quaternion
//-------------------------------------------------------
Bool TLDebug::CheckFloat(const TLMaths::TQuaternion& Value,const char* pSourceFunction)
{
	if ( !IsEnabled() )
		return TRUE;

	return CheckFloatType( Value.GetData(), pSourceFunction );
}


//-------------------------------------------------------
//	
//-------------------------------------------------------
Bool TLDebug::DoCheckIndexBreak(int Index,int Max,const char* pSourceFunction)
{
	//	construct a more descriptive string
	TTempString BreakString;
	BreakString.Appendf("Index out of bounds: 0 <= %d < %d", Index, Max );
	
	return Break( BreakString, pSourceFunction );
}

//-------------------------------------------------------
//	
//-------------------------------------------------------
Bool TLDebug::DoCheckRangeBreak(int Value,int Min,int Max,const char* pSourceFunction)
{
	//	construct a more descriptive string
	TTempString BreakString;
	BreakString.Appendf("Value out of range: %d <= %d <= %d", Min, Value, Max );
	
	return Break( BreakString, pSourceFunction );
}
