/*
 *  TPointerArray.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 28/10/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */



#pragma once

#include "TArray.h"




//-------------------------------------------------
//	de-referenced integral pointer type operator
//	this would be the equivelent of 
//	class float
//	{
//		operator==(const float* pThat)
//	}
//	requierd for TPointerArray<float> which would normally call the ==
//	operator on the TYPE (float)
//-------------------------------------------------
template<typename TYPE>
FORCEINLINE bool	operator==(const TYPE& This,const TYPE* pThat)
{
	return &This == pThat;
}

template<typename TYPE>
FORCEINLINE bool	operator==(const TYPE* pTypeA,const TYPE& TypeB)	
{
	return pTypeA == &TypeB;	
}

template<typename TYPEA,typename TYPEB>
FORCEINLINE bool	operator==(const TYPEA* pTypeA,const TYPEB& TypeB)	
{
	return (*pTypeA) == TypeB;	
}

template<typename TYPEA,typename TYPEB>
FORCEINLINE bool	operator==(const TYPEA** ppA,const TYPEB& B)
{
	return (!ppA) ? false : ((*ppA) == B);
}


namespace TLPointerArray
{
	template<typename TYPE,typename MATCH>
	bool IsThisLessThanThat(const TYPE* This,const TYPE* That);

	template<typename TYPE,typename MATCH>
	bool IsThisEqualThanThat(const TYPE* This,const TYPE* That);
}


template<typename TYPE,typename MATCH>
bool TLPointerArray::IsThisLessThanThat(const TYPE* This,const TYPE* That)
{
	//	null first rule...	
	if ( !This && !That )		return false;	//	both null
	else if ( !This && That )	return true;	//	this first
	else if ( This && !That )	return false;	//	that first
	else							
	{
		TSorter<TYPE*,MATCH> ThatSorter( (TYPE*)That );
		return (*This) < ThatSorter;
	}
}

template<typename TYPE,typename MATCH>
bool TLPointerArray::IsThisEqualThanThat(const TYPE* This,const TYPE* That)
{
	//	null first rule...	
	if ( !This && !That )		return true;	//	both null
	else if ( !This && That )	return false;	//	this first
	else if ( This && !That )	return false;	//	that first
	else							
	{
		TSorter<TYPE*,MATCH> ThatSorter( (TYPE*)That );
		return (*This) == ThatSorter;
	}
}



//----------------------------------------------------------------------------//
//	pointer specialised chop iterator. This is required because of awkward typing with templates
//	and because we need to dereference the pointers to do the checks (though this is done in the global == operators)
//	we also explicitly sort NULL items first and do NOT dereference them
//----------------------------------------------------------------------------//
template<typename TYPE,typename MATCH=TYPE*>
class TIteratorPointerChop : public TIterator<TYPE*>
{
public:
	virtual s32					FindIndex(const void* pMatch,TYPE* const* ppData,u32 FirstIndex,u32 Elements) const	{	return FindIndex( *(const MATCH*)pMatch, ppData, FirstIndex, Elements );	}
	s32							FindIndex(const MATCH& Match,TYPE* const* ppData,u32 FirstIndex,u32 Elements) const;
	
	TLArray::TSortOrder::Type	GetSortOrder() const	{	return TLArray::TSortOrder::Ascending;	}
	
private:
	s32							BinaryChop(const MATCH& Match,TYPE* const* ppData,const u32 Size) const;
};



//----------------------------------------------------------------------------//
//	gr: this probably isn't required any more as we don't have a PointerWalker...
//	TSortPolicyNone<TYPE*> can be used isntead
//----------------------------------------------------------------------------//
template<typename TYPE>
class TSortPolicyPointerNone : public TSortPolicy<TYPE*>
{
public:
	virtual TIterator<TYPE*>&	GetIterator(const TIteratorIdent<TYPE*>& IteratorIdent) const		{	return IteratorIdent.GetIterator();	}
	virtual void				OnAdded(TArray<TYPE*>& Array,u32 FirstIndex,u32 AddedCount)	{	TSortPolicy<TYPE*>::SetUnsorted();	}
};


//----------------------------------------------------------------------------//
//	Sorted pointer array. This is specialised to use the specialised Pointer ChopIterator
//----------------------------------------------------------------------------//
template<typename TYPE,typename MATCH=TYPE*,bool SORTONINSERT=true>
class TSortPolicyPointerSorted : public TSortPolicy<TYPE*>
{
public:
	TSortPolicyPointerSorted() :
		m_IsSorted			( false )
	{
	}
	
