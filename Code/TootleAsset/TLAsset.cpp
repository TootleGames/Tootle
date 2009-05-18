#include "TLAsset.h"
#include "TMesh.h"
#include "TFont.h"
#include "TMenu.h"
#include "TAudio.h"
#include "TScheme.h"
#include "TPath.h"
#include "TText.h"
#include "TTexture.h"
#include "TAssetTimeline.h"
#include "TAtlas.h"

#include "TLoadTask.h"
#include <TootleCore/TPtr.h>
#include <TootleCore/TEventChannel.h>
#include <TootleFileSys/TFileAsset.h>
#include <TootleFileSys/TLFileSys.h>

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
		//	gr: changed to break, if this fails the factory failed to create it... invalid type?
		TLDebug_Break( DebugString );
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
	TRef AssetType = pAsset->GetAssetType();

#ifdef _DEBUG
	TTempString DebugString("Deleting asset from factory... ");
	AssetRef.GetString( DebugString );
	DebugString.Append(" (");
	pAsset->GetAssetType().GetString( DebugString );
	DebugString.Append(")");
	TLDebug_Print( DebugString );
#endif

	//	delete from factory
	if ( g_pFactory->RemoveInstance( AssetRef ) )
	{
		
		// Do a notification to say the asset has been removed
		g_pFactory->OnAssetUnload(AssetRef, AssetType);
	}
	else
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
TPtr<TLAsset::TAsset>& TLAsset::LoadAsset(const TRef& AssetRef,Bool bBlocking,TRefRef ExpectedAssetType)
{
	if ( !g_pFactory )
	{
		TLDebug_Break("Asset factory expected");
		return TLPtr::GetNullPtr<TLAsset::TAsset>();
	}

	//	check it doesnt already exist
	{
		TPtr<TAsset>& pAssetPtr = GetAsset( AssetRef );
		TAsset* pAsset = pAssetPtr;

		//	if loaded, return straight away
		if ( pAsset && pAsset->IsLoaded() )
		{
			//	check type
			if ( ExpectedAssetType.IsValid() && pAsset->GetAssetType() != ExpectedAssetType )
			{
				#ifdef _DEBUG
				TTempString Debug_String("LoadAsset ");
				AssetRef.GetString( Debug_String );
				Debug_String.Append(": expected type ");
				ExpectedAssetType.GetString( Debug_String );
				Debug_String.Append(" but is type ");
				pAsset->GetAssetType().GetString( Debug_String );
				TLDebug_Print( Debug_String );
				#endif

				return TLPtr::GetNullPtr<TLAsset::TAsset>();
			}

			return pAssetPtr;
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

		//	check type
		if ( ExpectedAssetType.IsValid() && pLoadingAsset->GetAssetType() != ExpectedAssetType )
		{
			#ifdef _DEBUG
			TTempString Debug_String("LoadAsset ");
			AssetRef.GetString( Debug_String );
			Debug_String.Append(": expected type ");
			ExpectedAssetType.GetString( Debug_String );
			Debug_String.Append(" but is type ");
			pLoadingAsset->GetAssetType().GetString( Debug_String );
			TLDebug_Print( Debug_String );
			#endif

			return TLPtr::GetNullPtr<TLAsset::TAsset>();
		}

		return pLoadingAsset;
	}

	//	if we got here with a block load, then it failed to block load...
	if ( bBlocking )
	{
		TLDebug_Break("Block load failed... asynchornously loading...");
	}

	//	load in progress, add task to list for asynchronous load
	g_LoadTasks.AddUnique( pLoadTask );

	//	is loading, so if we have an expected type, we have to return NULL as
	//	this type WONT be cast correctly (it will be a TempAsset) - the TPtr system will
	//	probably not cast at all and send back a NULL TPtr anyway
	//	we only get this case when NOT blockloading AND expecting a type
	//if ( ExpectedAssetType.IsValid() && pLoadingAsset->GetAssetType() != ExpectedAssetType )
	if ( ExpectedAssetType.IsValid() )
		return TLPtr::GetNullPtr<TLAsset::TAsset>();

	return pLoadingAsset;
}


//----------------------------------------------------------
//	export an asset out to a .asset file - currently writes to the user file system
//----------------------------------------------------------
Bool TLAsset::SaveAsset(TRefRef AssetRef)
{
	//	get asset
	TPtr<TLAsset::TAsset>& pAsset = GetAsset( AssetRef, TRUE );
	if ( !pAsset )
	{
		TLDebug_Break("Asset expected");
		return FALSE;
	}

	//	gr: currently the order of this is a bit shit, filesys and asset load system need a bit of a fiddle before 
	//		I can fix this. todo, but not urgent

	//	create new asset file in a file sys
	//	make a list of file systems to try and write to
	//	local file system first and finally resort to the virtual file sys
	TPtrArray<TLFileSys::TFileSys> FileSystems;
	TLFileSys::GetFileSys( FileSystems, TRef(), "Local" );
	TLFileSys::GetFileSys( FileSystems, "Virtual", "Virtual" );

	//	make up new filename (with the right extension)
	TString NewFilename;
	pAsset->GetAssetRef().GetString( NewFilename );
	NewFilename.Append(".");
	TRef("asset").GetString( NewFilename );
	
	TPtr<TLFileSys::TFileAsset> pAssetFile = TLFileSys::CreateFileInFileSys( NewFilename, FileSystems, "Asset");

	//	failed to create file in any file sys
	if ( !pAssetFile )
	{
		//	failed to create new file
		TLDebug_Break("Failed to create new .asset file");
		return FALSE;
	}

	//	export asset to asset file
	if ( pAsset->Export( pAssetFile ) != SyncTrue )
		return FALSE;

	//	export asset file to plain file
	if ( pAssetFile->Export() != SyncTrue )
		return FALSE;

	//	get the file sys for the file so we can write to it
	TPtr<TLFileSys::TFileSys> pFileSys = pAssetFile->GetFileSys();
	if ( !pFileSys )
	{
		TLDebug_Break("Expected file sys on new asset file");
		return FALSE;
	}

	//	write file to filesys
	TPtr<TLFileSys::TFile> pFile = pAssetFile;
	if ( !pFileSys->WriteFile( pFile ) )
		return FALSE;

	//	all done!
	return TRUE;
}


//----------------------------------------------------------
//	instance an asset
//----------------------------------------------------------
TLAsset::TAsset* TLAsset::TAssetFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	switch ( TypeRef.GetData() )
	{
	case STRef(A,u,d,i,o):	return new TLAsset::TAudio( InstanceRef );	//	"audio"
	case STRef4(M,e,s,h):	return new TLAsset::TMesh( InstanceRef );	//	"mesh"
	case STRef4(F,o,n,t):	return new TLAsset::TFont( InstanceRef );	//	"font"
	case STRef4(M,e,n,u):	return new TLAsset::TMenu( InstanceRef );	//	"menu"
	case STRef(S,c,h,e,m):	return new TLAsset::TScheme( InstanceRef );	//	"scheme"
	case STRef(P,a,t,h,N):	return new TLAsset::TPathNetwork( InstanceRef );	//	"PathNetwork"
	case STRef4(T,e,x,t):	return new TLAsset::TText( InstanceRef );	//	"Text"
	case STRef(T,i,m,e,l):	return new TLAsset::TAssetTimeline( InstanceRef );	//	"Timeline"
	case STRef(T,e,x,t,u):	return new TLAsset::TTexture( InstanceRef );	//	"Texture"
	case STRef(A,t,l,a,s):	return new TLAsset::TAtlas( InstanceRef );	//	"Atlas" 

	//	gr: dumb asset - just stores data - consider turning this into a specific TBinaryTree/"Data" asset
	case STRef(A,s,s,e,t):	return new TLAsset::TAsset( TypeRef, InstanceRef );	//	"Asset"
	
	//	"nothing" asset - used as a placeholder whilst we convert file into a real asset
	case STRef4(T,e,m,p):	return new TLAsset::TTempAsset( InstanceRef );	//	"Temp"
	};

#ifdef _DEBUG
	TTempString Debug_String("Don't know how to make asset type ");
	TypeRef.GetString( Debug_String );
	TLDebug_Break( Debug_String );
#endif

	return NULL;
}

