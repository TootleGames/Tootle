


#define USE_SORT_POLICY
#define USE_ALLOCATOR_POLICY

//------------------------------------------------
//	initialise members
//------------------------------------------------
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::TArray(TSortFunc* pSortFunc,u16 GrowBy) :
m_Size			( 0 ),
m_pData			( NULL ),
m_Alloc			( 0 ),
m_pSortFunc		( pSortFunc ),
m_GrowBy		( GrowBy )
{
	if ( m_GrowBy == 0 )
	{
		TLArray::Debug::Break( "TArray GrowBy must be at least 1!", __FUNCTION__ );
		m_GrowBy = TArray_GrowByDefault;
	}
}

/*
//------------------------------------------------
//	initialise members and copy other array
//------------------------------------------------
template<typename TYPE>
TArray<TYPE>::TArray(const TArray<TYPE>& OtherArray) :
m_Size			( 0 ),
m_pData			( NULL ),
m_Alloc			( 0 ),
m_pSortFunc		( OtherArray.m_pSortFunc ),
m_Sorted		( OtherArray.m_Sorted ),
m_GrowBy		( OtherArray.m_GrowBy )
{
	Copy( OtherArray );
}
*/

//------------------------------------------------
//	cleanup
//------------------------------------------------
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::~TArray()
{
#ifdef USE_ALLOCATOR_POLICY		
	ALLOCATORPOLICY::Deallocate( m_pData );
#else										
	TLMemory::DeleteArray( m_pData );
#endif
	m_Alloc = 0;
	m_Size = 0;
}



template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
s32 TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::Add(const TYPE& val)
{
	//	add additional element to set
	if ( !SetSize( GetSize()+1 ) )
		return -1;

	//	set new last element
	TYPE& LastElement = ElementLast();
	LastElement = val;

#ifdef USE_SORT_POLICY	
	if(GetSize() > 1)
		SORTPOLICY::OnAdded( ElementAt( GetLastIndex()-1 ), LastElement);
#else	
	//	check to see if adding this element keeps the array sorted
	if ( SORTPOLICY::IsSorted() && GetSize() > 1 && m_pSortFunc )
	{
		TYPE& OldLastElement = ElementAt( GetLastIndex()-1 );

		//	adding this to the end would make the array unsorted
		TLArray::SortResult Sorted = m_pSortFunc( OldLastElement, LastElement, NULL );
		if ( Sorted == TLArray::IsGreater )
		{
			SORTPOLICY::SetSorted( FALSE );
		}
	}
#endif

	return GetLastIndex();
}


template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
s32 TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::Add(const TYPE* pData,u32 Length)
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
#ifdef USE_ALLOCATOR_POLICY		
		ALLOCATORPOLICY::CopyData( &ElementAt(FirstIndex), pData, Length );
#else											
		TLMemory::CopyData( &ElementAt(FirstIndex), pData, Length );
#endif
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



template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
TYPE* TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::AddNew()
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
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
template<typename MATCHTYPE>
Bool TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::Remove(const MATCHTYPE& val)
{
	s32 uIndex = FindIndex(val);

	if(uIndex==-1)
		return FALSE;

	return RemoveAt(uIndex);
}



//-------------------------------------------------------------------------
//	remove an element based on its index. doesnt affect sorted state
//-------------------------------------------------------------------------
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
Bool TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::RemoveAt(u32 Index)
{
	if ( (s32)Index>GetLastIndex() )
		return FALSE;

	//	gr: shifting from the NEXT element back... so shift FROM Index+1 which writes OVER Index+1-1
	ShiftArray( Index+1, -1 );

	return TRUE;
}



template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
s32 TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::InsertAt(u32 Index, const TYPE& val, Bool ForcePosition)
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
	SORTPOLICY::SetSorted(FALSE);

	return Index;
}


template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
s32 TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::InsertAt(u32 Index, const TYPE* val, u32 Length, Bool ForcePosition)
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
	SORTPOLICY::SetSorted( FALSE );

	return Index;
}


