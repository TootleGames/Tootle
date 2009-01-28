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
	TPtr<TYPE>&				GetInstance(TRefRef InstanceRef,Bool CreateNew=FALSE,TRefRef TypeRef=TRef());	//	get an instance from the list. If it doesnt exist option to create a new one

	template<class OBJECTTYPE>
	OBJECTTYPE*				GetInstanceObject(TRefRef InstanceRef);			//	get the object of an instance and cast as desired
	s32						CreateInstance(TPtr<TYPE>& pNewInstance,TRefRef InstanceRef,TRefRef TypeRef=TRef());						//	create a new instance and add to the list
	Bool					RemoveInstance(TRefRef InstanceRef);			//	remove an instance from the list

	TPtrArray<TYPE>&		GetInstanceArray()								{	return *this;	}
	const TPtrArray<TYPE>&	GetInstanceArray() const						{	return *this;	}

protected:
	virtual TYPE*			CreateObject(TRefRef InstanceRef,TRefRef TypeRef) = 0;
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
Bool TClassFactory<TYPE, STOREINSTANCES>::RemoveInstance(TRefRef InstanceRef)
{
	s32 InstanceIndex = TPtrArray<TYPE>::FindIndex( InstanceRef );

	//	doesnt exist
	if ( InstanceIndex == -1 )
		return FALSE;

	//	NULL now to make it easier to debug destruction
	TPtrArray<TYPE>::ElementAt( InstanceIndex ) = NULL;

	//	remove entry
	if ( !TPtrArray<TYPE>::RemoveAt( InstanceIndex ) )
		return FALSE;

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
	TYPE* pInstanceObject = pInstance.GetObject();
	return static_cast<OBJECTTYPE>(pInstanceObject);
#else
	//	gr: not sure why this doesn't compile on gcc...
	return pInstance.GetObject < OBJECTTYPE > ( );
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

