/*------------------------------------------------------
	Refcounted smart pointer.
	This version of the smart pointer DOES NOT require
	the object to be dervied from a type. (so you can have 
	smart pointers to u32's if you wish)

	Instead a pointer to the original object AND a refcounter
	is passed around.
	This does mean that you should NEVER assign a TPtr to an object
	blindly. Throughout the engine TPtr's should be passed around.
	The only cases where this HAS to be an exception really is for globals
	and member objects...

	To delete/release, use TPtr<X> = NULL;

	gr:
	I *think* it would be possible to implement Duane's dream
	pointer class this way. You could delete the original object
	externally and the smart pointer would know it's deleted.
	(the m_pCounter would be there, but the pointer it points to
	would be NULL)

	gr:
	If any functions are templated with a different TPtr type a TPtr<TYPE> 
	MUST be implemented. For some reason the code doesn't use it properly

	gr:
	There seems to be a bit of a problem (only a problem for debugging) where
	we lose RTTI. VTables are still valid and everything executes properly but
	the debug info is a bit misleading

	(Child is derived from Parent)
	
	TPtr<Parent> pPointer1 = new Child;	
	//	Pointer1's m_pCounter see's the object as a Child, 
	//	even though it's stored as a Parent*

	TPtr<Child> pPointer2 = pPointer1;	
	//	Pointer2's m_pCounter see's the object as a Parent. 
	//	even though it's stored as a Parent* and is actually a Child.
	//	the VTable of pPointer2->m_pObject is correct (points at Child functions)
	//	but debugger think's it's a Parent*
	//	any virtual functions call the Child's virtual still

-------------------------------------------------------*/
#pragma once
#include "TLMemory.h"
#include "TLDebug.h"
#include "TString.h"



template <typename TYPE>
class TPtr;


namespace TLPtr
{
	//---------------------------------------------------------
	//	gr: counter is now outside TPtr, this makes it easier to assign 
	//	TPtr's to the same object, but cast differently
	//
	//	refcounter type - owns original pointer and has a counter. Saves having a RefCount in every object we
	//	want to use a smart pointer for. A pointer to this counter is passed around. 
	//---------------------------------------------------------
	class Counter
	{
	public:
		Counter(void* pObject) :
			m_pObject	( pObject ),
			m_Count		( 1 )
		{
		}

		template<typename TYPE>
		FORCEINLINE TYPE*	GetObject()			{	return static_cast<TYPE*>( m_pObject );	}
		//virtual void		DeleteObject()		{	TLMemory::Delete( m_pObject );	}
		virtual void		DeleteObject()		{	m_pObject = NULL;	}

	public:
		u32			m_Count;

	private:
		void*		m_pObject;
	};

	template<typename TYPE>
	FORCEINLINE TPtr<TYPE>&	GetNullPtr();	//	get a static Null ptr for a type;
};


template <typename TYPE>
class TPtr
{
	template<typename OTHERTYPE> friend class TPtr;

public:
	template<typename OBJTYPE>
	FORCEINLINE TPtr(OBJTYPE* pObject)		: m_pCounter ( NULL )			{	AssignToObject( static_cast<TYPE*>(pObject) );	}
	FORCEINLINE TPtr(TYPE* pObject)			: m_pCounter ( NULL )			{	AssignToObject( static_cast<TYPE*>(pObject) );	}
	FORCEINLINE TPtr()						: m_pCounter ( NULL )			{	}
	template<typename OBJTYPE>
	FORCEINLINE TPtr(const TPtr<OBJTYPE>& Ptr)	: m_pCounter ( NULL )		{	AssignToPtr( Ptr );	}
	FORCEINLINE TPtr(const TPtr<TYPE>& Ptr)		: m_pCounter ( NULL )		{	AssignToPtr( Ptr );	}	//	gr: NEED to implement TYPE even though it should be covered by the templated OBJTYPE version otherwise it calls some other function entirely
	~TPtr()																	{	ReleaseObject();	}
	
	//	accessors
	FORCEINLINE Bool			IsValid() const								{	return (m_pCounter != NULL);	}
	FORCEINLINE u32				GetRefCount() const							{	return m_pCounter ? m_pCounter->m_Count : 0;	}

	//FORCEINLINE TYPE&			operator*()	const							{	return *GetObject();	}	//	gr: safer not to have this... unless required?? easier, safer and clearer to use GetPtr()
	FORCEINLINE TYPE*			operator->()								{	return m_pCounter->GetObject<TYPE>();	}	//	note: no pointer check here for efficieny
	FORCEINLINE const TYPE*		operator->() const							{	return m_pCounter->GetObject<TYPE>();	}	//	note: no pointer check here for efficieny
	FORCEINLINE TYPE*			GetObject()									{	return m_pCounter ? m_pCounter->GetObject<TYPE>() : NULL;	}
	FORCEINLINE const TYPE*		GetObject() const							{	return m_pCounter ? m_pCounter->GetObject<TYPE>() : NULL;	}
	template<typename OBJTYPE>
	FORCEINLINE OBJTYPE*		GetObject()									{	return m_pCounter->GetObject<OBJTYPE>();	}	//	note: no pointer check here for efficieny
	template<typename OBJTYPE>
	FORCEINLINE const OBJTYPE*	GetObject() const							{	return m_pCounter->GetObject<OBJTYPE>();	}	//	note: no pointer check here for efficieny

