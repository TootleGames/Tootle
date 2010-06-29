/*------------------------------------------------------

	Array for TPtrs, just has some extra functionality

-------------------------------------------------------*/
#pragma once
#include "TArray.h"
#include "TPtr.h"




//	this will require a <(TSorter<TYPE,MATCHTYPE>&) operator on your type
template<typename TYPE,typename MATCHTYPE>
FORCEINLINE bool			operator<(const TPtr<TYPE>& This,const TSorter<TPtr<TYPE>,MATCHTYPE>& ThatSorter)	
{
	//	null first rule...
	const TYPE* pThis = This.GetObjectPointer();
	const TYPE* pThat = (*ThatSorter).GetObjectPointer();

	if ( !pThis && !pThat )		return false;	//	both null
	if ( !pThis && pThat )		return true;	//	this first
	if ( pThis && !pThat )		return false;	//	that first

	//	make up a non ptr sorter to compare with
	const TYPE& That = *pThat;
	TSorter<TYPE,MATCHTYPE> Sorter( That );
	return (*pThis) < Sorter; 
}
	



template<typename TYPE,typename MATCH/*=TPtr<TYPE>*/ >
class TIteratorPtrChop : public TIterator<TPtr<TYPE> >
{
public:
	virtual s32	FindIndex(const void* pMatch,const TPtr<TYPE>* ppData,u32 FirstIndex,u32 Elements) const	{	return FindIndex( *(const MATCH*)pMatch, ppData, FirstIndex, Elements );	}
	s32			FindIndex(const MATCH& Match,const TPtr<TYPE>* ppData,u32 FirstIndex,u32 Elements) const
	{
		if ( Elements == 0 )
			return -1;
		
		s32 Index = BinaryChop( Match, 0, Elements, ppData, Elements );

		#if defined(ARRAY_SORT_CHECK)
		{
			if ( Index == -1 )
			{
				for ( u32 i=0;	i<Elements;	i++ )
				{
					const TPtr<TYPE>& pData = ppData[i];
					if ( pData == Match )
					{
						TLDebug_Break("Binary chop (or sort) broken. item found.");
						return i;
					}
				}
			}
		}
		#endif
		
		return Index;
	}
	
	TLArray::TSortOrder::Type	GetSortOrder() const	{	return TLArray::TSortOrder::Ascending;	}
	
private:
	//	binary search method (recursive)
	s32 BinaryChop(const MATCH& Match,u32 Low,s32 High,const TPtr<TYPE>* ppData,const u32 Size) const
	{
		if ( High < (s32)Low )
			return -1;
		
		u32 Mid = (Low + (u32)High) / 2;
		
		//	gr: i think the algorithm is wrong somewhere, array of size 3 gets here with low and high == 3, mid becomes 3, which is out of range!
		if ( Mid >= Size )
			return -1;
		
		//	see if we've found the element...
		const TPtr<TYPE>& pMidElement = ppData[Mid];
		
		if ( pMidElement == Match )
			return Mid;
		
		bool IsLess = (pMidElement < Match);
		
		//	search next half of array
		if ( IsLess == (GetSortOrder() == TLArray::TSortOrder::Ascending) )
			return BinaryChop( Match, Mid+1, High, ppData, Size );
		else
			return BinaryChop( Match, Low, Mid-1, ppData, Size );
	}
};


//----------------------------------------------------------------------------//
//	gr: like the TSortPolicyPoitnerNone, this is probably redundant now 
//----------------------------------------------------------------------------//
template<typename TYPE>
class TSortPolicyPtrNone : public TSortPolicy<TPtr<TYPE> >
{
public:
	virtual TIterator<TPtr<TYPE> >&	GetIterator(const TIteratorIdent<TPtr<TYPE> >& IteratorIdent) const	{	return IteratorIdent.GetIterator();	}
	virtual void					OnAdded(TArray<TPtr<TYPE> >& Array,u32 FirstIndex,u32 AddedCount)	{	}
};



//----------------------------------------------------------------------------//
//	gr: 
//----------------------------------------------------------------------------//

template<typename TYPE,typename MATCH=TPtr<TYPE>,bool SORTONINSERT=true>
class TSortPolicyPtrSorted : public TSortPolicy<TPtr<TYPE> >
{
public:
	TSortPolicyPtrSorted() :
		m_IsSorted			( false ),
		m_SortIteratorIdent	( GetIteratorIdent<TPtr<TYPE>,MATCH>() )
	{
	}
	
