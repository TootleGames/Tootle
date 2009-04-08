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
#endif

#if defined(TL_TARGET_IPOD)
typedef long unsigned int	u64;
typedef long signed int		s64;
#endif

#if defined(TL_TARGET_MAC)
typedef long unsigned int	u64;
typedef long signed int		s64;
#endif



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