	virtual void				Sort(TYPE** ppData,u32 Elements);	//	quick sort the data
	virtual bool				IsSorted() const					{	return m_IsSorted;	}
	virtual void				SetUnsorted()						{	m_IsSorted = false;	}
	
	virtual TIterator<TYPE*>&	GetIterator(const TIteratorIdent<TYPE*>& IteratorIdent) const;

	virtual void				OnAdded(TArray<TYPE*>& Array,u32 FirstIndex,u32 AddedCount);
	
private:
	void			Debug_VerifyIsSorted(TYPE** ppData,u32 Size) const;	
	u32				FindInsertPoint(TYPE** pData,u32 Low,u32 High,const TSorter<TYPE*,MATCH>& ValueSorter);
	void			SetSorted()							{	m_IsSorted = true;	}
	void			SwapElements(TYPE** ppData,s32 A,s32 B);
	void			QuickSort(TYPE** ppData, s32 First, s32 Last);	//	Quicksort recursive func

private:
	bool			m_IsSorted;			//	sorted state of the array
};




template<typename TYPE,bool AUTODELETE=false,class SORTPOLICY=TSortPolicyPointerNone<TYPE>,u32 GROWBY=10,class ARRAYTYPE=THeapArray<TYPE*,GROWBY,SORTPOLICY> >
class TPointerArray : public ARRAYTYPE 
{
private:
	typedef TPointerArray<TYPE,AUTODELETE,SORTPOLICY,GROWBY,ARRAYTYPE> TThis;
	typedef ARRAYTYPE TSuper;
public:
	TPointerArray()								{	}
	TPointerArray(const TArray<TYPE*>& Array)	{	TSuper::Copy(Array);	}

	~TPointerArray()				{	if ( AUTODELETE )	DeleteAll();	}
	
	FORCEINLINE bool				RemoveNull();	
	FORCEINLINE void				DeleteAll();
	
	template<class MATCHTYPE>
	FORCEINLINE TYPE*				FindPtr(const MATCHTYPE& val)		{	TYPE** ppElement = Find( val );			return ppElement ? *ppElement : NULL;	}
	template<class MATCHTYPE>
	FORCEINLINE const TYPE*			FindPtr(const MATCHTYPE& val) const	{	const TYPE** ppElement = Find( val );	return ppElement ? *ppElement : NULL;	}
	
	FORCEINLINE TThis&				operator=(const TArray<TYPE*>& Array)	{	TSuper::Copy( Array );	return *this;	}
};


template<typename TYPE,bool AUTODELETE,class SORTPOLICY,u32 GROWBY,class ARRAYTYPE>
FORCEINLINE bool TPointerArray<TYPE,AUTODELETE,SORTPOLICY,GROWBY,ARRAYTYPE>::RemoveNull()
{
	bool AnyRemoved = false;
	for ( s32 i=TSuper::GetSize()-1;	i>=0;	i-- )
	{
		if ( !TSuper::ElementAt(i) )
			AnyRemoved |= TSuper::RemoveAt(i);
	}
	return AnyRemoved;
}


template<typename TYPE,bool AUTODELETE,class SORTPOLICY,u32 GROWBY,class ARRAYTYPE>
FORCEINLINE void TPointerArray<TYPE,AUTODELETE,SORTPOLICY,GROWBY,ARRAYTYPE>::DeleteAll()
{
	for ( s32 i=TSuper::GetSize()-1;	i>=0;	i-- )
	{
	//	TLMemory::Delete( TSuper::ElementAt(i) );
	}
}


//----------------------------------------------------------------------------//
//	Yes it looks complex :)
//	TPointerSortedArray<MyClass,true,u32>	m_MyClassArraySortedByU32;
//----------------------------------------------------------------------------//
template<typename TYPE,bool AUTODELETE=false,typename SORTMATCH=TYPE*,u32 GROWBY=10 >
class TPointerSortedArray : public TPointerArray<TYPE,AUTODELETE,TSortPolicyPointerSorted<TYPE,SORTMATCH>,GROWBY>
{
private:
	typedef TPointerSortedArray<TYPE,AUTODELETE,SORTMATCH,GROWBY> TThis;
	typedef TPointerArray<TYPE,AUTODELETE,TSortPolicyPointerSorted<TYPE,SORTMATCH>,GROWBY> TSuper;
public:
	TPointerSortedArray()							{	}
	TPointerSortedArray(const TArray<TYPE*>& Array)	{	Copy(Array);	}

	FORCEINLINE TThis&				operator=(const TArray<TYPE*>& Array)	{	Copy( Array );		return *this;	}
};







