/*
 
	TLBinary type declarations and information, seperated from the TBinary/Stream type

 */
#include "TLTypes.h"
#include "TRef.h"


class TRef;

namespace TLBinary
{
	//	default unhandled type is just an invalid TRef which means no specific type
	template<typename TYPE>
	FORCEINLINE TRef	GetDataTypeRef()					{	return TRef();	}	
	
	//	get the size of the data a ref refers to, but only useful at compile time (so for templated functions) ... 
	//	... probably rarely used but in TLBinary::GetSizeOf() stops any potential code-errors
	template<u32 TYPEREF>
	FORCEINLINE u32		GetDataTypeSize()					{	return 0;	}
	u32					GetDataTypeSize(TRefRef TypeRef);
	
	//	get number of elements in this type ref
	template<u32 TYPEREF>
	FORCEINLINE u32		GetDataTypeElementCount()					{	return 0;	}
	u32					GetDataTypeElementCount(TRefRef TypeRef);
	
	//	same as above but ability to use a variable for the TYPE instead of explicitly using <XYZ>
	template<typename TYPE>
	FORCEINLINE TRef	GetDataTypeRef(const TYPE& Var)		{	return GetDataTypeRef<TYPE>();	}
	
#define TLBinary_Private_DeclareDataTypeRef(ELEMENTTYPE,TYPE,TYPEREF,ELEMENTCOUNT)		\
namespace TLBinary										\
{														\
template<> FORCEINLINE TRef	GetDataTypeRef<TYPE>()				{	return TYPEREF;	}	\
template<> FORCEINLINE u32	GetDataTypeSize<TYPEREF>()			{	return sizeof(TYPE);	}	\
template<> FORCEINLINE u32	GetDataTypeElementCount<TYPEREF>()	{	return ELEMENTCOUNT;	}	\
}
	
#define TLBinary_Private_DeclareDataTypeRefNoSize(ELEMENTTYPE,TYPE,TYPEREF)		\
namespace TLBinary									\
{													\
template<> FORCEINLINE TRef	GetDataTypeRef<TYPE>()		{	return TYPEREF;	}	\
}
	
	//	simple macro to associate type with ref by specialising the GetDataTypeRef template.
	//	if you don't specify this then it's likely your data will lose its type information when writing OUT to say XML
#define TLBinary_DeclareDataTypeRef(TYPE)		\
TLBinary_Private_DeclareDataTypeRef( TYPE, TYPE,		TLBinary_TypeRef( TYPE ), 1 );			\

#define TLBinary_DeclareDataTypeWithRef(TYPE,TYPEREF)		\
TLBinary_Private_DeclareDataTypeRef( TYPE, TYPE,		TYPEREF, 1 );			\

	//	another one to do TYPE and TYPE2,TYPE3,TYPE4
#define TLBinary_DeclareDataTypeRefAll(TYPE)	\
TLBinary_Private_DeclareDataTypeRef( TYPE, TYPE,		TLBinary_TypeRef( TYPE ), 1 );			\
TLBinary_Private_DeclareDataTypeRef( TYPE, Type2<TYPE>,	TLBinary_TypeNRef( Type2, TYPE ), 2 );	\
TLBinary_Private_DeclareDataTypeRef( TYPE, Type3<TYPE>,	TLBinary_TypeNRef( Type3, TYPE ), 3 );	\
TLBinary_Private_DeclareDataTypeRef( TYPE, Type4<TYPE>,	TLBinary_TypeNRef( Type4, TYPE ), 4 );	
	
#define TLBinary_DeclareDataTypeRefAllNoSize(TYPE)	\
TLBinary_Private_DeclareDataTypeRefNoSize( TYPE, TYPE,			TLBinary_TypeRef( TYPE ) );			\
TLBinary_Private_DeclareDataTypeRefNoSize( TYPE, Type2<TYPE>,	TLBinary_TypeNRef( Type2, TYPE ) );	\
TLBinary_Private_DeclareDataTypeRefNoSize( TYPE, Type3<TYPE>,	TLBinary_TypeNRef( Type3, TYPE ) );	\
TLBinary_Private_DeclareDataTypeRefNoSize( TYPE, Type4<TYPE>,	TLBinary_TypeNRef( Type4, TYPE ) );	
	
