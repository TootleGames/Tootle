/*------------------------------------------------------
	Debug routines - mostly dummy wrappers for the 
	GutGut debug stuff for now

	FAQ:
	Are you getting an error like...
		error C2664: 'TLDebug::Break' : cannot convert parameter 1 from 'const char [17]' to 'const TString &'
		error C2665: 'TLDebug::Break' : none of the 2 overloads could convert all the argument types

	Then include TString.h in your source.


	Just to clarify (because I think some of my debug code is wrong)
	if a debug check/break routine returns FALSE, then HALT/ABORT/QUIT
	if it returns TRUE, CONTINUE as if nothing wrong

	so;
	if ( CheckIndex() == FALSE )	FAIL
	if ( DebugBreak() == FALSE )	FAIL
	if ( DebugBreak() == TRUE )		CONTINUE

-------------------------------------------------------*/
#pragma once

#include "TLTypes.h"


// ENABLE_DEBUG_TRACE can now be used in debug or release.  This will enable the debug break, assert and printing.
// Ideally we want to have this functionality available at all times and switch it on/off based on some
// runtime setting but there is a large overhead to the debug printing code.  Perhaps we need to alter the
// debug code to be some kind of static object that publishers messages and have the console printing an optional subscriber?
// - it would be nice to be able to intercept debug calls and send them via the network to a network client for instance 
// rather than just being output to the console.  We could intercept these debug messages and send them to a file instead of or as well?
// We probably need a couple more debug routines too i.e. Debug_Informational, Debug_Critical that could be enabled/disabled based on 
// the release type. Rather than using the _DEBUG option we should try and use some kind of define with granularity 
// such as DEBUG_LEVEL == [ NONE | CRITICAL | MINIMAL |  ALL ] or some kind of runtime bitwise set of flags for each type so we can alter the 
// level of debug information at runtime? Things to consider is code within the debug that is ONLY available in debug or not.
#if defined(_DEBUG)
	#define ENABLE_DEBUG_TRACE
#else
	//#define ENABLE_DEBUG_TRACE
#endif

//	gr: todo: turn this into a game/engine option or something
//	gr: I've enabled this for mac because my mac is fast enough to run with it
//		maybe we need a user.h with a #define USER GRAHAM kinda thing 
//	enable this define to enable array/float/limit checks
#if defined(_DEBUG) && defined(TL_TARGET_MAC)
	#define DEBUG_CHECK_INDEXES
	#define DEBUG_CHECK_VALUES
#endif

//	macro interfaces to debug routines... why? because we want to catch the FILE and LINE where it breaks
//	using these will also be faster in release mode because the strings won't be constructed
//	gr: because of how _DEBUG is defined, if your lib is release the do-nothing-macros will do nothing. The use of these
//		is per-lib, not how the Core lib is built.
//		use TLDebug::IsEnabled() for more accurate global debug support
#if defined(ENABLE_DEBUG_TRACE)
	#define TLDebug_Break(String)					TLDebug::Break( String, (const char*)__FUNCTION__ )
	#define TLDebug_Assert(Condition,String)		( (Condition) ? TRUE : TLDebug::Break( String, (const char*)__FUNCTION__ ) )
	#define TLDebug_Print(String)					TLDebug::Print( String, (const char*)__FUNCTION__ )
	#define TLDebug_Warning(String)					TLDebug::Print( String, (const char*)__FUNCTION__ )
	#define TLDebug_FlushBuffer()					TLDebug::FlushBuffer()
#else
	#define TLDebug_Assert(Conditon,String)			FALSE	
	
	// [13/05/10] DB -	When testing for why the release build often fails when setting the optimisation level (to anything now) I was 
	//					able to swap the debug break from a FALSE define to a function returning FALSE that fixed the problem.
	//					It only seemed to occur for this routine ( I tested each of the other debug routines in turn and in combinations)
	//					and it could be coincidence due to volume of this call combined with code re-ordering, macro replacement as well 
	//					as the optimisation but for now this appeared to fix the issue.
	//#define TLDebug_Break(String)					FALSE	//	by default do NOT continue from breaks
	#define TLDebug_Break(String)					TLDebug::NoBreak()
	
	#define TLDebug_Print(String)					{}
	#define TLDebug_Warning(String)					{}
	#define TLDebug_FlushBuffer()					{}

//#define TLDebug_Assert(Condition,String)		( (Condition) ? TRUE : TLDebug::Break( String, (const char*)__FUNCTION__ ) )
//#define TLDebug_Break(String)					TLDebug::Break( String, (const char*)__FUNCTION__ )
//#define TLDebug_Print(String)					TLDebug::Print( String, (const char*)__FUNCTION__ )
//#define TLDebug_Warning(String)					TLDebug::Print( String, (const char*)__FUNCTION__ )
//#define TLDebug_FlushBuffer()					TLDebug::FlushBuffer()

#endif


#if defined(_DEBUG) && defined(DEBUG_CHECK_INDEXES)
	#define TLDebug_CheckIndex(Index,Max)			TLDebug::CheckIndex( Index, Max, (const char*)__FUNCTION__ )
	#define TLDebug_CheckInRange(Value,Min,Max)		TLDebug::CheckInRange( Value, Min, Max, (const char*)__FUNCTION__ )
#else
	#define TLDebug_CheckIndex(Index,Max)			TRUE	//	in realease just continue, assume index/value is fine
	#define TLDebug_CheckInRange(Value,Min,Max)		TRUE	//	in realease just continue, assume index/value is fine
