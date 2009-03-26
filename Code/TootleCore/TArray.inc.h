#include "TLMemory.h"



//------------------------------------------------
//	initialise members
//------------------------------------------------
template<typename TYPE>
TArray<TYPE>::TArray(TSortFunc* pSortFunc,u16 GrowBy) :
m_Size			( 0 ),
m_pData			( NULL ),
m_Alloc			( 0 ),
m_pSortFunc		( pSortFunc ),
m_Sorted		( TRUE ),
m_GrowBy		( GrowBy )
{
	if ( m_GrowBy == 0 )
	{
		TLArray::Debug::Break( "TArray GrowBy must be at least 1!", __FUNCTION__ );
		m_GrowBy = TArray_GrowByDefault;
	}
}


//------------------------------------------------
//	cleanup
//------------------------------------------------
template<typename TYPE>
TArray<TYPE>::~TArray()
{
	TLMemory::DeleteArray( m_pData );
	m_Alloc = 0;
	m_Size = 0;
}



template<typename TYPE>
s32 TArray<TYPE>::Add(const TYPE& val)
{
	//	add additional element to set
	if ( !SetSize( GetSize()+1 ) )
		return -1;

	//	set new last element
	TYPE& LastElement = ElementLast();
	LastElement = val;

	//	check to see if adding this element keeps the array sorted
	if ( IsSorted() && GetSize() > 1 && m_pSortFunc )
	{
		TYPE& OldLastElement = ElementAt( GetLastIndex()-1 );

		//	adding this to the end would make the array unsorted
		TLArray::SortResult Sorted = m_pSortFunc( OldLastElement, LastElement, NULL );
		if ( Sorted == TLArray::IsGreater )
		{
			SetSorted( FALSE );
		}
	}

	return GetLastIndex();
}


template<typename TYPE>
s32 TArray<TYPE>::Add(const TYPE* pData,u32 Length)
{
	//	check params
	if ( pData == NULL || Length == 0 )
		return -1;

	s32 FirstIndex = -1;

	if ( IsElementDataType() )
	{
		//	can just expand and memcpy
		FirstIndex = GetSize();
		u32 NewSize = GetSize() + Length;
		SetSize( NewSize );

		//	update length in case we can't add that many elements
		Length = GetSize() - FirstIndex;

		//	couldn't allocate any more spaces at all
		if ( Length == 0 )
			return -1;

		//	memcpy data in
		TLMemory::CopyData( &ElementAt(FirstIndex), pData, Length );
	}
	else
	{
		TLArray::Debug::PrintSizeWarning( m_pData[0], Length, __FUNCTION__ );

		//	pre-alloc data
		AddAllocSize( Length );

		//	add each member individually
		FirstIndex = Add( pData[0] );
		for ( u32 i=1;	i<Length;	i++ )
		{
			Add( pData[i] );
		}
	}

	return FirstIndex;
};



template<typename TYPE>
TYPE* TArray<TYPE>::AddNew()
{
	//	grow the array 
	if ( !SetSize( GetSize()+1 ) )
		return NULL;

	//	return new last element
	TYPE& LastElement = ElementLast();

	return &LastElement;
}


//-------------------------------------------------------------------------
//	remove an element explicitly
//-------------------------------------------------------------------------
template<typename TYPE>
template<typename MATCHTYPE>
Bool TArray<TYPE>::Remove(const MATCHTYPE& val)
{
	s32 uIndex = FindIndex(val);

	if(uIndex==-1)
		return FALSE;

	return RemoveAt(uIndex);
}



//-------------------------------------------------------------------------
//	remove an element based on its index. doesnt affect sorted state
//-------------------------------------------------------------------------
template<typename TYPE>
Bool TArray<TYPE>::RemoveAt(u32 Index)
{
	if ( (s32)Index>GetLastIndex() )
		return FALSE;

	//	gr: shifting from the NEXT element back... so shift FROM Index+1 which writes OVER Index+1-1
	ShiftArray( Index+1, -1 );

	return TRUE;
}



