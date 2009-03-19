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
//	print out some debug info for this step
//------------------------------------------------------------
void TLLoadTask::TLoadTaskMode::Debug_PrintStep(const char* pStepString)
{
	TTempString Debug_String("Loading ");
	GetAssetRef().GetString( Debug_String );
	Debug_String.Appendf(": %s", pStepString );
	TLDebug_Print( Debug_String );
}


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
	AddMode<Mode_AssetFileLoad>("AFLoad");
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
SyncBool TLAsset::TLoadTask::Update(float Timestep,Bool Blocking)
{
	do
	{
		//	update state machine
		TStateMachine::Update(Timestep);

		TRef CurrentModeRef = GetCurrentModeRef();

		//	if we're in the finished mode, we're finished
		if ( CurrentModeRef == "Finished" )
		{
			TRef AssetType = (m_pAssetFile ? m_pAssetFile->GetAssetTypeRef() : (u32)0);
			TLAsset::g_pFactory->OnAssetLoad(GetAssetRef(), AssetType, TRUE);
			return SyncTrue;
		}

		//	if in failed mode... or no mode, failed
		if ( !CurrentModeRef.IsValid() || CurrentModeRef == "Failed" )
		{
			#ifdef _DEBUG
			TTempString Debug_String("Failed to load asset ");
			GetAssetRef().GetString( Debug_String );
			TLDebug_Warning( Debug_String );
			#endif
			TRef AssetType = (m_pAssetFile ? m_pAssetFile->GetAssetTypeRef() : (u32)0);
			TLAsset::g_pFactory->OnAssetLoad(GetAssetRef(), AssetType, FALSE);
			return SyncFalse;
		}
	}
	while ( Blocking );

	//	otherwise still going
	return SyncWait;
}