	template<typename OBJTYPE>
	FORCEINLINE TPtr<TYPE>&		operator=(const TPtr<OBJTYPE>& Ptr)			{	AssignToPtr( Ptr );	return *this;	}			//	assign this pointer to an existing smart pointer
	FORCEINLINE TPtr<TYPE>&		operator=(const TPtr<TYPE>& Ptr)			{	AssignToPtr( Ptr );	return *this;	}			//	gr: NEED to implement TYPE even though it should be covered by the templated OBJTYPE version otherwise it calls some other function entirely
	template<typename OBJTYPE>
	FORCEINLINE TPtr<TYPE>&		operator=(OBJTYPE* pObject)					{	AssignToObject( static_cast<TYPE*>(pObject) );	return *this;	}	//	assign this pointer to an object
	FORCEINLINE TPtr<TYPE>&		operator=(TYPE* pObject)					{	AssignToObject( static_cast<TYPE*>(pObject) );	return *this;	}	//	assign this pointer to an object

	template<typename OBJTYPE>
	FORCEINLINE Bool			operator==(const OBJTYPE& Object) const;	//	gr: GENIUS! now == operators in our TYPE type work through TPtr! :)
	template<typename OBJTYPE>
	FORCEINLINE Bool			operator==(const TPtr<OBJTYPE>& Ptr) const	{	return GetObject() == Ptr.GetObject<TYPE>();	}
	FORCEINLINE Bool			operator==(const TPtr<TYPE>& Ptr) const		{	return GetObject() == Ptr.GetObject();	}
	FORCEINLINE Bool			operator==(const TYPE* pObject) const		{	return GetObject() == pObject;	}

	template<typename OBJTYPE>
	FORCEINLINE Bool			operator!=(const TPtr<OBJTYPE>& Ptr) const	{	return GetObject() != Ptr.GetObject<TYPE>();	}
	FORCEINLINE Bool			operator!=(const TPtr<TYPE>& Ptr) const		{	return GetObject() != Ptr.GetObject();	}
	FORCEINLINE Bool			operator!=(const TYPE* pObject) const		{	return GetObject() != pObject;	}
/*
	template<typename OBJTYPE>
	FORCEINLINE Bool			operator<(const OBJTYPE& Object) const;
	template<typename OBJTYPE>
	FORCEINLINE Bool			operator<(const TPtr<OBJTYPE>& Ptr) const;
	FORCEINLINE Bool			operator<(const TPtr<TYPE>& Ptr) const;
	FORCEINLINE Bool			operator<(const TYPE* pObject) const;
*/
	FORCEINLINE operator Bool	() const									{	return GetObject() != NULL;	} 

protected:
	FORCEINLINE TLPtr::Counter*	GetRefCounter() const						{	return m_pCounter;	}

private:
	template<typename OBJTYPE>
	void		AssignToPtr(const TPtr<OBJTYPE>& Ptr);			//	link to counter and increment
	void		AssignToObject(TYPE* pObject);					//	create a counter and link to this object
	void		ReleaseObject();								//	decrement the counter and delete if this is the last reference

private:
	//	gr: I've overloaded the counter, per-type, so when we're debugging we dont need to cast the void pointer
	class CounterType : public TLPtr::Counter
	{
	public:
		CounterType(TYPE* pObject) : 
			Counter			( pObject ), 
			m_pObjectType	( pObject )
		{
		}

		virtual void	DeleteObject()		{	TLMemory::Delete(m_pObjectType);	Counter::DeleteObject();	m_pObjectType = GetObject<TYPE>();	}	//	DO NOT DELETE m_pObjectType, just NULL it. parent class deletes

	private:
		TYPE*			m_pObjectType;	//	dont read or delete or modify this for anything, only used to read for debugging
	};

private:
	CounterType*		m_pCounter;		//	our actual counter object
};



