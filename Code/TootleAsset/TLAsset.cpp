#include "TLAsset.h"
#include "TMesh.h"
#include "TFont.h"
#include "TMenu.h"
#include "TAudio.h"
#include "TScheme.h"
#include "TPath.h"
#include "TText.h"
#include "TAssetScript.h"
#include "TLoadTask.h"
#include <TootleCore/TPtr.h>
#include <TootleCore/TEventChannel.h>
#include <TootleFileSys/TFileAsset.h>

namespace TLAsset
{
	TPtr<TLAsset::TAssetFactory>	g_pFactory;
	TPtrArray<TLoadTask>			g_LoadTasks;
};



//----------------------------------------------------------
//	asset sort
//----------------------------------------------------------
TLArray::SortResult	TLAsset::AssetSort(const TPtr<TLAsset::TAsset>& a,const TPtr<TLAsset::TAsset>& b,const void* pTestRef)
{
	const TRef& aRef = a->GetAssetRef();
	const TRef& bRef = pTestRef ? *(const TRef*)pTestRef : b->GetAssetRef();
	
	//	== turns into 0 (is greater) or 1(equals)
	return aRef < bRef ? TLArray::IsLess : (TLArray::SortResult)(aRef==bRef);	
}


//----------------------------------------------------------
//	get an asset ref that isn't in use (starting from base ref)
//----------------------------------------------------------
TRef TLAsset::GetFreeAssetRef(TRef BaseRef)
{
	//	start at a valid ref :)
	if ( !BaseRef.IsValid() )
		BaseRef.Increment();

	while ( g_pFactory->GetInstance( BaseRef ).IsValid() )
	{
		BaseRef.Increment();
	}

	return BaseRef;
}


//----------------------------------------------------------
//	return a pointer to an asset
//----------------------------------------------------------
TPtr<TLAsset::TAsset>& TLAsset::CreateAsset(TRefRef AssetRef,TRefRef AssetType)
{
	if ( !g_pFactory )
	{
		TLDebug_Break("Asset factory expected");
		return TLPtr::GetNullPtr<TLAsset::TAsset>();
	}

	TPtr<TAsset>& pNewAsset = g_pFactory->GetInstance( AssetRef, TRUE, AssetType );
	if ( !pNewAsset )
	{
		TTempString DebugString("Failed to create asset... ");
		AssetRef.GetString( DebugString );
		DebugString.Append(" (");
		AssetType.GetString( DebugString );
		DebugString.Append(")");
		TLDebug_Print( DebugString );
		return TLPtr::GetNullPtr<TLAsset::TAsset>();
	}
	else
	{
		TTempString DebugString("Created asset: ");
		AssetRef.GetString( DebugString );
		DebugString.Append(" (");
		AssetType.GetString( DebugString );
		DebugString.Append(")");
		TLDebug_Print( DebugString );
	}

	//	ensure it's the right type
	if ( pNewAsset->GetAssetType() != AssetType )
	{
#ifdef _DEBUG
		TTempString DebugString("Created/found asset ");
		pNewAsset->GetAssetRef().GetString( DebugString );
		DebugString.Append(" but is type ");
		pNewAsset->GetAssetType().GetString( DebugString );
		DebugString.Append(". Expected type ");
		AssetType.GetString( DebugString );
		TLDebug_Print( DebugString );
#endif
		TLDebug_Break("Wrong type of asset. If this is a new asset type, check the TAsset constructor in your asset types constructor.");
		return TLPtr::GetNullPtr<TLAsset::TAsset>();
	}

	return pNewAsset;
}


//----------------------------------------------------------
//	return a pointer to an asset
//----------------------------------------------------------
TPtr<TLAsset::TAsset>& TLAsset::GetAsset(TRefRef AssetRef,Bool LoadedOnly)
{
	if ( !g_pFactory )
	{
		TLDebug_Break("Asset factory expected");
		return TLPtr::GetNullPtr<TLAsset::TAsset>();
	}
	
	TPtr<TLAsset::TAsset>& pAsset = g_pFactory->GetInstance( AssetRef );
	
	if ( LoadedOnly )
	{
		if ( pAsset && !pAsset->IsLoaded() )
			return TLPtr::GetNullPtr<TLAsset::TAsset>();
	}

	if ( !pAsset && AssetRef.IsValid() )
	{
		TTempString DebugString("failed to find asset ");
		AssetRef.GetString( DebugString );
		TLDebug_Print( DebugString );
	}

	
	return pAsset;
}

	
//----------------------------------------------------------
//	get an array of assets of a certain type
//----------------------------------------------------------
void TLAsset::GetAssetArray(TPtrArray<TAsset>& AssetArray,TRefRef AssetType,Bool LoadedOnly)
{
	for ( u32 i=0;	i<g_pFactory->GetSize();	i++ )
	{
		TPtr<TAsset>& pAsset = g_pFactory->ElementAt(i);
		if ( pAsset->GetAssetType() != AssetType )
			continue;

		//	loaded only?
		if ( LoadedOnly && !pAsset->IsLoaded() )
			continue;

		AssetArray.Add( pAsset );
	}
}


