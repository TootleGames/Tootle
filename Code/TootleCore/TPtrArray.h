/*------------------------------------------------------

	Array for TPtrs, just has some extra functionality

-------------------------------------------------------*/
#pragma once
#include "TArray.h"
#include "TPtr.h"

//#define TEST_PTRARRAY_CHANGES

namespace TLPtrArray
{
	//	simple type sort function
	template<typename TYPE>
	TLArray::SortResult		SimpleSort(const TPtr<TYPE>& pa,const TPtr<TYPE>& pb,const void* pTestVal)
	{
		//	normally you KNOW what pTestVal's type will be and cast
		//	as the "default" sort func, we ASSUME that pTestVal is of TYPE type.
		const TPtr<TYPE>& TestWithPtr = pTestVal ? *(const TPtr<TYPE>*)pTestVal :  pb;

		const TYPE& a = *pa.GetObjectPointer();
		const TYPE& t = *TestWithPtr.GetObjectPointer();

		//	== turns into 0 (is greater) or 1(equals)
		return a < t ? TLArray::IsLess : (TLArray::SortResult)(a==t);	
	}
	
	template<typename TYPE>
	class SortPolicy_TPtrArray;		// Array sorting using TPtr's

}


template<typename TYPE>
class TLPtrArray::SortPolicy_TPtrArray : public TLArray::SortPolicy_Base< TPtr<TYPE> >
{
private:	
	virtual TLArray::SortResult	SortElementComparison(const TPtr<TYPE>& pa,const TPtr<TYPE>& pb,const void* pTestVal) const;
	
};

// Comparison routine for the key array sort policy.  Not sure if we can actually remove this and use the normal sort instead
// utilising the TPair == and < operators?
template<typename TYPE>
TLArray::SortResult TLPtrArray::SortPolicy_TPtrArray<TYPE>::SortElementComparison(const TPtr<TYPE>& pa,const TPtr<TYPE>& pb,const void* pTestVal) const
{
	//	normally you KNOW what pTestVal's type will be and cast
	//	as the "default" sort func, we ASSUME that pTestVal is of TYPE type.
	const TPtr<TYPE>& TestWithPtr = pTestVal ? *(const TPtr<TYPE>*)pTestVal :  pb;
	
	const TYPE& a = *pa.GetObjectPointer();
	const TYPE& t = *TestWithPtr.GetObjectPointer();
	
	//	== turns into 0 (is greater) or 1(equals)
	return a < t ? TLArray::IsLess : (TLArray::SortResult)(a==t);	
}




template <typename TYPE, class SORTPOLICY=TLArray::SortPolicy_None< TPtr<TYPE> >, class ALLOCATORPOLICY=TLArray::AllocatorPolicy_Default< TPtr<TYPE> > >
class TPtrArray : public TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >
{
public:
	typedef TLArray::SortResult(TSortFunc)(const TPtr<TYPE>&,const TPtr<TYPE>&,const void*);

public:
	TPtrArray(TSortFunc* pSortFunc=NULL,u16 GrowBy=TArray_GrowByDefault) : TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::TArray	( pSortFunc, GrowBy )	{	}
	
#ifdef TEST_PTRARRAY_CHANGES	
	TPtrArray(const TPtrArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>& OtherArray)
	{
		*this = OtherArray;
	}
#endif

	virtual ~TPtrArray()
	{
		TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::SetAll(NULL);
	}
	
	template<class MATCHTYPE>
	FORCEINLINE TPtr<TYPE>&			FindPtr(const MATCHTYPE& val);
		
	template<class MATCHTYPE>
	FORCEINLINE const TPtr<TYPE>&	FindPtr(const MATCHTYPE& val) const;
	
	FORCEINLINE s32					FindPtrIndex(const TPtr<TYPE>& pPtr);	//	find pointer index, if your class has sorting and the == operator matches, use the TArray FindIndex. But this is a handy Ptr specific one

