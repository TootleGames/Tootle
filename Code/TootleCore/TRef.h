//	test4
/*------------------------------------------------------
	Generic reference type. simple, database-independant
	system to convert a u32 to a string and back.

	This way we can assign names to assets, windows, objects in general,
	devices etc etc 

	Pros:
	+ Stores as u32
		+ Therefore easy to sort, search, compare etc
	+ very low memory consumption
	+ Database independent
	+ Converts to and from human-readable strings
	+ Possible expansion to u64? if we need more characters...

	Cons:
	- Character set limitation	(26 letters + X symbols)
		+ though conviniently can match the url enc character set
	- Character length limitation (Can only fit 5 characters into 32 bits)
		+ would be fixed with 64bit implementation
	- Inefficient string conversion (though shouldn't be required often)
		+ fixed via the static refs
	- Can't get a string from a u32 whilst debugging

-------------------------------------------------------*/
#pragma once
#include "TLTypes.h"
#include "TArray.h"
#include "TString.h" //gr: crap, but need to include this for the << template specialisation, wonder if I can declare it to get around this....ß

class TString;
class TRef;
class TTypedRef;
typedef const TRef& TRefRef;			//	TRefRef is just shorthand for passing ref's around
typedef const TTypedRef& TTypedRefRef;

#define TLRef_AlphabetSizeMax					(1<<(TLRef_BitsPerChar))	//	maximum in theory: 64
#define TLRef_AlphabetSize						41							//	actual number of characters in the alphabet.
#define TLRef_CharIndexBitMask					(TLRef_AlphabetSizeMax-1)	//	mask for the maximum index (in theory, the index could be greater than the alphabet size still)
#define TLRef_CharsPerRef						5
#define TLRef_BitsPerChar						6							//	32bits / TLRef_CharsPerRef

#define TRef_InvalidValue						0
#define TRef_Invalid							TRef( (u32)TRef_InvalidValue )