template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
s32 TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::Add(const TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>& Array)
{
	if ( !Array.GetSize() )
		return -1;

	s32 FirstPos = Add( Array.GetData(), Array.GetSize() );
	return FirstPos;
}


template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
s32 TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::AddUnique(const TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>& Array)
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


template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
void TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::Copy(const TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>& Array)
{
	if ( !SetSize(Array.GetSize()) )
		return;

	CopyElements( Array.GetData(), Array.GetSize(), 0 );
}



template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
template<typename OTHERTYPE, class OTHERSORTPOLICY, class OTHERALLOCATORPOLICY>
s32 TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::Add(const TArray<OTHERTYPE, OTHERSORTPOLICY, OTHERALLOCATORPOLICY>& Array)
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

template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
template<typename OTHERTYPE, class OTHERSORTPOLICY, class OTHERALLOCATORPOLICY>
void TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::Copy(const TArray<OTHERTYPE, OTHERSORTPOLICY, OTHERALLOCATORPOLICY>& Array)
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
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
Bool TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::CopyElements(const TYPE* pData,u32 Length,u32 Index)
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
#ifdef USE_ALLOCATOR_POLICY		
		ALLOCATORPOLICY::CopyData( &ElementAt(Index), pData, Length );
#else											
		TLMemory::CopyData( &ElementAt(Index), pData, Length );	
#endif
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
	SORTPOLICY::SetSorted(FALSE);

	return TRUE;
}


template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
void TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::Move(u32 CurrIndex,u32 NewIndex)
{
	TYPE Item = ElementAt(CurrIndex);
	RemoveAt(CurrIndex);
	InsertAt(NewIndex,Item);

	//	list may no longer be sorted
	SORTPOLICY::SetSorted(FALSE);
}


//---------------------------------------------------------
//	remove a range of elements from the array
//---------------------------------------------------------
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
void TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::RemoveAt(u32 Index,u32 Amount)
{
	u32 From = Index + Amount;
	s32 ShiftAmount = - (s32)Amount;

	ShiftArray( From, ShiftAmount );
}


template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
void TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::ShiftArray(u32 From, s32 Amount )
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
			
#ifdef USE_ALLOCATOR_POLICY		
			ALLOCATORPOLICY::MoveData( &ElementAt(ToIndex), &ElementAt(From), MoveAmount );
#else												
			TLMemory::MoveData( &ElementAt(ToIndex), &ElementAt(From), MoveAmount );
#endif
			
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

#ifdef USE_ALLOCATOR_POLICY		
			ALLOCATORPOLICY::Validate();
#else															
			TLMemory::Platform::MemValidate();
#endif
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
#ifdef USE_ALLOCATOR_POLICY		
				ALLOCATORPOLICY::MoveData( &ElementAt(CopyToFirst), &ElementAt(CopyFromFirst), (u32)MoveAmount );
#else																
				TLMemory::MoveData( &ElementAt(CopyToFirst), &ElementAt(CopyFromFirst), (u32)MoveAmount );
#endif
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
	//SORTPOLICY::SetSorted(FALSE);
}


//--------------------------------------------------------
//	Set a new size for the array, re-alloc as neccesary
//--------------------------------------------------------
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
Bool TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::SetSize(s32 NewSize)
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
			SORTPOLICY::SetSorted(TRUE);
		

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
	SORTPOLICY::SetSorted( uSize<2 );

	return TRUE;
}



