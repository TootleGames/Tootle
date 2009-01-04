/*------------------------------------------------------

	Base asset type + general asset interface

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TRef.h>
#include <TootleCore/TClassFactory.h>
#include <TootleCore/TBinaryTree.h>
#include <TootleCore/TManager.h>


namespace TLFileSys
{
	class TFile;
	class TFileAsset;
}

namespace TLAsset
{
	class TAsset;			//	base asset type
	
	class TAssetFactory;	//	asset factory
	class TLoadTask;		//	loading-asset task

	TPtr<TAsset>&		GetAsset(TRefRef AssetRef,Bool LoadedOnly=FALSE);	//	return a pointer to an asset - if LoadedOnly, returns a NULL pointer for assets that aren't loaded
	void				GetAssetArray(TPtrArray<TAsset>& AssetArray,TRefRef AssetType,Bool LoadedOnly=FALSE);	//	get an array of assets of a certain type
	TPtr<TAsset>&		LoadAsset(TRefRef AssetRef,  Bool bBlocking = FALSE);						//	load asset from a file systems
	TPtr<TAsset>&		CreateAsset(TRefRef AssetRef,TRefRef AssetType);	//	return a pointer to a new asset - mostly used for runtime asssets
	void				DeleteAsset(TRefRef AssetRef);						//	delete an asset

	TPtr<TLoadTask>		GetLoadTask(TRefRef AssetRef);						//	get the load task for this asset

	extern TPtr<TLAsset::TAssetFactory>	g_pFactory;
	
	TLArray::SortResult	AssetSort(const TPtr<TAsset>& a,const TPtr<TAsset>& b,const void* pTestRef);	//	asset sort
};



//------------------------------------------------------------
//	class factory for assets
//------------------------------------------------------------
class TLAsset::TAssetFactory : public TManager, public TClassFactory<TLAsset::TAsset>
{
public:
	TAssetFactory(TRefRef ManagerRef) :
		TManager						( ManagerRef ),
		TClassFactory<TLAsset::TAsset>	( &TLAsset::AssetSort )
	{
	}

protected:
	virtual TLAsset::TAsset*	CreateObject(TRefRef InstanceRef,TRefRef TypeRef);

	virtual SyncBool Update(float fTimeStep);
	virtual SyncBool Shutdown()			{	return TManager::Shutdown();	}
	
	virtual void			OnEventChannelAdded(TRefRef refPublisherID,TRefRef refChannelID);
	
};


