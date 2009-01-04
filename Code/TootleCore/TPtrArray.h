/*------------------------------------------------------

	Array for TPtrs, just has some extra functionality

-------------------------------------------------------*/
#pragma once
#include "TArray.h"
#include "TPtr.h"



#define TPTR_ARRAY_ENABLE_NULL_PTR



template <typename TYPE>
class TPtrArray : public TArray<TPtr<TYPE> >
{
public:
	typedef TLArray::SortResult(TSortFunc)(const TPtr<TYPE>&,const TPtr<TYPE>&,const void*);

public:
	TPtrArray(TSortFunc* pSortFunc=NULL) : TArray<TPtr<TYPE> >::TArray	( pSortFunc )	{	}

	template<class MATCHTYPE>
#ifdef TPTR_ARRAY_ENABLE_NULL_PTR
	inline TPtr<TYPE>&			FindPtr(const MATCHTYPE& val);
#else
	inline TPtr<TYPE>			FindPtr(const MATCHTYPE& val);
#endif
		
	template<class MATCHTYPE>
#ifdef TPTR_ARRAY_ENABLE_NULL_PTR
	inline const TPtr<TYPE>&	FindPtr(const MATCHTYPE& val) const;
#else
	inline const TPtr<TYPE>		FindPtr(const MATCHTYPE& val) const;
#endif

	inline TPtr<TYPE>&			GetPtrLast()						{	return (TArray<TPtr<TYPE> >::GetSize()>0) ? GetPtrAt( TArray<TPtr<TYPE> >::LastIndex() ) : g_pNullPtr;	}
	inline TPtr<TYPE>&			GetPtrAt(s32 Index)					{	return (Index == -1) ? g_pNullPtr : TArray<TPtr<TYPE> >::ElementAt( Index );	}

	inline TPtr<TYPE>&			AddPtr(const TPtr<TYPE>& val)		{	s32 Index = TArray<TPtr<TYPE> >::Add( val );	return GetPtrAt( Index );	}

	void						RemoveNull();						//	remove all NULL pointers from array

	//	operators
	inline TPtr<TYPE>&			operator[](s32 Index)				{	return TArray<TPtr<TYPE> >::ElementAt(Index);	}
	inline TPtr<TYPE>&			operator[](u32 Index)				{	return TArray<TPtr<TYPE> >::ElementAt(Index);	}
	inline const TPtr<TYPE>&	operator[](s32 Index) const			{	return TArray<TPtr<TYPE> >::ElementAtConst(Index);	}
	inline const TPtr<TYPE>&	operator[](u32 Index) const			{	return TArray<TPtr<TYPE> >::ElementAtConst(Index);	}

protected:
	virtual void				OnArrayShrink(u32 OldSize,u32 NewSize);	//	NULL pointers that have been "removed" but not deallocated

protected:
#ifdef TPTR_ARRAY_ENABLE_NULL_PTR
	static TPtr<TYPE>			g_pNullPtr;
#endif
};


template <typename TYPE>
TPtr<TYPE> TPtrArray<TYPE>::g_pNullPtr;



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

#ifdef TPTR_ARRAY_ENABLE_NULL_PTR
template<typename TYPE>
template<class MATCHTYPE>
inline TPtr<TYPE>& TPtrArray<TYPE>::FindPtr(const MATCHTYPE& val)
{
	u32 Index = FindIndex(val);
	if ( Index == -1 )
		return g_pNullPtr;
	
	return TArray<TPtr<TYPE> >::ElementAt(Index);	
}
#else

template<typename TYPE>
template<class MATCHTYPE>
inline TPtr<TYPE> TPtrArray<TYPE>::FindPtr(const MATCHTYPE& val)
{
	u32 Index = FindIndex(val);
	if ( Index == -1 )
		return NULL;
	
	return TArray<TPtr<TYPE> >::ElementAt(Index);	
}

#endif



#ifdef TPTR_ARRAY_ENABLE_NULL_PTR
template<typename TYPE>
template<class MATCHTYPE>
inline const TPtr<TYPE>& TPtrArray<TYPE>::FindPtr(const MATCHTYPE& val) const
{
	u32 Index = FindIndex(val);
	if ( Index == -1 )
		return g_pNullPtr;
	
	return TArray<TPtr<TYPE> >::ElementAtConst(Index);	
}
#else

template<typename TYPE>
template<class MATCHTYPE>
inline const TPtr<TYPE> TPtrArray<TYPE>::FindPtr(const MATCHTYPE& val) const
{
	u32 Index = FindIndex(val);
	if ( Index == -1 )
		return NULL;
	
	return TArray<TPtr<TYPE> >::ElementAtConst(Index);	
}

#endif


template<typename TYPE>
inline void TPtrArray<TYPE>::RemoveNull()
{
	for ( s32 i=TArray<TPtr<TYPE> >::LastIndex();	i>=0;	i-- )
	{
		const TPtr<TYPE>& pPtr = TArray<TPtr<TYPE> >::ElementAtConst(i);
		if ( pPtr )
			continue;

		//	is null, remove
		TArray<TPtr<TYPE> >::RemoveAt(i);
	}
}