//--------------------------------------------------------
//	reallocate the memory in the array to a new sized array
//--------------------------------------------------------
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
void TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::SetAllocSize(u32 NewSize)
{
	//	0 size specified delete all data
	if ( NewSize <= 0 )
	{
#ifdef USE_ALLOCATOR_POLICY		
		ALLOCATORPOLICY::Deallocate( m_pData );
#else		
		TLMemory::DeleteArray( m_pData );
#endif		
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
#ifdef USE_ALLOCATOR_POLICY		
	m_pData = ALLOCATORPOLICY::Allocate( NewAlloc );
#else			
	m_pData	= TLMemory::AllocArray<TYPE>( NewAlloc );
#endif	
	
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
#ifdef USE_ALLOCATOR_POLICY		
			ALLOCATORPOLICY::CopyData( m_pData, pOldData, CopySize );
#else					
			TLMemory::CopyData( m_pData, pOldData, CopySize );
#endif
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
#ifdef USE_ALLOCATOR_POLICY		
		ALLOCATORPOLICY::Deallocate( pOldData );
#else				
		TLMemory::DeleteArray( pOldData );
#endif
	}

}

//-------------------------------------------------------------------------
//	swap order of two elements
//-------------------------------------------------------------------------
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
void TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::SwapElements(u32 a, u32 b)
{
	//	don't do anything when trying to swap the "same" element
	if ( a == b )
		return;

	TYPE Tmp = ElementAt(a);
	ElementAt(a) = ElementAt(b);
	ElementAt(b) = Tmp;

	//	list may no longer be sorted
	SORTPOLICY::SetSorted(FALSE);
}



//-------------------------------------------------------------------------
//	make all elements the same
//-------------------------------------------------------------------------
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
void TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::SetAll(const TYPE& Val)
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
#ifdef USE_ALLOCATOR_POLICY		
				ALLOCATORPOLICY::CopyData( &ElementAt(i), &Val, 1 );
#else									
				TLMemory::CopyData( &ElementAt(i), &Val, 1 );
#endif
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
	SORTPOLICY::SetSorted(FALSE);
}


//-------------------------------------------------------------------------
//	initial sorting func does stuff that only needs to be done once
//-------------------------------------------------------------------------
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
FORCEINLINE void TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::Sort()
{
#ifdef USE_SORT_POLICY	
	SORTPOLICY::Sort(GetData(), GetSize());
#else
	//	already sorted or nothing to sort
	if ( SORTPOLICY::IsSorted() || GetSize() < 2 )
	{
		SORTPOLICY::SetSorted( TRUE );
		return;
	}

	//	do sort
	QuickSort( 0, GetSize()-1 );

	//	we're now sorted!
	SORTPOLICY::SetSorted( TRUE );
#endif
}


//-------------------------------------------------------------------------
//	initial sorting func does stuff that only needs to be done once
//-------------------------------------------------------------------------
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
FORCEINLINE void TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::SetSortOrder(const TLArray::SortOrder& order)
{
#ifdef USE_SORT_POLICY	
	SORTPOLICY::SetSortOrder(order, GetData(), GetSize());
#else
	
	u32 uSize = GetSize();
	// Simply swap all elements every time this is called for non-sorted arrays
	// Essentially a toogle of data order in the array but not sorted
	if(uSize > 1)
	{
		u32 uLast = uSize-1;
	
		// Rearrange all data elements
		for(u32 uIndex = 0; uIndex < uLast; uIndex++)
		{
			SwapElements(uIndex, uLast--);
		}	
	}
	
#endif
}


#ifndef USE_SORT_POLICY	

//-------------------------------------------------------------------------
//	Quicksort recursive func
//-------------------------------------------------------------------------
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
void TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::QuickSort(s32 First, s32 Last)
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
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
template<class MATCHTYPE>
s32 TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::FindIndexSorted(const MATCHTYPE& val,u32 Low,s32 High,const TYPE* pData) const
{
	if ( High < (s32)Low )
		return -1;

	u32 Mid = (Low + (u32)High) / 2;

	//	gr: i think the algorithm is wrong somewhere, array of size 3 gets here with low and high == 3, mid becomes 3, which is out of range!
	if ( Mid >= GetSize() )
		return -1;

	//	see if we've found the element...
	//	gr: array of data is pre-fetched now and provided, removes unncessary index check (may want to keep this in for _DEBUG builds?) and virtual call
	//const TYPE& MidElement = ElementAtConst( Mid );
	const TYPE& MidElement = pData[Mid];
	TLArray::SortResult Sort = m_pSortFunc( MidElement, MidElement, &val );
	if ( Sort == TLArray::IsEqual )
		return Mid;

	//	search next half of array
	if ( Sort == TLArray::IsLess )
		return FindIndexSorted( val, Mid+1, High, pData );
	else //if ( Sort == TLArray::IsGreater )
		return FindIndexSorted( val, Low, Mid-1, pData );
}

