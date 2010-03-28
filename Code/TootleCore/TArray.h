/*------------------------------------------------------
	Dynamic array type with various templated type options

	TYPE				The arrays object type
	SORTPOLICY			Sorting policy class - handles sorting of the array depending on class used.
						Default is the TLArray::SortPolicy_None<TYPE> class that provides no sorting
	ALLOCATORPOLICY		Allocator policy class - handles allocation and deallocation of the array data.
						Default is the TLArray::AllocatorPolicy_Default<TYPE> class
 
	Sorting:
	To make use of the sorting, your class needs to provide
	and == and a < operator. These operators MUST work on the 
	same types!
	you also need to provide == and < for itself, if your class
	is sorted by a member. eg; I want to sort assets by their refs
	so the following needs to be implemented. I dont know how to enforce
	this in code without turning Sorted into a sort-test function pointer 
	or something similar
	operator<(const TAsset& Asset)		{	this->Ref < Asset.Ref	}
	operator<(const TRef& Ref)			{	this->Ref < Ref	}
	operator==(const TRef& Ref)			{	this->Ref == Ref	}

-------------------------------------------------------*/
#pragma once
#include "TLTypes.h"
#include "TLDebug.h"

#include "TArraySort.h"  // Array Sorting policies

#include "TArrayAllocator.h"  // Allocator policies

#define USE_SORT_POLICY
#define USE_ALLOCATOR_POLICY


class TBinary;

namespace TLArray
{
	//	added this wrapper for TLDebug_Break so we don't include the string type
	namespace Debug
	{
		Bool	Break(const char* pErrorString,const char* pSourceFunction);
		void	Print(const char* pErrorString,const char* pSourceFunction);

		template<typename TYPE>
		FORCEINLINE void	PrintSizeWarning(const TYPE& DummyElement,u32 ElementCount,const char* pSourceFunction)
		{
			#ifdef _DEBUG
			if ( ElementCount > 300 )
				Print("Warning: Copying/Setting over 300 elements in a non-data type array", pSourceFunction );
			#endif
		}

	}

	#define TArray_GrowByDefault	4
};




template< typename TYPE, class SORTPOLICY=TLArray::SortPolicy_None<TYPE>, class ALLOCATORPOLICY=TLArray::AllocatorPolicy_Default<TYPE> >
class TArray : public SORTPOLICY, ALLOCATORPOLICY
{
	friend class TBinary;

public:
	typedef TLArray::SortResult(TSortFunc)(const TYPE&,const TYPE&,const void*);
	typedef TArray<TYPE,SORTPOLICY,ALLOCATORPOLICY> TArrayType;			//	short hand for this array type
public:
	TArray(TSortFunc* pSortFunc=NULL,u16 GrowBy=TArray_GrowByDefault);
	//	gr: DO NOT IMPLEMENT THIS - the problem is that it has to make use of the virtual functions in the constructor (bad). 
	//	When this is used with a fixed array (as self) it allocates memory and then won't use it (which doesn't cause a major problem) 
	//	but on release it seems to corrupt the heap for some reason.
	//	it may be possible to use it when specified as an explicit call, but it would still be using virtual functions in the constructor...
//	TArray(const TArray<TYPE>& OtherArray);

	virtual ~TArray();

	FORCEINLINE const u32&	GetSize() const							{	return m_Size;	}						//	number of elements
	FORCEINLINE bool		IsEmpty() const							{	return GetSize() == 0;	}
	FORCEINLINE s32			GetLastIndex() const					{	return (s32)GetSize() - 1;	};
	FORCEINLINE s32			GetRandIndex() const					{	return 0;} //TLMaths::Rand( GetSize() );	};
	virtual TYPE*		GetData()								{	return m_pData;	}
	virtual const TYPE*	GetData() const							{	return m_pData;	}
	FORCEINLINE u32			GetDataSize() const						{	return ( GetSize() * sizeof(TYPE) );	};	//	memory consumption of elements

	template<typename OTHERTYPE, class OTHERSORTPOLICY, class OTHERALLOCATORPOLICY>
	void				Copy(const TArray<OTHERTYPE, OTHERSORTPOLICY, OTHERALLOCATORPOLICY>& Array);	//	make this a copy of the specified array
	virtual void		Copy(const TArrayType& Array);			//	make this a copy of the specified array
	void				SetAll(const TYPE& Val);				//	set all elements to match this one (uses = operator)

