/*------------------------------------------------------

	Array for TPtrs, just has some extra functionality

-------------------------------------------------------*/
#pragma once
#include "TArray.h"
#include "TPtr.h"





template <typename TYPE>
class TPtrArray : public TArray<TPtr<TYPE> >
{
public:
	typedef TLArray::SortResult(TSortFunc)(const TPtr<TYPE>&,const TPtr<TYPE>&,const void*);

public:
	TPtrArray(TSortFunc* pSortFunc=NULL,u16 GrowBy=TArray_GrowByDefault) : TArray<TPtr<TYPE> >::TArray	( pSortFunc, GrowBy )	{	}

	template<class MATCHTYPE>
	inline TPtr<TYPE>&			FindPtr(const MATCHTYPE& val);
		
	template<class MATCHTYPE>
	inline const TPtr<TYPE>&	FindPtr(const MATCHTYPE& val) const;

	inline TPtr<TYPE>&			GetPtrLast()						{	return (TArray<TPtr<TYPE> >::GetSize()>0) ? GetPtrAt( TArray<TPtr<TYPE> >::GetLastIndex() ) : TLPtr::GetNullPtr<TYPE>();	}
	inline TPtr<TYPE>&			GetPtrAt(s32 Index)					{	return (Index == -1) ? TLPtr::GetNullPtr<TYPE>() : TArray<TPtr<TYPE> >::ElementAt( Index );	}

	inline TPtr<TYPE>&			AddPtr(const TPtr<TYPE>& val)		{	s32 Index = TArray<TPtr<TYPE> >::Add( val );	return GetPtrAt( Index );	}

	void						RemoveNull();						//	remove all NULL pointers from array
	
	template<typename FUNCTIONPOINTER>
	FORCEINLINE void			FunctionAll(FUNCTIONPOINTER pFunc);	//	execute this function on every member. will fail if the TYPE isn't a pointer of somekind

	//	operators
	inline TPtr<TYPE>&			operator[](s32 Index)				{	return TArray<TPtr<TYPE> >::ElementAt(Index);	}
	inline TPtr<TYPE>&			operator[](u32 Index)				{	return TArray<TPtr<TYPE> >::ElementAt(Index);	}
	inline const TPtr<TYPE>&	operator[](s32 Index) const			{	return TArray<TPtr<TYPE> >::ElementAtConst(Index);	}
	inline const TPtr<TYPE>&	operator[](u32 Index) const			{	return TArray<TPtr<TYPE> >::ElementAtConst(Index);	}

protected:
	virtual void				OnArrayShrink(u32 OldSize,u32 NewSize);	//	NULL pointers that have been "removed" but not deallocated
};



//----------------------------------------------------------
//	NULL pointers that have been "removed" but not deallocated
//----------------------------------------------------------
template<typename TYPE>
void TPtrArray<TYPE>::OnArrayShrink(u32 OldSize,u32 NewSize)
{
	for ( u32 i=NewSize;	i<OldSize;	i++ )
	{
		TArray<TPtr<TYPE> >::ElementAt(i) = NULL;
	}
}

template<typename TYPE>
template<class MATCHTYPE>
inline TPtr<TYPE>& TPtrArray<TYPE>::FindPtr(const MATCHTYPE& val)
{
	u32 Index = TArray<TPtr<TYPE> >::FindIndex(val);
	if ( Index == -1 )
		return TLPtr::GetNullPtr<TYPE>();
	
	return TArray<TPtr<TYPE> >::ElementAt(Index);	
}



template<typename TYPE>
template<class MATCHTYPE>
inline const TPtr<TYPE>& TPtrArray<TYPE>::FindPtr(const MATCHTYPE& val) const
{
	u32 Index = FindIndex(val);
	if ( Index == -1 )
		return TLPtr::GetNullPtr<TYPE>();
	
	return TArray<TPtr<TYPE> >::ElementAtConst(Index);	
}


template<typename TYPE>
inline void TPtrArray<TYPE>::RemoveNull()
{
	for ( s32 i=TArray<TPtr<TYPE> >::GetLastIndex();	i>=0;	i-- )
	{
		const TPtr<TYPE>& pPtr = TArray<TPtr<TYPE> >::ElementAtConst(i);
		if ( pPtr )
			continue;

		//	is null, remove
		TArray<TPtr<TYPE> >::RemoveAt(i);
	}
}



//----------------------------------------------------------------------
//	execute this function on every member. will fail if the TYPE isn't a pointer of somekind
//----------------------------------------------------------------------
template<typename TYPE>
template<typename FUNCTIONPOINTER>
FORCEINLINE void TPtrArray<TYPE>::FunctionAll(FUNCTIONPOINTER pFunc)
{
	for ( u32 i=0;	i<TArray<TPtr<TYPE> >::GetSize();	i++ )
	{
		TPtr<TYPE>& pPtr = TArray<TPtr<TYPE> >::ElementAt(i);
		TYPE* pObject = pPtr.GetObject();
		(pObject->*pFunc)();
	}
}

