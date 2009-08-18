#include "TLAsset.h"
#include "TMesh.h"
#include "TFont.h"
#include "TMenu.h"
#include "TAudio.h"
#include "TScheme.h"
#include "TPath.h"
#include "TText.h"
#include "TTexture.h"
#include "TTimeline.h"
#include "TAtlas.h"
#include "TParticle.h"

#include "TLoadTask.h"
#include <TootleCore/TPtr.h>
#include <TootleCore/TEventChannel.h>
#include <TootleFileSys/TFileAsset.h>
#include <TootleFileSys/TLFileSys.h>

namespace TLAsset
{
	TPtr<TLAsset::TAssetManager>		g_pManager;
	TPtrArray<TLoadTask>				g_LoadTasks;
};



//----------------------------------------------------------
//	asset sort
//----------------------------------------------------------
TLArray::SortResult	TLAsset::AssetSort(const TPtr<TLAsset::TAsset>& a,const TPtr<TLAsset::TAsset>& b,const void* pTestRef)
{
	const TTypedRef& aRef = a->GetAssetAndTypeRef();
	const TTypedRef& bRef = pTestRef ? *(const TTypedRef*)pTestRef : b->GetAssetAndTypeRef();
	
	//	== turns into 0 (is greater) or 1(equals)
	return aRef < bRef ? TLArray::IsLess : (TLArray::SortResult)(aRef==bRef);	
}


//----------------------------------------------------------
//	get an asset ref for this type that isn't in use (starting from base ref)
//----------------------------------------------------------
TTypedRef TLAsset::GetFreeAssetRef(TTypedRef BaseAndTypeRef)
{
	//	start at a valid ref :)
	if ( !BaseAndTypeRef.GetRef().IsValid() )
		BaseAndTypeRef.GetRef().Increment();

	//	keep incrementing the ref until we don't find a matching asset
	while ( GetAssetInstance( BaseAndTypeRef ).IsValid() )
	{
		BaseAndTypeRef.GetRef().Increment();
	}

	return BaseAndTypeRef;
}



//----------------------------------------------------------
//	delete an asset
//----------------------------------------------------------
Bool TLAsset::DeleteAsset(const TLAsset::TAsset* pAsset)	
{
	return DeleteAsset( pAsset->GetAssetAndTypeRef() );	
}



//----------------------------------------------------------
//	
//----------------------------------------------------------
TPtr<TLAsset::TAsset>& TLAsset::GetAssetInstance(const TTypedRef& AssetAndTypeRef)
{
	if ( !g_pManager )
		return TLPtr::GetNullPtr<TLAsset::TAsset>();

	TPtr<TAsset>& pAssetPtr = g_pManager->GetAsset( AssetAndTypeRef );
	return pAssetPtr;
}


