/*------------------------------------------------------
	Generic reference type. simple, database-independant
	system to convert a u32 to a string and back.

	This way we can assign names to assets, windows, objects in general,
	devices etc etc 

	Pros:
	- Stores as u32
	- Therefore easy to sort, search, compare etc
	- very low memory consumption
	- Database independent
	- Converts to and from human-readable strings
	- Possible expansion to u64? if we need more characters...

	Cons:
	- Character set limitation	(26 letters + X symbols)
	- Character length limitation (Can only fit 5 characters into 32 bits)
	- Inefficient string conversion (though shouldn't be required often)

	Todo:
	- Speed up construction of a ref by making a sorted key array char->index
		and use it in GetRefCharIndex() to get rid of the loop

-------------------------------------------------------*/
#pragma once
#include "TLTypes.h"

//	forward decalrations
template<typename TYPE> class TArray;
class TString;
class TRef;
typedef const TRef& TRefRef;	//	TRefRef is just shorthand for passing ref's around

#define TLRef_CharIndexBitMask					(41-1)
#define TLRef_BitsPerRefChar					6	//	32bits / TRef::g_CharsPerRef
#define TLRef_StaticCharIndex(Char)				TLRef::StaticCharIndex::Index_##Char
#define TLRef_StaticOffsetChar(CharOffset,Char)	((u32)(TLRef_StaticCharIndex(Char) << (CharOffset*TLRef_BitsPerRefChar)))
#define TRef_InvalidValue						0
#define TRef_Invalid							TRef( (u32)TRef_InvalidValue )
#define TRef_Static1(a)							TRef_Static(a,SPACE,SPACE,SPACE,SPACE)
#define TRef_Static2(a,b)						TRef_Static(a,b,SPACE,SPACE,SPACE)
#define TRef_Static3(a,b,c)						TRef_Static(a,b,c,SPACE,SPACE)
#define TRef_Static4(a,b,c,d)					TRef_Static(a,b,c,d,SPACE)
#define TRef_Static5(a,b,c,d,e)					TRef_Static(a,b,c,d,e)
#define TRef_Static(a,b,c,d,e)					((u32)(TLRef_StaticOffsetChar(0,a)|TLRef_StaticOffsetChar(1,b)|TLRef_StaticOffsetChar(2,c)|TLRef_StaticOffsetChar(3,d)|TLRef_StaticOffsetChar(4,e)))
#define STRef1(a)								TRef_Static1(a)
#define STRef2(a,b)								TRef_Static2(a,b)
#define STRef3(a,b,c)							TRef_Static3(a,b,c)
#define STRef4(a,b,c,d)							TRef_Static4(a,b,c,d)
#define STRef5(a,b,c,d,e)						TRef_Static5(a,b,c,d,e)
#define STRef(a,b,c,d,e)						TRef_Static(a,b,c,d,e)

namespace TLRef
{
	//	gr: extern this if you want to use it... sorry!
//	TLArray::SortResult		RefSort(const TRef& aRef,const TRef& bRef,const void* pTestVal);	//	simple ref-sort func - for arrays of TRef's
	extern u32				g_InvalidRefMask;

	// Use the g_InvalidRefMask to validate a tref ensuring it is valid
	FORCEINLINE s32 CreateValidTRef(s32 sValue)		{ return (sValue&(~g_InvalidRefMask)); }

