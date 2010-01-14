/*------------------------------------------------------
	Fixed length array. Same as a normal array but with 
	bounds checking and a bit easier to pass around to 
	functions

-------------------------------------------------------*/
#pragma once
#include "TLTypes.h"
#include "TLDebug.h"
#include "TArray.h"


template <typename TYPE,int SIZE, class SORTPOLICY=TLArray::SortPolicy_None<TYPE>, class ALLOCATORPOLICY=TLArray::AllocatorPolicy_Default<TYPE> >
class TFixedArray : public TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>
{
public:
	typedef TLArray::SortResult(TSortFunc)(const TYPE&,const TYPE&,const void*);

public:
	TFixedArray(u32 InitialSize=0,TSortFunc* pSortFunc=NULL) : TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::TArray	( pSortFunc )	{	TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::m_Size = InitialSize;	}	//	by default specify that the array is full to allocation

	virtual TYPE*		GetData()							{	return &m_Data[0];	}
	virtual const TYPE*	GetData() const						{	return &m_Data[0];	}
	virtual Bool		SetSize(s32 NewSize);				//	returns FALSE if we couldn't set the size this big - sets the array to the largest size it can
	virtual u32			GetAllocSize()	const				{	return SIZE;	}

	virtual TYPE&		ElementAt(u32 Index)				{	TLDebug_CheckIndex( Index, SIZE );	return m_Data[Index];	}
	virtual const TYPE&	ElementAtConst(u32 Index) const		{	TLDebug_CheckIndex( Index, SIZE );	return m_Data[Index];	}

	//	operators
	FORCEINLINE TYPE&		operator[](int Index)				{	return ElementAt(Index);	}
	FORCEINLINE TYPE&		operator[](u32 Index)				{	return ElementAt(Index);	}
	FORCEINLINE const TYPE&	operator[](int Index) const			{	return ElementAtConst(Index);	}
	FORCEINLINE const TYPE&	operator[](u32 Index) const			{	return ElementAtConst(Index);	}

protected:
	//	array
	virtual void		SetAllocSize(u32 NewSize)			{	};	//	shouldnt be called on fixed array

protected:
	TYPE				m_Data[SIZE];	//	actual data
};			



//------------------------------------------------------------
//	returns FALSE if we couldn't set the size this big -
//	sets the array to the largest size it can
//------------------------------------------------------------
template <typename TYPE,int SIZE, class SORTPOLICY, class ALLOCATORPOLICY>
Bool TFixedArray<TYPE,SIZE, SORTPOLICY, ALLOCATORPOLICY>::SetSize(s32 NewSize)
{
	if ( NewSize < 0 )
		NewSize = 0;

	if ( NewSize <= SIZE )
	{
		TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::m_Size = NewSize;
		return TRUE;
	}

	//	can't set the size this big
	//	set it as big as possible
	TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>::m_Size = SIZE;
	return FALSE;
}