SyncBool TLAsset::TAssetFactory::Initialise() 
{	
	if(TLMessaging::g_pEventChannelManager)
	{
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), "OnAssetChanged");

		return SyncTrue;
	}

	return SyncWait; 
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


void TLAsset::TAssetFactory::OnAssetLoad(TRefRef AssetRef, TRefRef AssetType, Bool bStatus)
{
	//	if the asset failed to load then ensure the asset object is marked as deleted
	if ( !bStatus )
	{
		//	if there is an asset - mark it as failed to load
		TPtr<TLAsset::TAsset> pAsset = GetAsset(AssetRef);
		if ( pAsset )
		{
			pAsset->SetLoadingState( TLAsset::LoadingState_Failed );

			//	gr: if it's a "temp" type, then delete the asset
			if ( pAsset->GetAssetType() == "temp" )
			{
				TLAsset::DeleteAsset( pAsset->GetAssetRef() );
			}
		}
	}

	TLMessaging::TMessage Message("OnAssetChanged");
	Message.Write(AssetRef);
	Message.Write(AssetType);
	Message.Write(TRef("Load"));
	Message.Write(bStatus);		// Successful/Failed load.  Would we want to let things know if the load failed?

	PublishMessage(Message);
}

void TLAsset::TAssetFactory::OnAssetUnload(TRefRef AssetRef, TRefRef AssetType)
{
	TLMessaging::TMessage Message("OnAssetChanged");
	Message.Write(AssetRef);
	Message.Write(AssetType);
	Message.Write(TRef("Unload"));
	Message.Write(TRUE);

	PublishMessage(Message);
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
		//	gr: stop loading files that have failed too
		if ( UpdateResult == SyncTrue || UpdateResult == SyncFalse )
			pTask = NULL;
	}

	//	remove null(completed) tasks
	g_LoadTasks.RemoveNull();

	return SyncTrue;
}