//---------------------------------------------------------
//	gr: GENIUS! now == operators in our TYPE type work through TPtr! :)
//---------------------------------------------------------
template <typename TYPE>
template<typename OBJTYPE>
Bool TPtr<TYPE>::operator==(const OBJTYPE& Object) const	
{
	const TYPE* pObjectA = GetObject();	
	return pObjectA ? (*pObjectA) == Object : FALSE;		
}
/*
template <typename TYPE>
template<typename OBJTYPE>	
FORCEINLINE Bool TPtr<TYPE>::operator<(const OBJTYPE& Object) const	
{
	const TYPE* pObjectA = GetObject();	
	return pObjectA ? (*pObjectA) < Object : FALSE;	
}

template <typename TYPE>
template<typename OBJTYPE>
FORCEINLINE Bool TPtr<TYPE>::operator<(const TPtr<OBJTYPE>& Ptr) const
{	
	const TYPE* pObjectA = GetObject();	
	const TYPE* pObjectB = Ptr.GetObject<TYPE>();	
	return (pObjectA && pObjectB) ? (*pObjectA) < (*pObjectB) : FALSE;	
}

template <typename TYPE>
FORCEINLINE Bool TPtr<TYPE>::operator<(const TPtr<TYPE>& Ptr) const	
{	
	const TYPE* pObjectA = GetObject();	
	const TYPE* pObjectB = Ptr.GetObject();	
	return (pObjectA && pObjectB) ? (*pObjectA) < (*pObjectB) : FALSE;	
}

template <typename TYPE>
FORCEINLINE Bool TPtr<TYPE>::operator<(const TYPE* pObject) const	
{	
	const TYPE* pObjectA = GetObject();	
	const TYPE* pObjectB = pObject;
	return (pObjectA && pObjectB) ? (*pObjectA) < (*pObjectB) : FALSE;	
}

*/

//----------------------------------------------------------
//	take ownership of this pointer
//----------------------------------------------------------
template <typename TYPE>
void TPtr<TYPE>::AssignToObject(TYPE* pObject)
{
	//	check we're not already assigned to this object
	if ( m_pCounter )
	{
		//	already assigned to this
		if ( m_pCounter->GetObject<TYPE>() == pObject )
			return;

		//	assigned to something else, release current counter
		ReleaseObject();
	}

	if ( m_pCounter )
	{
		TLDebug_Break( TString("Unexpected TPtr counter") );
	}

	//	nothing to assign to
	if ( !pObject )
		return;

	//	create a new counter to link to this object
	m_pCounter = new CounterType( pObject );

	return;	
}


//----------------------------------------------------------
//	link to counter and increment
//----------------------------------------------------------
template<typename TYPE>
template<typename OBJTYPE>
void TPtr<TYPE>::AssignToPtr(const TPtr<OBJTYPE>& Ptr)
{
#if defined(_DEBUG) && !defined(TL_TARGET_IPOD)
	//	gr: simple line to test compatibility of types at compile time
	OBJTYPE* pTestPointerFrom = NULL;
	TYPE* pTestPointerTo = static_cast<TYPE*>( pTestPointerFrom );
	pTestPointerTo;	//	gr: get rid of unused variable warning
#endif

	//	we are already assigned to this object... no change
	if ( GetRefCounter() == Ptr.GetRefCounter() )
		return;

	//	counter is different, release the current assignment (if any)
	ReleaseObject();

	//	now assign to this new counter...
	if ( Ptr.GetRefCounter() )
	{
		//	note: if TYPE and OBJTYPE are different (even if related) you lose the RTTI. the VTable is still valid 
		//		see header notes
		m_pCounter = static_cast<CounterType*>( Ptr.GetRefCounter() );
		//m_pCounter = dynamic_cast<CounterType*>( Ptr.GetRefCounter() );		//	fails to cast even though is valid
		//m_pCounter = reinterpret_cast<CounterType*>( Ptr.GetRefCounter() );
		//m_pCounter = (CounterType*)( Ptr.GetRefCounter() );

		if ( !m_pCounter )
		{
			TLDebug_Break("Pointer conversion has failed type check");
		}
	}

	//	... and increment pointer counter (if valid, m_pCounter could be happily be NULL here!)
	if ( m_pCounter )
		m_pCounter->m_Count++;
}


//----------------------------------------------------------
//	decrement the counter and delete if this is the last reference
//----------------------------------------------------------
template <typename TYPE>
void TPtr<TYPE>::ReleaseObject()
{
	//	nothing associated with this Ptr
	if ( !m_pCounter )
		return;

	//	decrement counter
	m_pCounter->m_Count--;

	//	release if needbe
	if ( m_pCounter->m_Count == 0 )
	{
		//	gr: remove pointer before deletion to avoid anything checking to see if this is non-null during destructors
		CounterType* pCounter = m_pCounter;
		m_pCounter = NULL;

		//	delete the object
		pCounter->DeleteObject();

		//	delete the object's counter
		TLMemory::Delete( pCounter );
	}

	//	remove our reference to this counter
	m_pCounter = NULL;
}






//----------------------------------------------------
//	get a static Null ptr for a type;
//----------------------------------------------------
template<typename TYPE>
FORCEINLINE TPtr<TYPE>&	TLPtr::GetNullPtr()
{
	static TPtr<TYPE> g_pNull;
	
	#ifdef _DEBUG
	{
		if ( g_pNull )
		{
			TLDebug_Break("Null TPtr should always be NULL!");
		}
	}
	#endif
	
	return g_pNull;
}
