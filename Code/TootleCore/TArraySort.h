/*
 *  ArraySort.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 26/10/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

namespace TLArray
{
	// No sorting policy class
	template<typename TYPE>
	class SortPolicy_None;

	// Base array sort policy class - used by all specialised array sorting policy classes
	template<typename TYPE>
	class SortPolicy_Base;

	// Quick sorting policy class for standard arrays
	template<typename TYPE>
	class SortPolicy_Quick;
	
	
	enum SortResult
	{
		IsLess = -1,	//	
		IsGreater = 0,	//	for (A==B) == FALSE == 0 == IsGreater
		IsEqual = 1,	//	for (A==B) == TRUE == 1 == IsEqual
	};
	
	// Sort order enum
	enum SortOrder 
	{
		Ascending = 0,
		Descending,
	};
	
}


// Array policy for no sorting
template<typename TYPE>
class TLArray::SortPolicy_None
{
protected:	
	~SortPolicy_None()	{}

	FORCEINLINE void	SetSorted(Bool IsSorted)				{};	
	FORCEINLINE Bool	IsSorted() const						{	return FALSE;	};	// Never soted using this policy
		
	FORCEINLINE void	OnAdded(const TYPE& OldLastElement, const TYPE& LastElement)	{}
		
	template<class MATCHTYPE>
	s32 FindIndex(const MATCHTYPE& val,const TYPE* pData, const u32 FromIndex, const u32 uSize) const;	

	void	SetSortOrder(const TLArray::SortOrder order, TYPE* pData, const u32 uSize);
	FORCEINLINE void	SwapElements(TYPE* pData, u32 a, u32 b);

};

// None sorted array FindIndex
template< typename TYPE>
template<class MATCHTYPE>
s32 TLArray::SortPolicy_None<TYPE>::FindIndex(const MATCHTYPE& val,const TYPE* pData, const u32 FromIndex, const u32 uSize) const
{		
	//	walk through elements
	for ( u32 i=FromIndex;	i<uSize;	i++ )
	{
		if ( pData[i] == val )
			return (s32)i;
	}
	return -1;
};


template< typename TYPE>
void TLArray::SortPolicy_None<TYPE>::SetSortOrder(const TLArray::SortOrder order, TYPE* pData, const u32 uSize)	
{
	// Simply swap all elements every time this is called for non-sorted arrays
	// Essentially a toogle of data order in the array but not sorted
	if(uSize > 1)
	{
		u32 uLast = uSize-1;
		
		// Rearrange all data elements
		for(u32 uIndex = 0; uIndex < uLast; uIndex++)
		{
			SwapElements(pData, uIndex, uLast--);
		}	
	}	
}

template< typename TYPE>
FORCEINLINE void TLArray::SortPolicy_None<TYPE>::SwapElements(TYPE* pData, u32 a, u32 b)
{
	//	don't do anything when trying to swap the "same" element
	if ( a == b )
		return;
	
	TYPE Tmp = pData[a];
	pData[a] = pData[b];
	pData[b] = Tmp;	
}









// Base policy for sorted arrays using the binary search method
// Deliberately avoids using virtual routines and should inline
template<typename TYPE>
class TLArray::SortPolicy_Base
{
		
protected:	
	SortPolicy_Base()	: m_SortOrder(Ascending), m_bSorted(FALSE)			{}
	virtual ~SortPolicy_Base()						{}	// NOTE: Needs to be virtual ATM for virtual SortElementComparison

	
	FORCEINLINE void	SetSorted(Bool IsSorted)				{	m_bSorted = IsSorted;	};			//	called when list order changes

	FORCEINLINE Bool	IsSorted() const						{	return m_bSorted;	};	//	is the list sorted?

	
	template<class MATCHTYPE>
	s32					FindIndex(const MATCHTYPE& val, TYPE* pData, const u32 FromIndex, const u32 uSize);
	
	template<class MATCHTYPE>
	s32					FindIndex(const MATCHTYPE& val,const TYPE* pData, const u32 FromIndex, const u32 uSize) const;

	FORCEINLINE void	Sort(TYPE* pData, u32 uSize);
	
	FORCEINLINE void	OnAdded(const TYPE& OldLastElement, const TYPE& LastElement);
	
	FORCEINLINE void	SetSortOrder(const TLArray::SortOrder order, TYPE* pData, const u32 uSize);

private:

	template<class MATCHTYPE>
	s32				FindIndexSorted(const MATCHTYPE& val,u32 Low,s32 High,const TYPE* pData, const u32 uSize) const;

	void			QuickSort(TYPE* pData, s32 First, s32 Last);
		
	void			SwapElements(TYPE* pData, u32 a, u32 b);

	void			OnSortOrderChanged(TYPE* pData, const u32 uSize);
	

	// Only virtual routine needed and hopefully can at some stgae be replaced or removed
	virtual SortResult		SortElementComparison(const TYPE& a,const TYPE& b,const void* pTestVal) const = 0;  // TYPE comparison routine

private:
	TLArray::SortOrder	m_SortOrder;		//	sort order - default is ascending
	Bool				m_bSorted;			//	set to true everytime the list is sorted. if any elemets are added, this becomes invalid	
};





// Sorted array Findindex
template< typename TYPE>
template<class MATCHTYPE>
s32 TLArray::SortPolicy_Base<TYPE>::FindIndex(const MATCHTYPE& val, TYPE* pData, const u32 FromIndex, const u32 uSize)
{		
	// More than 2 elements?  Sort array and use binary search method
	if(uSize > 2 )
	{
		if ( !IsSorted() )
			Sort(pData, uSize);
		
		//	make use of the binary search method as our list is in order
		//	gr: check is sorted again incase we couldn't sort for some reason
		if ( IsSorted() )
			return FindIndexSorted( val, 0, uSize-1, pData, uSize);
	}
		
	//	walk through elements
	for ( u32 i=FromIndex;	i<uSize;	i++ )
	{
		if ( pData[i] == val )
			return (s32)i;
	}
	return -1;
};


template< typename TYPE>
template<class MATCHTYPE>
s32 TLArray::SortPolicy_Base<TYPE>::FindIndex(const MATCHTYPE& val, const TYPE* pData, const u32 FromIndex, const u32 uSize) const
{		
	// More than 2 elements?  Sort array and use binary search method
	if(uSize > 2 )
	{
		if ( !IsSorted() )
		{
			//TLDebug_Break("Warning: unsorted array cannot be sorted because of const FindIndex()");
		}
		else
		{
			return FindIndexSorted( val, 0, uSize-1, pData, uSize );
		}
	}
	
	//	walk through elements
	for ( u32 i=FromIndex;	i<uSize;	i++ )
	{
		if ( pData[i] == val )
			return (s32)i;
	}
	return -1;
};


//----------------------------------------------------------------------
//	binary search method (recursive)
//----------------------------------------------------------------------
template< typename TYPE>
template<class MATCHTYPE>
s32 TLArray::SortPolicy_Base<TYPE>::FindIndexSorted(const MATCHTYPE& val,u32 Low,s32 High,const TYPE* pData, const u32 uSize) const
{
	if ( High < (s32)Low )
		return -1;
	
	u32 Mid = (Low + (u32)High) / 2;
	
	//	gr: i think the algorithm is wrong somewhere, array of size 3 gets here with low and high == 3, mid becomes 3, which is out of range!
	if ( Mid >= uSize )
		return -1;
	
	//	see if we've found the element...
	//	gr: array of data is pre-fetched now and provided, removes unncessary index check (may want to keep this in for _DEBUG builds?) and virtual call
	//const TYPE& MidElement = ElementAtConst( Mid );
	const TYPE& MidElement = pData[Mid];
	TLArray::SortResult Sort = SortElementComparison( MidElement, MidElement, &val );
	if ( Sort == TLArray::IsEqual )
		return Mid;
	
	//	search next half of array
	if ( Sort == TLArray::IsLess )
		return FindIndexSorted( val, Mid+1, High, pData, uSize );
	else //if ( Sort == TLArray::IsGreater )
		return FindIndexSorted( val, Low, Mid-1, pData, uSize );
}

template< typename TYPE>
FORCEINLINE void TLArray::SortPolicy_Base<TYPE>::SetSortOrder(TLArray::SortOrder order, TYPE* pData, const u32 uSize)
{
	if(m_SortOrder != order)
	{
		if(uSize > 1)
		{
			// If not sorted then sort first
			if(!IsSorted())
				Sort();

			// now re-arrange data
			OnSortOrderChanged(pData, uSize);
		}
	
		m_SortOrder = order;
	}
}

template< typename TYPE>
void TLArray::SortPolicy_Base<TYPE>::OnSortOrderChanged(TYPE* pData, const u32 uSize)
{
	u32 uLast = uSize-1;
	
	// Rearrange all data elements
	for(u32 uIndex = 0; uIndex < uLast; uIndex++)
	{
		SwapElements(pData, uIndex, uLast--);
	}	
}

template< typename TYPE>
FORCEINLINE void TLArray::SortPolicy_Base<TYPE>::Sort(TYPE* pData, u32 uSize)
{
	//	already sorted?
	if( IsSorted() )
		return;
	
	//	do sort
	QuickSort(pData, 0, (uSize-1) );
	
	//	we're now sorted!
	SetSorted( TRUE );
}


//-------------------------------------------------------------------------
//	Quicksort recursive func
//	Utilises the bubble sort method
//-------------------------------------------------------------------------
template< typename TYPE>
void TLArray::SortPolicy_Base<TYPE>::QuickSort(TYPE* pData, s32 First, s32 Last)
{
	//	check params
	if ( First >= Last )	return;
	if ( First == -1 )		return;
	if ( Last == -1 )		return;
	
	s32 End = First;
	for ( s32 Current=First+1;	Current<=Last;	Current++ )
	{
		if ( SortElementComparison( pData[Current], pData[First], NULL ) == TLArray::IsLess )
		{
			SwapElements(pData, ++End, Current );
		}
	}
	
	SwapElements( pData, First, End );
	QuickSort( pData, First, End-1 );
	QuickSort( pData, End+1, Last );
}


//-------------------------------------------------------------------------
//	swap order of two elements
//-------------------------------------------------------------------------
template< typename TYPE>
void TLArray::SortPolicy_Base<TYPE>::SwapElements(TYPE* pData, u32 a, u32 b)
{
	//	don't do anything when trying to swap the "same" element
	if ( a == b )
		return;
	
	TYPE Tmp = pData[a];
	pData[a] = pData[b];
	pData[b] = Tmp;
	
	//	list may no longer be sorted
	SetSorted(FALSE);
}


template< typename TYPE>
FORCEINLINE void TLArray::SortPolicy_Base<TYPE>::OnAdded(const TYPE& OldLastElement, const TYPE& LastElement)
{
	//	check to see if adding this element keeps the array sorted
	if ( IsSorted() )
	{		
		//	adding this to the end would make the array unsorted
		TLArray::SortResult Sorted = SortElementComparison( OldLastElement, LastElement, NULL );
		if ( Sorted == TLArray::IsGreater )
			SetSorted( FALSE );
	}
}






// Quick sorting policy for simple data arrays
template<typename TYPE>
class TLArray::SortPolicy_Quick : public TLArray::SortPolicy_Base<TYPE>
{
protected:
	virtual SortResult		SortElementComparison(const TYPE& a,const TYPE& b,const void* pTestVal) const;  // TYPE comparison routine

};

template< typename TYPE>
TLArray::SortResult TLArray::SortPolicy_Quick<TYPE>::SortElementComparison(const TYPE& a,const TYPE& b,const void* pTestVal) const
{
	//	normally you KNOW what pTestVal's type will be and cast
	//	as the "default" sort func, we ASSUME that pTestVal is of TYPE type.
	const TYPE& TestWith = pTestVal ? *(const TYPE*)pTestVal :  b;
	//	== turns into 0 (is greater) or 1(equals)
	return a < TestWith ? IsLess : (SortResult)(a==TestWith);	
}




