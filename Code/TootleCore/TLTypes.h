/*------------------------------------------------------
	Generic global types

	The TypeX (collection types) classes are templated versions of Vector3, 
	Vector2, Point etc but easy to implement with different
	basic types

	All members accessible via .x .y .z and .w (in that order)
	For bounds checking use the [] operator when accessing as 
	an array instead of ._t[N]

	My naming convention is TypeX - 
	float3 is a vector
	int2 is 2 ints
	Tree8 is 8 trees
	
	This naming convention is a little more confusing though
	for size-types;
	s82		- 2x s8's
	u323	- 3x u32's
	u164	- 4x u16

	gr: if we decide this is too unreadable we can revert to exlusively 
		using one of the following suggestions;
		Type3<u32>
		u32_3
		u32x3
	
-------------------------------------------------------*/
#pragma once


/*	gr: we don't use the UNICODE define, everything IS treated as unicode. 
		OS-specific stuff is handled by OS-specific wrappers (ie. wxWidgets on PC and mac)
		the engine itself now should avoid direct use of OS-specific unicode related stuff 
		(ie. windows and message boxes etc)
#if !defined(UNICODE)
#error Tootle engine should always be built as unicode now
#endif
*/

//	convert some automated build defines into more tangible ones
#if defined(TL_USER_graham) || defined(TL_USER_ZOIDBERG)
#define TL_USER_GRAHAM
#endif


//	compile time assert (FALSE will fail to compile)
#define TLCompileAssertWithVar(predicate,Var)	typedef char Var[2*((predicate)!=0)-1];
#define TLCompileAssertWithLine(predicate,line)	TLCompileAssertWithVar( (predicate) , TLCompileAssertion##line )
#define TLCompileAssert(predicate,msg)			TLCompileAssertWithLine( (predicate), __LINE__ )

//	small helper function to get the size of a literal array
#define sizeofarray(array)		( sizeof( (array) ) / sizeof( (array)[0] ) )

//	small helper function to get the size of a literal array
#define sizeofarray(array)		( sizeof( (array) ) / sizeof( (array)[0] ) )

//-------------------------------------------------------
//	basic types
//-------------------------------------------------------
typedef bool				Bool;
typedef	unsigned int		u32;
typedef	signed int			s32;
typedef	unsigned short		u16;
typedef	signed short		s16;
typedef	unsigned char		u8;
typedef	signed char			s8;

#if defined(TL_TARGET_PC)
typedef unsigned __int64	u64;
typedef signed __int64		s64;
typedef char				TChar8;
typedef wchar_t				TChar16;
typedef s32					TChar32;
#else
typedef long unsigned int	u64;
typedef long signed int		s64;
typedef char				TChar8;
typedef s16					TChar16;
typedef wchar_t				TChar32;
#endif

//	these aren't really required, but stops the use of wchar_t in case we change it later
//	shouldn't have to use these types explicitly very often so avoid where possible (hence strict constness)
typedef const wchar_t		TCharLiteral;
typedef TCharLiteral*		TStringLiteral;

//	our engine char type (TChar) is 16 bit, it's international, and smallest.
typedef TChar16				TChar;

//	if any of these asserts fail then either you have some erroneous flags, or the TChar* typedefs are not configuring correctly for your platform
TLCompileAssert( sizeof(TChar8) == sizeof(u8), "TChar8 not setup correctly." )
TLCompileAssert( sizeof(TChar16) == sizeof(u16), "TChar16 not setup correctly." )
TLCompileAssert( sizeof(TChar32) == sizeof(u32), "TChar32 not setup correctly." )


//	like wxT() or _T or _TEXT, make literal strings widestring. 
//	Mostly required when not using a TString which will do the automatic conversion for us
//	gr: in GCC 4.4 we can use the new prefixes to make specific length UTF literals (currently have to use -fshort-wchar compile flag to work around UTF32/16 issues)
//	U gives a UTF 32 literal
//	u gives a UTF 16 literal
//	L gives a system-defined literal (whcar_t)
#define TLMacroConCat(a,b)		a ## b
#if 0//defined(__GNUG__)	//	4.4
	#define TLCharString(Quote)		TLMacroConCat(u , Quote)
#else
	#define TLCharString(Quote)		TLMacroConCat(L , Quote)
#endif




//	gr: Remove these and use the c++ true and false in the code instead. or True/False to go with Bool
//	BOOL = TRUE/FALSE	//	c, defined randomly by different headers on different platforms
//	bool = true/false	//	c++, defined by compiler
//	Bool = True/False	//	non-existant.. safe to "overload"
#ifndef TRUE
	#define TRUE				true
#endif

#ifndef FALSE
	#define FALSE				false
#endif

#ifndef NULL
	#ifdef __cplusplus
		#define NULL    0
	#else
		#define NULL    ((void *)0)
	#endif
#endif

#define INLINE	inline

//	force inline if not already declared by OS headers
#if !defined(FORCEINLINE) && !defined(__GNUG__)
	#define FORCEINLINE __forceinline
#endif

#if !defined(FORCEINLINE) && defined(__GNUG__)
//	#define FORCEINLINE		NS_INLINE
	#define FORCEINLINE		inline
//#define FORCEINLINE	__attribute__((always_inline))
#endif


//	add warnings to deprecated functions/classes
#ifndef DEPRECATED
#if defined(__GNUG__)
	#define DEPRECATED	__attribute__((deprecated))
#else
	#define DEPRECATED	__declspec(deprecated) 
#endif
#endif

// Specify a symbol (class, function or variable) as visible outside of the build module 
// Only really useful for shared (dynamic) libraries but added in case we ever change from static libs
#ifndef EXPORT
#if defined(__GNUG__)
	#define EXPORT	__attribute__((visibility("default")))
	#define IMPORT	__attribute__((visibility("default")))