//------------------------------------------------------------
//	fetch the plain file
//------------------------------------------------------------
TRef Mode_Init::Update(float Timestep)
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
TRef Mode_GetPlainFile::Update(float Timestep)
{
	//	get the plain file for this asset (if we haven't already located it)
	if ( !GetPlainFile() )
	{
		GetPlainFile() = TLFileSys::GetFile( GetAssetRef() );

		//	didn't find a file to convert, there is no file with this ref at all
		if ( !GetPlainFile() )
		{
			return "Failed";
		}
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
TRef Mode_PlainFileLoad::Update(float Timestep)
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
TRef Mode_PlainFileCreateAssetFile::Update(float Timestep)
{
	//	create a new asset file
	TPtr<TLFileSys::TFileAsset> pNewFile;

	//	make a list of file systems to try and write to
	TPtrArray<TLFileSys::TFileSys> FileSystems;
	//	first try and put asset file into the file system that the file came from...
	TLFileSys::GetFileSys( FileSystems, GetPlainFile()->GetFileSysRef(), TRef() );
	//	then any local file sys
	TLFileSys::GetFileSys( FileSystems, TRef(), "Local" );
	//	and finally resort to the virtual file sys
	TLFileSys::GetFileSys( FileSystems, "Virtual", "Virtual" );

	//	make up new filename (with the right extension)
	TString NewFilename = GetPlainFile()->GetFilename();
	NewFilename.Append(".");
	TRef("asset").GetString( NewFilename );

	//	loop through file systems and try and create file
	for ( u32 i=0;	i<FileSystems.GetSize();	i++ )
	{
		TLFileSys::TFileSys& FileSys = *FileSystems[i];

		//	try and create file
		pNewFile = FileSys.CreateFile( NewFilename, "asset" );

		//	created file, break out of loop
		if ( pNewFile )
		{
			//	check file ref matches the one we based it from/our asset
			if ( pNewFile->GetFileRef() != GetPlainFile()->GetFileRef() )
			{
				TLDebug_Break("Newly created .asset file's file ref doesn't match the one it was based on");
			}

			//	debug info
			#ifdef _DEBUG
			{
				TTempString Debug_String("Created new .asset file ");
				Debug_String.Append( pNewFile->GetFilename() );
				Debug_String.Append(" in file sys ");
				FileSys.GetFileSysRef().GetString( Debug_String );
				Debug_String.Append(" (");
				FileSys.GetFileSysTypeRef().GetString( Debug_String );
				Debug_String.Append(")");
				TLDebug_Print( Debug_String );
			}
			#endif
			break;
		}
	}

	//	failed to create file in any file sys
	if ( !pNewFile )
	{
		//	failed to create new file
		TLDebug_Break("Failed to put new .asset file in ANY file sys!");
		return "Failed";
	}

	//	save this new file as our asset file
	GetAssetFile() = pNewFile;

	//	export plain file
	return "PFExport";
}



//------------------------------------------------------------
//	convert plain file to asset file
//------------------------------------------------------------
TRef Mode_PlainFileExport::Update(float Timestep)
{
	Debug_PrintStep("Exporting plain file to asset file");
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
TRef Mode_GetAssetFile::Update(float Timestep)
{
	#ifdef _DEBUG
	{
		TTempString Debug_String("Looking for ");
		GetAssetRef().GetString( Debug_String );
		Debug_String.Append(" asset file...");
		TLDebug_Print( Debug_String );
	}
	#endif

	//	get latest file for this asset ref
	TPtr<TLFileSys::TFile>& pFile = TLFileSys::GetFile( GetAssetRef() );

	//	if the latest file is an asset file, then assign it, otherwise leave it null and we'll attempt to convert
	if ( pFile && pFile->GetTypeRef() == "Asset" )
	{
		GetAssetFile() = pFile;
	}
	else if ( pFile )
	{
		TTempString Debug_String("Found newest file for ");
		GetAssetRef().GetString( Debug_String );
		Debug_String.Append(", but newest is type ");
		pFile->GetTypeRef().GetString( Debug_String );
		TLDebug_Print( Debug_String );

		//	save fetching it again
		GetPlainFile() = pFile;

		//	load (if required) then convert plain file
		return "PFGet";
	}
	else
	{
		//	no file at all with a matching name
		return "Failed";
	}

	//	need to load file
	if ( GetAssetFile()->IsLoaded() != SyncTrue || GetAssetFile()->GetFlags()( TLFileSys::TFile::OutOfDate ) )
	{
		return "AFLoad";
	}

	//	do export/create
	//	if our new asset file needs to be written back to a normal file, do that
	if ( GetAssetFile()->GetNeedsExport() )
	{
		return "AFExport";
	}
	else
	{
		//	just export to asset
		return "Acreate";
	}
}



//------------------------------------------------------------
//	load asset file from file sys
//------------------------------------------------------------
TRef Mode_AssetFileLoad::Update(float Timestep)
{
	//	load the file
	TPtr<TLFileSys::TFileSys> pFileSys = GetAssetFile()->GetFileSys();
	TPtr<TLFileSys::TFile> pAssetFile = GetAssetFile();
	SyncBool LoadResult = pFileSys->LoadFile( pAssetFile );
	if ( LoadResult == SyncWait )
		return TRef();
	
	if ( LoadResult == SyncFalse )
		return "Failed";

	//	do export/create (same path as end of Mode_GetAssetFile)
	//	if our new asset file needs to be written back to a normal file, do that
	if ( GetAssetFile()->GetNeedsExport() )
	{
		return "AFExport";
	}
	else
	{
		//	just export to asset
		return "Acreate";
	}
}


//------------------------------------------------------------
//	turn asset file back to plain file
//------------------------------------------------------------
TRef Mode_AssetFileImport::Update(float Timestep)
{
	Debug_PrintStep("Importing asset file");

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
TRef Mode_AssetFileExport::Update(float Timestep)
{
	Debug_PrintStep("Exporting asset file to plain file");

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
TRef Mode_AssetFileWrite::Update(float Timestep)
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
TRef Mode_CreateAsset::Update(float Timestep)
{
	Debug_PrintStep("Creating asset");

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
TRef Mode_AssetImport::Update(float Timestep)
{
	Debug_PrintStep("Importing asset file to asset");

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

