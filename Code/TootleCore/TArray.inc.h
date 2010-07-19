//	If there are less than N elements, don't bother looking up an iterator and 
//	possibly invoking a binary chop search. (this also helps us skip empty arrays)
//	we should do some judgement but from what I've read this is a good "standard"
#define TArray_SkipIteratorThreshold	5


template< typename TYPE>
s32 TArray<TYPE>::Add(const TYPE& val)
{
	//	add additional element to set
	u32 Size = GetSize();
	if ( !SetSize( Size+1 ) )
		return -1;

	u32 NewIndex = Size;
	Size = Size+1;	//	new size

	TYPE* pData = GetData();

	//	set new last element
	TYPE& LastElement = pData[NewIndex];
	LastElement = val;

	//	update sort policy
	GetSortPolicy().OnAdded( *this, NewIndex, 1 );

	return GetLastIndex();
}


template< typename TYPE>
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
		TLArray::Debug::PrintSizeWarning( GetData()[0], Length, __FUNCTION__ );

		//	pre-alloc data
		AddAllocSize( Length );

		//	add each member individually
		FirstIndex = Add( pData[0] );
		for ( u32 i=1;	i<Length;	i++ )
		{
			Add( pData[i] );
		}
	}

	//	update sort policy
	GetSortPolicy().OnAdded( *this, FirstIndex, Length );
	
	return FirstIndex;
};



template< typename TYPE>
TYPE* TArray<TYPE>::AddNew()
{
	//	grow the array 
	if ( !SetSize( GetSize()+1 ) )
		return NULL;

	//	update sort policy
	SetUnsorted();
	
	//	return new last element
	TYPE& LastElement = ElementLast();

	return &LastElement;
}


//-------------------------------------------------------------------------
//	remove an element explicitly
//-------------------------------------------------------------------------
template< typename TYPE>
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
template< typename TYPE>
Bool TArray<TYPE>::RemoveAt(u32 Index)
{
	if ( (s32)Index>GetLastIndex() )
		return FALSE;

	//	gr: shifting from the NEXT element back... so shift FROM Index+1 which writes OVER Index+1-1
	ShiftArray( Index+1, -1 );

	return TRUE;
}



template< typename TYPE>
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

	//	update sort policy
	GetSortPolicy().OnAdded( *this, Index, 1 );
		
	return Index;
}


template< typename TYPE>
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

	//	update sort policy
	GetSortPolicy().OnAdded( *this, Index, Length );
	
	return Index;
}


template<typename TYPE>
template<typename ARRAYTYPE>
s32 TArray<TYPE>::Add(const TArray<ARRAYTYPE>& Array)
{
	//	pre-alloc data
	AddAllocSize( Array.GetSize() );

	s32 FirstPos = -1;
	for ( u32 i=0;	i<Array.GetSize();	i++ )
	{
		s32 AddedPos = Add( Array[i] );
		if ( FirstPos==-1 )
			FirstPos = AddedPos;
	}
	
	//	update sort policy
	//	gr: done via Add()... batch it for speed?
//	GetSortPolicy().OnAdded( GetData(), GetSize(), Index, 1 );
	
	return FirstPos;
}

/*
//----------------------------------------------------------------
//	specialised for speed (will use memcpy if possible)
//----------------------------------------------------------------
template<typename TYPE>
template<>
s32 TArray<TYPE>::Add(const TArray<TYPE>& Array)
{
	if ( !Array.GetSize() )
		return -1;
	
	s32 FirstPos = Add( Array.GetData(), Array.GetSize() );
	
	//	assume no longer sorted
	SetUnsorted();	
	
	return FirstPos;
}
*/

template< typename TYPE>
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



template< typename TYPE>
template<typename ARRAYTYPE>
void TArray<TYPE>::Copy(const ARRAYTYPE& Array)
{
	Empty();	
	Add( Array );
}

/*
template< typename TYPE>
template<>
void TArray<TYPE>::Copy(const TArray<TYPE>& Array)
{
	Empty();
	Add( Array );
}
*/

