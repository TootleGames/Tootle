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
	class TAssetFactory;	//	asset factory
	class TLoadTask;		//	loading-asset task

	//	this replaces the old LoadAsset and GetAsset functions.
	//	get an asset from the sytem. If it's not loaded, it will try to.
	//	if Blocking is SyncTrue and the file is still loading then NULL will be returned.
	//	use SyncTrue to load, SyncFalse to NOT load, SyncWait to async-load
	TPtr<TAsset>&				GetAssetPtr(const TTypedRef& AssetAndTypeRef,SyncBool LoadAsset=SyncTrue);	
	template<class ASSETTYPE>
	FORCEINLINE ASSETTYPE*		GetAsset(TRefRef AssetRef,SyncBool LoadAsset=SyncTrue)		{	return GetAssetPtr( TTypedRef( AssetRef, ASSETTYPE::GetAssetType_Static() ), LoadAsset ).GetObject<ASSETTYPE>();	}
	FORCEINLINE TLAsset::TAsset* GetAsset(const TTypedRef& AssetAndTypeRef,SyncBool LoadAsset=SyncTrue)		{	return GetAssetPtr( AssetAndTypeRef, LoadAsset ).GetObject();	}
	FORCEINLINE TLAsset::TAsset* GetAsset(TRefRef AssetRef,TRefRef AssetType,SyncBool LoadAsset=SyncTrue)	{	return GetAsset( TTypedRef( AssetRef, AssetType ), LoadAsset );	}
	template<class ASSETTYPE>
	TPtr<ASSETTYPE>				GetAssetPtr(TRefRef AssetRef,SyncBool LoadAsset=SyncTrue);
	TPtr<TLAsset::TAsset>&		GetAssetInstance(const TTypedRef& AssetAndTypeRef);				//	not really for general public usage

	//	wrapper to just simply load an asset - returns TRUE if currently loaded, SyncWait if loading, SyncFalse if failed
	SyncBool					LoadAsset(const TTypedRef& AssetAndTypeRef,Bool BlockLoad);
	FORCEINLINE SyncBool		LoadAsset(TRefRef AssetRef,TRefRef AssetType,Bool BlockLoad)	{	return LoadAsset( TTypedRef( AssetRef, AssetType ), BlockLoad );	}
	template<class ASSETTYPE>
	FORCEINLINE SyncBool		LoadAsset(TRefRef AssetRef,Bool BlockLoad)						{	return LoadAsset( TTypedRef( AssetRef, ASSETTYPE::GetType_Static() ), BlockLoad );	}

	TPtr<TAsset>&				CreateAsset(const TTypedRef& AssetAndTypeRef);		//	return a pointer to a new asset - mostly used for runtime asssets
	FORCEINLINE TPtr<TAsset>&	CreateAsset(TRefRef AssetRef,TRefRef AssetType)		{	return CreateAsset( TTypedRef( AssetRef, AssetType ) );	}

	TTypedRef					GetFreeAssetRef(TTypedRef BaseAndTypeRef);			//	get an asset ref that isn't in use (starting from base ref)
	FORCEINLINE TTypedRef		GetFreeAssetRef(TRefRef AssetRef,TRefRef AssetType)	{	return GetFreeAssetRef( TTypedRef( AssetRef, AssetType ) );	}

	Bool						SaveAsset(const TTypedRef& AssetAndTypeRef);		//	write an asset's current state back to the file system - fails if the asset isn't currently loaded
	FORCEINLINE Bool			SaveAsset(TRefRef AssetRef,TRefRef AssetType)		{	return SaveAsset( TTypedRef( AssetRef, AssetType ) );	}
	Bool						SaveAsset(const TLAsset::TAsset* pAsset);

	void						DeleteAsset(const TTypedRef& AssetAndTypeRef);		//	delete an asset from the asset factory
	FORCEINLINE void			DeleteAsset(TRefRef AssetRef,TRefRef AssetType)		{	DeleteAsset( TTypedRef( AssetRef, AssetType ) );	}
	void						DeleteAsset(const TLAsset::TAsset* pAsset);


	TLArray::SortResult			AssetSort(const TPtr<TAsset>& a,const TPtr<TAsset>& b,const void* pTestRef);	//	asset sort

	extern TPtr<TLAsset::TAssetFactory>	g_pFactory;
};



//------------------------------------------------------------
//	class factory for assets
//------------------------------------------------------------
class TLAsset::TAssetFactory : public TLCore::TManager, public TClassFactory<TLAsset::TAsset>
{
public:
	TAssetFactory(TRefRef ManagerRef) :
		TLCore::TManager				( ManagerRef ),
		TClassFactory<TLAsset::TAsset>	( &TLAsset::AssetSort )
	{
		//	gr: make grow by quite big for assets - cops and robbers has thousands of assets, takes a while to init with
		//		a small growby
		SetGrowBy( 200 );
	}

	// Asset events - should be private but called from the LoadTask
	void	OnAssetLoad(const TTypedRef& AssetAndTypeRef, Bool bStatus);
	void	OnAssetUnload(const TTypedRef& AssetAndTypeRef);

protected:
	virtual TLAsset::TAsset*	CreateObject(TRefRef InstanceRef,TRefRef TypeRef);

	virtual SyncBool			Initialise();
	virtual SyncBool			Update(float fTimeStep);
	virtual SyncBool			Shutdown();
	
	virtual void				OnEventChannelAdded(TRefRef refPublisherID,TRefRef refChannelID);
	
};


//------------------------------------------------------------
//	wrapper to just simply load an asset - returns TRUE if currently loaded, SyncWait if loading, SyncFalse if failed
//------------------------------------------------------------
template<class ASSETTYPE>
TPtr<ASSETTYPE> TLAsset::GetAssetPtr(TRefRef AssetRef,SyncBool LoadAsset)
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

	