//----------------------------------------------------------
//	Replaces the old LoadAsset and GetAsset functions.
//	get an asset from the sytem. If it's not loaded, it will try to.
//	if Blocking is FALSE then if the file is still [async] loading then NULL will be returned.
//	use SyncTrue to load, SyncFalse to NOT load, SyncWait to async-load
//----------------------------------------------------------
TPtr<TLAsset::TAsset>& TLAsset::GetAssetPtr(const TTypedRef& AssetAndTypeRef,SyncBool LoadAsset)
{
	if ( !g_pManager )
	{
		TLDebug_Break("Asset factory expected");
		return TLPtr::GetNullPtr<TLAsset::TAsset>();
	}

	//	invalid params
	if ( !AssetAndTypeRef.IsValid() )
	{
		TLDebug_Break("GetAsset called for asset missing ref and/or type");
		return TLPtr::GetNullPtr<TLAsset::TAsset>();
	}

	//	check it doesnt already exist
	{
		TPtr<TLAsset::TAsset>& pAsset = GetAssetInstance( AssetAndTypeRef );

		//	if loaded, return straight away
		if ( pAsset && pAsset->IsLoaded() )
		{
			return pAsset;
		}

		//	asset is not loaded (whether it exists or not), and we don't want to load it, so return NULL
		if ( LoadAsset == SyncFalse )
		{
			return TLPtr::GetNullPtr<TLAsset::TAsset>();
		}
	}

	//	look for an existing loading task
	TPtr<TLAsset::TLoadTask> pLoadTask = TLAsset::GetLoadTask( AssetAndTypeRef );

	//	create a new load task if we dont have one already
	if ( !pLoadTask )
	{
		pLoadTask = new TLoadTask( AssetAndTypeRef );
	}

	//	missing task (failed to alloc?)
	if ( !pLoadTask )
		return TLPtr::GetNullPtr<TLAsset::TAsset>();

	//	do first update, if it fails then we can abort early and fail immedietly
	SyncBool FirstUpdateResult = pLoadTask->Update( 0.f, (LoadAsset == SyncTrue) );

	//	get loading/loaded asset
	TPtr<TAsset>& pLoadingAsset = pLoadTask->GetAsset();

	//	verify the correct result
	if ( FirstUpdateResult == SyncTrue )
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
		if ( pLoadingAsset->GetAssetType() != AssetAndTypeRef.GetTypeRef() )
		{
			#ifdef _DEBUG
			TTempString Debug_String("LoadAsset ");
			AssetAndTypeRef.GetString( Debug_String );
			Debug_String.Append(" but is type ");
			pLoadingAsset->GetAssetType().GetString( Debug_String );
			TLDebug_Print( Debug_String );
			#endif

			return TLPtr::GetNullPtr<TLAsset::TAsset>();
		}

		return pLoadingAsset;
	}

	//	if we got here with a block load, then it failed to block load...
	if ( LoadAsset == SyncTrue )
	{
		TLDebug_Break("Block load failed... asynchornously loading...");
	}

	//	load in progress, add task to list for asynchronous load
	g_LoadTasks.AddUnique( pLoadTask );

	//	is loading, so if we have an expected type, we have to return NULL as
	//	this type WONT be cast correctly (it will be a TempAsset) - the TPtr system will
	//	probably not cast at all and send back a NULL TPtr anyway
	//	we only get this case when NOT blockloading AND expecting a type
	//	gr: now always expect a type... so we have to return NULL as it's not loaded yet
	//		no such thing as a temp asset any more
	//if ( ExpectedAssetType.IsValid() )
		return TLPtr::GetNullPtr<TLAsset::TAsset>();

	return pLoadingAsset;
}


//----------------------------------------------------------
//	write an asset's current state back to the file system - fails if the asset isn't currently loaded
//----------------------------------------------------------
Bool TLAsset::SaveAsset(const TLAsset::TAsset* pAsset)	
{
	return SaveAsset( pAsset->GetAssetAndTypeRef() );	
}