//-------------------------------------------------------------------------
//	copy Length elements from another source into our array at Index
//-------------------------------------------------------------------------
template< typename TYPE>
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
		TLArray::Debug::PrintSizeWarning( GetData()[0], Length, __FUNCTION__ );

		//	have to copy via = (or constructors would be nice...)
		for ( u32 i=0;	i<Length;	i++ )
		{
			ElementAt(i+Index) = pData[i];
		}
	}

	//	update sort policy
	GetSortPolicy().OnAdded( *this, Index, Length );

	return TRUE;
}


template< typename TYPE>
void TArray<TYPE>::Move(u32 CurrIndex,u32 NewIndex)
{
	TYPE Item = ElementAt(CurrIndex);
	RemoveAt(CurrIndex);
	InsertAt(NewIndex,Item);

	//	list may no longer be sorted
	SetUnsorted();	
}


//---------------------------------------------------------
//	remove a range of elements from the array
//---------------------------------------------------------
template< typename TYPE>
void TArray<TYPE>::RemoveAt(u32 Index,u32 Amount)
{
	u32 From = Index + Amount;
	s32 ShiftAmount = - (s32)Amount;

	ShiftArray( From, ShiftAmount );
}


template< typename TYPE>
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
			#if defined(_DEBUG)
			{
				//	gr: should be <=? this MoveData() doesnt seem to corrupt heap..
				for	( s32 i=0;	i<(s32)MoveAmount;	i++ )
				{
					s32 FromIndex = From + i;
					s32 DestIndex = ToIndex + i;
					//ElementAt(DestIndex) = ElementAt(FromIndex);
					TLDebug::CheckIndex( FromIndex, GetSize(), __FUNCTION__ );
					TLDebug::CheckIndex( DestIndex, GetSize(), __FUNCTION__ );
				}

				TLMemory::Platform::MemValidate();
			}
			#endif
		}
		else
		{
			//	warning here would be nice if amount is really really big
			//	work in reverse so we don't overwrite things we haven't copied yet
			for ( s32 i=GetLastIndex();	i>(s32)From;	i-- )
			{
				s32 FromIndex = i-1;
				s32 DestIndex = i;
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
}


//--------------------------------------------------------
//	Set a new size for the array, re-alloc as neccesary
//--------------------------------------------------------
template< typename TYPE>
Bool TArray<TYPE>::SetSize(s32 NewSize)
{
	//	check param
	if ( NewSize < 0 )
		NewSize = 0;

	u32 uSize = (u32)NewSize;

	//	no change in size
	if ( GetSize() == uSize )
		return true;

	if ( uSize <= GetAllocSize() )
	{
		//	dont need to expand our array
		OnArrayShrink( GetSize(), uSize );
		DoSetSize( uSize );
		return true;
	}

	//	need to allocate more data
	SetAllocSize( uSize );

	//	failed to alloc enough data
	if ( GetAllocSize() < uSize )
		return false;

	//	ensure size is set correctly
	DoSetSize( uSize );

	//	list must be sorted if there will be 1 or less elements, otherwise assume new elements will make it out of order
	//	gr: slightly dangerous, but for now don't change this state as we want to keep it's sorted state for the post-add call to the sort policy
	//SetUnsorted();	

	return true;
}




//-------------------------------------------------------------------------
//	swap order of two elements
//-------------------------------------------------------------------------
template< typename TYPE>
void TArray<TYPE>::SwapElements(u32 a, u32 b)
{
	//	don't do anything when trying to swap the "same" element
	if ( a == b )
		return;

	TYPE Tmp = ElementAt(a);
	ElementAt(a) = ElementAt(b);
	ElementAt(b) = Tmp;

	//	list may no longer be sorted
	SetUnsorted();	
}



//-------------------------------------------------------------------------
//	make all elements the same
//-------------------------------------------------------------------------
template< typename TYPE>
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
		TLArray::Debug::PrintSizeWarning( GetData()[0], GetSize(), __FUNCTION__ );

		for ( u32 i=0;	i<GetSize();	i++ )
		{
			ElementAt(i) = Val;
		}
	}

	//	if they're all the same... they're sorted! but for now I don't want to put in
	//	a SetSortd virtual on the sortpolicy for this one case
//	GetSortPolicy().SetSorted();
	SetUnsorted();
}