template<typename TYPE,typename MATCH>
s32 TIteratorPointerChop<TYPE,MATCH>::FindIndex(const MATCH& Match,TYPE* const* ppData,u32 FirstIndex,u32 Elements) const
{
	if ( Elements == 0 )
		return -1;

	s32 Index = BinaryChop( Match, ppData, Elements );
	
	#if defined(ARRAY_SORT_CHECK)
	{
		if ( Index == -1 )
		{
			s32 FoundAt = -1;
			for ( u32 i=0;	i<Elements;	i++ )
			{
				const TYPE* pData = ppData[i];
				if ( FoundAt != -1 && pData && *pData == Match )
				{
					FoundAt = i;
				}
				
				//	check if sort is broken
				if ( i > 0 )
				{
					const TYPE* pPrev = ppData[i-1];
					TSorter<TYPE*,MATCH> SortPrev( (TYPE*)pPrev );
					if ( *pData < SortPrev )
					{
						TLDebug_Break("Data is not sorted");
					}
				}
			}
			
			if ( FoundAt != -1 )
			{
				TLDebug_Break("Binary chop (or sort) broken. item found.");
			}
		}
	}
	#endif
	
	return Index;
}


template<typename TYPE,typename MATCH>
s32 TIteratorPointerChop<TYPE,MATCH>::BinaryChop(const MATCH& Match,TYPE* const* ppData,const u32 Size) const
{
	s32 low = 0;
	s32 high = Size;
	while (low < high) 
	{
		s32 mid = (low + high) / 2;
		if ( mid == high )
		{
			TLDebug_Break("Algorithm error: This will access out of range");
			break;
		}

		//	always fail to match a NULL entry
		const TYPE* pMid = ppData[mid];
		bool MatchIsGreater = pMid ? ((*pMid) < Match) : false;
		if ( MatchIsGreater )
			low = mid + 1; 
		else
			//can't be high = mid-1: here A[mid] >= value,
			//so high can't be < mid if A[mid] == value
			high = mid; 
	}

	// high == low, using high or low depends on taste 
	if ( low < (s32)Size )
	{
		const TYPE* pLow = ppData[low];
		if ( pLow && (*pLow == Match) )
			return low;
	}

	return -1;  
}


//----------------------------------------------------------------------------//
//	quick sort the data
//----------------------------------------------------------------------------//
template<typename TYPE,typename MATCH,bool SORTONINSERT>
void TSortPolicyPointerSorted<TYPE,MATCH,SORTONINSERT>::Sort(TYPE** ppData,u32 Elements)
{
	if ( ppData && Elements > 0 && !IsSorted() )	
	{
		QuickSort( ppData, 0, Elements-1 );	
		SetSorted();
		Debug_VerifyIsSorted(ppData,Elements);	
	}
}

template<typename TYPE,typename MATCH,bool SORTONINSERT>
TIterator<TYPE*>& TSortPolicyPointerSorted<TYPE,MATCH,SORTONINSERT>::GetIterator(const TIteratorIdent<TYPE*>& IteratorIdent) const
{
	//	if we're sorted, and the requested ident matches our ident then we can do a binary chop
	//	as we're a matching TYPE/MATCHTYPE
	if ( IsSorted() && IteratorIdent == GetIteratorIdent<TYPE*,MATCH>() )
	{
		static TIteratorPointerChop<TYPE,MATCH> It;
		return It;
	}
	else
	{
		return IteratorIdent.GetIterator();
	}
}