	namespace StaticCharIndex
	{
		//	const char	g_RefCharTable[g_RefCharTable_Size+1]		= {	" abcdefghijklmnopqrstuvwxyz0123456789?-#_"	};
		//	const char	g_RefCharTableAlt[g_RefCharTable_Size+1]	= {	" ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789?-#_"	};
		enum
		{
			Index_SPACE = 0,
			Index_a = 1,
			Index_b,
			Index_c,
			Index_d,
			Index_e,
			Index_f,
			Index_g,
			Index_h,
			Index_i,
			Index_j,
			Index_k,
			Index_l,
			Index_m,
			Index_n,
			Index_o,
			Index_p,
			Index_q,
			Index_r,
			Index_s,
			Index_t,
			Index_u,
			Index_v,
			Index_w,
			Index_x,
			Index_y,
			Index_z,
			Index_A = 1,
			Index_B,
			Index_C,
			Index_D,
			Index_E,
			Index_F,
			Index_G,
			Index_H,
			Index_I,
			Index_J,
			Index_K,
			Index_L,
			Index_M,
			Index_N,
			Index_O,
			Index_P,
			Index_Q,
			Index_R,
			Index_S,
			Index_T,
			Index_U,
			Index_V,
			Index_W,
			Index_X,
			Index_Y,
			Index_Z,
			Index_ZERO, 
			Index_ONE, 
			Index_TWO, 
			Index_THREE, 
			Index_FOUR, 
			Index_FIVE, 
			Index_SIX, 
			Index_SEVEN, 
			Index_EIGHT, 
			Index_NINE,
			Index_QUESTION, 
			Index_DASH, 
			Index_HASH, 
			Index_UNDERSCORE
		};
	}

}




//---------------------------------------------------------
//	ref type
//---------------------------------------------------------
class TRef
{
public:
	static const u32	g_CharsPerRef = 5;

public:
	TRef() 													:	m_Ref	( TRef_InvalidValue )	{	}
	TRef(u32 Ref)											:	m_Ref	( Ref )				{	Debug_IsValid();	}	//	gr: call IsValid() to check for invalid values
	TRef(const TRef& Ref)									:	m_Ref	( Ref.GetData() )	{	}
	TRef(const TString& RefString) 							:	m_Ref	( TRef_InvalidValue )	{	Set( RefString );	}
	TRef(const TString* pRefString) 						:	m_Ref	( TRef_InvalidValue )	{	if ( pRefString )	Set( *pRefString );	}
	TRef(const char* pRefString) 							:	m_Ref	( TRef_InvalidValue )	{	Set( pRefString );	}
	TRef(const TArray<char>& RefStringChars)				:	m_Ref	( TRef_InvalidValue )	{	Set( RefStringChars );	}

	void				SetInvalid()							{	m_Ref = TRef_InvalidValue;	}
	void				Set(u32 Ref)							{	m_Ref = Ref;	Debug_IsValid();	}	//	gr: call IsValid() to check for invalid values
	void				Set(const TRef& Ref)					{	Set( Ref.GetData() );	}
	void				Set(const TString& RefString);			//	pull out 5 characters and set from this string
	void				Set(const char* pRefString);			//	pull out 5 characters and set from this string
	void				Set(const TArray<char>& RefStringChars);	//	set from array of 5 chars

	const TRef&			Increment();							//	increment the reference - don't just increment the u32 though! do it systematticly - returns itself so you can construct another TRef from the incremented version of this

	const u32&					GetData() const							{	return m_Ref;	}
	void						GetString(TString& RefString,Bool Capitalise=FALSE) const;	//	convert ref to a string
	FORCEINLINE void			Debug_IsValid() const;					//	do a debug break on invalid refs - no release runtime functionality (use the normal one if you're checking IsValid)
	FORCEINLINE Bool			IsValid() const;						//	check for invalid bits being set etc

	FORCEINLINE Bool			operator<(const TRef& Ref) const		{	return GetData() < Ref.GetData();	}
	FORCEINLINE Bool			operator>(const TRef& Ref) const		{	return GetData() > Ref.GetData();	}
	FORCEINLINE void			operator=(u32 Ref)						{	Set( Ref );	}
	FORCEINLINE void			operator=(const TRef& Ref)				{	Set( Ref );	}
	FORCEINLINE void			operator=(const char* pString)			{	Set( pString );	}
	FORCEINLINE void			operator=(const TString& RefString)		{	Set( RefString );	}
	FORCEINLINE void			operator=(const TString* pRefString)	{	if ( pRefString ) Set( *pRefString ); else SetInvalid();	}