#endif



//----------------------------------------------------------------------
//	get the index of a matching element. -1 if none matched
//----------------------------------------------------------------------
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
template<class MATCHTYPE>
s32 TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::FindIndex(const MATCHTYPE& val,u32 FromIndex) const
{
	u32 uSize = GetSize();
	if ( uSize == 0 )
		return -1;

#ifdef USE_SORT_POLICY	
	return SORTPOLICY::FindIndex(val, GetData(), FromIndex, uSize);
#else
	if ( m_pSortFunc && uSize > 2 )
	{
		if ( !SORTPOLICY::IsSorted() )
		{
			TLArray::Debug::Print("Warning; unsorted array cannot be sorted because of const FindIndex()", __FUNCTION__ );
			//Sort();
		}

		//	make use of the binary chop as our list is in order
		if ( SORTPOLICY::IsSorted() )
			return FindIndexSorted( val, 0, GetSize(), GetData() );
	}
	
	//	walk through elements
	const TYPE* pFirstElement = &ElementAtConst(0);
	for ( u32 i=FromIndex;	i<uSize;	i++ )
	{
		//	gr: less safe, but faster access...
		if ( pFirstElement[i] == val )
			return (s32)i;
	}
	return -1;
	
#endif

};


//----------------------------------------------------------------------
//	get the index of a matching element. -1 if none matched
//----------------------------------------------------------------------
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
template<class MATCHTYPE>
s32 TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::FindIndex(const MATCHTYPE& val,u32 FromIndex)
{
	u32 uSize = GetSize();
	if ( uSize == 0 )
		return -1;

#ifdef USE_SORT_POLICY	
	return SORTPOLICY::FindIndex(val, GetData(), FromIndex, uSize);
#else	
	//	if this is a sorted array, do a sort when we need to, then we can use a binary chop
	if ( m_pSortFunc && uSize > 2 )
	{
		if ( !SORTPOLICY::IsSorted() )
			Sort();

		//	make use of the binary chop as our list is in order
		//	gr: check is sorted again incase we couldn't sort for some reason
		if ( SORTPOLICY::IsSorted() )
			return FindIndexSorted( val, 0, GetSize()-1, GetData() );
	}

	//	walk through elements
	const TYPE* pFirstElement = &ElementAtConst(0);
	for ( u32 i=FromIndex;	i<uSize;	i++ )
	{
		if ( pFirstElement[i] == val )
			return (s32)i;
	}
	return -1;
#endif
};


//----------------------------------------------------------------------
//	matches elements but specificlly doesnt use sorting. Use this if you need to find a match that the array is not sorted by
//----------------------------------------------------------------------
template<typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
template<class MATCHTYPE>
s32 TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::FindIndexNoSort(const MATCHTYPE& val,u32 FromIndex) const
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
//	same as FindIndex but works backwards through the array
//----------------------------------------------------------------------
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
template<class MATCHTYPE>
s32 TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::FindIndexReverse(const MATCHTYPE& val,s32 FromIndex) const
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
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
template<class MATCHTYPE>
u32 TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::FindAll(TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>& Array,const MATCHTYPE& val)
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
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
template<typename FUNCTIONPOINTER>
FORCEINLINE void TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::FunctionAll(FUNCTIONPOINTER pFunc)
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
template< typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
template<typename FUNCTIONPOINTER>
FORCEINLINE void TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::FunctionAllAsParam(FUNCTIONPOINTER pFunc)
{
	for ( u32 i=0;	i<GetSize();	i++ )
	{
		TYPE& Element = ElementAt(i);
		(*pFunc)( Element );
	}
}