	virtual Bool		SetSize(s32 Size);
	void				Empty(Bool Dealloc=FALSE)				{	SetSize(0);	if ( Dealloc )	SetAllocSize(0);	};
	virtual u32			GetAllocSize() const					{	return m_Alloc;	}
	virtual u32			GetMaxAllocSize() const					{	return GetAllocSize();	}	//	todo: implement this properly!
	virtual void		SetAllocSize(u32 Size);					//	set new amount of allocated data
	FORCEINLINE void	AddAllocSize(u32 Size)					{	SetAllocSize( GetSize() + Size );	}	//	alloc N extra data than we already have
	virtual void		Compact()								{	SetAllocSize( GetSize() );	}	//	free-up over-allocated data
	FORCEINLINE void	SetGrowBy(u16 GrowBy)					{	m_GrowBy = (GrowBy == 0) ? TArray_GrowByDefault : GrowBy;	}

	virtual TYPE&		ElementAt(u32 Index)					{	TLDebug_CheckIndex( Index, GetSize() );	return m_pData[Index];	}
	virtual const TYPE&	ElementAtConst(u32 Index) const			{	TLDebug_CheckIndex( Index, GetSize() );	return m_pData[Index];	}
	FORCEINLINE TYPE&		ElementLast()							{	return ElementAt( GetLastIndex() );	};
	FORCEINLINE const TYPE&	ElementLastConst() const				{	return ElementAtConst( GetLastIndex() );	};
	DEPRECATED TYPE&		ElementRand()							{	return ElementAt( GetRandIndex() );	}
	DEPRECATED const TYPE&	ElementRandConst() const				{	return ElementAtConst( GetRandIndex() );	}

	virtual s32			Add(const TYPE& val);					//	add an element onto the end of the list
	FORCEINLINE s32		AddUnique(const TYPE& val)				{	s32 Index = FindIndex( val );	return (Index == -1) ? Add( val ) : Index;	}
	virtual s32			Add(const TYPE* pData,u32 Length=1);	//	add a number of elements onto the end of the list
	virtual s32			Add(const TArrayType& Array);			//	add a whole array of this type onto the end of the list
	template<typename OTHERTYPE, class OTHERSORTPOLICY, class OTHERALLOCATORPOLICY>
	s32					Add(const TArray<OTHERTYPE, OTHERSORTPOLICY, OTHERALLOCATORPOLICY>& Array);	//	add a whole array of this type onto the end of the list
	s32					AddUnique(const TArrayType& Array);		//	add each element from an array using AddUnique
	virtual TYPE*		AddNew();								//	add a new element onto the end of the array and return it. often fastest because if we dont need to grow there's no copying
	template <class MATCHTYPE>
	FORCEINLINE Bool	Remove(const MATCHTYPE& val);				// remove the specifed object
	virtual Bool		RemoveAt(u32 Index);					//	remove an element from the array at the specified index
	void				RemoveAt(u32 Index,u32 Amount);			//	remove a range of elements from the array
	Bool				RemoveLast()							{	return ( GetSize() ? RemoveAt( GetLastIndex() ) : FALSE );	};
	virtual s32			InsertAt(u32 Index, const TYPE& val,Bool ForcePosition=FALSE);
	virtual s32			InsertAt(u32 Index, const TYPE* val, u32 Length, Bool ForcePosition=FALSE);
	virtual s32			InsertAt(u32 Index, const TArrayType& array, Bool ForcePosition=FALSE)		{	return InsertAt( Index, array.GetData(), array.GetSize(), ForcePosition );	};
	
	FORCEINLINE void	Sort();	
	FORCEINLINE void	SetSortOrder(const TLArray::SortOrder& order);	

#ifdef	USE_SORT_POLICY
	DEPRECATED FORCEINLINE void	SwapElements(u32 a, u32 b);				//	swap 2 elements in the array - DB Deprecated.  Do we want to allow this externally?  The sort policy implements this internally now so this shouldn't be necessary
#else	
	FORCEINLINE void	SwapElements(u32 a, u32 b);				//	swap 2 elements in the array
#endif

	template <class MATCHTYPE>
	s32					FindIndex(const MATCHTYPE& val,u32 FromIndex=0) const;	//	get the index of a matching element. -1 if none matched
	template <class MATCHTYPE>
	s32					FindIndex(const MATCHTYPE& val,u32 FromIndex=0);		//	get the index of a matching element. -1 if none matched

	template <class MATCHTYPE>	
	s32					FindIndexReverse(const MATCHTYPE& val,s32 FromIndex=-1) const;	//	same as FindIndex but works backwards through the array

	template <class MATCHTYPE>
	DEPRECATED s32					FindIndexNoSort(const MATCHTYPE& val,u32 FromIndex=0) const;	//	matches elements but specificlly doesnt use sorting. Use this if you need to find a match that the array is not sorted by

