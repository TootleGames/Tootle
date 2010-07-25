/*------------------------------------------------------

	TArray is the base array interface.
 
-------------------------------------------------------*/
#pragma once
#include "TLTypes.h"
#include "TLDebug.h"
#include "TLRandom.h"

//	enable this to do extensive array sorting checks at runtime. The sort checks are always done in the unit test
//	gr: I've only enabled this on the mac as my PC (in VM) is too slow, but my mac is fast enough
#if defined(_DEBUG) && defined(TL_USER_GR) && defined(TL_TARGET_MAC)
	#define ARRAY_SORT_CHECK
#endif

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

#include "TArraySort.h"  // Array Sorting policies
#include "TArrayAllocator.h"  // Allocator policies




template<typename TYPE>
class TArray
{
	friend class TBinary;
public:
	typedef TArray<TYPE> TArrayType;			//	short hand for this array type
public:
	virtual ~TArray()		{	}

	virtual u32				GetSize() const=0;					//	number of elements
	FORCEINLINE bool		IsEmpty() const						{	return GetSize() == 0;	}
	FORCEINLINE s32			GetRandomIndex() const				{	return TLMaths::Rand( GetSize() );	};
	FORCEINLINE s32			GetLastIndex() const				{	return (s32)GetSize() - 1;	};
	virtual TYPE*			GetData()=0;
	virtual const TYPE*		GetData() const=0;
	FORCEINLINE u32			GetDataSize() const					{	return ( GetSize() * sizeof(TYPE) );	};	//	memory consumption of elements
	FORCEINLINE Bool		IsElementDataType() const			{	return TLCore::IsDataType<TYPE>();	}

	template<typename ARRAYTYPE>
	void					Copy(const ARRAYTYPE& Array);			//	make this a copy of the specified array
//	template<> void			Copy(const TArrayType& Array);			//	make this a copy of the specified array
	void					SetAll(const TYPE& Val);				//	set all elements to match this one (uses = operator)

	Bool					SetSize(s32 Size);						//	resize the array.
	void					Empty(Bool Dealloc=FALSE)				{	SetSize(0);	if ( Dealloc )	SetAllocSize(0);	};
	virtual u32				GetAllocSize() const=0;					//	get the currently allocated amount of data
	virtual u32				GetMaxAllocSize() const					{	return TLTypes::GetIntegerMax<s32>();	}	//	gr: though technically we can store u32 elements, we use s32 because sometimes we return signed indexes.
	virtual bool			SetAllocSize(u32 Size)=0;				//	set new amount of allocated data
	FORCEINLINE void		AddAllocSize(u32 Size)					{	SetAllocSize( GetSize() + Size );	}	//	alloc N extra data than we already have
	void					Compact()								{	SetAllocSize( GetSize() );	}	//	free-up over-allocated data

	TYPE&					ElementAt(u32 Index)					{	TLDebug_CheckIndex( Index, GetSize() );	return GetData()[Index];	}
	const TYPE&				ElementAtConst(u32 Index) const			{	TLDebug_CheckIndex( Index, GetSize() );	return GetData()[Index];	}
	FORCEINLINE TYPE&		ElementLast()							{	return ElementAt( GetLastIndex() );	};
	FORCEINLINE const TYPE&	ElementLastConst() const				{	return ElementAtConst( GetLastIndex() );	};
	FORCEINLINE TYPE&		ElementRandom()							{	return ElementAt( GetRandomIndex() );	}
	FORCEINLINE const TYPE&	ElementRandomConst() const				{	return ElementAtConst( GetRandomIndex() );	}

	s32					Add(const TYPE& val);					//	add an element onto the end of the list
	FORCEINLINE s32		AddUnique(const TYPE& val)				{	s32 Index = FindIndex( val );	return (Index == -1) ? Add( val ) : Index;	}
	s32					Add(const TYPE* pData,u32 Length=1);	//	add a number of elements onto the end of the list
	template<class ARRAYTYPE>
	s32					Add(const TArray<ARRAYTYPE>& Array);	//	add a whole array of this type onto the end of the list
//	template<> s32		Add(const TArrayType& Array)			{	return Add( Array.GetData(), Array.GetSize() );	}
	s32					AddUnique(const TArrayType& Array);		//	add each element from an array using AddUnique
	TYPE*				AddNew();								//	add a new element onto the end of the array and return it. often fastest because if we dont need to grow there's no copying
	template<class MATCHTYPE>
	FORCEINLINE Bool	Remove(const MATCHTYPE& val);			// remove the specifed object
	Bool				RemoveAt(u32 Index);					//	remove an element from the array at the specified index
	void				RemoveAt(u32 Index,u32 Amount);			//	remove a range of elements from the array
	Bool				RemoveLast()							{	return ( GetSize() ? RemoveAt( GetLastIndex() ) : FALSE );	};
	s32					InsertAt(u32 Index, const TYPE& val,Bool ForcePosition=FALSE);
	s32					InsertAt(u32 Index, const TYPE* val, u32 Length, Bool ForcePosition=FALSE);
	s32					InsertAt(u32 Index, const TArrayType& array, Bool ForcePosition=FALSE)		{	return InsertAt( Index, array.GetData(), array.GetSize(), ForcePosition );	};