#else
	#define EXPORT __declspec(dllexport)
	#define IMPORT __declspec(dllimport)
#endif
#endif


#define STRINGIFY(x) #x
// PLATFORMHEADER header include define to make it easier to include a platform header without having to test each platform
// in the code in numerous places
#if defined(TL_TARGET_IPOD)
	#define PLATFORMHEADER(Header)	STRINGIFY(IPod/IPod##Header)
#elif defined(TL_TARGET_IPAD)
	#define PLATFORMHEADER(Header)	STRINGIFY(IPad/IPad##Header)
#elif defined(TL_TARGET_MAC)
	#define PLATFORMHEADER(Header)	STRINGIFY(Mac/Mac##Header)
#elif defined(_MSC_EXTENSIONS) && defined(TL_TARGET_PC)
	#define PLATFORMHEADER(Header)	STRINGIFY(PC/PC##Header)
#else
	#error "Invalid platform for project"
#endif


//-------------------------------------------------------
//	tempalted integer ranges
//-------------------------------------------------------
namespace TLTypes
{
	template<typename TYPE> FORCEINLINE s32	GetIntegerMin(TYPE x=0)		{	return 0;	}
	template<> FORCEINLINE s32				GetIntegerMin(u8 x)			{	return 0;	}
	template<> FORCEINLINE s32				GetIntegerMin(s8 x)			{	return -127;	}
	template<> FORCEINLINE s32				GetIntegerMin(u16 x)		{	return 0;	}
	template<> FORCEINLINE s32				GetIntegerMin(s16 x)		{	return -32767;	}
	template<> FORCEINLINE s32				GetIntegerMin(u32 x)		{	return 0;	}
	template<> FORCEINLINE s32				GetIntegerMin(s32 x)		{	return -2147483647;	}

	template<typename TYPE> u32				GetIntegerMax(TYPE x=0)		{	return 0;	}
	template<> FORCEINLINE u32				GetIntegerMax(u8 x)			{	return 255;	}
	template<> FORCEINLINE u32				GetIntegerMax(s8 x)			{	return 127;	}
	template<> FORCEINLINE u32				GetIntegerMax(u16 x)		{	return 65535;	}
	template<> FORCEINLINE u32				GetIntegerMax(s16 x)		{	return 32767;	}
	template<> FORCEINLINE u32				GetIntegerMax(u32 x)		{	return 4294967295U;	}
	template<> FORCEINLINE u32				GetIntegerMax(s32 x)		{	return 2147483647;	}
}


//-------------------------------------------------------
//	Tootle specific, but global, types
//	gr: rename to TTBool?
//	I made SyncBool an enum at eurcom because of the usual
//	type confusions. Could have Wait as 2 or ff to fit in a bool
//	but then too often a if (Result) will be TRUE if Result is Wait
//	which tends NOT to be what we want
//-------------------------------------------------------
typedef enum
{
	SyncFalse	= FALSE,
	SyncTrue	= TRUE,
	SyncWait	= 0xffffffff,

} SyncBool;


//-------------------------------------------------------
//	inherited from this type to prevent copying (stops compiler-generated copy constructors/=operators
//-------------------------------------------------------
class TNoCopy
{
private:
	explicit TNoCopy(const TNoCopy& That)	{}
	
	FORCEINLINE TNoCopy&	operator=(const TNoCopy& That)	{	return *this;	}

public:
	TNoCopy()		{}
};


namespace TLCore
{
	//	generic global function to tell if a type can be memcpy'd safely for arrays,
	//	overload (by using the function below) for any dumb classes. i.e. anything 
	//	that does NOT require a complex constructor (classes WITHOUT pointers)
	//	and for classes with NO VIRTUAL FUNCTIONS
	template<typename TYPE>
	FORCEINLINE Bool	IsDataType()
	{
		return FALSE;
	}
	
	//	use this macro to declare a type as memcpy-able
	#define TLCore_DeclareIsDataType(T)	\
		namespace TLCore				\
		{								\
			template<> FORCEINLINE Bool IsDataType<T>()	{	return TRUE;	}	\
			template<> FORCEINLINE Bool IsDataType< Type2<T> >()	{	return TRUE;	}	\
			template<> FORCEINLINE Bool IsDataType< Type3<T> >()	{	return TRUE;	}	\
			template<> FORCEINLINE Bool IsDataType< Type4<T> >()	{	return TRUE;	}	\
		}
}



//-------------------------------------------------------
//	collection types
//-------------------------------------------------------
template <class TYPE>
class Type2;
template <class TYPE>
class Type3;
template <class TYPE>
class Type4;


TLCore_DeclareIsDataType( u8 );
TLCore_DeclareIsDataType( s8 );
//TLCore_DeclareIsDataType( char );	//	gr: vs compiler seems to see s8 and char as different types
TLCore_DeclareIsDataType( TChar8 );
#if !defined(__GNUG__)
TLCore_DeclareIsDataType( TChar16 );
#endif
//TLCore_DeclareIsDataType( TChar );
TLCore_DeclareIsDataType( u16 );
TLCore_DeclareIsDataType( s16 );
TLCore_DeclareIsDataType( u32 );
TLCore_DeclareIsDataType( s32 );
TLCore_DeclareIsDataType( float );
TLCore_DeclareIsDataType( u64 );
TLCore_DeclareIsDataType( SyncBool );




typedef Type2<int>		int2;
typedef Type3<int>		int3;
typedef Type4<int>		int4;

typedef Type2<float>	float2;
typedef Type3<float>	float3;
typedef Type4<float>	float4;


#include "TLTypes.inc.h"
