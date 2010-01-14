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

//#include "TSmallObject.h"
//#define TEST_COUNTER_USING_SOA

template <typename TYPE>
class TPtr;


namespace TLDebug
{
	Bool				Break(const TString& String,const char* pSourceFunction);
}


namespace TLPtr
{
	namespace Private
	{
		//	allocate a counter object
		//	the counter allocation is awkward because of template function specialisation restrictions. We *have* to implement this for every COUNTERTYPE we use :(
		template<typename TYPE>			FORCEINLINE void	NewCounter(TYPE& Object,u32*& pCounter);
		
		//	delete counter object (default assumes some kind of integer)
		template<typename COUNTERTYPE>	FORCEINLINE void	DeleteCounter(COUNTERTYPE*& pCounter);
		
		//	get the counter from a counter object (default assumes some kind of integer)
		template<typename COUNTERTYPE>	FORCEINLINE u32&	GetCounter(COUNTERTYPE& Counter)					{	return Counter;	}
	}

	template<typename TYPE>
	FORCEINLINE TPtr<TYPE>&	GetNullPtr();	//	get a static Null ptr for a type;
};


template <typename TYPE>
class TPtr
{
	template<typename OTHERTYPE> friend class TPtr;
	typedef u32 COUNTERTYPE;		//	todo: change this into a template param

public:
	template<typename OBJTYPE>
	FORCEINLINE TPtr(OBJTYPE* pObject)		: m_pCounter ( NULL ), m_pObject ( NULL )			{	AssignToObject( static_cast<TYPE*>(pObject) );	}
	FORCEINLINE TPtr(TYPE* pObject)			: m_pCounter ( NULL ), m_pObject ( NULL )			{	AssignToObject( static_cast<TYPE*>(pObject) );	}
	FORCEINLINE TPtr()						: m_pCounter ( NULL ), m_pObject ( NULL )			{	}
	template<typename OBJTYPE>
	FORCEINLINE TPtr(const TPtr<OBJTYPE>& Ptr)	: m_pCounter ( NULL ), m_pObject ( NULL )		{	AssignToPtr( Ptr );	}
	FORCEINLINE TPtr(const TPtr<TYPE>& Ptr)		: m_pCounter ( NULL ), m_pObject ( NULL )		{	AssignToPtr( Ptr );	}	//	gr: NEED to implement TYPE even though it should be covered by the templated OBJTYPE version otherwise it calls some other function entirely
	~TPtr()																						{	ReleaseObject();	}
	
	//	accessors
	FORCEINLINE Bool			IsValid() const								{	return (m_pObject != NULL);	}
	FORCEINLINE u32				GetRefCount() const							{	return m_pCounter ? TLPtr::Private::GetCounter( *m_pCounter ) : 0;	}
	
	FORCEINLINE TYPE*			operator->()								{	return m_pObject;	}	//	note: no pointer check here for efficieny
	FORCEINLINE const TYPE*		operator->() const							{	return m_pObject;	}	//	note: no pointer check here for efficieny
	FORCEINLINE TYPE*			GetObjectPointer()							{	return m_pObject;	}	//	note: no pointer check here for efficieny
	FORCEINLINE const TYPE*		GetObjectPointer() const					{	return m_pObject;	}	//	note: no pointer check here for efficieny
	template<typename OBJTYPE>
	FORCEINLINE OBJTYPE*		GetObjectPointer()							{	return static_cast<OBJTYPE*>( m_pObject );	}	//	note: no pointer check here for efficieny
	template<typename OBJTYPE>
	FORCEINLINE const OBJTYPE*	GetObjectPointer() const					{	return static_cast<const OBJTYPE*>( m_pObject );	}	//	note: no pointer check here for efficieny
	FORCEINLINE					operator TYPE*() const						{	return m_pObject;	}

	template<typename OBJTYPE>
	FORCEINLINE TPtr<TYPE>&		operator=(const TPtr<OBJTYPE>& Ptr)			{	AssignToPtr( Ptr );	return *this;	}			//	assign this pointer to an existing smart pointer
	FORCEINLINE TPtr<TYPE>&		operator=(const TPtr<TYPE>& Ptr)			{	AssignToPtr( Ptr );	return *this;	}			//	gr: NEED to implement TYPE even though it should be covered by the templated OBJTYPE version otherwise it calls some other function entirely
	template<typename OBJTYPE>
	FORCEINLINE TPtr<TYPE>&		operator=(OBJTYPE* pObject)					{	AssignToObject( static_cast<TYPE*>(pObject) );	return *this;	}	//	assign this pointer to an object
	FORCEINLINE TPtr<TYPE>&		operator=(TYPE* pObject)					{	AssignToObject( static_cast<TYPE*>(pObject) );	return *this;	}	//	assign this pointer to an object

