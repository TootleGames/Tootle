/*
 *  ArraySort.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 26/10/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once


//	forward declaration
template<typename TYPE>
class TArray;


//---------------------------------------------
//	this is a stub class to specialise the < operator for different
//	sort policies. (== is not required, we only check for less-than)
//	this allows you to specify the sort-comparison in the class against different types
//	
//	the This object in the TSorter is another element in the array.
//
//	class TSceneNode
//	{
//		bool	operator<(const TSorter<TSceneNode,TRef>& That)			{	return GetNodeRef() < That.This().GetNodeRef();	}	//	sort by ref
//		bool	operator<(const TSorter<TSceneNode,TSceneNode*>& That)	{	return this < That.This();	}						//	sort by pointer
//	}
//---------------------------------------------
template<typename TYPE,typename MATCHTYPE>
class TSorter
{
public:
	TSorter(const TYPE& This) :
		pThis	( &This )
	{
	}
	
	FORCEINLINE const TYPE&		This() const					{	return *pThis;	}
	FORCEINLINE					operator const TYPE*() const	{	return pThis;	}
	FORCEINLINE const TYPE*		operator->() const				{	return pThis;	}

private:
	const TYPE*	pThis;		//	never null
};


//------------------------------------------------------------
//	default operator for simple cases (eg. float vs float)
//------------------------------------------------------------
template<typename TYPE>
FORCEINLINE bool	operator<(const TYPE& This,const TSorter<TYPE,TYPE>& That)
{
	return (This < *That);
}


//---------------------------------------------------------
//	simple sort operator for pointers
//---------------------------------------------------------
template<typename TYPE>
FORCEINLINE bool	operator<(const TYPE* pThis,const TSorter<TYPE*,TYPE*>& That)
{
	//	null first rule...
	
	if ( !pThis && !That.This() )		return false;	//	both null
	if ( !pThis && That.This() )		return true;	//	this first
	if ( pThis && !That.This() )		return false;	//	that first
	
	//	gr: replace with just < That ?
	return (*pThis) < (*That.This());
}


//-------------------------------------------------------
//	simple pointer match
//	gr: this one may be redundant...
//-------------------------------------------------------
template<typename TYPE>
inline bool operator==(const TSorter<TYPE*,TYPE*>& Sorter,const TYPE* pMatch)
{
	return Sorter.This() == pMatch;
}


//-------------------------------------------------------
//	simple wrapper for sorter on a pointer
//-------------------------------------------------------
template<typename TYPE,typename MATCHTYPE>
inline bool operator==(const TSorter<TYPE*,MATCHTYPE>& Sorter,const MATCHTYPE& Match)
{
	const TYPE* pThis = (*Sorter);
	if ( !pThis )
		return false;
	
	return (*pThis) == Match;
}





namespace TLArray
{
	namespace TSortOrder
	{
		enum Type
		{
			Ascending = 0,
			Descending,
		};
	}
}



//----------------------------------------------------------------------------//
//	Base iterator. TArray only knows of this type.
//	this is a functoid.
//----------------------------------------------------------------------------//
template<typename TYPE>
class TIterator
{
public:
	virtual s32	FindIndex(const void* pMatch,const TYPE* pData,u32 FirstIndex,u32 Elements)const=0;
};


//----------------------------------------------------------------------------//
//	simple iterator which just walks through the array and does a plain == to match
//	gr; does this need to use ==(TSorter<TYPE,MATCH>) ?
//----------------------------------------------------------------------------//
template<typename TYPE,typename MATCH=TYPE>
class TIteratorWalk : public TIterator<TYPE>
{
public:
	virtual s32	FindIndex(const void* pMatch,const TYPE* pData,u32 FirstIndex,u32 Elements) const	{	return FindIndex( *(const MATCH*)pMatch, pData, FirstIndex, Elements );	}
	s32			FindIndex(const MATCH& Match,const TYPE* pData,u32 FirstIndex,u32 Elements) const;
};



//----------------------------------------------------------------------------//
//	binary chop iterator. The array MUST be sorted beforehand. We cannot store the sorted state 
//	here as it's a wrapper class (can't remember proper terminology) and not instanced
//----------------------------------------------------------------------------//
template<typename TYPE,typename MATCH=TYPE>
class TIteratorChop : public TIterator<TYPE>
{
public:
	virtual s32					FindIndex(const void* pMatch,const TYPE* pData,u32 FirstIndex,u32 Elements) const	{	return FindIndex( *(const MATCH*)pMatch, pData, FirstIndex, Elements );	}
	s32							FindIndex(const MATCH& Match,const TYPE* pData,u32 FirstIndex,u32 Elements) const;
	TLArray::TSortOrder::Type	GetSortOrder() const	{	return TLArray::TSortOrder::Ascending;	}

private:
	s32							BinaryChop(const MATCH& Match,u32 Low,s32 High,const TYPE* pData,const u32 Size) const;
};



//----------------------------------------------------------------------------//
//	simple walk iterator
//----------------------------------------------------------------------------//
template<typename TYPE,typename MATCH>
s32 TIteratorWalk<TYPE,MATCH>::FindIndex(const MATCH& Match,const TYPE* pData,u32 FirstIndex,u32 Elements) const
{
	for ( u32 i=FirstIndex;	i<Elements;	i++ )
	{
		if ( pData[i] == Match )
			return i;
	}
	return -1;
}


//----------------------------------------------------------------------------//
//	binary chop iterator
//----------------------------------------------------------------------------//
template<typename TYPE,typename MATCH>
s32 TIteratorChop<TYPE,MATCH>::FindIndex(const MATCH& Match,const TYPE* pData,u32 FirstIndex,u32 Elements) const
{
	if ( Elements == 0 )
		return -1;
	
	s32 Index = BinaryChop( Match, 0, Elements, pData, Elements );
	
	#if defined(ARRAY_SORT_CHECK)
	{
		if ( Index == -1 )
		{
			for ( u32 i=0;	i<Elements;	i++ )
			{
				if ( pData[i] == Match )
				{
					TLDebug_Break("Binary chop (or sort) broken. item found.");
					return Index;
				}
			}
		}
	}
	#endif
	return Index;
}


//----------------------------------------------------------------------------//
//	binary search method (recursive)
//----------------------------------------------------------------------------//
template<typename TYPE,typename MATCH>
s32 TIteratorChop<TYPE,MATCH>::BinaryChop(const MATCH& Match,u32 Low,s32 High,const TYPE* pData,const u32 Size) const
{
	if ( High < (s32)Low )
		return -1;
	
	u32 Mid = (Low + (u32)High) / 2;
	
	//	gr: i think the algorithm is wrong somewhere, array of size 3 gets here with low and high == 3, mid becomes 3, which is out of range!
	if ( Mid >= Size )
		return -1;
	
	//	see if we've found the element...
	const TYPE& MidElement = pData[Mid];
	if ( MidElement == Match )
		return Mid;
	
	//	search next half of array
	if ( (MidElement < Match) == (GetSortOrder() == TLArray::TSortOrder::Ascending) )
		return BinaryChop( Match, Mid+1, High, pData, Size );
	else
		return BinaryChop( Match, Low, Mid-1, pData, Size );
}



namespace Private
{
	//	The address of this templated function is the ident. Just imagine the pointer is an integer 
	//	generated by the compiler everytime we come across another case of GetIteratorIdent()
	template<typename TYPE,typename MATCH>
	TIterator<TYPE>&	IteratorIdentFunc()	
	{
		static TIteratorWalk<TYPE,MATCH> It;
		return It;
	}
	
}


//----------------------------------------------------------------------------//
//	this is the iterator-specialisation identifier. Really, this is just a pointer to the iterator-walker factory func, but we treat
//	the [function]pointer as our ident value. This class just hides that away AND provides the exposed factory function
//----------------------------------------------------------------------------//
template<typename TYPE>
class TIteratorIdent
{
private:
	typedef TIterator<TYPE>&(*TFunc)();
	
public:
	TIteratorIdent(TFunc Func)
	{
		//	gr: can't seem to use the constructor initialiser list for this. Odd.
		m_Func = Func;
	}

	TIterator<TYPE>&	GetIterator() const				{	return (*m_Func)();	}	//	"instance" the iterator for this ident

	inline bool			operator==(const TIteratorIdent& Other) const	{	return m_Func == Other.m_Func;	}

private:
	TFunc				m_Func;	//	function to instance the iterator AND the address is used as the identifier (though this usage is hidden via the == operator)
};



//----------------------------------------------------------------------------//
//	generate an iterator ident for these two params; GetIteratorIdent<A,B>()
//----------------------------------------------------------------------------//
template<typename TYPE,typename MATCH>
TIteratorIdent<TYPE>	GetIteratorIdent()	
{
	return TIteratorIdent<TYPE>( &Private::IteratorIdentFunc<TYPE,MATCH> );
}




//----------------------------------------------------------------------------//
//	Base sort policy class. TArray is only aware of this type
//----------------------------------------------------------------------------//
template<typename TYPE>
class TSortPolicy
{
	friend class TArray<TYPE>;
public:
	virtual TIterator<TYPE>&	GetIterator(const TIteratorIdent<TYPE>& IteratorIdent) const=0;		//	create iterator

	virtual void				SetSortOrder(TLArray::TSortOrder::Type SortOrder)	{	}
	virtual void				Sort(TYPE* pData,u32 Elements)	{	}
	virtual bool				IsSorted() const				{	return false;	}
	virtual void				SetUnsorted()					{	}

	virtual void				OnAdded(TArray<TYPE>& Array,u32 FirstIndex,u32 AddedCount)=0;		//	called when elements have been added somewhere in the array. Used for insert sort
};


//----------------------------------------------------------------------------//
//	default sort policy just returns walker iterators which the calling function
//	already generated (but it didn't realise it :) and always claims the array is unsorted
//----------------------------------------------------------------------------//
template<typename TYPE>
class TSortPolicyNone : public TSortPolicy<TYPE>
{
public:
	virtual TIterator<TYPE>&	GetIterator(const TIteratorIdent<TYPE>& IteratorIdent) const	{	return IteratorIdent.GetIterator();	}
	virtual void				OnAdded(TArray<TYPE>& Array,u32 FirstIndex,u32 AddedCount)		{	}
};


//----------------------------------------------------------------------------//
//	sorted sort policy. Upon element adding it does it's best to keep the array sorted by 
//	moving the newly-added items around into the position where they should be.
//	This quicksorts on request if the array is unsorted, but rarely the case because of the insert sort.
//----------------------------------------------------------------------------//
template<typename TYPE,typename MATCH=TYPE,bool SORTONINSERT=true>
class TSortPolicySorted : public TSortPolicy<TYPE>
{
public:
	TSortPolicySorted() :
		m_IsSorted			( false )
	{
	}
	
	virtual TIterator<TYPE>&	GetIterator(const TIteratorIdent<TYPE>& IteratorIdent) const;	//	get a binary-chop iterator if we're sorted and if the iterator request is for our match/sort type
	virtual void				OnAdded(TArray<TYPE>& Array,u32 FirstIndex,u32 AddedCount);		//	keep the array sorted as much as possible

	virtual void				Sort(TYPE* pData,u32 Elements)	{	if ( pData && Elements > 0 && !IsSorted() )	QuickSort( pData, 0, Elements-1 );	SetSorted();	Debug_VerifyIsSorted(pData,Elements);	};		//	do quick sort
	virtual bool				IsSorted() const				{	return m_IsSorted;	}
	virtual void				SetUnsorted()					{	m_IsSorted = false;	}

private:
	void			Debug_VerifyIsSorted(TYPE* pData,u32 Size) const;
	u32				FindInsertPoint(TYPE* pData,u32 Low,u32 High,const TSorter<TYPE,MATCH>& ValueSorter);
	void			SetSorted()										{	m_IsSorted = true;	}
	void			SwapElements(TYPE* pData,s32 A,s32 B);
	void			QuickSort(TYPE* pData, s32 First, s32 Last);	//	Quicksort recursive func
	
private:
	bool			m_IsSorted;			//	sorted state of the array
};




//----------------------------------------------------------------------------//
//	get a binary-chop iterator if we're sorted and if the iterator request is for our match/sort type
//----------------------------------------------------------------------------
template<typename TYPE,typename MATCH,bool SORTONINSERT>
TIterator<TYPE>& TSortPolicySorted<TYPE,MATCH,SORTONINSERT>::GetIterator(const TIteratorIdent<TYPE>& IteratorIdent) const
{
	//	if we're sorted, and the requested ident matches our ident then we can do a binary chop
	//	as we're a matching TYPE/MATCHTYPE
	if ( IsSorted() && IteratorIdent == GetIteratorIdent<TYPE,MATCH>() )
	{
		static TIteratorChop<TYPE,MATCH> It;
		return It;
	}
	else
	{
		return IteratorIdent.GetIterator();
	}
}


//----------------------------------------------------------------------------//
//	keep the array sorted as much as possible
//----------------------------------------------------------------------------//
template<typename TYPE,typename MATCH,bool SORTONINSERT>
void TSortPolicySorted<TYPE,MATCH,SORTONINSERT>::OnAdded(TArray<TYPE>& Array,u32 FirstIndex,u32 AddedCount)
{
	//	if the array wasn't already sorted, then don't bother doing any clever stuff
	if ( !IsSorted() )
		return;

	//	if the array was previously empty don't do anything
	if ( FirstIndex == 0 )
	{
		if ( AddedCount <= 1 )
			SetSorted();
		else
			SetUnsorted();
		return;
	}
	
	//	for fast access
	TYPE* pData = Array.GetData();

	//	quick(?) abort (and stay sorted) if the new elements are in order when being placed at the end of the array
	u32 LastAdded = FirstIndex + AddedCount - 1;
	for ( u32 Index=FirstIndex;	Index<=LastAdded;	Index++ )
	{
		TYPE& Previous = pData[Index-1];
		TSorter<TYPE,MATCH> LastSorter( Previous );

		//	broken the sorted order
		if ( pData[Index] < LastSorter )
		{
			SetUnsorted();
			break;
		}

		//	this element is in the right place, pretend it was already there
		FirstIndex++;
		AddedCount--;
	}

	//	and in case we're inserted somewhere in the middle of the array, check after us too
	bool InsertedAtEnd = (FirstIndex+AddedCount >= Array.GetSize());
	if ( IsSorted() && !InsertedAtEnd )
	{
		u32 LastAdded = FirstIndex + AddedCount - 1;
		TYPE& Last = pData[LastAdded];
		TYPE& Next = pData[LastAdded+1];
		TSorter<TYPE,MATCH> NextSorter( Next );

		//	if it's NOT lower then it's in the wrong place
		if ( !(Last < NextSorter) )
			SetUnsorted();
	}

	//	still sorted with these new element's positioning :)
	if ( IsSorted() )
		return;

	//	sorting of these elements won't be corrected
	if ( !InsertedAtEnd || !SORTONINSERT )
		return;

	//	insert the new elements at the right place
	for ( u32 i=0;	i<AddedCount;	i++ )
	{
		u32 Index = FirstIndex + i;
		TSorter<TYPE,MATCH> Value( pData[Index] );

		//	check against all the elements before Index
		u32 InsertIndex = FindInsertPoint( pData, 0, Index - 1, Value );

		//	already in the right place (at the end of the existing array)
		//	gr: should this have already been caught?
		if ( InsertIndex == Index )
			continue;

		//	save our new element
		TYPE Temp = pData[Index];
		
		//	remove it because the shift is going to add a space
		Array.RemoveAt(Index);

		//	shift array over so we have our new space
		Array.ShiftArray( InsertIndex, 1 );

		//	insert new element
		//	note: in case the remove or shift has moved pData's allocation, we need to refresh it
		pData = Array.GetData();
		pData[InsertIndex] = Temp;
	}

	//	elements have been placed in the sorted points in the array
	SetSorted();
	Debug_VerifyIsSorted( Array.GetData(), Array.GetSize() );
}

template<typename TYPE,typename MATCH,bool SORTONINSERT>
void TSortPolicySorted<TYPE,MATCH,SORTONINSERT>::SwapElements(TYPE* pData,s32 A,s32 B)
{
	TYPE Temp = pData[A];
	pData[A] = pData[B];
	pData[B] = Temp;
}


//----------------------------------------------------------------------------//
//	Quicksort recursive func
//----------------------------------------------------------------------------//
template<typename TYPE,typename MATCH,bool SORTONINSERT>
void TSortPolicySorted<TYPE,MATCH,SORTONINSERT>::QuickSort(TYPE* pData, s32 First, s32 Last)
{
	//	check params
	if ( First >= Last )	return;
	if ( First == -1 )		return;
	if ( Last == -1 )		return;
	
	s32 End = First;
	TSorter<TYPE,MATCH> Sorter( pData[First] );
	for ( s32 Current=First+1;	Current<=Last;	Current++ )
	{
		if ( pData[Current] < Sorter )
		{
			SwapElements( pData, ++End, Current );
		}
	}
	
	SwapElements( pData, First, End );
	QuickSort( pData, First, End-1 );
	QuickSort( pData, End+1, Last );
}


template<typename TYPE,typename MATCH,bool SORTONINSERT>
void TSortPolicySorted<TYPE,MATCH,SORTONINSERT>::Debug_VerifyIsSorted(TYPE* pData,u32 Size) const
{
	if ( !IsSorted() )
		return;
	
	for ( u32 i=1;	i<Size;	i++ )
	{
		TSorter<TYPE,MATCH> Prev( pData[i-1] );
		if ( pData[i] < Prev )
		{
			TLDebug_Break("\"sorted\" array is out of order");		
		}
	}
}


template<typename TYPE,typename MATCH,bool SORTONINSERT>
u32 TSortPolicySorted<TYPE,MATCH,SORTONINSERT>::FindInsertPoint(TYPE* pData,u32 Low,u32 High,const TSorter<TYPE,MATCH>& ValueSorter)
{
	//	if high and low are the same, we've probably come down to one element in the array to compare with.
	//	we either go before or after it
	if ( High == Low )
	{
		//	if lower than low, insert at Low
		if ( pData[Low] < ValueSorter )
			return Low+1;
		else
			return Low;
	}
	
	#if defined(_DEBUG)
	{
		if ( High < Low )
		{
			TLDebug_Break("Algorithm Error? go up the stack, we might fit between these");
			return High;
		}
	}
	#endif
	
	//	lower than low? insert before Low
	if ( !(pData[Low] < ValueSorter) )
		return Low;
	
	//	higher than high? insert after High
	if ( pData[High] < ValueSorter )
		return High+1;

	//	therefore: Low < Value < High
	//	fit right between two elements? insert before high
	if ( High - Low == 1 )
		return High;
	
	u32 Mid = (Low + High)/2;
	#if defined(_DEBUG)
	{
		if ( Mid == Low || Mid == High )
		{
			TLDebug_Break("Algorithm Error");
		}
	}
	#endif
	
	//	check next half of the array
	if ( pData[Mid] < ValueSorter )	// >= Mid
	{
		//	go somewhere in the top half...
		//	already >Mid and already <High so could be <Mid+1 or >High-1
		return FindInsertPoint( pData, Mid+1, High, ValueSorter );
	}
	else // <Mid
	{
		//	go somewhere in the bottom half...
		//	already <Mid and already >Low so could be <Low+1 or >Mid-1
		return FindInsertPoint( pData, Low, Mid-1, ValueSorter );
	}
}