	FORCEINLINE Bool			operator==(const TRef& Ref) const		{	return GetData() == Ref.GetData();	}
	FORCEINLINE Bool			operator!=(const TRef& Ref) const		{	return GetData() != Ref.GetData();	}

private:
	u32					CharToRefAlphabet(const char& Char,u32 RefCharIndex=0);		//	convert a char to ref-alphabet bits
	u32					GetRefCharIndex(u32 Index) const;							//	pull out the alphabet index from a ref
	void				Set(const TArray<u32>& RefIndexes);		//	set from array of ref indexes

	void				Debug_TruncatedRefString(const TString& RefString) const;	//	debug that the specified string was truncated to fit in as a ref
	void				Debug_BreakInvalidRef() const;								//	break with invalid ref message

protected:
	u32					m_Ref;
};




//---------------------------------------------------------
//	check for invalid bits being set etc
//---------------------------------------------------------
FORCEINLINE void TRef::Debug_IsValid() const
{
#ifdef _DEBUG
	//	break if ref is invalid - probably invalidly-generated
	if ( (m_Ref & TLRef::g_InvalidRefMask) != 0x0 )
	{
		Debug_BreakInvalidRef();
	}
#endif
}

//---------------------------------------------------------
//	check for invalid bits being set etc
//---------------------------------------------------------
FORCEINLINE Bool TRef::IsValid() const
{
	//	not valid if zero
	if ( m_Ref == TRef_InvalidValue )
		return FALSE;

	//	check for no invalid bits
	return (m_Ref & TLRef::g_InvalidRefMask) == 0x0;
}






//------------------------------------------------------------
//	This is a pair of refs. Commonly used to put a ref and a specific TypeRef together
//	we could re-use this class for anything that needs two refs, or even a double-sized ref (instead of a 64bit one)
//------------------------------------------------------------
class TTypedRef
{
public:
	TTypedRef()																											{};
	TTypedRef(const TTypedRef& TypedRef) :		m_Ref ( TypedRef.GetRef() ),	m_TypeRef ( TypedRef.GetTypeRef() )		{};
	TTypedRef(TRefRef Ref,TRefRef TypeRef) :	m_Ref ( Ref ),					m_TypeRef ( TypeRef )					{};

	FORCEINLINE TRefRef		GetRef() const						{	return m_Ref;	}
	FORCEINLINE TRefRef		GetTypeRef() const					{	return m_TypeRef;	}

	FORCEINLINE void		Set(const TTypedRef& TypedRef)		{	m_Ref = TypedRef.GetRef();	m_TypeRef = TypedRef.GetTypeRef();	}
	FORCEINLINE void		SetRef(TRefRef Ref)					{	m_Ref = Ref;	}
	FORCEINLINE void		SetTypeRef(TRefRef TypeRef)			{	m_TypeRef = TypeRef;	}

	FORCEINLINE Bool		operator<(const TTypedRef& TypedRef) const;
	FORCEINLINE Bool		operator==(const TTypedRef& TypedRef) const	{	return (GetRef() == TypedRef.GetRef()) && (GetTypeRef()==TypedRef.GetTypeRef());	}
	FORCEINLINE Bool		operator!=(const TTypedRef& TypedRef) const	{	return (GetRef() != TypedRef.GetRef()) || (GetTypeRef()!=TypedRef.GetTypeRef());	}

private:
	TRef		m_Ref;
	TRef		m_TypeRef;
};


FORCEINLINE Bool TTypedRef::operator<(const TTypedRef& TypedRef) const	
{	
	//	check main ref first
	if ( GetRef() < TypedRef.GetRef() )			
		return TRUE;
	else if ( GetRef() > TypedRef.GetRef() )	
		return FALSE;
	
	//	main ref matches, so check secondary ref
	return ( GetTypeRef() < TypedRef.GetTypeRef() );
}






TLCore_DeclareIsDataType( TRef );
TLCore_DeclareIsDataType( TTypedRef );