//----------------------------------------------------------
//	write an asset's current state back to the file system - fails if the asset isn't currently loaded
//----------------------------------------------------------
Bool TLAsset::SaveAsset(const TTypedRef& AssetAndTypeRef)
{
	//	get asset
	TPtr<TAsset>& pAssetPtr = GetAssetInstance( AssetAndTypeRef );
	if ( !pAssetPtr || (pAssetPtr && !pAssetPtr->IsLoaded() ) )
	{
		TLDebug_Break("Failed to save asset as it isn't loaded");
		return FALSE;
	}

	//	todo: find out the original file system the asset came from...

	//	create new asset file in a file sys
	//	make a list of file systems to try and write to
	//	local file system first and finally resort to the virtual file sys
	TPtrArray<TLFileSys::TFileSys> FileSystems;
	TLFileSys::GetFileSys( FileSystems, TRef(), "Local" );
	TLFileSys::GetFileSys( FileSystems, "Virtual", "Virtual" );

	//	make new file
	TPtr<TLFileSys::TFileAsset> pAssetFile = TLFileSys::CreateAssetFileInFileSys( AssetAndTypeRef, FileSystems);

	//	failed to create file in any file sys
	if ( !pAssetFile )
	{
		//	failed to create new file
		TLDebug_Break("Failed to create new .Asset file");
		return FALSE;
	}

	//	export asset to asset file
	if ( pAssetPtr->Export( pAssetFile ) != SyncTrue )
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


//------------------------------------------------------------
//	wrapper to just simply load an asset - returns TRUE if currently loaded, SyncWait if loading, SyncFalse if failed
//------------------------------------------------------------
SyncBool TLAsset::LoadAsset(const TTypedRef& AssetAndTypeRef,Bool BlockLoad)
{
	if ( !g_pManager )
	{
		TLDebug_Break("Asset factory expected");
		return SyncFalse;
	}

	//	invalid params
	if ( !AssetAndTypeRef.IsValid() )
	{
		TLDebug_Break("GetAsset called for asset missing ref and/or type");
		return SyncFalse;
	}

	//	get the asset object
	TPtr<TAsset>& pAssetPtr = GetAssetInstance( AssetAndTypeRef );

	//	doesnt exist, start a load and see what the result is
	if ( !pAssetPtr )
	{
		TPtr<TAsset>& pNewAssetPtr = GetAssetPtr( AssetAndTypeRef, BlockLoad ? SyncTrue : SyncWait );

		if ( pNewAssetPtr )
		{
			//	loaded okay
			return SyncTrue;
		}
		else if ( BlockLoad )
		{
			//	still not loaded, (no asset returned) if it's a block load, it failed
			return SyncFalse;
		}
		else
		{
			//	not a block load, find the load task...
			TLAsset::TLoadTask* pLoadTask = TLAsset::GetLoadTask( AssetAndTypeRef );

			//	get the load task state, if no load task, and no asset... assume it failed
			SyncBool LoadTaskState = pLoadTask ? pLoadTask->GetLoadingState() : SyncFalse;

			return LoadTaskState;
		}
	}

	//	asset exists, check its state
	switch ( pAssetPtr->GetLoadingState() )
	{
	case TLAsset::LoadingState_Loading:
	case TLAsset::LoadingState_Init:
		{
			if ( !BlockLoad )
				return SyncWait;

			//	block load (this should continue an existing load...)
			TPtr<TAsset>& pLoadedAssetPtr = GetAssetPtr( AssetAndTypeRef, SyncTrue );
			return pLoadedAssetPtr ? SyncTrue : SyncFalse;
		}
		break;
	
	case TLAsset::LoadingState_Loaded:
		return SyncTrue;

	case TLAsset::LoadingState_Failed:
		return SyncFalse;

	case TLAsset::LoadingState_Deleted:
		TLDebug_Break("Unexpected state - deleted assets should not be in the asset factory");
		return SyncFalse;

	default:
		TLDebug_Break("Unknown loading state of asset");
		return SyncFalse;
	}
}





TLAsset::TAssetManager::TAssetManager(TRefRef ManagerRef) :
	TLCore::TManager				( ManagerRef ),
	m_Assets						( &TLAsset::AssetSort, 100 )
{
	//	add a core asset factory type
	TPtr<TAssetFactory> pFactory = new TAssetFactory;
	AddAssetFactory( pFactory );
}



SyncBool TLAsset::TAssetManager::Initialise() 
{	
	if(TLMessaging::g_pEventChannelManager)
	{
		TLMessaging::g_pEventChannelManager->RegisterEventChannel(this, GetManagerRef(), "OnAssetChanged");

		return SyncTrue;
	}

	return SyncWait; 
}



void TLAsset::TAssetManager::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
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


void TLAsset::TAssetManager::OnAssetLoad(const TTypedRef& AssetAndTypeRef, Bool bStatus)
{
	//	if the asset failed to load then ensure the asset object is marked as deleted
	if ( !bStatus )
	{
		//	if there is an asset - mark it as failed to load
		TPtr<TLAsset::TAsset>& pAsset = GetAssetInstance(AssetAndTypeRef);
		if ( pAsset )
		{
			pAsset->SetLoadingState( TLAsset::LoadingState_Failed );
		}
	}

	TLMessaging::TMessage Message("OnAssetChanged");
	Message.Write( AssetAndTypeRef.GetRef() );
	Message.Write( AssetAndTypeRef.GetTypeRef() );
	Message.Write(TRef("Load"));
	Message.Write(bStatus);		// Successful/Failed load.  Would we want to let things know if the load failed?

	PublishMessage(Message);
}

void TLAsset::TAssetManager::OnAssetUnload(const TTypedRef& AssetAndTypeRef)
{
	TLMessaging::TMessage Message("OnAssetChanged");
	Message.Write( AssetAndTypeRef.GetRef() );
	Message.Write( AssetAndTypeRef.GetTypeRef() );
	Message.Write(TRef("Unload"));
	Message.Write( (Bool)TRUE );

	PublishMessage(Message);
}




SyncBool TLAsset::TAssetManager::Update(float fTimeStep)
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


SyncBool TLAsset::TAssetManager::Shutdown()
{
	//	free tasks
	g_LoadTasks.Empty( TRUE );

	//	free assets
	m_Assets.Empty(TRUE);

	//	free factories
	m_Factories.Empty(TRUE);

	return TManager::Shutdown();	
}

	
//----------------------------------------------------------
//	return a pointer to a new asset - mostly used for runtime asssets
//----------------------------------------------------------
TPtr<TLAsset::TAsset>& TLAsset::TAssetManager::CreateAsset(const TTypedRef& AssetAndTypeRef)
{
	//	check for existing asset
	TPtr<TAsset>& pOldAsset = GetAsset( AssetAndTypeRef );
	if ( pOldAsset )
		return pOldAsset;

	//	make new instance
	for ( u32 f=0;	f<m_Factories.GetSize();	f++ )
	{
		TAssetFactory& Factory = *m_Factories[f];

		//	create instance
		TPtr<TAsset> pNewAsset;
		Factory.CreateInstance( pNewAsset, AssetAndTypeRef.GetRef(), AssetAndTypeRef.GetTypeRef() );
		if ( !pNewAsset )
			continue;

		//	ensure it's the right type
		if ( pNewAsset->GetAssetType() != AssetAndTypeRef.GetTypeRef() )
		{
	#ifdef _DEBUG
			TTempString DebugString("Created asset ");
			pNewAsset->GetAssetAndTypeRef().GetString( DebugString );
			DebugString.Append(" but expected type ");
			AssetAndTypeRef.GetTypeRef().GetString( DebugString );
			TLDebug_Print( DebugString );
	#endif
			TLDebug_Break("Wrong type of asset. If this is a new asset type, check the TAsset() constructor in your new asset types constructor.");
			return TLPtr::GetNullPtr<TLAsset::TAsset>();
		}
		
		TTempString DebugString("Created asset: ");
		pNewAsset->GetAssetAndTypeRef().GetString( DebugString );
		TLDebug_Print( DebugString );

		//	add to asset list
		TPtr<TAsset>& pRealNewAsset = m_Assets.AddPtr( pNewAsset );

		//	return ptr ref from asset array
		return pRealNewAsset;
	}

	TTempString DebugString("Failed to create asset... ");
	AssetAndTypeRef.GetString( DebugString );
	//	gr: changed to break, if this fails the factory failed to create it... invalid type?
	TLDebug_Break( DebugString );
	return TLPtr::GetNullPtr<TLAsset::TAsset>();
}


//----------------------------------------------------------
//	delete an asset - returns true if it did exist
//----------------------------------------------------------
Bool TLAsset::TAssetManager::DeleteAsset(const TTypedRef& AssetAndTypeRef)
{
	//	find the existing asset
	s32 AssetIndex = m_Assets.FindIndex( AssetAndTypeRef );

	//	doesnt exist, nothing to delete
	if ( AssetIndex == -1 )
	{
		#ifdef _DEBUG
			TTempString DebugString("Failed to delete asset ");
			AssetAndTypeRef.GetString( DebugString );
			TLDebug_Print( DebugString );
		#endif
		return FALSE;
	}

	//	mark asset as unavailible in case it's lingering around somewhere to help debugging
	TPtr<TAsset>& pAssetPtr = m_Assets[AssetIndex];
	if ( pAssetPtr )
		pAssetPtr->SetLoadingState( TLAsset::LoadingState_Deleted );

	//	remove from array
	m_Assets.RemoveAt(AssetIndex);
	
	//	pAssetPtr is now invalid

	#ifdef _DEBUG
		TTempString DebugString("Deleted asset ");
		AssetAndTypeRef.GetString( DebugString );
		TLDebug_Print( DebugString );
	#endif

	// Do a notification to say the asset has been removed
	OnAssetUnload( AssetAndTypeRef );

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
	case STRef(T,i,m,e,l):	return new TLAsset::TTimeline( InstanceRef );	//	"Timeline"
	case STRef(T,e,x,t,u):	return new TLAsset::TTexture( InstanceRef );	//	"Texture"
	case STRef(A,t,l,a,s):	return new TLAsset::TAtlas( InstanceRef );	//	"Atlas" 
	case STRef(P,a,r,t,i):	return new TLAsset::TParticle( InstanceRef );	//	"Particle" 

	//	gr: dumb asset - just stores data - consider turning this into a specific TBinaryTree/"Data" asset
	case STRef(A,s,s,e,t):	return new TLAsset::TAsset( TLAsset::TAsset::GetAssetType_Static(), InstanceRef );	//	"Asset"
	};

	return NULL;
}


