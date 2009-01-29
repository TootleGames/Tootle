#include "TLoadTask.h"
#include <TootleFileSys/TLFileSys.h>
#include <TootleFileSys/TFileAsset.h>
#include "TAsset.h"


//	gr: for testing (for me at least) dont turn file into an asset file
#define ENABLE_OUTPUT_ASSET_FILE	//	default enabled


namespace TLAsset
{
	extern TPtrArray<TLoadTask>			g_LoadTasks;
};

using namespace TLLoadTask;

//------------------------------------------------------------
//	
//------------------------------------------------------------
TLAsset::TLoadTask::TLoadTask(TRefRef AssetRef) :
	m_AssetRef	( AssetRef )
{
	//	init modes
	AddMode<Mode_Init>("Init");
	AddMode<Mode_GetPlainFile>("PFGet");
	AddMode<Mode_PlainFileLoad>("PFLoad");
	AddMode<Mode_PlainFileCreateAssetFile>("PFcAF");
	AddMode<Mode_PlainFileExport>("PFexport");
	AddMode<Mode_GetAssetFile>("AFGet");
	AddMode<Mode_AssetFileImport>("AFImport");
	AddMode<Mode_AssetFileExport>("AFExport");
	AddMode<Mode_AssetFileWrite>("AFWrite");
	AddMode<Mode_CreateAsset>("ACreate");
	AddMode<Mode_AssetImport>("AImport");
	AddMode<Mode_Finished>("Finished");
	AddMode<Mode_Finished>("Failed");
}


//------------------------------------------------------------
//	update load
//------------------------------------------------------------
SyncBool TLAsset::TLoadTask::Update(Bool Blocking)
{
	do
	{
		//	update state machine
		TStateMachine::Update();

		TRef CurrentModeRef = GetCurrentModeRef();

		//	if we're in the finished mode, we're finished
		if ( CurrentModeRef == "Finished" )
			return SyncTrue;

		//	if in failed mode... or no mode, failed
		if ( !CurrentModeRef.IsValid() || CurrentModeRef == "Failed" )
			return SyncFalse;
	}
	while ( Blocking );

	//	otherwise still going
	return SyncWait;
}





//------------------------------------------------------------
//	fetch the plain file
//------------------------------------------------------------
TRef Mode_Init::Update()
{
	//	if no asset, create a placeholder asset
	TPtr<TLAsset::TAsset> pAsset = GetAsset();
	if ( !pAsset )
	{
		pAsset = TLAsset::CreateAsset( GetAssetRef(), "Temp" );

		//	problem creating placeholder asset...
		if ( !pAsset )
			return "Failed";
	}

	return "AFGet";
}
			

//------------------------------------------------------------
//	fetch the plain file
//------------------------------------------------------------
TRef Mode_GetPlainFile::Update()
{
	//	get list of file systems to check through
	TPtrArray<TLFileSys::TFileSys> FileSysList;
	TLFileSys::GetFileSys( FileSysList, TRef(), TRef() );

	//	get the plain file for this asset
	GetPlainFile() = TLFileSys::FindFile( FileSysList, GetAssetRef() );

	//	didn't find a file to convert, there is no file with this ref at all
	if ( !GetPlainFile() )
	{
		return "Failed";
	}

	//	is the file out of date?? reload it!
	if ( GetPlainFile()->IsLoaded() != SyncTrue || GetPlainFile()->GetFlags()( TLFileSys::TFile::OutOfDate ) )
	{
		return "PFLoad";
	}

	return "PFcAF";
}


//------------------------------------------------------------
//	load plain file from file sys
//------------------------------------------------------------
TRef Mode_PlainFileLoad::Update()
{
	//	load the plain file
	TPtr<TLFileSys::TFileSys> pFileSys = GetPlainFile()->GetFileSys();
	SyncBool LoadResult = pFileSys->LoadFile( GetPlainFile() );
	if ( LoadResult == SyncWait )
		return TRef();
	
	if ( LoadResult == SyncFalse )
		return "Failed";

	//	convert to AssetFile
	return "PFcAF";
}


//------------------------------------------------------------
//	create asset file from plain file
//------------------------------------------------------------
TRef Mode_PlainFileCreateAssetFile::Update()
{
	//	create a new asset file
	TPtr<TLFileSys::TFileAsset> pNewFile;

	//	put asset file into the file system that the file came from...
#ifdef ENABLE_OUTPUT_ASSET_FILE
	pNewFile = CreateFile( GetPlainFile()->GetFileSysRef() );
#endif

	//	if that didnt work, try putting it in the virtual file sys
	if ( !pNewFile )
	{
		pNewFile = CreateFile("Virtual");
	}

	//	failed to create (even into the virtual file sys?)
	if ( !pNewFile )
	{
		//	failed to create new file
		return "Failed";
	}

	//	save this new file as our asset file
	GetAssetFile() = pNewFile;

	//	export plain file
	return "PFExport";
}


TPtr<TLFileSys::TFileAsset> Mode_PlainFileCreateAssetFile::CreateFile(TRefRef FileSysRef)
{
	//	get the file sys
	TPtr<TLFileSys::TFileSys> pFileSys = TLFileSys::GetFileSys( FileSysRef );
	if ( !pFileSys )
	{
		return NULL;
	}

	//	make up new filename (with the right extension)
	TString NewFilename = GetPlainFile()->GetFilename();
	NewFilename.Append(".");
	TRef("asset").GetString( NewFilename );

	//	create file
	TPtr<TLFileSys::TFileAsset> pNewFile = pFileSys->CreateFile( NewFilename, "asset" );
	return pNewFile;
}