template<typename TYPE,typename MATCH,bool SORTONINSERT>
void TSortPolicyPointerSorted<TYPE,MATCH,SORTONINSERT>::OnAdded(TArray<TYPE*>& Array,u32 FirstIndex,u32 AddedCount)
{
	//	if the array wasn't already sorted, then don't bother doing any clever stuff
	if ( !IsSorted() )
		return;

	//	if the array was previously empty don't do anything
	if ( FirstIndex == 0 )
	{
		if ( AddedCount <= 1 )
		{
			SetSorted();
			Debug_VerifyIsSorted(Array.GetData(),Array.GetSize());
		}
		else
		{
			SetUnsorted();
		}
		return;
	}
	
	//	for fast access
	TYPE** pData = Array.GetData();

	//	quick(?) abort (and stay sorted) if the new elements are in order when being placed at the end of the array
	u32 LastAdded = FirstIndex + AddedCount - 1;
	for ( u32 Index=FirstIndex;	Index<=LastAdded;	Index++ )
	{
		bool IsLess = TLPointerArray::IsThisLessThanThat<TYPE,MATCH>( pData[Index], pData[Index-1] );

		//	broken the sorted order
		if ( IsLess )
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

		//	null first rule...	
		bool IsLess = TLPointerArray::IsThisLessThanThat<TYPE,MATCH>( pData[LastAdded], pData[LastAdded+1] );

		//	if it's NOT lower then it's in the wrong place
		if ( !IsLess )
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
		TSorter<TYPE*,MATCH> Value( pData[Index] );

		//	check against all the elements before Index
		u32 InsertIndex = (!pData[Index]) ? 0 : FindInsertPoint( pData, 0, Index - 1, Value );

		//	already in the right place (at the end of the existing array)
		//	gr: should this have already been caught?
		if ( InsertIndex == Index )
			continue;

		//	save our new element
		TYPE* Temp = pData[Index];

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
	Debug_VerifyIsSorted(Array.GetData(),Array.GetSize());
}


template<typename TYPE,typename MATCH,bool SORTONINSERT>
void TSortPolicyPointerSorted<TYPE,MATCH,SORTONINSERT>::Debug_VerifyIsSorted(TYPE** ppData,u32 Size) const
{
	#if defined(ARRAY_SORT_CHECK)
	{
		if ( !IsSorted() )
			return;
	
		for ( u32 i=1;	i<Size;	i++ )
		{
			const TYPE* pPrev = ppData[i-1];
			const TYPE* pThis = ppData[i];
			if ( TLPointerArray::IsThisLessThanThat<TYPE,MATCH>( pThis, pPrev ) )
			{
				TLDebug_Break("\"sorted\" array is out of order");		
			}
		}
	}
	#endif
}


template<typename TYPE,typename MATCH,bool SORTONINSERT>
u32 TSortPolicyPointerSorted<TYPE,MATCH,SORTONINSERT>::FindInsertPoint(TYPE** pData,u32 Low,u32 High,const TSorter<TYPE*,MATCH>& ValueSorter)
{
	//	if high and low are the same, we've probably come down to one element in the array to compare with.
	//	we either go before or after it
	if ( High == Low )
	{
		//	if lower than low, insert at Low
		if ( *(pData[Low]) < ValueSorter )
			return Low+1;
		else
			return Low;
	}
	
	#if defined(_DEBUG)
	{
		if ( High < Low )
		{
			TLDebug_Break("Algorithm Error? go up the stack, we might fit between these");
		}
	}
	#endif

	//	lower than low? insert before Low
	if ( !(*(pData[Low]) < ValueSorter) )
		return Low;
	
	//	higher than high? insert after High
	if ( *(pData[High]) < ValueSorter )
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
	if ( *(pData[Mid]) < ValueSorter )	// >= Mid
	{
		//	go somewhere in the top half...
		//	already >Mid and already <High so could be <Mid+1 or >High-1
		return FindInsertPoint( pData, Mid+1, High, ValueSorter );
	}
	else // <Mid
	{
		//	go somewhere in the bottom half...
		//	already <Mid and already >Low so could be <Low+1 or >Mid-1
		return FindInsertPoint( pData, Low, Mid, ValueSorter );
	}
}



template<typename TYPE,typename MATCH,bool SORTONINSERT>
void TSortPolicyPointerSorted<TYPE,MATCH,SORTONINSERT>::SwapElements(TYPE** ppData,s32 A,s32 B)
{
	TYPE* Temp = ppData[A];
	ppData[A] = ppData[B];
	ppData[B] = Temp;
}


template<typename TYPE,typename MATCH,bool SORTONINSERT>
void TSortPolicyPointerSorted<TYPE,MATCH,SORTONINSERT>::QuickSort(TYPE** ppData, s32 First, s32 Last)
{
	//	check params
	if ( First >= Last )	return;
	if ( First == -1 )		return;
	if ( Last == -1 )		return;
	
	s32 End = First;
	TYPE*& pFirst = ppData[First];
	TSorter<TYPE*,MATCH> Sorter( pFirst );
	for ( s32 Current=First+1;	Current<=Last;	Current++ )
	{
		const TYPE* pDataCurrent = ppData[Current];
	
		//	null first rule...	
		bool IsLess;
		if ( !pDataCurrent && !pFirst )		IsLess = false;	//	both null
		else if ( !pDataCurrent && pFirst )	IsLess = true;	//	this first
		else if ( pDataCurrent && !pFirst )	IsLess = false;	//	that first
		else								IsLess = (*pDataCurrent) < Sorter;

		if ( IsLess )
		{
			SwapElements( ppData, ++End, Current );
		}
	}
	
	SwapElements( ppData, First, End );
	QuickSort( ppData, First, End-1 );
	QuickSort( ppData, End+1, Last );
}
