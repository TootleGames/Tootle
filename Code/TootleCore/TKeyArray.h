/*------------------------------------------------------
	
	Key array. Allows you to access/reference elements by
	non sequential numbers/types
	so you can have

	Array[100] = 'x';
	Array[2] = 'a';

	and only contain two elements

-------------------------------------------------------*/
#pragma once
#include "TArray.h"
#include "TPtr.h"


//	namespace only used by TKeyArray
namespace TLKeyArray
{
	template<typename KEYTYPE,typename TYPE>
	class TPair
	{
	public:
		TPair()											{}		//	note: members are not initialised, but required for the array mem alloc
		TPair(const KEYTYPE& Key,const TYPE& Item) : 
			m_Key	( Key ),
			m_Item	( Item )
		{
		}

		inline Bool	operator<(const KEYTYPE& Key) const					{	return m_Key < Key;	}
		inline Bool	operator<(const TPair<KEYTYPE,TYPE>& Pair) const	{	return m_Key < Pair.m_Key;	}
		inline void	operator=(const TPair<KEYTYPE,TYPE>& Pair)			{	m_Key = Pair.m_Key;	m_Item = Pair.m_Item;	}
		inline Bool	operator==(const KEYTYPE& Key) const				{	return m_Key == Key;	}
		inline Bool	operator==(const TPair<KEYTYPE,TYPE>& Pair) const	{	return (m_Key == Pair.m_Key) && (m_Item == Pair.m_Item);	}

	public:
		KEYTYPE		m_Key;
		TYPE		m_Item;
	};


	//	sort function for key arrays	
	template<typename KEYTYPE,typename TYPE>
	TLArray::SortResult KeyArraySort(const TLKeyArray::TPair<KEYTYPE,TYPE>& PairA,const TLKeyArray::TPair<KEYTYPE,TYPE>& PairB,const void* pVal);
};



//-------------------------------------------------------
//	declare the Pair type as a Data type if both key and item are data types
//-------------------------------------------------------
/*
namespace TLCore
{
	template<>
	template<typename KEYTYPE,typename TYPE>
	FORCEINLINE Bool IsDataType<TLKeyArray::TPair<KEYTYPE,TYPE> >()	
	{
		return TLCore::IsDataType<KEYTYPE>() && TLCore::IsDataType<TYPE>();	
	}
}
*/

//-------------------------------------------------------
//	key array type. access everything via your key (could be a
//	integer, float or specific other types)
//	there is only every 1 instance of a key in the array
//-------------------------------------------------------
template<typename KEYTYPE,typename TYPE>
class TKeyArray
{
public:
	typedef TLKeyArray::TPair<KEYTYPE,TYPE> PAIRTYPE;

public:
	TKeyArray() : m_Array( &TLKeyArray::KeyArraySort<KEYTYPE,TYPE> )						{	}

	FORCEINLINE u32				GetSize() const							{	return m_Array.GetSize();	}

	FORCEINLINE void			Empty(Bool Dealloc=FALSE)				{	m_Array.Empty( Dealloc );	}
	FORCEINLINE void			SetAllocSize(u32 Size)					{	m_Array.SetAllocSize( Size );	}
	FORCEINLINE void			SetGrowBy(u16 GrowBy)					{	m_Array.SetGrowBy( GrowBy );	}

	FORCEINLINE TYPE*			Find(const KEYTYPE& Key)				{	PAIRTYPE* pPair = m_Array.Find( Key );	return pPair ? &pPair->m_Item : NULL;	}
	FORCEINLINE const TYPE*		Find(const KEYTYPE& Key) const			{	const PAIRTYPE* pPair = m_Array.FindConst( Key );	return pPair ? &pPair->m_Item : NULL;	}
	FORCEINLINE TYPE&			ElementAt(u32 Index)					{	return GetPairAt( Index ).m_Item;	}
	FORCEINLINE const TYPE&		ElementAt(u32 Index) const				{	return GetPairAt( Index ).m_Item;	}
	FORCEINLINE PAIRTYPE&		GetPairAt(u32 Index)					{	return m_Array.ElementAt(Index);	}
	FORCEINLINE const PAIRTYPE&	GetPairAt(u32 Index) const				{	return m_Array.ElementAtConst(Index);	}
	FORCEINLINE TYPE&			ElementLast()							{	return ElementAt( m_Array.GetLastIndex() );	};
	FORCEINLINE const TYPE&		ElementLastConst() const				{	return ElementAtConst( m_Array.GetLastIndex() );	};
	FORCEINLINE Bool			Exists(const KEYTYPE& Key)				{	return m_Array.Exists( Key );	}
	FORCEINLINE Bool			Exists(const KEYTYPE& Key) const		{	return m_Array.Exists( Key );	}

	const KEYTYPE*				FindKey(const TYPE& Item,const KEYTYPE* pPreviousMatch=NULL) const;	//	find the key for a pair matching this item
	TYPE*						Add(const KEYTYPE& Key,const TYPE& Item,Bool Overwrite=TRUE);	//	add an item with this key. if the key already exists the item is overwritten if specified, returns the item for the key, overwritten or not. NULL if failed
	TYPE*						AddNew(const KEYTYPE& Key);				//	add a new key and un-set item to the list - returns NULL if the key already exists. returns the item for the key
	Bool						Remove(const KEYTYPE& Key);				//	remove the item with this key. returns if anything was removed
	Bool						RemoveItem(const TYPE& Item,Bool RemoveAllMatches=FALSE);	//	remove matching item. (for when we dont know the key) returns if anything was removed

protected:
	TArray<TLKeyArray::TPair<KEYTYPE,TYPE> >		m_Array;		//	array of pairs
};			