	FORCEINLINE bool	IsSorted() const								{	return GetSortPolicy().IsSorted();	}
	FORCEINLINE void	SetSortOrder(TLArray::TSortOrder::Type Order)	{	GetSortPolicy().SetSortOrder( Order );	}
	FORCEINLINE void	Sort()											{	GetSortPolicy().Sort( GetData(), GetSize() );	}
	FORCEINLINE void	SetUnsorted()									{	GetSortPolicy().SetUnsorted();	}
	DEPRECATED FORCEINLINE void	SwapElements(u32 a, u32 b);				//	swap 2 elements in the array - DB Deprecated.  Do we want to allow this externally?  The sort policy implements this internally now so this shouldn't be necessary


	//	simple templated item index finding (defined inline as some compilers need to have template functions for template classes inside the declaration)
	template<typename MATCHTYPE> s32		FindIndex(const MATCHTYPE& Match,u32 FromIndex=0);	
	template<typename MATCHTYPE> s32		FindIndex(const MATCHTYPE& Match,u32 FromIndex=0) const;
	template<typename MATCHTYPE> s32		FindIndexReverse(const MATCHTYPE& Match,s32 FromIndex=-1) const;
	
	template<class MATCHTYPE> TYPE*			Find(const MATCHTYPE& val)					{	s32 Index = FindIndex(val);	return (Index==-1) ? NULL : &ElementAt(Index);	};
	template<class MATCHTYPE> const TYPE*	FindConst(const MATCHTYPE& val) const		{	u32 Index = FindIndex(val);	return (Index==-1) ? NULL : &ElementAtConst(Index);	};

	template<class MATCHTYPE,class ARRAYTYPE>	
	u32								FindAll(ARRAYTYPE& Array,const MATCHTYPE& val);		//	find all matches to this value and put them in an array
		
	template<class MATCHTYPE> Bool	Exists(const MATCHTYPE& val) const		{	return FindIndex(val)!=-1;	};
	template<class MATCHTYPE> Bool	Exists(const MATCHTYPE& val)			{	return FindIndex(val)!=-1;	};

	template<typename FUNCTIONPOINTER>
	FORCEINLINE void				FunctionAll(FUNCTIONPOINTER pFunc);				//	execute this function on every member. will fail if the TYPE isn't a pointer of somekind
	template<typename FUNCTIONPOINTER>
	FORCEINLINE void				FunctionAllAsParam(FUNCTIONPOINTER pFunc);		//	execute this function for every member as a parameter. Like FunctionAll but can be used with other types of elements.

	bool					Debug_VerifyIsSorted() const;				//	if the array claims to be sorted, ensure it is
	
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

	FORCEINLINE TArrayType&	operator=(const TArray<TYPE>& Array)			{	Copy( Array );		return *this;	}
	FORCEINLINE Bool		operator<(const TArray<TYPE>& Array) const		{	return FALSE;	}
	FORCEINLINE bool		operator==(const TArray<TYPE>& Array) const;
	FORCEINLINE bool		operator!=(const TArray<TYPE>& Array) const;

public:	//	gr: temporarily exposed for the sort policies
	void								ShiftArray(u32 From, s32 Amount );			//	move blocks of data in the array

protected:
	virtual void						DoSetSize(u32 Size)=0;						//	set a new size. no return as the size has already been calculated to be okay
	virtual TSortPolicy<TYPE>&			GetSortPolicy()=0;							//	get the sort policy
	virtual const TSortPolicy<TYPE>&	GetSortPolicy() const=0;					//	get the sort policy
	virtual void						Move(u32 CurrIndex,u32 NewIndex);			//	remove from list in one place and insert it back in
	Bool								CopyElements(const TYPE* pData,u32 Length,u32 Index=0);

	virtual void						OnArrayShrink(u32 OldSize,u32 NewSize)		{	}	//	callback when the array is SHRUNK but nothing is deallocated. added so specific types can clean up as needed (e.g. NULL pointers that are "removed" but not deallocated
};			



