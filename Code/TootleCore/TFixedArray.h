/*------------------------------------------------------
	Fixed length array. Same as a normal array but with 
	bounds checking and a bit easier to pass around to 
	functions

-------------------------------------------------------*/
#pragma once
#include "TLTypes.h"
#include "TLDebug.h"
#include "TArray.h"


template <typename TYPE,u32 SIZE, class SORTPOLICY=TSortPolicyNone<TYPE> >
class TFixedArray : public TArray<TYPE>
{
private:
	typedef TFixedArray<TYPE,SIZE,SORTPOLICY> TThis;

public:
	//	if using this default constructor, there is no garuntee as to the contents of the array! (depends where it was allocated, stack, memheap, CRT, and what kind of build)
	TFixedArray() :
		m_Size			( 0 ),
		m_SortPolicy	()
	{
	}

	TFixedArray(const TArray<TYPE>& Array) :
		m_Size			( 0 ),
		m_SortPolicy	()
	{
		Copy(Array);
	}

	//	gr: if you specify an initial size, you also need to specify an initial value
	TFixedArray(u32 InitialSize,const TYPE& InitialValue) :
		m_Size			( 0 ),
		m_SortPolicy	()
	{
		if ( TArray<TYPE>::SetSize( InitialSize ) )
		{
			TArray<TYPE>::SetAll( InitialValue );
		}
	}

	virtual u32			GetSize() const						{	return m_Size;	}
	virtual TYPE*		GetData()							{	return m_Size ? &m_Data[0] : NULL;	}
	virtual const TYPE*	GetData() const						{	return m_Size ? &m_Data[0] : NULL;	}
	virtual bool		SetAllocSize(u32 NewSize)			{	return NewSize <= SIZE;	}
	virtual u32			GetAllocSize()	const				{	return SIZE;	}
	virtual u32			GetMaxAllocSize() const				{	return SIZE;	}	//	gr: should be the min of this and the base version, but I presume we'll never have a fixed array with 2147483647 elements...

	FORCEINLINE TThis&	operator=(const TArray<TYPE>& Array)	{	Copy( Array );		return *this;	}
	FORCEINLINE TThis&	operator=(const TThis& Array)			{	Copy( Array );		return *this;	}

protected:
	virtual void		DoSetSize(u32 NewSize)				{	m_Size = NewSize;	}
	virtual TSortPolicy<TYPE>&			GetSortPolicy()				{	return m_SortPolicy;	}
	virtual const TSortPolicy<TYPE>&	GetSortPolicy() const		{	return m_SortPolicy;	}

private:
	TYPE				m_Data[SIZE];	//	actual data
	u32					m_Size;			//	running size
	SORTPOLICY			m_SortPolicy;	//	sort policy
};			