template<typename TYPE>
s32 TArray<TYPE>::InsertAt(u32 Index, const TYPE& val, Bool ForcePosition)
{
	//	need to add it onto the end
	if ( (s32)Index > GetLastIndex() )
	{
		if ( ForcePosition )
		{
			//	expand list to fit index
			if ( !SetSize(Index+1) )
				return -1;
		}
		else
		{
			//	set index so its at the end
			if ( !SetSize( GetSize()+1 ) )
				return -1;
			Index = GetLastIndex();
		}
	}
	else if ( (s32)Index <= GetLastIndex() )	//	dont need to shift if its the last index
	{
		ShiftArray(Index,1);
	}

	//	set the val
	ElementAt(Index) = val;

	//	list may no longer be sorted
	SetSorted(FALSE);

	return Index;
}


template<typename TYPE>
s32 TArray<TYPE>::InsertAt(u32 Index, const TYPE* val, u32 Length, Bool ForcePosition)
{
	//	need to add it onto the end
	if ( (s32)Index > GetLastIndex() )
	{
		if ( ForcePosition )
		{
			//	expand list to fit index
			if ( !SetSize(Index+Length) )
				return -1;
		}
		else
		{
			//	set index so its at the end
			Index = GetSize();
			if ( !SetSize( GetSize()+Length ) )
				return -1;
			//Index = GetLastIndex();
		}
	}
	else if ( (s32)Index <= GetLastIndex() )
	{
		//	dont need to shift if its the last index
		ShiftArray(Index,Length);
	}

	//	set the values
	CopyElements( val, Length, Index);

	//	assume no longer sorted
	SetSorted( FALSE );

	return Index;
}


template<typename TYPE>
s32 TArray<TYPE>::Add(const TArray<TYPE>& Array)
{
	if ( !Array.GetSize() )
		return -1;

	s32 FirstPos = Add( Array.GetData(), Array.GetSize() );
	return FirstPos;
}


template<typename TYPE>
s32 TArray<TYPE>::AddUnique(const TArray<TYPE>& Array)
{
	s32 FirstPos = -1;

	for ( u32 i=0;	i<Array.GetSize();	i++ )
	{
		s32 NewIndex = AddUnique( Array[i] );
		if ( NewIndex != -1 )
			FirstPos = NewIndex;
	}

	return FirstPos;
}


template<typename TYPE>
void TArray<TYPE>::Copy(const TArray<TYPE>& Array)
{
	if ( !SetSize(Array.GetSize()) )
		return;

	CopyElements( Array.GetData(), Array.GetSize(), 0 );
}



template<typename TYPE>
template <typename OTHERTYPE>
s32 TArray<TYPE>::Add(const TArray<OTHERTYPE>& Array)
{
	Empty();

	//	pre-alloc data
	AddAllocSize( Array.GetSize() );

	s32 FirstIndex = -1;
	for ( u32 i=0;	i<Array.GetSize();	i++ )
	{
		s32 AddIndex = Add( Array[i] );
		if ( FirstIndex == -1 )
			FirstIndex = AddIndex;
	}

	return FirstIndex;
}

template<typename TYPE>
template<typename OTHERTYPE>
void TArray<TYPE>::Copy(const TArray<OTHERTYPE>& Array)
{
	Empty();

	//	pre-alloc data
	AddAllocSize( Array.GetSize() );

	for ( u32 i=0;	i<Array.GetSize();	i++ )
	{
		if ( Add( Array[i] ) == -1 )
			break;
	}
}

//-------------------------------------------------------------------------
//	copy Length elements from another source into our array at Index
//-------------------------------------------------------------------------
template<typename TYPE>
Bool TArray<TYPE>::CopyElements(const TYPE* pData,u32 Length,u32 Index)
{
	//	check for size/allocation errors
	if ( Length < 1 )
		return FALSE;

	//	make sure we have enough data
	u32 NewSize = Index + Length;
	if ( GetSize() < NewSize )
	{
		SetSize( NewSize );

		//	update length to the amount we can actually copy
		Length = NewSize - Index;

		//	couldnt add any elements
		if ( Length == 0 )
			return FALSE;
	}

	//	copy data
	if ( IsElementDataType() )
	{
		//	copy a load of raw data
		TLMemory::CopyData( &ElementAt(Index), pData, Length );	
	}
	else
	{
		TLArray::Debug::PrintSizeWarning( m_pData[0], Length, __FUNCTION__ );

		//	have to copy via = (or constructors would be nice...)
		for ( u32 i=0;	i<Length;	i++ )
		{
			ElementAt(i+Index) = pData[i];
		}
	}

	//	list may no longer be sorted
	SetSorted(FALSE);

	return TRUE;
}


