/*------------------------------------------------------
	
	Templated class factory class.
	
	These factorys are based around Ref's so there should
	be plenty of common code.

	Overload these for your own creation routines.

	The basic idea is that each item is unique and is referenced
	by a Ref. The types are Ref'd too. 

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TPtrArray.h>
#include <TootleCore/TRef.h>


#define TClassFactory_GrowByDefault	40


template<class TYPE, bool STOREINSTANCES = true>
class TClassFactory : public TPtrArray<TYPE>
{
public:
	typedef TLArray::SortResult(TSortFunc)(const TPtr<TYPE>&,const TPtr<TYPE>&,const void*);

public:
	TClassFactory(TSortFunc* pSortFunc=NULL,u16 GrowBy=TClassFactory_GrowByDefault) : TPtrArray<TYPE>	( pSortFunc, GrowBy )	{	}

	template<class MATCHTYPE>
	TPtr<TYPE>&				GetInstance(const MATCHTYPE& Match)				{	return FindPtr(Match);	}
	TPtr<TYPE>&				GetInstance(TRefRef InstanceRef,Bool CreateNew=FALSE,TRefRef TypeRef=TRef());	//	get an instance from the list. If it doesnt exist option to create a new one. if STOREINSTANCES is false, this will never return a valid ptr.. use CreateInstance isntead

	template<class OBJECTTYPE>
	OBJECTTYPE*				GetInstanceObject(TRefRef InstanceRef);			//	get the object of an instance and cast as desired
	s32						CreateInstance(TPtr<TYPE>& pNewInstance,TRefRef InstanceRef,TRefRef TypeRef=TRef());						//	create a new instance and add to the list
	template<class MATCHTYPE>
	Bool					RemoveInstance(const MATCHTYPE& Match);			//	remove an instance from the list

	TPtrArray<TYPE>&		GetInstanceArray()								{	return *this;	}
	const TPtrArray<TYPE>&	GetInstanceArray() const						{	return *this;	}

	TRef					GetFreeInstanceRef(TRef BaseRef=TRef()) const;	//	get an unused ref for an instance

protected:
	virtual TYPE*			CreateObject(TRefRef InstanceRef,TRefRef TypeRef) = 0;

	void					Debug_CheckFactoryIntegrity();					//	make sure there are no dupes, and no NULL entries
};



//--------------------------------------------------------------
//	an object factory is the same as a class factory but it 
//	updates all it's elements and asynchornously shuts them all down
//	when the factory is shutdown
//--------------------------------------------------------------
template<class TYPE>
class TObjectFactory : public TClassFactory<TYPE>
{
public:
	typedef TLArray::SortResult(TSortFunc)(const TPtr<TYPE>&,const TPtr<TYPE>&,const void*);
	
public:
	TObjectFactory(TSortFunc* pSortFunc=NULL) : TClassFactory<TYPE>	( pSortFunc )	{	}
	~TObjectFactory()		{	ShutdownObjects();	}

	virtual SyncBool		Shutdown()			{	return ShutdownObjects();	}

	virtual void			UpdateObjects();
	virtual SyncBool		ShutdownObjects();
};











//--------------------------------------------------------------
//	get an instance from the list. If it doesnt exist option to create a new one
//--------------------------------------------------------------
template<class TYPE, bool STOREINSTANCES>
TPtr<TYPE>& TClassFactory<TYPE, STOREINSTANCES>::GetInstance(TRefRef InstanceRef,Bool CreateNew,TRefRef TypeRef)
{
	if ( STOREINSTANCES )
	{
		//	find the existing ptr in the array and return it
		TPtr<TYPE>& pInstance = TPtrArray<TYPE>::FindPtr( InstanceRef );

		if ( pInstance )
			return pInstance;
	}

	//	go through the create instance
	if ( CreateNew )
	{
		TPtr<TYPE> pNewInstance;
		s32 NewInstanceIndex = CreateInstance( pNewInstance, InstanceRef, TypeRef );

		if ( NewInstanceIndex == -1 || !STOREINSTANCES )
			return TLPtr::GetNullPtr<TYPE>();

		Debug_CheckFactoryIntegrity();

		return TPtrArray<TYPE>::ElementAt( NewInstanceIndex );
	}

	return TLPtr::GetNullPtr<TYPE>();
}


//--------------------------------------------------------------
//	create a new instance and add to the list
//--------------------------------------------------------------
template<class TYPE, bool STOREINSTANCES>
s32 TClassFactory<TYPE, STOREINSTANCES>::CreateInstance(TPtr<TYPE>& pNewInstance,TRefRef InstanceRef,TRefRef TypeRef)
{
	//	get the ptr's index
	s32 InstancePtrIndex = TPtrArray<TYPE>::FindIndex( InstanceRef );

	//	doesnt exist, create a new one and add to the array
	if ( InstancePtrIndex == -1 )
	{
		//	use the overloaded function to create the object
		TYPE* pObject = CreateObject( InstanceRef, TypeRef );
		if ( !pObject )
			return -1;

		if(STOREINSTANCES)
		{
			//	add to the array
			InstancePtrIndex = TPtrArray<TYPE>::Add( pObject );

			Debug_CheckFactoryIntegrity();

			//	clean up if failed
			if ( InstancePtrIndex == -1 )
			{
				TLMemory::Delete( pObject );
			}
		}
		else
		{
			pNewInstance = pObject;
			return -1;
		}
	}

	if ( InstancePtrIndex != -1 )
	{
		//	success, fetch
		pNewInstance = TPtrArray<TYPE>::ElementAt( InstancePtrIndex );
	}
	
	return InstancePtrIndex;
}


//--------------------------------------------------------------
//	remove an instance from the list
//--------------------------------------------------------------
template<class TYPE, bool STOREINSTANCES>
template<typename MATCHTYPE>
Bool TClassFactory<TYPE, STOREINSTANCES>::RemoveInstance(const MATCHTYPE& Match)
{
	s32 InstanceIndex = TPtrArray<TYPE>::FindIndex( Match );

	//	doesnt exist
	if ( InstanceIndex == -1 )
		return FALSE;

	//	NULL now to make it easier to debug destruction
	TPtrArray<TYPE>::ElementAt( InstanceIndex ) = NULL;

	//	remove entry
	if ( !TPtrArray<TYPE>::RemoveAt( InstanceIndex ) )
		return FALSE;

	Debug_CheckFactoryIntegrity();

	return TRUE;
}


//--------------------------------------------------------------
//	get the object of an instance and cast as desired
//--------------------------------------------------------------
template<class TYPE, bool STOREINSTANCES>
template<class OBJECTTYPE>
OBJECTTYPE* TClassFactory<TYPE, STOREINSTANCES>::GetInstanceObject(TRefRef InstanceRef)
{
	//	get the instance 
	TPtr<TYPE>& pInstance = GetInstance( InstanceRef, FALSE );
	if ( !pInstance )
		return NULL;

#if defined(__GNUG__)
	TYPE* pInstanceObject = pInstance.GetObjectPointer();
	return static_cast<OBJECTTYPE>(pInstanceObject);
#else
	//	gr: not sure why this doesn't compile on gcc...
	return pInstance.GetObjectPointer < OBJECTTYPE > ( );
#endif
}


//--------------------------------------------------------------
//	get an unused ref for an instance
//--------------------------------------------------------------
template<class TYPE, bool STOREINSTANCES>
TRef TClassFactory<TYPE, STOREINSTANCES>::GetFreeInstanceRef(TRef BaseRef) const
{
	//	if we don't store instances we can't do much here
	if ( !STOREINSTANCES )
	{
		TLDebug_Break("Cannot search for a free ref on a factory that doesn't store the instances");
		return TRef();	//	return BaseRef;
	}

	//	keep searching until we find one unused
	while ( GetInstanceArray().Exists( BaseRef ) )
	{
		BaseRef.Increment();
	}

	return BaseRef;
}


//--------------------------------------------------------------
//	make sure there are no dupes, and no NULL entries
//--------------------------------------------------------------
template<class TYPE, bool STOREINSTANCES>
void TClassFactory<TYPE, STOREINSTANCES>::Debug_CheckFactoryIntegrity()
{
#ifdef _DEBUG
	for ( s32 i=TPtrArray<TYPE>::GetLastIndex();	i>=0;	i-- )
	{
		TPtr<TYPE>& pInstance = TPtrArray<TYPE>::ElementAt(i);

		//	if null...
		if ( !pInstance.IsValid() )
		{
			TLDebug_Break("Null entry in class factory");
			TPtrArray<TYPE>::RemoveAt(i);
			continue;
		}

		//	check for duplicates
		for ( s32 d=i-1;	d>=0;	d-- )
		{
			if ( TPtrArray<TYPE>::ElementAt(d) == pInstance )
			{
				TLDebug_Break("Duplicate entry in class factory");
				TPtrArray<TYPE>::RemoveAt(i);
				break;
			}
		}
	}
#endif
}

//--------------------------------------------------------------
//	update all the elements in the factory
//--------------------------------------------------------------
template<class TYPE>
void TObjectFactory<TYPE>::UpdateObjects()
{
	//	update each object
	for ( u32 i=0;	i<TClassFactory<TYPE>::GetSize();	i++ )
	{
		//	get object
		TPtr<TYPE>& pObject = TClassFactory<TYPE>::ElementAt(i);
		if ( !pObject )
			continue;

		//	update
		pObject->Update();
	}
}


//--------------------------------------------------------------
//	shutdown all the elements in the factory
//--------------------------------------------------------------
template<class TYPE>
SyncBool TObjectFactory<TYPE>::ShutdownObjects()
{
	SyncBool ShutdownResult = SyncTrue;

	//	update each object
	for ( u32 i=0;	i<TClassFactory<TYPE>::GetSize();	i++ )
	{
		//	get object
		TPtr<TYPE>& pObject = TClassFactory<TYPE>::ElementAt(i);
		if ( !pObject )
			continue;

		//	shutdown object
		SyncBool Result = pObject->Shutdown();

		//	if object failed to shutdown return immediately
		if ( Result == SyncFalse )
			return Result;

		//	has shutdown, NULL(delete) from list
		if ( Result != SyncWait )
			pObject = NULL;

		//	if it hasn't shutdown we treat the whole shutdown process with the same result
		if ( Result != SyncTrue )
			ShutdownResult = Result;
	}

	//	if complete, make sure we empty the list
	if ( ShutdownResult == SyncTrue )
	{
		TClassFactory<TYPE>::Empty( TRUE );
	}

	return ShutdownResult;
}