	template <class MATCHTYPE>	
	FORCEINLINE TYPE*		Find(const MATCHTYPE& val)					{	u32 Index = FindIndex(val);	return (Index==-1) ? NULL : &ElementAt(Index);	};

	template<class MATCHTYPE>	
	u32					FindAll(TArrayType& Array,const MATCHTYPE& val);		//	find all matches to this value and put them in an array

	template<class MATCHTYPE>	
	FORCEINLINE const TYPE*	FindConst(const MATCHTYPE& val) const		{	u32 Index = FindIndex(val);	return (Index==-1) ? NULL : &ElementAtConst(Index);	};

	template<class MATCHTYPE>
	FORCEINLINE Bool			Exists(const MATCHTYPE& val) const			{	return FindIndex(val)!=-1;	};
	template<class MATCHTYPE>
	FORCEINLINE Bool			Exists(const MATCHTYPE& val)				{	return FindIndex(val)!=-1;	};

	template<typename FUNCTIONPOINTER>
	FORCEINLINE void	FunctionAll(FUNCTIONPOINTER pFunc);				//	execute this function on every member. will fail if the TYPE isn't a pointer of somekind
	template<typename FUNCTIONPOINTER>
	FORCEINLINE void	FunctionAllAsParam(FUNCTIONPOINTER pFunc);		//	execute this function for every member as a parameter. Like FunctionAll but can be used with other types of elements.

	//	operators
	FORCEINLINE TYPE&		operator[](s32 Index)						{	return ElementAt(Index);	}
	FORCEINLINE TYPE&		operator[](u32 Index)						{	return ElementAt(Index);	}
	FORCEINLINE const TYPE&	operator[](s32 Index) const					{	return ElementAtConst(Index);	}
	FORCEINLINE const TYPE&	operator[](u32 Index) const					{	return ElementAtConst(Index);	}

	//	append operator - this means you can now initialise an array in a constructor like so (without a massive performance penalty, but convience can be worth it:)
	//	: m_Array ( TFixedArray<TYPE>() << 0 << 1 << 2 << 3 )
	//	no need to wait for the gcc array initialiser :) (4.5 spec IIRC)
	//	not tested this to see if we can use it in a global, but will try. An in-place array type will be made up and can serve that purpose too.
	FORCEINLINE TArrayType&	operator<<(const TYPE& val)					{	Add( val );		return *this;	}

	FORCEINLINE TArrayType&	operator=(const TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>& Array)			{	Copy( Array );		return *this;	}
	FORCEINLINE Bool		operator<(const TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>& Array) const		{	return FALSE;	}
	FORCEINLINE Bool		operator==(const TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>& Array) const	{	return (this == &Array);	}

protected:

#ifndef	USE_SORT_POLICY
	void				QuickSort(s32 First, s32 Last);

	//	binary chop search
	template<class MATCHTYPE>
	s32					FindIndexSorted(const MATCHTYPE& val,u32 Low,s32 High,const TYPE* pData) const;	
#endif	

	virtual void		Move(u32 CurrIndex,u32 NewIndex);			//	remove from list in one place and insert it back in
	Bool				CopyElements(const TYPE* pData,u32 Length,u32 Index=0);
	void				ShiftArray(u32 From, s32 Amount );			//	move blocks of data in the array

	FORCEINLINE Bool	IsElementDataType() const					{	return TLCore::IsDataType<TYPE>();	}

	virtual void		OnArrayShrink(u32 OldSize,u32 NewSize)		{	}	//	callback when the array is SHRUNK but nothing is deallocated. added so specific types can clean up as needed (e.g. NULL pointers that are "removed" but not deallocated

protected:
	u32					m_Size;			//	runtime size of the array

	TSortFunc*			m_pSortFunc;	// Sort routine pointer.  Now that sorting is abstracted out of the TArray class we should be able to remove this and just have custom policies for specific sorting instead

private:
	TYPE*				m_pData;		//	array itself
	u32					m_Alloc;		//	allocated amount of data in the array
	u16					m_GrowBy;		//	amount to grow array by. performance benefit is worth the byte :)
};			



/*
//	gr: we ignore the size warning for arrays as we cannot doing anything about it :(
template<>
template<typename ARRAYTYPE>
FORCEINLINE void TLArray::Debug::PrintSizeWarning(const TArray<ARRAYTYPE>& DummyElement,u32 ElementCount,const char* pSourceFunction)
{
}
*/


#include "TArray.inc.h"


