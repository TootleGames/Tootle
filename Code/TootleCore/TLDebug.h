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


//	gr: turn this into a game/engine option or something
//	enable this define to enable array/float/limit checks
#define DEBUG_CHECK_VALUES


//	macro interfaces to debug routines... why? because we want to catch the FILE and LINE where it breaks
//	using these will also be faster in release mode because the strings won't be constructed
//	gr: because of how _DEBUG is defined, if your lib is release the do-nothing-macros will do nothing. The use of these
//		is per-lib, not how the Core lib is built.
//		use TLDebug::IsEnabled() for more accurate global debug support
#if defined(_DEBUG)
	#define TLDebug_Break(String)					TLDebug::Break( String, (const char*)__FILE__, __LINE__ )
	#define TLDebug_Print(String)					TLDebug::Print( String, (const char*)__FILE__, __LINE__ )
#else
	#define TLDebug_Break(String)					FALSE	//	by default do NOT continue from breaks
	#define TLDebug_Print(String)					{}
#endif


#if defined(_DEBUG) && defined(DEBUG_CHECK_VALUES)
	#define TLDebug_CheckIndex(Index,Max)			TLDebug::CheckIndex( Index, Max, (const char*)__FILE__, __LINE__ )
	#define TLDebug_CheckInRange(Value,Min,Max)		TLDebug::CheckInRange( Value, Min, Max, (const char*)__FILE__, __LINE__ )
	#define TLDebug_CheckFloat(Value)				TLDebug::CheckFloatType( Value, (const char*)__FILE__, __LINE__ )
#else
	#define TLDebug_CheckIndex(Index,Max)			TRUE	//	by in realease just continue, assume index/value is fine
	#define TLDebug_CheckInRange(Value,Min,Max)		TRUE	//	by in realease just continue, assume index/value is fine
	#define TLDebug_CheckFloat(Value)				TRUE	//	by in realease just continue, assume index/value is fine
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
	extern Bool			g_IsEnabled;		//	gr: require a global Debug-is-enabled flag as the use of _DEBUG cannot garuntee the same results project to project if using a mix of release and debug
	extern Bool			g_IsBreaking;		//	stops recursive debug break calls

	FORCEINLINE Bool	IsEnabled()						{	return g_IsEnabled;	}
	FORCEINLINE Bool	IsBreaking()					{	return g_IsBreaking;	}	//	check this if function callbacks or threads etc and dont do anything!
	
	void				GetLastBreak(TString& String);														//	get our last break position appended to this string
	void				SetLastBreak(const char* pSourceFile,const int SourceLine);							//	note last break place in the code

	Bool				Break(const TString& String,const char* pSourceFile,const int SourceLine);			//	halt! return TRUE to ignore the error and continue
	inline Bool			Break(const TString& String)														{	return Break( String, NULL, -1 );	}
	void				Print(const TString& String,const char* pSourceFile,const int SourceLine);			//	print to console
	inline void			Print(const TString& String)														{	Print( String, NULL, -1 );	}

	FORCEINLINE Bool	CheckIndex(int Index,int Max,const char* pSourceFile,const int SourceLine);			//	check & assert if index is out of bounds. Max is NOT inclusive... Min <= N < Max
	FORCEINLINE Bool	CheckIndex(int Index,int Max)														{	return CheckIndex( Index, Max, NULL, -1 );	}
	FORCEINLINE Bool	CheckInRange(int Value,int Min,int Max,const char* pSourceFile,const int SourceLine);	//	check & assert if range is out of bounds. Max IS inclusive... Min <= N <= Max
	FORCEINLINE Bool	CheckInRange(int Value,int Min,int Max)												{	return CheckInRange( Value, Min, Max, NULL, -1 );	}

	Bool				CheckFloat(const float& Value,const char* pSourceFile,const int SourceLine);		//	check float is valid
	Bool				CheckFloat(const TLMaths::TMatrix& Value,const char* pSourceFile,const int SourceLine);		//	check float is valid
	Bool				CheckFloat(const TLMaths::TQuaternion& Value,const char* pSourceFile,const int SourceLine);		//	check float is valid

	template<typename TYPE> Bool	CheckFloatType(const TYPE& Value,const char* pSourceFile,const int SourceLine);
	template<> FORCEINLINE Bool		CheckFloatType(const float& Value,const char* pSourceFile,const int SourceLine)					{	return CheckFloat( Value, pSourceFile, SourceLine );	}
	template<> FORCEINLINE Bool		CheckFloatType(const TLMaths::TMatrix& Value,const char* pSourceFile,const int SourceLine)		{	return CheckFloat( Value, pSourceFile, SourceLine );	}
	template<> FORCEINLINE Bool		CheckFloatType(const TLMaths::TQuaternion& Value,const char* pSourceFile,const int SourceLine)	{	return CheckFloat( Value, pSourceFile, SourceLine );	}


//private:
	Bool				DoCheckIndexBreak(int Index,int Max,const char* pSourceFile,const int SourceLine);
	Bool				DoCheckRangeBreak(int Value,int Min,int Max,const char* pSourceFile,const int SourceLine);


	namespace Platform 
	{
		void		Print(const TString& String);	//	platform specific debug output
		Bool		Break(const TString& String);	//	return FALSE to stop app, TRUE and will attempt to continue
	}
}





//-------------------------------------------------------
//	check a generic float type
//-------------------------------------------------------
template<typename TYPE> 
Bool TLDebug::CheckFloatType(const TYPE& Value,const char* pSourceFile,const int SourceLine)
{
	if ( !IsEnabled() )
		return TRUE;

	for ( u32 i=0;	i<Value.GetSize();	i++ )
	{
		if ( !CheckFloat( Value[i], pSourceFile, SourceLine ) )
			return FALSE;
	}

	return TRUE;
}




//-------------------------------------------------------
//	check & assert if index is out of bounds. Max is NOT inclusive... Min <= N < Max
//-------------------------------------------------------
FORCEINLINE Bool TLDebug::CheckIndex(int Index,int Max,const char* pSourceFile,const int SourceLine)
{
	if ( !IsEnabled() )
		return TRUE;

	//	index is in bounds
	if ( Index >= 0 && Index < Max )
		return TRUE;

	return DoCheckIndexBreak( Index, Max, pSourceFile, SourceLine );
}


//-------------------------------------------------------
//	check & assert if value is out of range. Max IS inclusive... Min <= N <= Max
//-------------------------------------------------------
FORCEINLINE Bool TLDebug::CheckInRange(int Value,int Min,int Max,const char* pSourceFile,const int SourceLine)
{
	if ( !IsEnabled() )
		return TRUE;

	//	value is in bounds
	if ( Value >= Min &&Value <= Max )
		return TRUE;

	return DoCheckRangeBreak( Value, Min, Max, pSourceFile, SourceLine );

}