#define TLRef_StaticCharIndex(Char)				TLRef::StaticCharIndex::Index_##Char
#define TLRef_StaticOffsetChar(CharOffset,Char)	((u32)(TLRef_StaticCharIndex(Char) << (CharOffset*TLRef_BitsPerChar)))

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
	extern u32				g_InvalidRefMask;
	void					GenerateCharLookupTable();

	// Use the g_InvalidRefMask to validate a tref ensuring it is valid
	FORCEINLINE u32			GetValidTRef(u32 Value)		{	return (Value&(~g_InvalidRefMask));	}

	namespace StaticCharIndex
	{
		//	these indexes should match up to the alphabet indexes for the appropriate character
		enum
		{
			Index_SPACE = 0,
			Index_ASERISK = 0,
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
			Index_APOSTRAPHE, 
			Index_DASH, 
			Index_EXCLAMATION, 
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
	TRef() 													:	m_Ref	( TRef_InvalidValue )	{	}
	TRef(u32 Ref)											:	m_Ref	( Ref )				{	Debug_IsValid();	}	//	gr: call IsValid() to check for invalid values
	TRef(const TRef& Ref)									:	m_Ref	( Ref.GetData() )	{	}
	TRef(const TString& RefString) 							:	m_Ref	( TRef_InvalidValue )	{	Set( RefString );	}
	TRef(const TString* pRefString) 						:	m_Ref	( TRef_InvalidValue )	{	if ( pRefString )	Set( *pRefString );	}
	TRef(const char* pRefString) 							:	m_Ref	( TRef_InvalidValue )	{	Set( pRefString );	}
	TRef(const TArray<char>& RefStringChars)	:	m_Ref	( TRef_InvalidValue )	{	Set( RefStringChars );	}

	FORCEINLINE void			SetInvalid()							{	m_Ref = TRef_InvalidValue;	}
	FORCEINLINE void			Set(u32 Ref)							{	m_Ref = Ref;	Debug_IsValid();	}	//	gr: call IsValid() to check for invalid values
	FORCEINLINE void			Set(const TRef& Ref)					{	Set( Ref.GetData() );	}
	void						Set(const TString& RefString);			//	pull out 5 characters and set from this string
	void						Set(const char* pRefString);			//	pull out 5 characters and set from this string
	void						Set(const TArray<char>& RefStringChars);	//	set from array of 5 chars

	const TRef&					Increment();							//	increment the reference - don't just increment the u32 though! do it systematticly - returns itself so you can construct another TRef from the incremented version of this

	FORCEINLINE const u32&		GetData() const							{	return m_Ref;	}
	void						GetString(TString& RefString,bool Capitalise=FALSE,bool Trim=FALSE,bool UrlSafe=FALSE) const;	//	convert ref to a string. if Trim then any white spaces are removed at the end of the string. If UrlSafe then we use the alternative symbol for Space instead of a space so the string doesnt need to be url encoded
	FORCEINLINE void			GetUrlString(TString& RefString) const	{	GetString( RefString, FALSE, FALSE, TRUE );	}		//	get url-safe ref as a string
	FORCEINLINE void			Debug_IsValid() const;					//	do a debug break on invalid refs - no release runtime functionality (use the normal one if you're checking IsValid)
	FORCEINLINE bool			IsValid() const;						//	check for invalid bits being set etc

	FORCEINLINE bool			operator<(const TRef& Ref) const		{	return GetData() < Ref.GetData();	}
	FORCEINLINE bool			operator>(const TRef& Ref) const		{	return GetData() > Ref.GetData();	}
	FORCEINLINE TRef&			operator=(u32 Ref)						{	Set( Ref );	return *this;	}
	FORCEINLINE TRef&			operator=(const TRef& Ref)				{	Set( Ref );	return *this;	}
	FORCEINLINE TRef&			operator=(const char* pString)			{	Set( pString );	return *this;	}
	FORCEINLINE TRef&			operator=(const TString& RefString)		{	Set( RefString );	return *this;	}
	FORCEINLINE TRef&			operator=(const TString* pRefString)	{	if ( pRefString ) Set( *pRefString ); else SetInvalid();	return *this;	}

	FORCEINLINE bool			operator==(const TRef& Ref) const		{	return GetData() == Ref.GetData();	}
	FORCEINLINE bool			operator!=(const TRef& Ref) const		{	return GetData() != Ref.GetData();	}

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
FORCEINLINE bool TRef::IsValid() const
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

	FORCEINLINE bool		IsValid() const						{	return GetRef().IsValid() && GetTypeRef().IsValid();	}

	FORCEINLINE TRef&		GetRef()							{	return m_Ref;	}
	FORCEINLINE TRefRef		GetRef() const						{	return m_Ref;	}
	FORCEINLINE TRef&		GetTypeRef()						{	return m_TypeRef;	}
	FORCEINLINE TRefRef		GetTypeRef() const					{	return m_TypeRef;	}
	DEPRECATED void			GetString(TString& RefString,bool Capitalise) const;
	void					GetString(TString& RefString) const;

	void					Set(const TString& RefString);		//	set from string in the same format as GetString
	FORCEINLINE void		Set(const TTypedRef& TypedRef)		{	m_Ref = TypedRef.GetRef();	m_TypeRef = TypedRef.GetTypeRef();	}
	FORCEINLINE void		SetRef(TRefRef Ref)					{	m_Ref = Ref;	}
	FORCEINLINE void		SetTypeRef(TRefRef TypeRef)			{	m_TypeRef = TypeRef;	}

	FORCEINLINE bool		operator<(const TTypedRef& TypedRef) const;
	FORCEINLINE bool		operator==(const TTypedRef& TypedRef) const	{	return (GetRef() == TypedRef.GetRef()) && (GetTypeRef()==TypedRef.GetTypeRef());	}
	FORCEINLINE bool		operator!=(const TTypedRef& TypedRef) const	{	return (GetRef() != TypedRef.GetRef()) || (GetTypeRef()!=TypedRef.GetTypeRef());	}
	FORCEINLINE TTypedRef&	operator=(const TString& RefString)		{	Set( RefString );	return *this;	}

private:
	TRef		m_Ref;
	TRef		m_TypeRef;
};


FORCEINLINE bool TTypedRef::operator<(const TTypedRef& TypedRef) const	
{	
	//	check main ref first
	if ( GetRef() < TypedRef.GetRef() )			
		return TRUE;
	else if ( GetRef() > TypedRef.GetRef() )	
		return FALSE;
	
	//	main ref matches, so check secondary ref
	return ( GetTypeRef() < TypedRef.GetTypeRef() );
}


//--------------------------------------------------------
//	specialised TString append operators
//--------------------------------------------------------
template<> FORCEINLINE TString& operator<<(TString& String,const TRef& Ref)			{	Ref.GetString( String );	return String;	}	
template<> FORCEINLINE TString& operator<<(TString& String,const Type2<TRef>& v)	{	return TLString::AppendType2(String,v);	}
template<> FORCEINLINE TString& operator<<(TString& String,const Type3<TRef>& v)	{	return TLString::AppendType3(String,v);	}
template<> FORCEINLINE TString& operator<<(TString& String,const Type4<TRef>& v)	{	return TLString::AppendType4(String,v);	}
template<> FORCEINLINE TString& operator<<(TString& String,const TTypedRef& Ref)	{	Ref.GetString( String );	return String;	}	
template<> FORCEINLINE TString& operator<<(TString& String,const Type2<TTypedRef>& v)	{	return TLString::AppendType2(String,v);	}
template<> FORCEINLINE TString& operator<<(TString& String,const Type3<TTypedRef>& v)	{	return TLString::AppendType3(String,v);	}
template<> FORCEINLINE TString& operator<<(TString& String,const Type4<TTypedRef>& v)	{	return TLString::AppendType4(String,v);	}

template<> FORCEINLINE void operator>>(const TString& String,TRef& Ref)			{	Ref = String;	}
template<> FORCEINLINE void operator>>(const TString& String,TTypedRef& Ref)	{	Ref = String;	}	