//----------------------------------------------------------
//	delete an asset
//----------------------------------------------------------
void TLAsset::DeleteAsset(TRefRef AssetRef)
{
	//	find the existing asset
	//XXXX
	TPtr<TAsset> pAsset = GetAsset(AssetRef);

	//	unknown asset, nothing to do
	if ( !pAsset )
		return;

	//	mark asset as unavailible
	pAsset->SetLoadingState( TLAsset::LoadingState_Deleted );

	TTempString DebugString("Deleting asset from factory... ");
	AssetRef.GetString( DebugString );
	DebugString.Append(" (");
	pAsset->GetAssetType().GetString( DebugString );
	DebugString.Append(")");
	TLDebug_Print( DebugString );

	//	delete from factory
	if ( !g_pFactory->RemoveInstance( AssetRef ) )
	{
		TTempString DebugString("Deleting asset from factory... ");
		AssetRef.GetString( DebugString );
		DebugString.Append(" not found");
		TLDebug_Print( DebugString );
	}
}

//----------------------------------------------------------
//	load asset from a file system
//----------------------------------------------------------
TPtr<TLAsset::TAsset>& TLAsset::LoadAsset(const TRef& AssetRef, Bool bBlocking)
{
	if ( !g_pFactory )
	{
		TLDebug_Break("Asset factory expected");
		return TLPtr::GetNullPtr<TLAsset::TAsset>();
	}

	//	check it doesnt already exist
	{
		TPtr<TAsset>& pAsset = GetAsset( AssetRef );
		if ( pAsset )
		{
			if ( pAsset->IsLoaded() )
				return pAsset;
		}
	}

	//	look for an existing loading task
	TPtr<TLAsset::TLoadTask> pLoadTask = TLAsset::GetLoadTask( AssetRef );

	//	create a new load task if we dont have one already
	if ( !pLoadTask )
	{
		pLoadTask = new TLoadTask( AssetRef );
	}

	//	missing task (failed to alloc?)
	if ( !pLoadTask )
		return TLPtr::GetNullPtr<TLAsset::TAsset>();

	//	do first update, if it fails then we can abort early and fail immedietly
	SyncBool FirstUpdateResult = pLoadTask->Update( 0.f, bBlocking );

	//	get loading/loaded asset
	TPtr<TAsset>& pLoadingAsset = pLoadTask->GetAsset();

	//	verify the correct result
	if ( FirstUpdateResult != SyncFalse )
	{
		//	asset expected
		if ( !pLoadingAsset )
		{
			if ( !TLDebug_Break("Asset expected") )
				FirstUpdateResult = SyncFalse;
		}
	}

	//	failed
	if ( FirstUpdateResult == SyncFalse )
	{
		pLoadTask = NULL;
		return TLPtr::GetNullPtr<TLAsset::TAsset>();
	}

	//	finished already! dont need to add the task to the queue
	if ( FirstUpdateResult == SyncTrue )
	{
		pLoadTask = NULL;
		return pLoadingAsset;
	}

	//	if we got here with a block load, then it failed to block load...
	if ( bBlocking )
	{
		TLDebug_Break("Block load failed... asynchornously loading...");
	}

	//	load in progress, add task to list for asynchronous load
	g_LoadTasks.AddUnique( pLoadTask );
		
	return pLoadingAsset;
}
	

//----------------------------------------------------------
//	instance an asset
//----------------------------------------------------------
TLAsset::TAsset* TLAsset::TAssetFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	if ( TypeRef == "Audio" )
		return new TLAsset::TAudio( InstanceRef );

	if ( TypeRef == "Mesh" )
		return new TLAsset::TMesh( InstanceRef );

	if ( TypeRef == "Font" )
		return new TLAsset::TFont( InstanceRef );
	
	if ( TypeRef == "Menu" )
		return new TLAsset::TMenu( InstanceRef );
	
	if ( TypeRef == "Scheme" )
		return new TLAsset::TScheme( InstanceRef );

	if ( TypeRef == "PathNetwork" )
		return new TLAsset::TPathNetwork( InstanceRef );
	
	if ( TypeRef == "Text" )
		return new TLAsset::TText( InstanceRef );
	
	if ( TypeRef == "AScript" )
		return new TLAsset::TAssetScript( InstanceRef );
	

	//	gr: dumb asset - just stores data - consider turning this into a specific TBinaryTree/"Data" asset
	if ( TypeRef == "Asset" )
		return new TLAsset::TAsset( InstanceRef, TypeRef );

	//	"nothing" asset - used as a placeholder whilst we convert file into a real asset
	if ( TypeRef == "Temp" )
		return new TLAsset::TTempAsset( InstanceRef );
	

	return NULL;
}


void TLAsset::TAssetFactory::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{
	if(refPublisherID == "CORE")
	{
		// Subscribe to the update messages
		if(refChannelID == TLCore::UpdateRef)
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
	}
	
	// Super event channel routine
	TManager::OnEventChannelAdded(refPublisherID, refChannelID);
}



SyncBool TLAsset::TAssetFactory::Update(float fTimeStep)
{
	//	update manager
	if ( TManager::Update( fTimeStep ) == SyncFalse )
		return SyncFalse;

	//	update load tasks, FIFO
	for ( u32 t=0;	t<g_LoadTasks.GetSize();	t++ )
	{
		TPtr<TLoadTask>& pTask = g_LoadTasks[t];
		if ( !pTask )
			continue;

		//	update task
		SyncBool UpdateResult = pTask->Update( 0.f, FALSE );

		//	all complete!
		if ( UpdateResult == SyncTrue )
			pTask = NULL;
	}

	//	remove null(completed) tasks
	g_LoadTasks.RemoveNull();

	return SyncTrue;
}



//------------------------------------------------------------
//	get the load task for this asset
//------------------------------------------------------------
TPtr<TLAsset::TLoadTask> TLAsset::GetLoadTask(TRefRef AssetRef)
{
	//	
	return g_LoadTasks.FindPtr( AssetRef );
}