	virtual void				Sort(TPtr<TYPE>* ppData,u32 Elements)	{	if ( ppData && Elements > 0 && !IsSorted() )	QuickSort( ppData, 0, Elements-1 );	SetSorted();	Debug_VerifyIsSorted(ppData,Elements);	};		//	do quick sort
	virtual bool				IsSorted() const						{	return m_IsSorted;	}
	virtual void				SetUnsorted()							{	m_IsSorted = false;	}
	
	virtual TIterator<TPtr<TYPE> >&	GetIterator(const TIteratorIdent<TPtr<TYPE> >& IteratorIdent) const;
	virtual void					OnAdded(TArray<TPtr<TYPE> >& Array,u32 FirstIndex,u32 AddedCount);
	
private:
	void	Debug_VerifyIsSorted(TPtr<TYPE>* pData,u32 Size) const;	
	void	SetSorted()						{	m_IsSorted = true;	}
	u32		FindInsertPoint(TPtr<TYPE>* pData,u32 Low,u32 High,const TSorter<TPtr<TYPE>,MATCH>& ValueSorter);
	void	QuickSort(TPtr<TYPE>* ppData, s32 First, s32 Last);
	void	SwapElements(TPtr<TYPE>* ppData,s32 A,s32 B);	
private:
	TIteratorIdent<TPtr<TYPE> >	m_SortIteratorIdent;
	bool						m_IsSorted;			//	sorted state of the array
};




template<typename TYPE,typename MATCH,bool SORTONINSERT>
TIterator<TPtr<TYPE> >& TSortPolicyPtrSorted<TYPE,MATCH,SORTONINSERT>::GetIterator(const TIteratorIdent<TPtr<TYPE> >& IteratorIdent) const
{
	//	if we're sorted, and the requested ident matches our ident then we can do a binary chop
	//	as we're a matching TYPE/MATCHTYPE
	if ( IsSorted() && IteratorIdent == m_SortIteratorIdent )
	{
		static TIteratorPtrChop<TYPE,MATCH> It;
		return It;
	}
	else
	{
		return IteratorIdent.GetIterator();
	}
}

template<typename TYPE,typename MATCH,bool SORTONINSERT>
void TSortPolicyPtrSorted<TYPE,MATCH,SORTONINSERT>::OnAdded(TArray<TPtr<TYPE> >& Array,u32 FirstIndex,u32 AddedCount)
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
	
	//	pre-fetch data
	TPtr<TYPE>* pData = Array.GetData();
	
	//	quick(?) abort (and stay sorted) if the new elements are in order when being placed at the end of the array
	u32 LastAdded = FirstIndex + AddedCount - 1;
	for ( u32 Index=FirstIndex;	Index<=LastAdded;	Index++ )
	{
		TPtr<TYPE>& Previous = pData[Index-1];
		TPtr<TYPE>& pIndex = pData[Index];
		
		TSorter<TPtr<TYPE>,MATCH> LastSorter( Previous );
		bool IsLess = (pIndex) < LastSorter;
		
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
		TPtr<TYPE>& Last = pData[LastAdded];
		TPtr<TYPE>& Next = pData[LastAdded+1];
		TSorter<TPtr<TYPE>,MATCH> NextSorter( Next );
		bool IsLess = (Last < NextSorter);
		
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
		TSorter<TPtr<TYPE>,MATCH> Value( pData[Index] );
		
		//	check against all the elements before Index
		u32 InsertIndex = FindInsertPoint( pData, 0, Index - 1, Value );
		
		//	already in the right place (at the end of the existing array)
		//	gr: should this have already been caught?
		if ( InsertIndex == Index )
			continue;
		
		//	save a copy
		TPtr<TYPE> Temp = pData[Index];
		
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
u32 TSortPolicyPtrSorted<TYPE,MATCH,SORTONINSERT>::FindInsertPoint(TPtr<TYPE>* pData,u32 Low,u32 High,const TSorter<TPtr<TYPE>,MATCH>& ValueSorter)
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
			return High;
		}
	}
#endif
	
	//	check next half of the array
	if ( pData[Mid] < ValueSorter )	// >= Mid
	{
		//	go somewhere in the top half...
		//	already >Mid and already <High so could be <Mid+1 or >High-1
		return FindInsertPoint( pData, Mid+1, High-1, ValueSorter );
	}
	else // <Mid
	{
		//	go somewhere in the bottom half...
		//	already <Mid and already >Low so could be <Low+1 or >Mid-1
		return FindInsertPoint( pData, Low+1, Mid-1, ValueSorter );
	}
}