	//	gr: accessor to get a define of a TRef for a type. You need to use these for case statements, plus it helps to pre-compile refs more
#define TLBinary_TypeRef(Type)				TLBinary_TypeRef_##Type
#define TLBinary_TypeNRef(TypeN,Type)		TLBinary_TypeRef_##TypeN##_##Type
	
	
	
	
	//	some special types we can't get from variables
	//	gr: "hex" versions are to detect hex-representation in XML.
	//		the "u8" data is written as integers
#define TLBinary_TypeRef_Hex8				TRef_Static4(H,e,x,EIGHT)
#define TLBinary_TypeRef_Hex16				TRef_Static(H,e,x,ONE,SIX)
#define TLBinary_TypeRef_Hex32				TRef_Static(H,e,x,THREE,TWO)
#define TLBinary_TypeRef_Hex64				TRef_Static(H,e,x,SIX,FOUR)
#define TLBinary_TypeRef_String				TRef_Static(S,t,r,i,n)
#define TLBinary_TypeRef_WideString			TRef_Static(W,S,t,r,i)
	
	FORCEINLINE TRef	GetDataTypeRef_Hex8()				{	return TLBinary_TypeRef(Hex8);	}
	FORCEINLINE TRef	GetDataTypeRef_Hex16()				{	return TLBinary_TypeRef(Hex16);	}
	FORCEINLINE TRef	GetDataTypeRef_Hex32()				{	return TLBinary_TypeRef(Hex32);	}
	FORCEINLINE TRef	GetDataTypeRef_Hex64()				{	return TLBinary_TypeRef(Hex64);	}
	FORCEINLINE TRef	GetDataTypeRef_String()				{	return TLBinary_TypeRef(String);	}
	FORCEINLINE TRef	GetDataTypeRef_WideString()			{	return TLBinary_TypeRef(WideString);	}
};




#define TLBinary_TypeRef_u8				TRef_Static2(u,EIGHT)
#define TLBinary_TypeRef_Type2_u8		TRef_Static4(u,EIGHT,UNDERSCORE,TWO)
#define TLBinary_TypeRef_Type3_u8		TRef_Static4(u,EIGHT,UNDERSCORE,THREE)
#define TLBinary_TypeRef_Type4_u8		TRef_Static4(u,EIGHT,UNDERSCORE,FOUR)
#define TLBinary_TypeRef_u16			TRef_Static3(u,ONE,SIX)
#define TLBinary_TypeRef_Type2_u16		TRef_Static5(u,ONE,SIX,UNDERSCORE,TWO)
#define TLBinary_TypeRef_Type3_u16		TRef_Static5(u,ONE,SIX,UNDERSCORE,THREE)
#define TLBinary_TypeRef_Type4_u16		TRef_Static5(u,ONE,SIX,UNDERSCORE,FOUR)
#define TLBinary_TypeRef_u32			TRef_Static3(u,THREE,TWO)
#define TLBinary_TypeRef_Type2_u32		TRef_Static5(u,THREE,TWO,UNDERSCORE,TWO)
#define TLBinary_TypeRef_Type3_u32		TRef_Static5(u,THREE,TWO,UNDERSCORE,THREE)
#define TLBinary_TypeRef_Type4_u32		TRef_Static5(u,THREE,TWO,UNDERSCORE,FOUR)
#define TLBinary_TypeRef_u64			TRef_Static3(u,SIX,FOUR)
#define TLBinary_TypeRef_Type2_u64		TRef_Static5(u,SIX,FOUR,UNDERSCORE,TWO)
#define TLBinary_TypeRef_Type3_u64		TRef_Static5(u,SIX,FOUR,UNDERSCORE,THREE)
#define TLBinary_TypeRef_Type4_u64		TRef_Static5(u,SIX,FOUR,UNDERSCORE,FOUR)
#define TLBinary_TypeRef_s8				TRef_Static2(s,EIGHT)
#define TLBinary_TypeRef_Type2_s8		TRef_Static4(s,EIGHT,UNDERSCORE,TWO)
#define TLBinary_TypeRef_Type3_s8		TRef_Static4(s,EIGHT,UNDERSCORE,THREE)
#define TLBinary_TypeRef_Type4_s8		TRef_Static4(s,EIGHT,UNDERSCORE,FOUR)
#define TLBinary_TypeRef_char			TLBinary_TypeRef_s8
#define TLBinary_TypeRef_Type2_char		TLBinary_TypeRef_Type2_s8
#define TLBinary_TypeRef_Type3_char		TLBinary_TypeRef_Type3_s8
#define TLBinary_TypeRef_Type4_char		TLBinary_TypeRef_Type4_s8
#define TLBinary_TypeRef_s16			TRef_Static3(s,ONE,SIX)
#define TLBinary_TypeRef_Type2_s16		TRef_Static5(s,ONE,SIX,UNDERSCORE,TWO)
#define TLBinary_TypeRef_Type3_s16		TRef_Static5(s,ONE,SIX,UNDERSCORE,THREE)
#define TLBinary_TypeRef_Type4_s16		TRef_Static5(s,ONE,SIX,UNDERSCORE,FOUR)
#define TLBinary_TypeRef_s32			TRef_Static3(s,THREE,TWO)
#define TLBinary_TypeRef_Type2_s32		TRef_Static5(s,THREE,TWO,UNDERSCORE,TWO)
#define TLBinary_TypeRef_Type3_s32		TRef_Static5(s,THREE,TWO,UNDERSCORE,THREE)
#define TLBinary_TypeRef_Type4_s32		TRef_Static5(s,THREE,TWO,UNDERSCORE,FOUR)
#define TLBinary_TypeRef_s64			TRef_Static3(s,SIX,FOUR)
#define TLBinary_TypeRef_Type2_s64		TRef_Static5(s,SIX,FOUR,UNDERSCORE,TWO)
#define TLBinary_TypeRef_Type3_s64		TRef_Static5(s,SIX,FOUR,UNDERSCORE,THREE)
#define TLBinary_TypeRef_Type4_s64		TRef_Static5(s,SIX,FOUR,UNDERSCORE,FOUR)
#define TLBinary_TypeRef_float			TRef_Static3(f,l,t)
#define TLBinary_TypeRef_Type2_float	TRef_Static4(f,l,t,TWO)
#define TLBinary_TypeRef_Type3_float	TRef_Static4(f,l,t,THREE)
#define TLBinary_TypeRef_Type4_float	TRef_Static4(f,l,t,FOUR)
#define TLBinary_TypeRef_float2			TLBinary_TypeNRef(Type2,float)
#define TLBinary_TypeRef_float3			TLBinary_TypeNRef(Type3,float)
#define TLBinary_TypeRef_float4			TLBinary_TypeNRef(Type4,float)
#define TLBinary_TypeRef_Bool			TRef_Static4(B,o,o,l)
#define TLBinary_TypeRef_Type2_Bool		TRef_Static(B,o,o,l,TWO)
#define TLBinary_TypeRef_Type3_Bool		TRef_Static(B,o,o,l,THREE)
#define TLBinary_TypeRef_Type4_Bool		TRef_Static(B,o,o,l,FOUR)