	FORCEINLINE TPtr<TYPE>&			GetPtrLast();						//	fast version to return the last TPtr
	FORCEINLINE const TPtr<TYPE>&	GetPtrLast() const;					//	fast version to return the last TPtr
	FORCEINLINE TPtr<TYPE>&			GetPtrAt(s32 Index);				//	fast version to return TPtr reference at index
	FORCEINLINE const TPtr<TYPE>&	GetPtrAtConst(s32 Index) const;				//	fast version to return TPtr reference at index

	FORCEINLINE TPtr<TYPE>&			AddPtr(const TPtr<TYPE>& val);		//	add TPtr to array and return the new [more permanant] TPtr reference
	FORCEINLINE TPtr<TYPE>&			AddNewPtr(TYPE* pVal);				//	add a pointer to the array, this is quite fast, but ONLY use it for pointers that are NOT in TPtr's already. use like; AddNewPtr( new TObject() );. CANNOT be a const pointer. This should stop us using this function for pointers that might already be in a TPtr

	FORCEINLINE Bool				RemovePtr(const TPtr<TYPE>& pPtr)	{	s32 Index = FindPtrIndex( pPtr );	return (Index==-1) ? FALSE : TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::RemoveAt( Index );	}	//	remove pointer from array. use the TArray::Remove when possible. Only remvoes first matching instance
	void							RemoveNull();						//	remove all NULL pointers from array
		
	template<typename FUNCTIONPOINTER>
	FORCEINLINE void				FunctionAll(FUNCTIONPOINTER pFunc);	//	execute this function on every member. will fail if the TYPE isn't a pointer of somekind

	//	operators
	FORCEINLINE TPtr<TYPE>&			operator[](s32 Index)				{	return TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::ElementAt(Index);	}
	FORCEINLINE TPtr<TYPE>&			operator[](u32 Index)				{	return TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::ElementAt(Index);	}
	FORCEINLINE const TPtr<TYPE>&	operator[](s32 Index) const			{	return TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::ElementAtConst(Index);	}
	FORCEINLINE const TPtr<TYPE>&	operator[](u32 Index) const			{	return TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::ElementAtConst(Index);	}

protected:
	virtual void				OnArrayShrink(u32 OldSize,u32 NewSize);	//	NULL pointers that have been "removed" but not deallocated
};



//----------------------------------------------------------
//	NULL pointers that have been "removed" but not deallocated
//----------------------------------------------------------
template<typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
void TPtrArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::OnArrayShrink(u32 OldSize,u32 NewSize)
{
	for ( u32 i=NewSize;	i<OldSize;	i++ )
	{
		TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::ElementAt(i) = NULL;
	}
}

template<typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
template<class MATCHTYPE>
FORCEINLINE TPtr<TYPE>& TPtrArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::FindPtr(const MATCHTYPE& val)
{
	u32 Index = TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::FindIndex(val);
	if ( Index == -1 )
		return TLPtr::GetNullPtr<TYPE>();
	
	return TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::ElementAt(Index);	
}



template<typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
template<class MATCHTYPE>
FORCEINLINE const TPtr<TYPE>& TPtrArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::FindPtr(const MATCHTYPE& val) const
{
	u32 Index = FindIndex(val);
	if ( Index == -1 )
		return TLPtr::GetNullPtr<TYPE>();
	
	return TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::ElementAtConst(Index);	
}


template<typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
FORCEINLINE void TPtrArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::RemoveNull()
{
	for ( s32 i=TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::GetLastIndex();	i>=0;	i-- )
	{
		const TPtr<TYPE>& pPtr = TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::ElementAtConst(i);
		if ( pPtr )
			continue;

		//	is null, remove
		TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::RemoveAt(i);
	}
}