//----------------------------------------------------------------------
//	get the index of a matching element. -1 if none matched
//----------------------------------------------------------------------
template<typename TYPE>
template<class MATCHTYPE>
s32 TArray<TYPE>::FindIndex(const MATCHTYPE& val,u32 FromIndex)
{
	u32 Size = GetSize();

	//	save a load of work here...
	if ( Size < TArray_SkipIteratorThreshold )
	{
		const TYPE* pData = (Size == 0) ? NULL : GetData();
		for ( u32 i=FromIndex;	pData && i<Size;	i++ )
		{
			if ( pData[i] == val )
				return (s32)i;
		}
		return -1;
	}

	//	sort array in case we can use the sort iterator
	Sort();
	
	//	get iterator
	TIterator<TYPE>& Iterator = GetSortPolicy().GetIterator( GetIteratorIdent<TYPE,MATCHTYPE>() );
	return Iterator.FindIndex( &val, GetData(), FromIndex, GetSize() );
}


//----------------------------------------------------------------------
//	get the index of a matching element. -1 if none matched
//----------------------------------------------------------------------
template< typename TYPE>
template<class MATCHTYPE>
s32 TArray<TYPE>::FindIndex(const MATCHTYPE& val,u32 FromIndex) const
{
	u32 Size = GetSize();

	//	save a load of work here...
	if ( Size < TArray_SkipIteratorThreshold )
	{
		const TYPE* pData = (Size == 0) ? NULL : GetData();
		for ( u32 i=FromIndex;	pData && i<Size;	i++ )
		{
			if ( pData[i] == val )
				return (s32)i;
		}
		return -1;
	}

	//	get iterator
	TIterator<TYPE>& Iterator = GetSortPolicy().GetIterator( GetIteratorIdent<TYPE,MATCHTYPE>() );
	return Iterator.FindIndex( &val, GetData(), FromIndex, Size );
}


//----------------------------------------------------------------------
//	same as FindIndex but works backwards through the array
//----------------------------------------------------------------------
template< typename TYPE>
template<class MATCHTYPE>
s32 TArray<TYPE>::FindIndexReverse(const MATCHTYPE& val,s32 FromIndex) const
{
	if ( FromIndex < 0 )
		FromIndex = GetSize() + FromIndex;

	const TYPE* pData = GetData();
	for ( s32 i=FromIndex;	i>=0;	i-- )
	{
		if ( pData[i] == val )
			return (s32)i;
	}

	return -1;
};


//----------------------------------------------------------------------
//	find all matches to this value and put them in an array
//----------------------------------------------------------------------
template< typename TYPE>
template<class MATCHTYPE,class ARRAYTYPE>	
u32 TArray<TYPE>::FindAll(ARRAYTYPE& Array,const MATCHTYPE& val)
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
template< typename TYPE>
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
template< typename TYPE>
template<typename FUNCTIONPOINTER>
FORCEINLINE void TArray<TYPE>::FunctionAllAsParam(FUNCTIONPOINTER pFunc)
{
	for ( u32 i=0;	i<GetSize();	i++ )
	{
		TYPE& Element = ElementAt(i);
		(*pFunc)( Element );
	}
}


template<typename TYPE>
FORCEINLINE bool TArray<TYPE>::operator==(const TArray<TYPE>& Array) const
{
	//	simple fast check in case we're comparing the same objects
	if ( this == &Array )
		return true;

	u32 Size = GetSize();
	
	//	different sizes
	if ( Size != Array.GetSize() )
		return false;
	
	//	make sure all elements match
	for ( u32 i=0;	i<Size;	i++ )
	{
		const TYPE& a = this->ElementAtConst(i);
		const TYPE& b = Array.ElementAtConst(i);
		
		if ( a == b )
			continue;
		
		//	elements are different
		return false;
	}
	
	//	all elements and size matched
	return true;
}


template<typename TYPE>
FORCEINLINE bool TArray<TYPE>::operator!=(const TArray<TYPE>& Array) const
{
	//	simple fast check in case we're comparing the same objects
	if ( this == &Array )
		return false;
	
	u32 Size = GetSize();
	
	//	different sizes
	if ( Size != Array.GetSize() )
		return true;
	
	//	make sure all elements match
	for ( u32 i=0;	i<Size;	i++ )
	{
		const TYPE& a = this->ElementAtConst(i);
		const TYPE& b = Array.ElementAtConst(i);
		
		if ( a == b )
			continue;

		//	elements are different
		return true;
	}
	
	//	all elements and size matched
	return false;
}