TLBinary_DeclareDataTypeRefAll( u8 );
TLBinary_DeclareDataTypeRefAll( u16 );
TLBinary_DeclareDataTypeRefAll( u32 );
TLBinary_DeclareDataTypeRefAll( u64 );
TLBinary_DeclareDataTypeRefAll( s8 );
TLBinary_DeclareDataTypeRefAllNoSize( char );	//	gr: pc compiler thinks this is a different type to s8...
TLBinary_DeclareDataTypeRefAll( s16 );
TLBinary_DeclareDataTypeRefAll( s32 );
TLBinary_DeclareDataTypeRefAll( s64 );
TLBinary_DeclareDataTypeRefAll( float );
TLBinary_DeclareDataTypeRefAll( Bool );




//	as an exception, we declare the TRef binary types here, but all other non-core types should put this in their own files
#define TLBinary_TypeRef_TRef			TRef_Static4(T,R,e,f)
#define TLBinary_TypeRef_Type2_TRef		TRef_Static(T,R,e,f,TWO)
#define TLBinary_TypeRef_Type3_TRef		TRef_Static(T,R,e,f,THREE)
#define TLBinary_TypeRef_Type4_TRef		TRef_Static(T,R,e,f,FOUR)
TLBinary_DeclareDataTypeRefAll( TRef );

#define TLBinary_TypeRef_TTypedRef			TRef_Static(T,y,R,e,f)
#define TLBinary_TypeRef_Type2_TTypedRef	TRef_Static(T,y,R,f,TWO)
#define TLBinary_TypeRef_Type3_TTypedRef	TRef_Static(T,y,R,f,THREE)
#define TLBinary_TypeRef_Type4_TTypedRef	TRef_Static(T,y,R,f,FOUR)
TLBinary_DeclareDataTypeRefAll( TTypedRef );


