/*------------------------------------------------------

	Base asset type + general asset interface

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TRef.h>
#include <TootleCore/TClassFactory.h>
#include <TootleCore/TBinaryTree.h>
#include <TootleCore/TManager.h>

#include "TAsset.h"

namespace TLFileSys
{
	class TFile;
	class TFileAsset;
}

namespace TLAsset
{
	class TAssetManager;	//	asset manager
	class TAssetFactory;	//	asset factory
	class TLoadTask;		//	loading-asset task

	//	this replaces the old LoadAsset and GetAsset functions.
	//	get an asset from the sytem. If it's not loaded, it will try to.
	//	if Blocking is SyncTrue and the file is still loading then NULL will be returned.
	//	use SyncTrue to load, SyncFalse to NOT load, SyncWait to async-load
	TPtr<TAsset>&				GetAssetPtr(const TTypedRef& AssetAndTypeRef,SyncBool LoadAsset=SyncTrue);	
	template<class ASSETTYPE>
	FORCEINLINE ASSETTYPE*		GetAsset(TRefRef AssetRef,SyncBool LoadAsset=SyncTrue)		{	return GetAssetPtr( TTypedRef( AssetRef, ASSETTYPE::GetAssetType_Static() ), LoadAsset ).GetObjectPointer<ASSETTYPE>();	}
	FORCEINLINE TLAsset::TAsset* GetAsset(const TTypedRef& AssetAndTypeRef,SyncBool LoadAsset=SyncTrue)		{	return GetAssetPtr( AssetAndTypeRef, LoadAsset ).GetObjectPointer();	}
	FORCEINLINE TLAsset::TAsset* GetAsset(TRefRef AssetRef,TRefRef AssetType,SyncBool LoadAsset=SyncTrue)	{	return GetAsset( TTypedRef( AssetRef, AssetType ), LoadAsset );	}
	template<class ASSETTYPE>
	FORCEINLINE TPtr<ASSETTYPE>	GetAssetPtr(TRefRef AssetRef,SyncBool LoadAsset=SyncTrue);
	TPtr<TLAsset::TAsset>&		GetAssetInstance(const TTypedRef& AssetAndTypeRef);				//	not really for general public usage

	template<class ASSETTYPE>
	u32							GetAllAssets(TPtrArray<ASSETTYPE>& AssetArray);					//	get an array containing all the assets of this type

	//	wrapper to just simply load an asset - returns TRUE if currently loaded, SyncWait if loading, SyncFalse if failed
	SyncBool					LoadAsset(const TTypedRef& AssetAndTypeRef,Bool BlockLoad);
	FORCEINLINE SyncBool		LoadAsset(TRefRef AssetRef,TRefRef AssetType,Bool BlockLoad)	{	return LoadAsset( TTypedRef( AssetRef, AssetType ), BlockLoad );	}
	template<class ASSETTYPE>
	FORCEINLINE SyncBool		LoadAsset(TRefRef AssetRef,Bool BlockLoad)						{	return LoadAsset( TTypedRef( AssetRef, ASSETTYPE::GetType_Static() ), BlockLoad );	}
	Bool						LoadAllAssets(TRefRef AssetType);					//	load all assets from the file system of this type that we can identify. (ie. won't load non-compiled assets). Returns TRUE if we found any new assets of this type

	FORCEINLINE TPtr<TAsset>&	CreateAsset(const TTypedRef& AssetAndTypeRef);		//	return a pointer to a new asset - mostly used for runtime asssets
	FORCEINLINE TPtr<TAsset>&	CreateAsset(TRefRef AssetRef,TRefRef AssetType)		{	return CreateAsset( TTypedRef( AssetRef, AssetType ) );	}

	TTypedRef					GetFreeAssetRef(TTypedRef BaseAndTypeRef);			//	get an asset ref that isn't in use (starting from base ref)
	FORCEINLINE TTypedRef		GetFreeAssetRef(TRefRef AssetRef,TRefRef AssetType)	{	return GetFreeAssetRef( TTypedRef( AssetRef, AssetType ) );	}

	Bool						SaveAsset(const TTypedRef& AssetAndTypeRef);		//	write an asset's current state back to the file system - fails if the asset isn't currently loaded
	FORCEINLINE Bool			SaveAsset(TRefRef AssetRef,TRefRef AssetType)		{	return SaveAsset( TTypedRef( AssetRef, AssetType ) );	}
	Bool						SaveAsset(const TLAsset::TAsset* pAsset);

	FORCEINLINE Bool			DeleteAsset(const TTypedRef& AssetAndTypeRef);		//	delete an asset from the asset Manager
	FORCEINLINE Bool			DeleteAsset(TRefRef AssetRef,TRefRef AssetType)		{	return DeleteAsset( TTypedRef( AssetRef, AssetType ) );	}
	Bool						DeleteAsset(const TLAsset::TAsset* pAsset);


	TLArray::SortResult			AssetSort(const TPtr<TAsset>& a,const TPtr<TAsset>& b,const void* pTestRef);	//	asset sort

	extern TPtr<TLAsset::TAssetManager>	g_pManager;
};



//------------------------------------------------------------
//	an asset factory, overload this to add your own asset types
//------------------------------------------------------------
class TLAsset::TAssetFactory : public TClassFactory<TLAsset::TAsset,FALSE>
{
public:

protected:
	virtual TLAsset::TAsset*	CreateObject(TRefRef InstanceRef,TRefRef TypeRef);	
};


//------------------------------------------------------------
//	class Manager for assets
//------------------------------------------------------------
class TLAsset::TAssetManager : public TLCore::TManager
{
	friend class TLAsset::TLoadTask;
public:
	TAssetManager(TRefRef ManagerRef);

	void						AddAssetFactory(TPtr<TAssetFactory>& pFactory)		{	m_Factories.Add( pFactory );	}

	TPtr<TAsset>&				CreateAsset(const TTypedRef& AssetAndTypeRef);		//	return a pointer to a new asset - mostly used for runtime asssets
	TPtr<TAsset>&				GetAsset(const TTypedRef& AssetAndTypeRef)			{	return m_Assets.FindPtr( AssetAndTypeRef );	}
	TPtrArray<TAsset>&			GetAllAssets()										{	return m_Assets;	}
	Bool						DeleteAsset(const TTypedRef& AssetAndTypeRef);

protected:
	virtual SyncBool			Initialise();
	virtual SyncBool			Update(float fTimeStep);
	virtual SyncBool			Shutdown();
	
	virtual void				OnEventChannelAdded(TRefRef refPublisherID,TRefRef refChannelID);

	// Asset events - should be private but called from the LoadTask
	void						OnAssetLoad(const TTypedRef& AssetAndTypeRef, Bool bStatus);
	void						OnAssetUnload(const TTypedRef& AssetAndTypeRef);

private:
	TPtrArray<TAssetFactory>	m_Factories;	//	asset factories, including default
	TPtrArray<TAsset>			m_Assets;		//	global list of assets, they're not stored in the factory, stored in a single array instead
};


//------------------------------------------------------------
//	wrapper to just simply load an asset - returns TRUE if currently loaded, SyncWait if loading, SyncFalse if failed
//------------------------------------------------------------
template<class ASSETTYPE>
FORCEINLINE TPtr<ASSETTYPE> TLAsset::GetAssetPtr(TRefRef AssetRef,SyncBool LoadAsset)
{
	//	make up asset+type ref
	TRef AssetTypeRef = ASSETTYPE::GetAssetType_Static();
	TTypedRef AssetAndTypeRef( AssetRef, AssetTypeRef );

	//	fetch base asset ptr
	TPtr<TLAsset::TAsset>& pAsset = GetAssetPtr( AssetAndTypeRef, LoadAsset );

	//	debug check for invalid casting
	if ( pAsset && pAsset->GetAssetType() != AssetTypeRef )
	{
		#ifdef _DEBUG
		{
			TTempString Debug_String("fetched asset Ptr for ");
			AssetAndTypeRef.GetString( Debug_String );
			Debug_String.Append(" but asset's type is ");
			pAsset->GetAssetType().GetString( Debug_String );
			TLDebug_Break( Debug_String );
		}
		#endif
		return TPtr<ASSETTYPE>(NULL);
	}

	return pAsset;	
}

	



//----------------------------------------------------------
//	return a pointer to an asset
//----------------------------------------------------------
FORCEINLINE TPtr<TLAsset::TAsset>& TLAsset::CreateAsset(const TTypedRef& AssetAndTypeRef)
{
	if ( !g_pManager )
	{
		TLDebug_Break("Asset manager expected");
		return TLPtr::GetNullPtr<TLAsset::TAsset>();
	}

	return g_pManager->CreateAsset( AssetAndTypeRef );
}


//----------------------------------------------------------
//	delete an asset from the manager
//----------------------------------------------------------
FORCEINLINE Bool TLAsset::DeleteAsset(const TTypedRef& AssetAndTypeRef)
{
	if ( !g_pManager )
		return FALSE;
	
	return g_pManager->DeleteAsset( AssetAndTypeRef );
}


//----------------------------------------------------------
//	get an array containing all the assets of this type
//----------------------------------------------------------
template<class ASSETTYPE>
u32 TLAsset::GetAllAssets(TPtrArray<ASSETTYPE>& AssetArray)
{
	if ( !g_pManager )
		return 0;

	//	pre-fetch the asset type 
	TRef AssetType = ASSETTYPE::GetAssetType_Static();
	
	//	look through all the assets to find matching types
	TPtrArray<TLAsset::TAsset>& AllAssets = g_pManager->GetAllAssets();
	for ( u32 i=0;	i<AllAssets.GetSize();	i++ )
	{
		//	check type
		TPtr<TLAsset::TAsset>& pAsset = AllAssets[i];
		if ( pAsset->GetAssetType() != AssetType )
			continue;

		//	cast and add to array
		AssetArray.Add( pAsset );
	}

	return AssetArray.GetSize();
}

