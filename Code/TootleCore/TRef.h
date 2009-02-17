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


namespace TLRef
{
	//	gr: extern this if you want to use it... sorry!
//	TLArray::SortResult		RefSort(const TRef& aRef,const TRef& bRef,const void* pTestVal);	//	simple ref-sort func - for arrays of TRef's
}


//---------------------------------------------------------
//	ref type
//---------------------------------------------------------
class TRef
{
public:
	static const u32	g_CharsPerRef = 5;

public:
	TRef() 													:	m_Ref	( 0 )	{	}
	TRef(u32 Ref)											:	m_Ref	( Ref )	{	}
	TRef(const TRef& Ref)									:	m_Ref	( Ref.GetData() )	{	}
	TRef(const TString& RefString) 							:	m_Ref	( 0 )	{	Set( RefString );	}
	TRef(const TString* pRefString) 						:	m_Ref	( 0 )	{	if ( pRefString )	Set( *pRefString );	}
	TRef(const char* pRefString) 							:	m_Ref	( 0 )	{	Set( pRefString );	}
	TRef(const TArray<char>& RefStringChars)				:	m_Ref	( 0 )	{	Set( RefStringChars );	}

	void				SetInvalid()							{	m_Ref = 0;	}
	void				Set(u32 Ref)							{	m_Ref = Ref;	}
	void				Set(const TRef& Ref)					{	Set( Ref.GetData() );	}
	void				Set(const TString& RefString);			//	pull out 5 characters and set from this string
	void				Set(const char* pRefString);			//	pull out 5 characters and set from this string
	void				Set(const TArray<char>& RefStringChars);	//	set from array of 5 chars

	void				Increment();							//	increment the reference - don't just increment the u32 though! do it systematticly

	const u32&			GetData() const							{	return m_Ref;	}
	void				GetString(TString& RefString) const;	//	convert ref to a string
	Bool				IsValid() const;						//	check for invalid bits being set etc

	inline Bool			operator<(const TRef& Ref) const		{	return GetData() < Ref.GetData();	}
	inline void			operator=(u32 Ref)						{	Set( Ref );	}
	inline void			operator=(const TRef& Ref)				{	Set( Ref );	}
	inline void			operator=(const char* pString)			{	Set( pString );	}
	inline void			operator=(const TString& RefString)		{	Set( RefString );	}
	inline void			operator=(const TString* pRefString)	{	if ( pRefString ) Set( *pRefString ); else SetInvalid();	}

	inline Bool			operator==(const TRef& Ref) const		{	return GetData() == Ref.GetData();	}
	inline Bool			operator!=(const TRef& Ref) const		{	return GetData() != Ref.GetData();	}

private:
	u32					CharToRefAlphabet(const char& Char,u32 RefCharIndex=0);		//	convert a char to ref-alphabet bits
	u32					GetRefCharIndex(u32 Index) const;							//	pull out the alphabet index from a ref
	void				Set(const TArray<u32>& RefIndexes);		//	set from array of ref indexes

	void				Debug_TruncatedRefString(const TString& RefString);			//	debug that the specified string was truncated to fit in as a ref

protected:
	u32					m_Ref;
};



TLCore_DeclareIsDataType( TRef );