template<typename TYPE>
void TArray<TYPE>::Move(u32 CurrIndex,u32 NewIndex)
{
	TYPE Item = ElementAt(CurrIndex);
	RemoveAt(CurrIndex);
	InsertAt(NewIndex,Item);

	//	list may no longer be sorted
	SetSorted(FALSE);
}


//---------------------------------------------------------
//	remove a range of elements from the array
//---------------------------------------------------------
template<typename TYPE>
void TArray<TYPE>::RemoveAt(u32 Index,u32 Amount)
{
	u32 From = Index + Amount;
	s32 ShiftAmount = - (s32)Amount;

	ShiftArray( From, ShiftAmount );
}


template<typename TYPE>
void TArray<TYPE>::ShiftArray(u32 From, s32 Amount )
{
	//	nothing to do
	if ( Amount == 0 )	
		return;

	//	emptying the array
	s32 NewSize = GetSize() + Amount;
	if ( NewSize <= 0 )
	{
		SetSize( 0 );
		return;
	}

	if ( Amount > 0 )
	{
		u32 OldSize = GetSize();
		if ( !SetSize( NewSize ) )
			NewSize = GetSize();

		//	if we failed to allocate all the space we need to dont overwrite further than we can
		Amount = NewSize - OldSize;
		if ( Amount <= 0 )
			return;

		if ( IsElementDataType() )
		{
			//	use memmove if we can
			u32 ToIndex = From + Amount;

			//	gr: bugfix: we only move the elements AFTER(and including) from, not ALL the elements, 
			//	this was moving too much memory past the end
			//u32 MoveAmount = OldSize;
			s32 MoveAmount = OldSize - From;
			
			TLMemory::MoveData( &ElementAt(ToIndex), &ElementAt(From), MoveAmount );

			//	testing
			//	gr: should be <=? this MoveData() doesnt seem to corrupt heap..
			for ( s32 i=0;	i<(s32)MoveAmount;	i++ )
			{
				s32 FromIndex = From + i;
				s32 DestIndex = ToIndex + i;
				//ElementAt(DestIndex) = ElementAt(FromIndex);
				TLDebug::CheckIndex( FromIndex, GetSize(), __FUNCTION__ );
				TLDebug::CheckIndex( DestIndex, GetSize(), __FUNCTION__ );
			}

			TLMemory::Platform::MemValidate();
		}
		else
		{
			u32 ToIndex = From + Amount;
		
			u32 MoveAmount = OldSize;

			//	warning here would be nice if amount is really really big
			//	work in reverse so we don't overwrite things we haven't copied yet
			//for ( s32 i=GetLastIndex();	i>(s32)From;	i-- )
			for ( s32 i=MoveAmount-1;	i>=0;	i-- )
			{
				s32 FromIndex = From + i;
				s32 DestIndex = ToIndex + i;
				ElementAt(DestIndex) = ElementAt(FromIndex);
			}
		}
	} 
	else if ( Amount < 0 )
	{
		//	if from outside the array, then we dont need to move anything. 
		//	this probably means RemoveAt() has been called to remove the last element
		if ( (s32)From > GetLastIndex() )
		{
			SetSize( NewSize );
			return;
		}

		s32 AmountPositive = -Amount;

		s32 CopyFromFirst = From;
		s32 CopyToFirst = From - AmountPositive;

		//	number of elements we're keeping
		s32 MoveAmount = GetSize() - From;

		//	make sure we don't try to copy into a negative space (eg if we're moving elements 2..5, 4 spaces left, we don't want to place them at -2..1
		while ( CopyToFirst < 0 )
		{
			CopyToFirst++;
			CopyFromFirst++;
			MoveAmount--;
		}

		if ( MoveAmount > 0 )
		{
			if ( IsElementDataType() )
			{
				TLMemory::MoveData( &ElementAt(CopyToFirst), &ElementAt(CopyFromFirst), (u32)MoveAmount );
			}
			else
			{
				for ( s32 i=0;	i<MoveAmount;	i++ )
				{
					s32 DestIndex = CopyToFirst + i;
					s32 FromIndex = CopyFromFirst + i;

					ElementAt(DestIndex) = ElementAt(FromIndex);	//	+shift
				}
			}
		}
		else
		{
			TLArray::Debug::Print("Nothing moved", __FUNCTION__ );
		}

		//	shrink size to remove the bits we cut off
		SetSize( NewSize );
	}

	//	list may no longer be sorted
	//	gr: not determined by this func any more. Remove()'s dont break sort order
	//SetSorted(FALSE);
}