//------------------------------------------------------------
//	convert plain file to asset file
//------------------------------------------------------------
TRef Mode_PlainFileExport::Update()
{
	SyncBool ExportResult = GetPlainFile()->Export( GetAssetFile() );

	if ( ExportResult == SyncWait )
		return TRef();

	//	failed to export our file into an asset file
	if ( ExportResult == SyncFalse )
		return "Failed";

	return "AFExport";
}


//------------------------------------------------------------
//	fetch asset file
//------------------------------------------------------------
TRef Mode_GetAssetFile::Update()
{
	//	get the asset file for this asset

	//	get list of file systems to check through
	TPtrArray<TLFileSys::TFileSys> FileSysList;
	TLFileSys::GetFileSys( FileSysList, TRef(), TRef() );

	//	find matching asset file
	GetAssetFile() = TLFileSys::FindFile( FileSysList, GetAssetRef(), "Asset" );

	//	no asset file, need to find a plain file to load to convert first
	if ( !GetAssetFile() )
	{
		return "PFGet";
	}

	//	if our new asset file needs to be written back to a normal file, do that
	if ( GetAssetFile()->GetNeedsExport() )
	{
		return "AFExport";
	}

	//	just export to asset
	return "Acreate";
}


//------------------------------------------------------------
//	turn asset file back to plain file
//------------------------------------------------------------
TRef Mode_AssetFileImport::Update()
{
	if ( !GetAssetFile() )
	{
		TLDebug_Break("Asset file expected");
		return "Failed";
	}

	//	attempt to write our new file back into our filesytem
	//	convert the asset file to a plain file first, then write that
	SyncBool ImportResult = GetAssetFile()->Import();
	if ( ImportResult == SyncWait )
		return TRef();

	if ( ImportResult == SyncFalse )
		return "Failed";

	//	imported, now create asset from this file
	return "ACreate";
}


//------------------------------------------------------------
//	turn asset file back to plain file
//------------------------------------------------------------
TRef Mode_AssetFileExport::Update()
{
	//	attempt to write our new file back into our filesytem
	//	convert the asset file to a plain file first, then write that
	SyncBool ExportResult = GetAssetFile()->Export();
	if ( ExportResult == SyncWait )
		return TRef();

	if ( ExportResult == SyncFalse )
		return "Failed";

	return "AFWrite";
}

//------------------------------------------------------------
//	save asset file back to file sys
//------------------------------------------------------------
TRef Mode_AssetFileWrite::Update()
{
	if ( !GetAssetFile() )
	{
		TLDebug_Break("Asset file expected");
		return "Failed";
	}

	TPtr<TLFileSys::TFileSys> pFileSys = TLFileSys::GetFileSys( GetAssetFile()->GetFileSysRef() );
	if ( pFileSys )
	{
		//	write file
		TPtr<TLFileSys::TFile> pAssetFile = GetAssetFile();
		SyncBool WriteResult = pFileSys->WriteFile( pAssetFile );
		if ( WriteResult == SyncWait )
			return TRef();
	}

	//	failed or didnt, just continue
	return "Acreate";
}


//------------------------------------------------------------
//	create new asset
//------------------------------------------------------------
TRef Mode_CreateAsset::Update()
{
	if ( !GetAssetFile() )
	{
		TLDebug_Break("no asset file");
		return TRef();
	}

	//	check in case this asset file needs importing
	if ( GetAssetFile()->GetNeedsImport() )
	{
		return "AFImport";
	}

	TRefRef NewAssetType = GetAssetFile()->GetAssetTypeRef();

	//	if we have an asset, but its the wrong type... destroy it
	TPtr<TLAsset::TAsset> pAsset = GetAsset();
	if ( pAsset )
	{
		if ( pAsset->GetAssetType() != NewAssetType )
		{
			TLAsset::DeleteAsset( GetAssetRef() );
			pAsset = NULL;
		}
	}

	//	create asset
	if ( !pAsset )
	{
		//	create new asset from the factory
		pAsset = TLAsset::CreateAsset( GetAssetRef(), NewAssetType );
		//	failed to create
		if ( !pAsset )
			return "Failed";
	}
	
	//	gr: mark asset as importing so we don't think it's failed...
	//	this is to fix the system on the ipod (dunno why it's just the ipod that struggles like this)
	//	but the pre-load system was checking for assets being loaded, and caught the asset at this state
	//	ready to be imported, correct type created, but loading state still in default (which is FALSE)
	//	maybe make default state wait? maybe that's confusing, may need to change to Init,Fail,Wait,True...
	//pAsset->SetLoadingState( SyncWait );
				

	return "AImport";
}


//------------------------------------------------------------
//	turn asset file into asset
//------------------------------------------------------------
TRef Mode_AssetImport::Update()
{
	//	export from assetfile to asset
	TPtr<TLAsset::TAsset> pAsset = GetAsset();
	pAsset->Import( GetAssetFile() );

	//	change mode depending on loading state
	TLAsset::TLoadingState LoadingState = pAsset->GetLoadingState();
	switch ( LoadingState )
	{
		case TLAsset::LoadingState_Loaded:
		{
			TTempString Debug_String("Imported asset okay ");
			pAsset->GetAssetRef().GetString( Debug_String );
			Debug_String.Append(" (");
			pAsset->GetAssetType().GetString( Debug_String );
			Debug_String.Appendf(") %x", pAsset.GetObject() );
			TLDebug_Print( Debug_String );
		
			return "Finished";
		}

		case TLAsset::LoadingState_Loading:
			//	still loading
			return TRef();

		default:
		case TLAsset::LoadingState_Failed:
			return "Failed";
	}
}