	template<typename OBJTYPE>
	FORCEINLINE Bool			operator==(const OBJTYPE& Object) const		{	const TYPE* pObjectA = GetObjectPointer();	return pObjectA ? (*pObjectA) == Object : FALSE;	}	//	gr: oddly, this doesn't work if we use m_pObject directly... only using GetObjectPointer ? (maybe the explicit const pointer use is what removes the ambiguity)
	template<typename OBJTYPE>
	FORCEINLINE Bool			operator==(const TPtr<OBJTYPE>& Ptr) const	{	return m_pObject == Ptr.GetObjectPointer<TYPE>();	}
	FORCEINLINE Bool			operator==(const TPtr<TYPE>& Ptr) const		{	return m_pObject == Ptr.GetObjectPointer();	}
	FORCEINLINE Bool			operator==(const TYPE* pObject) const		{	return m_pObject == pObject;	}

	template<typename OBJTYPE>
	FORCEINLINE Bool			operator!=(const TPtr<OBJTYPE>& Ptr) const	{	return m_pObject != Ptr.GetObjectPointer<TYPE>();	}
	FORCEINLINE Bool			operator!=(const TPtr<TYPE>& Ptr) const		{	return m_pObject != Ptr.GetObjectPointer();	}
	FORCEINLINE Bool			operator!=(const TYPE* pObject) const		{	return m_pObject != pObject;	}

	FORCEINLINE operator Bool	() const									{	return IsValid();	} //	gr: compiler usually uses the TYPE* operator for if ( Ptr ) checks now

protected:
	FORCEINLINE COUNTERTYPE*	GetCounter() const							{	return m_pCounter;	}

private:
	template<typename OBJTYPE>
	FORCEINLINE void			AssignToPtr(const TPtr<OBJTYPE>& Ptr);			//	link to counter and increment
	FORCEINLINE void			AssignToObject(TYPE* pObject);					//	create a counter and link to this object
	FORCEINLINE void			ReleaseObject();								//	decrement the counter and delete if this is the last reference
	
private:
	FORCEINLINE void			IncrementCounter()								{	u32& RealCounter = GetCounterValue();	RealCounter++;	}
	FORCEINLINE void			DecrementCounter()								{	u32& RealCounter = GetCounterValue();	RealCounter--;	}
	FORCEINLINE u32&			GetCounterValue()								{	return TLPtr::Private::GetCounter( *m_pCounter );	}
	FORCEINLINE void			NewCounter()									{	TLPtr::Private::NewCounter( *m_pObject, m_pCounter );	}			//	default type, the object IS the counter
	FORCEINLINE void			DeleteCounter()									{	TLPtr::Private::DeleteCounter( m_pCounter );	}		//	default type is the object, so nothing to delete, just NULL the counter pointer
	
private:
	COUNTERTYPE*		m_pCounter;		//	this is a pointer to the global counter (a u32)
	TYPE*				m_pObject;		//	this is a pointer to the real object. its stored in our type. due to the way the new system works, this poitner could be non-null but deleted. But if you always use isvalid/etc(m_pCounter should be null) then not a problem! The theory goes that this will be valid anyway, if we reach zero count, then this should be the last pointer!
};


//----------------------------------------------------------
//	take ownership of this pointer
//----------------------------------------------------------
template <typename TYPE>
FORCEINLINE void TPtr<TYPE>::AssignToObject(TYPE* pObject)
{
	//	check we're not already assigned to this object
	if ( m_pCounter )
	{
		//	already assigned to this
		if ( m_pObject == pObject )
			return;

		//	assigned to something else, release current counter
		ReleaseObject();
	}

	if ( m_pCounter || m_pObject )
	{
		TLDebug_Break( TString("Unexpected TPtr counter/object") );
	}

	//	nothing to assign to
	if ( !pObject )
		return;

	//	create a new counter...
	NewCounter();
	IncrementCounter();

	//	store pointer to object
	m_pObject = pObject;

	return;	
}