template<typename TYPE,typename MATCH,bool SORTONINSERT>
void TSortPolicyPtrSorted<TYPE,MATCH,SORTONINSERT>::QuickSort(TPtr<TYPE>* ppData, s32 First, s32 Last)
{
	//	check params
	if ( First >= Last )	return;
	if ( First == -1 )		return;
	if ( Last == -1 )		return;
	
	s32 End = First;
	TSorter<TPtr<TYPE>,MATCH> Sorter( ppData[First] );
	for ( s32 Current=First+1;	Current<=Last;	Current++ )
	{
		const TPtr<TYPE>& pDataCurrent = ppData[Current];
		if ( pDataCurrent < Sorter )
		{
			SwapElements( ppData, ++End, Current );
		}
	}
	
	SwapElements( ppData, First, End );
	QuickSort( ppData, First, End-1 );
	QuickSort( ppData, End+1, Last );
}

template<typename TYPE,typename MATCH,bool SORTONINSERT>
void TSortPolicyPtrSorted<TYPE,MATCH,SORTONINSERT>::SwapElements(TPtr<TYPE>* ppData,s32 A,s32 B)
{
	TPtr<TYPE> Temp = ppData[A];
	ppData[A] = ppData[B];
	ppData[B] = Temp;
}





template<typename TYPE,typename MATCH,bool SORTONINSERT>
void TSortPolicyPtrSorted<TYPE,MATCH,SORTONINSERT>::Debug_VerifyIsSorted(TPtr<TYPE>* pData,u32 Size) const
{
#if defined(ARRAY_SORT_CHECK)
	{
		if ( !IsSorted() )
			return;
		
		for ( u32 i=1;	i<Size;	i++ )
		{
			TSorter<TPtr<TYPE>,MATCH> Prev( pData[i-1] );
			if ( pData[i] < Prev )
			{
				TLDebug_Break("\"sorted\" array is out of order");			
			}
		}
	}
#endif
}











template<typename TYPE,u32 GROWBY=10,class SORTPOLICY=TSortPolicyPtrNone<TYPE> >
class TPtrArray : public THeapArray<TPtr<TYPE>,GROWBY,SORTPOLICY>
{
private:
	typedef THeapArray<TPtr<TYPE>,GROWBY,SORTPOLICY> TSuper;
	typedef TPtrArray<TYPE,GROWBY,SORTPOLICY> TThis;
	
public:
	TPtrArray()	{}
	TPtrArray(const TPtrArray<TYPE, GROWBY, SORTPOLICY>& Array)	{	TSuper::Copy( Array );	}
	virtual ~TPtrArray()						{	TSuper::SetAll(TPtr<TYPE>(NULL));	}
	
	template<class MATCHTYPE>
	FORCEINLINE TPtr<TYPE>&			FindPtr(const MATCHTYPE& val);
		
	template<class MATCHTYPE>
	FORCEINLINE const TPtr<TYPE>&	FindPtr(const MATCHTYPE& val) const;
	
	FORCEINLINE TPtr<TYPE>&			GetPtrLast()						{	return (TSuper::GetSize()>0) ? GetPtrAt( TSuper::GetLastIndex() ) : TLPtr::GetNullPtr<TYPE>();	}
	FORCEINLINE const TPtr<TYPE>&	GetPtrLast() const					{	return (TSuper::GetSize()>0) ? GetPtrAtConst( TSuper::GetLastIndex() ) : TLPtr::GetNullPtr<TYPE>();	}
	FORCEINLINE TPtr<TYPE>&			GetPtrAt(s32 Index)					{	return (Index >= 0) ? TSuper::ElementAt( Index ) : TLPtr::GetNullPtr<TYPE>();	}
	FORCEINLINE const TPtr<TYPE>&	GetPtrAtConst(s32 Index) const		{	return (Index >= 0) ? TSuper::ElementAtConst( Index ) : TLPtr::GetNullPtr<TYPE>();	}
	
	FORCEINLINE TPtr<TYPE>&			AddPtr(const TPtr<TYPE>& val);		//	add TPtr to array and return the new [more permanant] TPtr reference
	FORCEINLINE TPtr<TYPE>&			AddNewPtr(TYPE* pVal);				//	add a pointer to the array, this is quite fast, but ONLY use it for pointers that are NOT in TPtr's already. use like; AddNewPtr( new TObject() );. CANNOT be a const pointer. This should stop us using this function for pointers that might already be in a TPtr

	bool							RemovePtr(const TPtr<TYPE>& val)	{	s32 Index = TSuper::FindIndex( val.GetObjectPointer() );	return TSuper::RemoveAt( Index );	}
	void							RemoveNull();						//	remove all NULL pointers from array
		
