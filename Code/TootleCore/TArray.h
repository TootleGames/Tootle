/*------------------------------------------------------
	Dynamic array type with various templated type options

	TYPE		the arrays object type
	SORTED		true/false, inserts objects in order 
				benefit is that searches are quicker
				requires the > operator
				array is always ascending

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


class TBinary;

namespace TLArray
{
	enum ElementType
	{
		ClassType,	//	if our elements are classes we need to construct the elements and use = to copy, DONT USE MEMCPY!
		DataType,	//	data elements means we can use dumb mem copys, mem sets etc
	};

	//	added this wrapper for TLDebug_Break so we don't include the string type
	namespace Debug
	{
		Bool	Break(const char* pErrorString,const char* pSourceFile,const int SourceLine);
		void	Print(const char* pErrorString,const char* pSourceFile,const int SourceLine);
	}

	#define TArray_GrowByDefault	4

	enum SortResult
	{
		IsLess = -1,	//	
		IsGreater = 0,	//	for (A==B) == FALSE == 0 == IsGreater
		IsEqual = 1,	//	for (A==B) == TRUE == 1 == IsEqual
	};

	
	//	simple type sort function
	template<typename TYPE>
	inline SortResult	SimpleSort(const TYPE& a,const TYPE& b,const void* pTestVal)
	{
		//	normally you KNOW what pTestVal's type will be and cast
		//	as the "default" sort func, we ASSUME that pTestVal is of TYPE type.
		const TYPE& TestWith = pTestVal ? *(const TYPE*)pTestVal :  b;
		//	== turns into 0 (is greater) or 1(equals)
		return a < TestWith ? IsLess : (SortResult)(a==TestWith);	
	}

};





template<typename TYPE>
class TArray
{
	friend class TBinary;
public:
	typedef TLArray::SortResult(TSortFunc)(const TYPE&,const TYPE&,const void*);

public:
	TArray(TSortFunc* pSortFunc=NULL,u8 GrowBy=TArray_GrowByDefault);
	virtual ~TArray();

	virtual u32			GetSize() const							{	return m_Size;	}						//	number of elements
	inline s32			LastIndex() const						{	return (s32)GetSize() - 1;	};
	virtual TYPE*		GetData()								{	return m_pData;	}
	virtual const TYPE*	GetData() const							{	return m_pData;	}
	inline u32			GetDataSize() const						{	return ( GetSize() * sizeof(TYPE) );	};	//	memory consumption of elements

	virtual void		Copy(const TArray<TYPE>& Array);	//	make this a copy of the specified array
	void				SetAll(const TYPE& Val);				//	set all elements to match this one (uses = operator)

	virtual Bool		SetSize(s32 size);
	void				Empty(Bool Dealloc=FALSE)				{	SetSize(0);	if ( Dealloc )	SetAllocSize(0);	};
	virtual u32			GetAllocSize() const					{	return m_Alloc;	}
	virtual void		SetAllocSize(u32 size);					//	set new amount of allocated data
	virtual void		Compact()								{	SetAllocSize( GetSize() );	}	//	free-up over-allocated data

	virtual TYPE&		ElementAt(u32 Index)					{	TLDebug_CheckIndex( Index, GetSize() );	return m_pData[Index];	}
	virtual const TYPE&	ElementAtConst(u32 Index) const			{	TLDebug_CheckIndex( Index, GetSize() );	return m_pData[Index];	}
	inline TYPE&		ElementLast()							{	return ElementAt( LastIndex() );	};
	inline const TYPE&	ElementLastConst() const				{	return ElementAtConst( LastIndex() );	};

	virtual s32			Add(const TYPE& val);					//	add an element onto the end of the list
	FORCEINLINE s32		AddUnique(const TYPE& val)				{	s32 Index = FindIndex( val );	return (Index == -1) ? Add( val ) : Index;	}
	virtual s32			Add(const TYPE* pData,u32 Length=1);	//	add a number of elements onto the end of the list
	virtual s32			Add(const TArray<TYPE>& Array);	//	add a whole array of this type onto the end of the list
	virtual TYPE*		AddNew();								//	add a new element onto the end of the array and return it. often fastest because if we dont need to grow there's no copying
	Bool				Remove(const TYPE& val);				// remove the specifed object
	virtual Bool		RemoveAt(u32 Index);					//	remove an element from the array at the specified index
	void				RemoveAt(u32 Index,u32 Amount);			//	remove a range of elements from the array
	Bool				RemoveLast()								{	return ( GetSize() ? RemoveAt( LastIndex() ) : FALSE );	};
	virtual s32			Insert(u32 Index, const TYPE& val,Bool ForcePosition=FALSE);
	virtual s32			Insert(u32 Index, const TYPE* val, u32 Length, Bool ForcePosition=FALSE);
	virtual s32			Insert(u32 Index, const TArray<TYPE>& array, Bool ForcePosition=FALSE)		{	return Insert( Index, array.GetData(), array.GetSize(), ForcePosition );	};
	
	FORCEINLINE void	Sort();									//	quick sort the array - this func bails out early if it doesn't need sorting
	FORCEINLINE Bool	IsSorted() const						{	return m_Sorted;	};	//	is the list sorted?
	FORCEINLINE void	SetUnsorted()							{	m_Sorted = FALSE;	}	//	set list as not sorted if we KNOW we might have broken the order
	FORCEINLINE void	SwapElements(u32 a, u32 b);				//	swap 2 elements in the array

	template <class MATCHTYPE>
	s32					FindIndex(const MATCHTYPE& val,u32 FromIndex=0) const;	//	get the index of a matching element. -1 if none matched
	template <class MATCHTYPE>
	s32					FindIndex(const MATCHTYPE& val,u32 FromIndex=0);		//	get the index of a matching element. -1 if none matched

	template <class MATCHTYPE>	
	s32					FindIndexReverse(const MATCHTYPE& val,s32 FromIndex=-1) const;	//	same as FindIndex but works backwards through the array

	template <class MATCHTYPE>	
	inline TYPE*		Find(const MATCHTYPE& val)					{	u32 Index = FindIndex(val);	return (Index==-1) ? NULL : &ElementAt(Index);	};

	template<class MATCHTYPE>	
	u32					FindAll(TArray<TYPE>& Array,const MATCHTYPE& val);		//	find all matches to this value and put them in an array

	template<class MATCHTYPE>	
	inline const TYPE*	FindConst(const MATCHTYPE& val) const		{	u32 Index = FindIndex(val);	return (Index==-1) ? NULL : &ElementAtConst(Index);	};

	template<class MATCHTYPE>
	inline Bool			Exists(const MATCHTYPE& val) const			{	return FindIndex(val)!=-1;	};
	template<class MATCHTYPE>
	inline Bool			Exists(const MATCHTYPE& val)				{	return FindIndex(val)!=-1;	};

	//	operators
	inline TYPE&		operator[](s32 Index)						{	return ElementAt(Index);	}
	inline TYPE&		operator[](u32 Index)						{	return ElementAt(Index);	}
	inline const TYPE&	operator[](s32 Index) const					{	return ElementAtConst(Index);	}
	inline const TYPE&	operator[](u32 Index) const					{	return ElementAtConst(Index);	}

	inline void			operator=(const TArray<TYPE>& Array)			{	Copy( Array );	}
	inline Bool			operator<(const TArray<TYPE>& Array) const	{	return FALSE;	}
	inline Bool			operator==(const TArray<TYPE>& Array) const	{	return (this == &Array);	}

protected:
	void				QuickSort(s32 First, s32 Last);
	FORCEINLINE void	SetSorted(Bool IsSorted)					{	m_Sorted = IsSorted;	};			//	called when list order changes

	//	binary chop search
	template<class MATCHTYPE>
	s32					FindIndexSorted(const MATCHTYPE& val,s32 Low,s32 High) const;

	virtual void		Move(u32 CurrIndex,u32 NewIndex);			//	remove from list in one place and insert it back in
	Bool				CopyElements(const TYPE* pData,u32 Length,u32 Index=0);
	void				ShiftArray(u32 From, s32 Amount );			//	move blocks of data in the array

	FORCEINLINE TLArray::ElementType	GetElementType() const;		//	return data or class type to decide whether to use memcpy's or constructors and = operators. it's a speed/safety thing
	FORCEINLINE Bool					IsElementDataType() const	{	return GetElementType() == TLArray::DataType;	}

	virtual void		OnArrayShrink(u32 OldSize,u32 NewSize)		{	}	//	callback when the array is SHRUNK but nothing is deallocated. added so specific types can clean up as needed (e.g. NULL pointers that are "removed" but not deallocated

protected:
	u32					m_Size;			//	runtime size of the array
	TSortFunc*			m_pSortFunc;
	Bool				m_Sorted;		//	set to true everytime the list is sorted. if any elemets are added, this becomes invalid

private:
	TYPE*				m_pData;		//	array itself
	u32					m_Alloc;		//	allocated amount of data in the array
	u8					m_GrowBy;		//	amount to grow array by. performance benefit is worth the byte :)
};			



//	gr: some specialisations for the element type. they're here instead 
//	of array.inc.h as they're type specific rather than just generic code
template<typename TYPE>
FORCEINLINE TLArray::ElementType TArray<TYPE>::GetElementType() const
{
	//	default assumes we're a class. we specialise non-class ones that we can
	return TLArray::ClassType;
}

#define DECLARE_TARRAY_DATATYPE(TYPE)	\
	template<> FORCEINLINE TLArray::ElementType TArray<TYPE>::GetElementType() const			{	return TLArray::DataType;	}	\
	template<> FORCEINLINE TLArray::ElementType TArray<Type2<TYPE> >::GetElementType() const	{	return TLArray::DataType;	}	\
	template<> FORCEINLINE TLArray::ElementType TArray<Type3<TYPE> >::GetElementType() const	{	return TLArray::DataType;	}	\
	template<> FORCEINLINE TLArray::ElementType TArray<Type4<TYPE> >::GetElementType() const	{	return TLArray::DataType;	}	


DECLARE_TARRAY_DATATYPE( u8 )
DECLARE_TARRAY_DATATYPE( s8 )
DECLARE_TARRAY_DATATYPE( u16 )
DECLARE_TARRAY_DATATYPE( s16 )
DECLARE_TARRAY_DATATYPE( u32 )
DECLARE_TARRAY_DATATYPE( s32 )
DECLARE_TARRAY_DATATYPE( float )
DECLARE_TARRAY_DATATYPE( u64 )

//DECLARE_TARRAY_DATATYPE( SyncBool )
template<> FORCEINLINE TLArray::ElementType TArray<SyncBool>::GetElementType() const			{	return TLArray::DataType;	}






#include "TArray.inc.h"