/*
//	gr: we ignore the size warning for arrays as we cannot doing anything about it :(
template<>
template<typename ARRAYTYPE>
FORCEINLINE void TLArray::Debug::PrintSizeWarning(const TArray<ARRAYTYPE>& DummyElement,u32 ElementCount,const char* pSourceFunction)
{
}
*/

//----------------------------------------------------------
//	gr: this is the new dynamically allocated array!
//----------------------------------------------------------
template<typename TYPE,u32 GROWBY=10,class SORTPOLICY=TSortPolicyNone<TYPE> >
class THeapArray : public TArray<TYPE>
{
private:
	typedef TArray<TYPE> TSuper;
	typedef THeapArray<TYPE,GROWBY,SORTPOLICY> TThis;
public:
	THeapArray() :
		m_pData			( NULL ),
		m_Size			( 0 ),
		m_Alloc			( 0 ),
		m_SortPolicy	()
	{
	}
	THeapArray(const TThis& Array) :
		m_pData			( NULL ),
		m_Size			( 0 ),
		m_Alloc			( 0 ),
		m_SortPolicy	()
	{
		TSuper::Copy( Array );
	}
	THeapArray(const TArray<TYPE>& Array) :
		m_pData			( NULL ),
		m_Size			( 0 ),
		m_Alloc			( 0 ),
		m_SortPolicy	()
	{
		TSuper::Copy( Array );
	}
	
	virtual ~THeapArray()		
	{ 
		// Release the array data
		if(m_pData)
		{
			TLMemory::DeleteArray( m_pData );
			m_Alloc = 0;
			m_Size	= 0;
			m_pData = NULL;
		}
	}


	virtual u32			GetSize() const		{	return m_Size;	}
	virtual TYPE*		GetData()			{	return m_pData;	}
	virtual const TYPE*	GetData() const		{	return m_pData;	}
	virtual bool		SetAllocSize(u32 NewSize);
	virtual u32			GetAllocSize() const	{	return m_Alloc;	}

	FORCEINLINE TThis&	operator=(const TArray<TYPE>& Array)	{	TSuper::Copy( Array );		return *this;	}
	FORCEINLINE TThis&	operator=(const TThis& Array)			{	TSuper::Copy( Array );		return *this;	}
	
protected:
	virtual void		DoSetSize(u32 Size) {	m_Size = Size;	}

	virtual TSortPolicy<TYPE>&			GetSortPolicy()				{	return m_SortPolicy;	}
	virtual const TSortPolicy<TYPE>&	GetSortPolicy() const		{	return m_SortPolicy;	}
	
private:
	TYPE*				m_pData;		//	array itself
	u32					m_Size;			//	runtime size of the array
	u32					m_Alloc;		//	allocated amount of data in the array
	SORTPOLICY			m_SortPolicy;	//	sort policy - gr: we can save a little memory by inheriting from this class, but we will need to rename the functions
};	




//--------------------------------------------------------
//	reallocate the memory in the array to a new sized array
//--------------------------------------------------------
template<typename TYPE,u32 GROWBY,class SORTPOLICY>
bool THeapArray<TYPE,GROWBY,SORTPOLICY>::SetAllocSize(u32 NewSize)
{
	//	0 size specified delete all data
	if ( NewSize <= 0 )
	{
		TLMemory::DeleteArray( m_pData );
		m_Alloc = 0;
		m_Size	= 0;
		return true;
	}
	
	//	pad out new alloc size
	u32 NewAlloc = NewSize + GROWBY - (NewSize % GROWBY);
	
	//	no need to change allocation
	if ( NewSize <= m_Alloc )
		return true;
	
	//	save off the old data
	//u32 OldAlloc = m_Alloc;
	TYPE* pOldData = m_pData;
	
	//	alloc new data
	TYPE* pNewData = TLMemory::AllocArray<TYPE>( NewAlloc );
	//	failed to alloc...
	if ( !pNewData )
	{
		TLArray::Debug::Break("Failed to allocate array", __FUNCTION__ );
		return false;
	}
	
	//	update alloc info
	m_pData = pNewData;
	m_Alloc = NewAlloc;
	
	//	force removal of items if we have less mem allocated now
	if ( m_Size > m_Alloc )
		m_Size = m_Alloc;
	
	//	copy old elements
	if ( pOldData )
	{
		//	copy as many of the old items as possible
		u32 CopySize = m_Size;
		if ( NewAlloc < CopySize )
			CopySize = NewAlloc;
		
		if ( TArray<TYPE>::IsElementDataType() )
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
	
	return true;
}

#include "TArray.inc.h"