template<typename KEYTYPE,typename TYPE>
TLArray::SortResult	TLKeyArray::KeyArraySort(const TLKeyArray::TPair<KEYTYPE,TYPE>& PairA,const TLKeyArray::TPair<KEYTYPE,TYPE>& PairB,const void* pVal)
{
	const KEYTYPE& Key = PairA.m_Key;
	const KEYTYPE& OtherKey = pVal ? *((const KEYTYPE*)pVal) : PairB.m_Key;
	return Key < OtherKey ? TLArray::IsLess : (TLArray::SortResult)(Key == OtherKey);
}



//-------------------------------------------------------
//	specialised key array for TPtr's that returns empty TPtr 
//	when failing to find things, rather than NULL pointers to TPtr's
//-------------------------------------------------------
template<typename KEYTYPE,typename TYPE>
class TPtrKeyArray : public TKeyArray<KEYTYPE,TPtr<TYPE> >
{
private:
	typedef TLKeyArray::TPair<KEYTYPE,TPtr<TYPE> > PAIRTYPE;

public:
	TPtr<TYPE>&			FindPtr(const KEYTYPE& Key)				{	PAIRTYPE* pPair = TKeyArray<KEYTYPE,TPtr<TYPE> >::m_Array.Find( Key );			return pPair ? pPair->m_Item : TLPtr::GetNullPtr<TYPE>();	}
	const TPtr<TYPE>&	FindPtr(const KEYTYPE& Key) const		{	const PAIRTYPE* pPair = TKeyArray<KEYTYPE,TPtr<TYPE> >::m_Array.Find( Key );	return pPair ? pPair->m_Item : TLPtr::GetNullPtr<TYPE>();	}
};			



//-------------------------------------------------------
//	find the key for a pair matching this item
//-------------------------------------------------------
template<typename KEYTYPE,typename TYPE>
const KEYTYPE* TKeyArray<KEYTYPE,TYPE>::FindKey(const TYPE& Item,const KEYTYPE* pPreviousMatch) const
{
	Bool FoundLastMatch = pPreviousMatch ? FALSE : TRUE;
	
	//	loop through the list and match item
	for ( u32 i=0;	i<m_Array.GetSize();	i++ )
	{
		const PAIRTYPE& Pair = m_Array[i];

		//	not this one, next!
		if ( Pair.m_Item != Item )
			continue;

		//	found the item, if we're not hunting for the previous match then return it
		if ( FoundLastMatch )
			return &Pair.m_Key;

		//	looking for previous match... is this it?
		if ( pPreviousMatch == &Pair.m_Key )
			FoundLastMatch = TRUE;
	}

	//	none/no more found
	return NULL;
}


//-------------------------------------------------------
//	add an item with this key. if the key already exists the item is overwritten. 
//	returns if the item was NEW. if false is returned the key's item was overwritten
//-------------------------------------------------------
template<typename KEYTYPE,typename TYPE>
TYPE* TKeyArray<KEYTYPE,TYPE>::Add(const KEYTYPE& Key,const TYPE& Item,Bool Overwrite)
{
	//	have we already got a pair for this key?
	PAIRTYPE* pPair = m_Array.Find( Key );

	//	update the item in the pair and return FALSE for no change
	if ( pPair )
	{
		if ( Overwrite )
			pPair->m_Item = Item;

		return &pPair->m_Item;
	}

	//	add new pair
	s32 AddIndex = m_Array.Add( PAIRTYPE( Key, Item ) );
	if ( AddIndex == -1 )
		return NULL;

	return &ElementAt( AddIndex );
}


//-------------------------------------------------------
//	add a new key and un-set item to the list - returns NULL if the key already exists. returns the item for the key
//-------------------------------------------------------
template<typename KEYTYPE,typename TYPE>
TYPE* TKeyArray<KEYTYPE,TYPE>::AddNew(const KEYTYPE& Key)
{
	//	have we already got a pair for this key?
	PAIRTYPE* pPair = m_Array.Find( Key );
	if ( pPair )
		return NULL;

	//	add new pair
	s32 AddIndex = m_Array.Add( PAIRTYPE( Key, TYPE() ) );
	if ( AddIndex == -1 )
		return NULL;

	return &ElementAt( AddIndex );
}


//-------------------------------------------------------
//	remove the item with this key. returns if anything was removed
//-------------------------------------------------------
template<typename KEYTYPE,typename TYPE>
Bool TKeyArray<KEYTYPE,TYPE>::Remove(const KEYTYPE& Key)
{
	//	get the index for the existing pair
	s32 PairIndex = m_Array.FindIndex( Key );

	//	key doesnt exist
	if ( PairIndex == -1 )
		return FALSE;

	//	remove from array
	m_Array.RemoveAt( PairIndex );

	return TRUE;
}




//-------------------------------------------------------
//	remove matching item. (for when we dont know the key) returns if anything was removed
//-------------------------------------------------------
template<typename KEYTYPE,typename TYPE>
Bool TKeyArray<KEYTYPE,TYPE>::RemoveItem(const TYPE& Item,Bool RemoveAllMatches)
{
	Bool AnyRemoved = FALSE;

	//	loop through the list and remove matched items
	for ( s32 i=m_Array.GetLastIndex();	i>=0;	i-- )
	{
		const PAIRTYPE& Pair = m_Array[i];

		//	not this one, next!
		if ( Pair.m_Item != Item )
			continue;

		//	is a match, remove
		m_Array.RemoveAt( i );

		AnyRemoved |= TRUE;

		//	only remove first match
		if ( !RemoveAllMatches )
			break;
	}

	return AnyRemoved;
}