//--------------------------------------------------------
//	Set a new size for the array, re-alloc as neccesary
//--------------------------------------------------------
template<typename TYPE>
Bool TArray<TYPE>::SetSize(s32 NewSize)
{
	//	check param
	if ( NewSize < 0 )
		NewSize = 0;

	u32 uSize = (u32)NewSize;

	//	no change in size
	if ( m_Size == uSize )
		return TRUE;

	if ( uSize <= GetAllocSize() )
	{
		//	dont need to expand our array
		OnArrayShrink( m_Size, uSize );

		m_Size = uSize;

		if ( m_Size < 2 )
			SetSorted(TRUE);
		

		return TRUE;
	}

	//	need to allocate more data
	SetAllocSize( uSize );

	//	failed to alloc enough data
	if ( GetAllocSize() < uSize )
		return FALSE;

	//	ensure size is set correctly
	m_Size = uSize;

	//	list must be sorted if there will be 1 or less elements, otherwise assume new elements will make it out of order
	SetSorted( uSize<2 );

	return TRUE;
}



//--------------------------------------------------------
//	reallocate the memory in the array to a new sized array
//--------------------------------------------------------
template<typename TYPE>
void TArray<TYPE>::SetAllocSize(u32 NewSize)
{
	//	0 size specified delete all data
	if ( NewSize <= 0 )
	{
		TLMemory::DeleteArray( m_pData );
		m_Alloc = 0;
		m_Size	= 0;
		return;
	}

	//	pad out new alloc size
	u32 NewAlloc = NewSize;
	NewAlloc += m_GrowBy;
	NewAlloc -= NewAlloc % m_GrowBy;

	//	no change in allocation
	if ( NewSize == m_Alloc )
		return;

	//	save off the old data
	//u32 OldAlloc = m_Alloc;
	TYPE* pOldData = m_pData;

	//	alloc new data
	m_pData	= TLMemory::AllocArray<TYPE>( NewAlloc );
	
	//	failed to alloc...
	if ( !m_pData )
	{
		TLArray::Debug::Break("Failed to allocate array", __FUNCTION__ );
		NewAlloc = 0;
	}

	//	update alloc amount
	m_Alloc = NewAlloc;

	//	force removal of items if we have less mem allocated now
	if ( m_Size > m_Alloc )
		m_Size = m_Alloc;

	//	construct new elements
//	if ( NewAlloc > OldAlloc )
//		TLMemory::ConstructArray<TYPE>( &m_pData[OldAlloc], NewAlloc-OldAlloc );

	//	copy old elements
	if ( pOldData )
	{
		u32 CopySize = m_Size;
		if ( NewAlloc < CopySize )
			CopySize = NewAlloc;

		if ( IsElementDataType() )
		{
			TLMemory::CopyData( m_pData, pOldData, CopySize );
		}
		else
		{
			TLArray::Debug::PrintSizeWarning( m_pData[0], CopySize, __FUNCTION__ );

			for ( u32 i=0;	i<CopySize;	i++ )
			{
				m_pData[i] = pOldData[i];
			}
		}
	
		//	delete old data
		TLMemory::DeleteArray( pOldData );
	}

}


//-------------------------------------------------------------------------
//	swap order of two elements
//-------------------------------------------------------------------------
template<typename TYPE>
void TArray<TYPE>::SwapElements(u32 a, u32 b)
{
	TYPE Tmp = ElementAt(a);
	ElementAt(a) = ElementAt(b);
	ElementAt(b) = Tmp;

	//	list may no longer be sorted
	SetSorted(FALSE);
}