//----------------------------------------------------------
//	link to counter and increment
//----------------------------------------------------------
template<typename TYPE>
template<typename OBJTYPE>
FORCEINLINE void TPtr<TYPE>::AssignToPtr(const TPtr<OBJTYPE>& Ptr)
{
	//	we are already assigned to this object... no change
	if ( GetCounter() == Ptr.GetCounter() )
		return;

	//	counter is different, release the current assignment (if any)
	ReleaseObject();

	//	nothing to do
	if( !Ptr )
		return;
	
	//	and store pointer to the actual object
	//	use a static_cast to allow TPtr<B> (derived from A) = TPtr<A>
	//	dynamic_cast is a runtime-type check
#ifdef _DEBUG
	const TYPE* pConstObject = dynamic_cast<const TYPE*>( Ptr.GetObjectPointer() );

	//	cast failed, break and static cast if we continue
	if ( !pConstObject )
	{
		if ( TLDebug::Break("Failed to cast pointer type to other type", __FUNCTION__ ) )
		{
			pConstObject = static_cast<const TYPE*>( Ptr.GetObjectPointer() );
		}
	}
#else
	const TYPE* pConstObject = static_cast<const TYPE*>( Ptr.GetObjectPointer() );
#endif

	//	cast succeeded
	if ( pConstObject )
	{
		//	special case to allow a cast down, technically we shouldn't allow TYPE* p = const TYPE* o though...
		//	maybe we should change this so we can't assign a const TPtr to a TPtr...
		m_pObject = const_cast<TYPE*>( pConstObject );
		
		//	store pointer to counter
		m_pCounter = Ptr.GetCounter();

		//	... and increment pointer counter
		IncrementCounter();
	}
	
}


//----------------------------------------------------------
//	decrement the counter and delete if this is the last reference
//----------------------------------------------------------
template <typename TYPE>
FORCEINLINE void TPtr<TYPE>::ReleaseObject()
{
	//	nothing associated with this Ptr
	if ( !m_pCounter )
		return;

	//	decrement counter
	DecrementCounter();

	//	delete object if needbe
	if ( GetCounterValue() == 0 )
	{
		//	delete data and null pointers
		DeleteCounter();

		TLMemory::Delete( m_pObject );
	}
	else
	{
		//	just un-associate ourselves with this object and counter as counter is > 0 so something is still using them
		m_pCounter = NULL;
		m_pObject = NULL;
	}
}




//----------------------------------------------------------
//	allocate a new counter object (u32 specialisation)
//----------------------------------------------------------
template<typename TYPE>
FORCEINLINE void TLPtr::Private::NewCounter(TYPE& Object,u32*& pCounter)		
{
#ifdef _DEBUG
	if ( pCounter )
	{
		TLDebug_Break("Allocating counter in smart pointer when it already has one. Design error!");
		DeleteCounter(pCounter);
	}
#endif	
	
#ifdef TEST_COUNTER_USING_SOA
	pCounter = TLMemory::SOAAllocate<u32>(sizeof(u32), TRUE);
#else
	pCounter = new u32;
#endif
	
	//	initialise counter
	*pCounter = 0;
}


//----------------------------------------------------------
//	delete a generic counter object
//----------------------------------------------------------
template<typename COUNTERTYPE>
FORCEINLINE void TLPtr::Private::DeleteCounter(COUNTERTYPE*& pCounter)		
{
#ifdef _DEBUG
	if ( !pCounter )
	{
		TLDebug_Break("Trying to delete counter that doesn't exist. Design error!");
		return;
	}
#endif	
	
#ifdef TEST_COUNTER_USING_SOA
	TLMemory::SOADelete( pCounter );
#else
	delete pCounter;
#endif
	pCounter = NULL;
}



//------------------------------------------------------------------
//	this is the intrusive part of the intrusive smart pointer.
//	contains the reference count, but doesn't do anything else.
//------------------------------------------------------------------
class TIntPtrObject
{
public:
	TIntPtrObject() : m_PtrCounter ( 0 )	{}
	
	u32&		GetPtrRefCounter()		{	return m_PtrCounter;	}
	//	FORCEINLINE operator++()			{	m_PtrCounter++;	}
	//	FORCEINLINE operator--()			{	m_PtrCounter--;	}
	
private:
	u32			m_PtrCounter;		//	ref count for this object
};

//	specialisations for the TIntPtrObject counter type
namespace TLPtr
{
	namespace Private
	{
		template<typename TYPE>	FORCEINLINE void NewCounter(TYPE& Object,TIntPtrObject*& pCounter)	{	pCounter = &Object;	}
		template<>				FORCEINLINE void DeleteCounter(TIntPtrObject*& pCounter)			{	pCounter = NULL;	}
		template<>				FORCEINLINE u32& GetCounter(TIntPtrObject& Counter)					{	return Counter.GetPtrRefCounter();	}
	}
}



//----------------------------------------------------
//	get a static Null ptr for a type;
//----------------------------------------------------
template<typename TYPE>
FORCEINLINE TPtr<TYPE>&	TLPtr::GetNullPtr()
{
	static TPtr<TYPE> g_pNull;
	
	if ( g_pNull )
	{
		TLDebug_Break("Null TPtr should always be NULL!");
	}
	
	return g_pNull;
}