#endif

#if defined(_DEBUG) && defined(DEBUG_CHECK_VALUES)
	#define TLDebug_CheckFloat(Value)				TLDebug::CheckFloatType( Value, (const char*)__FUNCTION__ )
#else
	#define TLDebug_CheckFloat(Value)				TRUE	//	in realease just continue, assume index/value is fine
#endif



//	forward declaration
class TString;

namespace TLMaths
{
	class TMatrix;
	class TQuaternion;
}


namespace TLDebug
{
	namespace Platform 
	{
		void		PrintToBuffer(const TString& String);	//	platform specific debug output - buffered
		Bool		Break(const TString& String);	//	return FALSE to stop app, TRUE and will attempt to continue
		void		FlushBuffer();
	}
	
	extern Bool			g_IsEnabled;		//	gr: require a global Debug-is-enabled flag as the use of _DEBUG cannot garuntee the same results project to project if using a mix of release and debug
	extern Bool			g_IsBreaking;		//	stops recursive debug break calls

	FORCEINLINE Bool	IsEnabled()						{	return g_IsEnabled;	}
	FORCEINLINE Bool	IsBreaking()					{	return g_IsBreaking;	}	//	check this if function callbacks or threads etc and dont do anything!
	
	void				GetLastBreak(TString& String);														//	get our last break position appended to this string
	void				SetLastBreak(const char* pSourceFunction);											//	note last break place in the code

	Bool				Break(const TString& String,const char* pSourceFunction);							//	halt! return TRUE to ignore the error and continue
	Bool				Break(const char* String,const char* pSourceFunction);								//	Break for where we extern the use of break and can't include strings
	FORCEINLINE Bool	Break(const TString& String)														{	return Break( String, NULL );	}
	void				Print(const TString& String,const char* pSourceFunction);							//	print to console
	FORCEINLINE void	Print(const TString& String)														{	Print( String, NULL );	}
	
	FORCEINLINE Bool	NoBreak()		{}
		
	FORCEINLINE void	FlushBuffer()	{	Platform::FlushBuffer(); }
	
	FORCEINLINE Bool	CheckIndex(int Index,int Max,const char* pSourceFunction);							//	check & assert if index is out of bounds. Max is NOT inclusive... Min <= N < Max
	FORCEINLINE Bool	CheckIndex(int Index,int Max)														{	return CheckIndex( Index, Max, NULL );	}
	FORCEINLINE Bool	CheckInRange(int Value,int Min,int Max,const char* pSourceFunction);				//	check & assert if range is out of bounds. Max IS inclusive... Min <= N <= Max
	FORCEINLINE Bool	CheckInRange(int Value,int Min,int Max)												{	return CheckInRange( Value, Min, Max, NULL );	}

	//	todo: replace the specialisations for a generic template function and specialise it in the class-specific places and leave the basic float check here
	Bool				CheckFloat(const float& Value,const char* pSourceFunction);							//	check float is valid
	Bool				CheckFloat(const TLMaths::TMatrix& Value,const char* pSourceFunction);				//	check float is valid
	Bool				CheckFloat(const TLMaths::TQuaternion& Value,const char* pSourceFunction);			//	check float is valid

	template<typename TYPE> Bool	CheckFloatType(const TYPE& Value,const char* pSourceFunction);
	template<> FORCEINLINE Bool		CheckFloatType(const float& Value,const char* pSourceFunction)					{	return CheckFloat( Value, pSourceFunction );	}
	template<> FORCEINLINE Bool		CheckFloatType(const TLMaths::TMatrix& Value,const char* pSourceFunction)		{	return CheckFloat( Value, pSourceFunction );	}
	template<> FORCEINLINE Bool		CheckFloatType(const TLMaths::TQuaternion& Value,const char* pSourceFunction)	{	return CheckFloat( Value, pSourceFunction );	}


//private:
	Bool				DoCheckIndexBreak(int Index,int Max,const char* pSourceFunction);
	Bool				DoCheckRangeBreak(int Value,int Min,int Max,const char* pSourceFunction);


}





//-------------------------------------------------------
//	check a generic float type
//-------------------------------------------------------
template<typename TYPE> 
Bool TLDebug::CheckFloatType(const TYPE& Value,const char* pSourceFunction)
{
	if ( !IsEnabled() )
		return TRUE;

	for ( u32 i=0;	i<Value.GetSize();	i++ )
	{
		if ( !CheckFloat( Value[i], pSourceFunction ) )
			return FALSE;
	}

	return TRUE;
}




//-------------------------------------------------------
//	check & assert if index is out of bounds. Max is NOT inclusive... Min <= N < Max
//-------------------------------------------------------
FORCEINLINE Bool TLDebug::CheckIndex(int Index,int Max,const char* pSourceFunction)
{
	if ( !IsEnabled() )
		return TRUE;

	//	index is in bounds
	if ( Index >= 0 && Index < Max )
		return TRUE;

	return DoCheckIndexBreak( Index, Max, pSourceFunction );
}


//-------------------------------------------------------
//	check & assert if value is out of range. Max IS inclusive... Min <= N <= Max
//-------------------------------------------------------
FORCEINLINE Bool TLDebug::CheckInRange(int Value,int Min,int Max,const char* pSourceFunction)
{
	if ( !IsEnabled() )
		return TRUE;

	//	value is in bounds
	if ( Value >= Min &&Value <= Max )
		return TRUE;

	return DoCheckRangeBreak( Value, Min, Max, pSourceFunction );

}