//----------------------------------------------------------
//	load all assets from the file system of this type that we can identify. (ie. won't load non-compiled assets)
//	Returns TRUE if we found any new assets of this type
//----------------------------------------------------------
Bool TLAsset::LoadAllAssets(TRefRef AssetType)
{
	//	update file lists first
	TLFileSys::g_pFactory->UpdateFileLists();

	//	get a list of all files...
	TArray<TRef> FileList;
	TLFileSys::GetFileList( FileList );

	//	get a list of all matching type assets we've found from the file sys
	TArray<TTypedRef> AssetList;

	//	go through all the files...
	for ( u32 f=0;	f<FileList.GetSize();	f++ )
	{
		//	fetch the group of files
		TPtr<TLFileSys::TFileGroup>& pFileGroup = TLFileSys::GetFileGroup( FileList[f] );
		if ( !pFileGroup )
		{
			TLDebug_Break("file group expected");
			continue;
		}

		//	fetch the newest asset-type file from the group
		TPtr<TLFileSys::TFile>& pFile = pFileGroup->GetNewestFile( TRef_Static(A,s,s,e,t) );

		//	if null returned, this ref doesn't represent any asset files...
		if ( !pFile )
			continue;

		//	is an asset file type, cast it (extra type check)
		TPtr<TLFileSys::TFileAsset> pAssetFile = pFile;
		if ( !pAssetFile )
		{
			TLDebug_Break("Found a file when looking for asset files, that isn't an asset file");
			continue;
		}

		//	file needs to load header
		if ( !pAssetFile->IsHeaderLoaded() || pAssetFile->GetNeedsImport() )
		{
			//	load plain file
			if ( !pAssetFile->IsLoaded() )
			{
				TLFileSys::TFileSys* pFileSys = pAssetFile->GetFileSys();
				if ( !pFileSys )
				{
					TLDebug_Break("File sys missing from file");
					continue;
				}
				//	block load from file sys
				SyncBool LoadResult = SyncWait;
				int SafetyCounter = 100;
				while ( LoadResult == SyncWait && SafetyCounter-->0 )
					LoadResult = pFileSys->LoadFile( pFile );

				//	failed to load from file sys
				if ( LoadResult != SyncTrue )
					continue;
			}

			//	import asset file
			if ( pAssetFile->GetNeedsImport() )
			{
				//	todo: just need to import header here!
				//	block import file
				SyncBool LoadResult = SyncWait;
				int SafetyCounter = 100;
				while ( LoadResult == SyncWait && SafetyCounter-->0 )
					LoadResult = pAssetFile->Import();
				
				//	failed to import plain file
				if ( LoadResult != SyncTrue )
					continue;
			}

			//	imported broken/out dated asset file?
			if ( !pAssetFile->IsHeaderLoaded() )
				continue;
		}

		//	asset file is loaded, see what kind of asset is inside
		TTypedRef AssetFileAssetRef = pAssetFile->GetAssetAndTypeRef();
		if ( !AssetFileAssetRef.IsValid() )
		{
			TLDebug_Break("Invalid type of asset recognised by AssetFile");
			continue;
		}
			
		//	check type is the same type of asset we're looking for
		if ( AssetFileAssetRef.GetTypeRef() != AssetType )
			continue;

		//	it's the right type! add to list and we'll see if we need to load it
		AssetList.Add( AssetFileAssetRef );

		TTempString Debug_String("Found asset in file system: ");
		AssetFileAssetRef.GetString( Debug_String );
		Debug_String.Appendf(" from file %s", pAssetFile->GetFilename().GetData() );
		TLDebug_Print( Debug_String );
	}

	//	track how many we load
	Bool LoadedNewAsset = FALSE;

	//	now have a list of assets of the correct type (loaded or not), load ones that need loading
	for ( u32 a=0;	a<AssetList.GetSize();	a++ )
	{
		TTypedRef AssetRef = AssetList[a];

		//	see if it's currently loaded - dont need to try and load it if it is 
		//	(we can just use LoadAsset below but we want to know what NEW assets we've loaded)
		TLAsset::TAsset* pAsset = GetAsset( AssetRef, SyncFalse );
		if ( pAsset )
			continue;

		//	not loaded, block load it
		SyncBool AssetLoadState = LoadAsset( AssetRef, TRUE );

		//	failed to load
		if ( AssetLoadState != SyncTrue )
			continue;

		//	loaded this asset
		LoadedNewAsset |= TRUE;
	}

	return LoadedNewAsset;
}