	template<typename FUNCTIONPOINTER>
	FORCEINLINE void				FunctionAll(FUNCTIONPOINTER pFunc);	//	execute this function on every member. will fail if the TYPE isn't a pointer of somekind

	FORCEINLINE TThis&				operator=(const TPtrArray<TYPE, GROWBY, SORTPOLICY>& Array)	{	TSuper::Copy( Array );	return *this;	}
	
protected:
	virtual void					OnArrayShrink(u32 OldSize,u32 NewSize);	//	NULL pointers that have been "removed" but not deallocated
};



//----------------------------------------------------------
//	NULL pointers that have been "removed" but not deallocated
//----------------------------------------------------------
template<typename TYPE,u32 GROWBY,class SORTPOLICY>
void TPtrArray<TYPE,GROWBY,SORTPOLICY>::OnArrayShrink(u32 OldSize,u32 NewSize)
{
	for ( u32 i=NewSize;	i<OldSize;	i++ )
	{
		TSuper::ElementAt(i) = NULL;
	}
}

template<typename TYPE,u32 GROWBY,class SORTPOLICY>
template<class MATCHTYPE>
FORCEINLINE TPtr<TYPE>& TPtrArray<TYPE,GROWBY,SORTPOLICY>::FindPtr(const MATCHTYPE& val)
{
	s32 Index = TSuper::FindIndex(val);
	if ( Index == -1 )
		return TLPtr::GetNullPtr<TYPE>();
	
	return TSuper::ElementAt(Index);	
}



template<typename TYPE,u32 GROWBY,class SORTPOLICY>
template<class MATCHTYPE>
FORCEINLINE const TPtr<TYPE>& TPtrArray<TYPE,GROWBY,SORTPOLICY>::FindPtr(const MATCHTYPE& val) const
{
	u32 Index = TSuper::FindIndex(val);
	if ( Index == -1 )
		return TLPtr::GetNullPtr<TYPE>();
	
	return TSuper::ElementAtConst(Index);	
}


template<typename TYPE,u32 GROWBY,class SORTPOLICY>
FORCEINLINE void TPtrArray<TYPE,GROWBY,SORTPOLICY>::RemoveNull()
{
	for ( s32 i=TSuper::GetLastIndex();	i>=0;	i-- )
	{
		const TPtr<TYPE>& pPtr = TSuper::ElementAtConst(i);
		if ( pPtr )
			continue;

		//	is null, remove
		TSuper::RemoveAt(i);
	}
}


//----------------------------------------------------------------------
//	execute this function on every member. will fail if the TYPE isn't a pointer of somekind
//----------------------------------------------------------------------
template<typename TYPE,u32 GROWBY,class SORTPOLICY>
template<typename FUNCTIONPOINTER>
FORCEINLINE void TPtrArray<TYPE,GROWBY,SORTPOLICY>::FunctionAll(FUNCTIONPOINTER pFunc)
{
	for ( u32 i=0;	i<TSuper::GetSize();	i++ )
	{
		TPtr<TYPE>& pPtr = TSuper::ElementAt(i);
		TYPE* pObject = pPtr.GetObjectPointer();
		(pObject->*pFunc)();
	}
}



//----------------------------------------------------------------------
//	add TPtr to array and return the new [more permanant] TPtr reference
//----------------------------------------------------------------------
template<typename TYPE,u32 GROWBY,class SORTPOLICY>
FORCEINLINE TPtr<TYPE>& TPtrArray<TYPE,GROWBY,SORTPOLICY>::AddPtr(const TPtr<TYPE>& val)
{
	s32 Index = TSuper::Add( val );	
	return GetPtrAt( Index );	
}

//----------------------------------------------------------------------
//	add a pointer to the array, this is quite fast, but ONLY use it for pointers that are NOT in TPtr's already. use like; AddNewPtr( new TObject() );
//----------------------------------------------------------------------
template<typename TYPE,u32 GROWBY,class SORTPOLICY>
FORCEINLINE TPtr<TYPE>& TPtrArray<TYPE,GROWBY,SORTPOLICY>::AddNewPtr(TYPE* pVal)	
{	
	//	get a new TPtr to use...
	TPtr<TYPE>* ppNewPtr = TSuper::AddNew();

	//	failed to grow array?
	if ( !ppNewPtr )
		return TLPtr::GetNullPtr<TYPE>();

	//	set the contents of this pointer we have allocated for ourselves
	TPtr<TYPE>& pNewPtr = *ppNewPtr;

	//	setup TPtr, this should create the new counter
	pNewPtr = pVal;

	return pNewPtr;
}