//----------------------------------------------------------------------
//	execute this function on every member. will fail if the TYPE isn't a pointer of somekind
//----------------------------------------------------------------------
template<typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
template<typename FUNCTIONPOINTER>
FORCEINLINE void TPtrArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::FunctionAll(FUNCTIONPOINTER pFunc)
{
	for ( u32 i=0;	i<TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::GetSize();	i++ )
	{
		TPtr<TYPE>& pPtr = TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::ElementAt(i);
		TYPE* pObject = pPtr.GetObjectPointer();
		(pObject->*pFunc)();
	}
}



//----------------------------------------------------------------------
//	fast version to return the last TPtr
//----------------------------------------------------------------------
template<typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
FORCEINLINE TPtr<TYPE>& TPtrArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::GetPtrLast()
{	
	return (TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::GetSize()>0) ? GetPtrAt( TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::GetLastIndex() ) : TLPtr::GetNullPtr<TYPE>();	
}


//----------------------------------------------------------------------
//	fast version to return the last TPtr
//----------------------------------------------------------------------
template<typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
FORCEINLINE const TPtr<TYPE>& TPtrArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::GetPtrLast() const
{	
	return (TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY>::GetSize()>0) ? GetPtrAtConst( TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::GetLastIndex() ) : TLPtr::GetNullPtr<TYPE>();	
}


//----------------------------------------------------------------------
//	fast version to return TPtr reference at index
//----------------------------------------------------------------------
template<typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
FORCEINLINE TPtr<TYPE>& TPtrArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::GetPtrAt(s32 Index)
{	
	return (Index == -1) ? TLPtr::GetNullPtr<TYPE>() : TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::ElementAt( Index );	
}


//----------------------------------------------------------------------
//	fast version to return TPtr reference at index
//----------------------------------------------------------------------
template<typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
FORCEINLINE const TPtr<TYPE>& TPtrArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::GetPtrAtConst(s32 Index) const
{	
	return (Index == -1) ? TLPtr::GetNullPtr<TYPE>() : TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::ElementAtConst( Index );	
}


//----------------------------------------------------------------------
//	add TPtr to array and return the new [more permanant] TPtr reference
//----------------------------------------------------------------------
template<typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
FORCEINLINE TPtr<TYPE>& TPtrArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::AddPtr(const TPtr<TYPE>& val)
{
	s32 Index = TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::Add( val );	
	return GetPtrAt( Index );	
}

//----------------------------------------------------------------------
//	add a pointer to the array, this is quite fast, but ONLY use it for pointers that are NOT in TPtr's already. use like; AddNewPtr( new TObject() );
//----------------------------------------------------------------------
template<typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
FORCEINLINE TPtr<TYPE>& TPtrArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::AddNewPtr(TYPE* pVal)	
{	
	//	get a new TPtr to use...
	TPtr<TYPE>* ppNewPtr = TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::AddNew();

	//	failed to grow array?
	if ( !ppNewPtr )
		return TLPtr::GetNullPtr<TYPE>();

	//	set the contents of this pointer we have allocated for ourselves
	TPtr<TYPE>& pNewPtr = *ppNewPtr;

	//	setup TPtr, this should create the new counter
	pNewPtr = pVal;

	return pNewPtr;
}


//----------------------------------------------------------------------
//	find pointer index, if your class has sorting and the == operator matches, 
//	use the TArray FindIndex. But this is a handy Ptr specific one
//----------------------------------------------------------------------
template<typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>
FORCEINLINE s32 TPtrArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::FindPtrIndex(const TPtr<TYPE>& pPtr)
{
	for ( u32 i=0;	i<TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::GetSize();	i++ )
	{
		const TPtr<TYPE>& pElementPtr = TArray<TPtr<TYPE>, SORTPOLICY, ALLOCATORPOLICY >::ElementAtConst( i );

		//	do pointer address comparison
		if ( pElementPtr.GetObjectPointer() == pPtr.GetObjectPointer() )
			return (s32)i;
	}

	//	no matches
	return -1;
}