//-------------------------------------------------------------------------
//	make all elements the same
//-------------------------------------------------------------------------
template<typename TYPE>
void TArray<TYPE>::SetAll(const TYPE& Val)
{
	if ( IsElementDataType() )
	{
		/*
		//	if we can use memset, use it
		if ( sizeof(TYPE) == 1 )	//	1 byte s
		{
			memset( GetData(), (u8)Val, GetSize() );
		}
		else
		*/
		{
			//	memcpy each element
			for ( u32 i=0;	i<GetSize();	i++ )
			{
				//	gr: memcpy the Val over our 1 ELEMENT
				TLMemory::CopyData( &ElementAt(i), &Val, 1 );
			}
		}
	}
	else
	{
		TLArray::Debug::PrintSizeWarning( m_pData[0], GetSize(), __FUNCTION__ );

		for ( u32 i=0;	i<GetSize();	i++ )
		{
			ElementAt(i) = Val;
		}
	}

	//	list is no longer sorted
	SetSorted(FALSE);
}


//-------------------------------------------------------------------------
//	initial sorting func does stuff that only needs to be done once
//-------------------------------------------------------------------------
template<typename TYPE>
inline void TArray<TYPE>::Sort()
{
	//	already sorted or nothing to sort
	if ( m_Sorted || GetSize() < 2 )
	{
		SetSorted( TRUE );
		return;
	}

	//	do sort
	QuickSort( 0, GetSize()-1 );

	//	we're now sorted!
	SetSorted( TRUE );
}


//-------------------------------------------------------------------------
//	Quicksort recursive func
//-------------------------------------------------------------------------
template<typename TYPE>
void TArray<TYPE>::QuickSort(s32 First, s32 Last)
{
	//	check params
	if ( First >= Last )	return;
	if ( First == -1 )		return;
	if ( Last == -1 )		return;

	s32 End = First;
	for ( s32 Current=First+1;	Current<=Last;	Current++ )
	{
		if ( m_pSortFunc( ElementAtConst(Current), ElementAtConst(First), NULL ) == TLArray::IsLess )
		{
			SwapElements( ++End, Current );
		}
	}

	SwapElements( First, End );
	QuickSort( First, End-1 );
	QuickSort( End+1, Last );
}


//----------------------------------------------------------------------
//	binary chop search
//	recursive version
//----------------------------------------------------------------------
template<typename TYPE>
template<class MATCHTYPE>
s32 TArray<TYPE>::FindIndexSorted(const MATCHTYPE& val,u32 Low,s32 High) const
{
	if ( High < (s32)Low )
		return -1;

	u32 Mid = (Low + (u32)High) / 2;

	//	gr: i think the algorithm is wrong somewhere, array of size 3 gets here with low and high == 3, mid becomes 3, which is out of range!
	if ( Mid >= GetSize() )
		return -1;

	//	see if we've found the element...
	const TYPE& MidElement = ElementAtConst( Mid );
	TLArray::SortResult Sort = m_pSortFunc( MidElement, MidElement, &val );
	if ( Sort == TLArray::IsEqual )
		return Mid;

	//	search next half of array
	if ( Sort == TLArray::IsLess )
		return FindIndexSorted( val, Mid+1, High );
	else //if ( Sort == TLArray::IsGreater )
		return FindIndexSorted( val, Low, Mid-1 );
}


//----------------------------------------------------------------------
//	matches elements but specificlly doesnt use sorting. Use this if you need to find a match that the array is not sorted by
//----------------------------------------------------------------------
template<typename TYPE>
template<class MATCHTYPE>
s32 TArray<TYPE>::FindIndexNoSort(const MATCHTYPE& val,u32 FromIndex) const
{
	u32 Size = GetSize();
	if ( Size == 0 )
		return -1;

	//	search elements
	const TYPE* pFirstElement = &ElementAtConst(0);
	for ( u32 i=FromIndex;	i<Size;	i++ )
	{
		//	gr: less safe, but faster access...
		if ( pFirstElement[i] == val )
			return (s32)i;
	}
	return -1;
};


//----------------------------------------------------------------------
//	get the index of a matching element. -1 if none matched
//----------------------------------------------------------------------
template<typename TYPE>
template<class MATCHTYPE>
s32 TArray<TYPE>::FindIndex(const MATCHTYPE& val,u32 FromIndex) const
{
	u32 Size = GetSize();
	if ( Size == 0 )
		return -1;

	if ( m_pSortFunc && Size > 2 )
	{
		if ( !IsSorted() )
		{
			TLArray::Debug::Print("Warning; unsorted array cannot be sorted because of const FindIndex()", __FUNCTION__ );
			//Sort();
		}

		//	make use of the binary chop as our list is in order
		if ( IsSorted() )
			return FindIndexSorted( val, 0, GetSize() );
	}

	//	search elements
	const TYPE* pFirstElement = &ElementAtConst(0);
	for ( u32 i=FromIndex;	i<Size;	i++ )
	{
		//	gr: less safe, but faster access...
		if ( pFirstElement[i] == val )
			return (s32)i;
	}
	return -1;
};


//----------------------------------------------------------------------
//	get the index of a matching element. -1 if none matched
//----------------------------------------------------------------------
template<typename TYPE>
template<class MATCHTYPE>
s32 TArray<TYPE>::FindIndex(const MATCHTYPE& val,u32 FromIndex)
{
	u32 Size = GetSize();
	if ( Size == 0 )
		return -1;

	//	if this is a sorted array, do a sort when we need to, then we can use a binary chop
	if ( m_pSortFunc && Size > 2 )
	{
		Sort();

		//	make use of the binary chop as our list is in order
		if ( IsSorted() )
			return FindIndexSorted( val, 0, GetSize()-1 );
	}

	//	search elements
	const TYPE* pFirstElement = &ElementAtConst(0);
	for ( u32 i=FromIndex;	i<Size;	i++ )
	{
		if ( pFirstElement[i] == val )
			return (s32)i;
	}
	return -1;
};


//----------------------------------------------------------------------
//	same as FindIndex but works backwards through the array
//----------------------------------------------------------------------
template<typename TYPE>
template<class MATCHTYPE>
s32 TArray<TYPE>::FindIndexReverse(const MATCHTYPE& val,s32 FromIndex) const
{
	if ( FromIndex == -1 )
		FromIndex = GetSize()-1;

	for ( s32 i=FromIndex;	i>=0;	i-- )
	{
		if ( ElementAtConst(i) == val )
			return (s32)i;
	}
	return -1;
};


//----------------------------------------------------------------------
//	find all matches to this value and put them in an array
//----------------------------------------------------------------------
template<typename TYPE>
template<class MATCHTYPE>
u32 TArray<TYPE>::FindAll(TArray<TYPE>& Array,const MATCHTYPE& val)
{
	u32 InitialSize = Array.GetSize();

	for ( u32 i=0;	i<GetSize();	i++ )
	{
		const TYPE& Element = ElementAtConst(i);
		if ( Element == val )
			Array.Add( Element );
	}

	return Array.GetSize() - InitialSize;
}


//----------------------------------------------------------------------
//	execute this function on every member. will fail if the TYPE isn't a pointer of somekind
//----------------------------------------------------------------------
template<typename TYPE>
template<typename FUNCTIONPOINTER>
FORCEINLINE void TArray<TYPE>::FunctionAll(FUNCTIONPOINTER pFunc)
{
	for ( u32 i=0;	i<GetSize();	i++ )
	{
		TYPE& Element = ElementAt(i);
		(Element->*pFunc)();
	}
}


//----------------------------------------------------------------------
//	execute this function for every member as a parameter. Like FunctionAll but can be used with other types of elements.
//----------------------------------------------------------------------
template<typename TYPE>
template<typename FUNCTIONPOINTER>
FORCEINLINE void TArray<TYPE>::FunctionAllAsParam(FUNCTIONPOINTER pFunc)
{
	for ( u32 i=0;	i<GetSize();	i++ )
	{
		TYPE& Element = ElementAt(i);
		(*pFunc)( Element );
	}
}
