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
	AddMode<Mode_PlainFileCreateTempAssetFile>("PFcAF");
	AddMode<Mode_PlainFileExport>("PFexport");
	
	AddMode<Mode_GetAssetFile>("AFGet");
	AddMode<Mode_AssetFileLoad>("AFLoad");
	AddMode<Mode_AssetFileImport>("AFImport");
	AddMode<Mode_AssetFileExport>("AFExport");
	AddMode<Mode_AssetFileCreate>("AFCreate");
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
			//	gr: quick check to make sure the asset's state is correct
			TPtr<TLAsset::TAsset>& pAsset = GetAsset();
			if ( pAsset )
			{
				//	make sure loading state is set correctly
				if ( pAsset->IsLoaded() )
				{
					//	notification of sucess
					TRef AssetType = (m_pAssetFile ? m_pAssetFile->GetAssetTypeRef() : (u32)0);
					TLAsset::g_pFactory->OnAssetLoad(GetAssetRef(), AssetType, TRUE);
					return SyncTrue;
				}
				else
				{
					TLDebug_Break("Asset not marked as loaded after successfull load - changing LoadTask to failure");
					SetMode("Failed");
				}
			}
			else
			{
				TLDebug_Break("Asset expected - changing LoadTask to failure");
				SetMode("Failed");
			}
		}

		//	if in failed mode... or no mode, failed
		if ( !CurrentModeRef.IsValid() || CurrentModeRef == "Failed" )
		{
			#ifdef _DEBUG
			TTempString Debug_String("Failed to load asset ");
			GetAssetRef().GetString( Debug_String );
			TLDebug_Warning( Debug_String );
			#endif

			//	notification of failure
			TRef AssetType = (m_pAssetFile ? m_pAssetFile->GetAssetTypeRef() : TRef_Invalid);
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
		{
			TLDebug_Break("failed to create TEMP asset");
			return "Failed";
		}
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
			TTempString Debug_String("failed to get plain file for asset ");
			GetAssetRef().GetString( Debug_String );
			TLDebug_Break( Debug_String );
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
	TPtr<TLFileSys::TFile>& pPlainFile = GetPlainFile();
	if ( !pPlainFile )
	{
		TLDebug_Break("Plain file expected");
		return "Failed";
	}

	//	load the plain file
	TPtr<TLFileSys::TFileSys> pFileSys = pPlainFile->GetFileSys();
	SyncBool LoadResult = pFileSys->LoadFile( pPlainFile );
	if ( LoadResult == SyncWait )
		return TRef();
	
	if ( LoadResult == SyncFalse )
	{
		TTempString Debug_String("Failed to load plain file ");
		Debug_String.Append( pPlainFile->GetFilename() );
		TLDebug_Break( Debug_String );
		return "Failed";
	}

	//	convert to AssetFile
	return "PFcAF";
}


//------------------------------------------------------------
//	create asset file from plain file
//------------------------------------------------------------
TRef Mode_PlainFileCreateTempAssetFile::Update(float Timestep)
{
	if ( GetAssetFile().IsValid() )
	{
		TLDebug_Break("Expected no asset file at this point...");
		return "Failed";
	}

	if ( GetTempAssetFile().IsValid() )
	{
		TLDebug_Break("Expected no temporary asset file at this point...");
		return "Failed";
	}

	//	create a temporary file with no file system (helps us identify that it's very temporary)
	GetTempAssetFile() = new TLFileSys::TFileAsset( GetAssetRef(), "Asset" );

	//	export plain file
	return "PFExport";
}



//------------------------------------------------------------
//	convert plain file to asset file
//------------------------------------------------------------
TRef Mode_PlainFileExport::Update(float Timestep)
{
	Debug_PrintStep("Exporting plain file to asset file");
	TLFileSys::TFile* pPlainFile = GetPlainFile();
	if ( !pPlainFile )
	{
		TLDebug_Break("Plain file expected");
		return "Failed";
	}

	TPtr<TLFileSys::TFileAsset> pAssetFile = GetTempAssetFile();
	if ( !pAssetFile )
	{
		TLDebug_Break("Asset file expected");
		return "Failed";
	}

	//	export plain file to asset file
	SyncBool ExportResult = pPlainFile->Export( pAssetFile );
	if ( ExportResult == SyncWait )
		return TRef();

	//	failed to export our file into an asset file
	if ( ExportResult == SyncFalse )
	{
		TTempString Debug_String("Failed to export plain file ");
		Debug_String.Append( pPlainFile->GetFilename() );
		Debug_String.Append(" to asset file."); 
		TLDebug_Break( Debug_String );

		TPtr<TLFileSys::TFileSys> pFileSys = pAssetFile->GetFileSys();
		if ( pFileSys )
		{
			Debug_String = "Deleting asset file";
			Debug_String.Append( pAssetFile->GetFilename() );
			if ( pFileSys->DeleteFile( pAssetFile->GetFileAndTypeRef() ) == SyncFalse )
				Debug_String.Append(" Failed!");
			TLDebug_Break( Debug_String );
		}

		return "Failed";
	}

	//	create temporary asset file, now create the real one and start writing to file system
	return "AFCreate";
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
		TTempString Debug_String("Failed to find any file with a file name/ref matching ");
		GetAssetRef().GetString( Debug_String );
		//TLDebug_Break( Debug_String );
		TLDebug_Print( Debug_String );
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
	TPtr<TLFileSys::TFileAsset>& pAssetFile = GetAssetFile();
	if ( !pAssetFile )
	{
		TLDebug_Break("Asset file expected");
		return "Failed";
	}

	//	load the file
	TPtr<TLFileSys::TFileSys> pFileSys = pAssetFile->GetFileSys();
	if ( !pFileSys )
	{
		TLDebug_Break("File is missing an owner file system. Files should ALWAYS have an owner file system");
		return "Failed";
	}

	//	load the asset file
	TPtr<TLFileSys::TFile> pFile = pAssetFile;
	SyncBool LoadResult = pFileSys->LoadFile( pFile );
	if ( LoadResult == SyncWait )
		return TRef();
	
	if ( LoadResult == SyncFalse )
	{
		TTempString Debug_String("File sys ");
		pFileSys->GetFileSysRef().GetString( Debug_String );
		Debug_String.Append(" failed to load file ");
		Debug_String.Append( pAssetFile->GetFilename() );
		TLDebug_Break( Debug_String );
		return "Failed";
	}

	//	do export/create (same path as end of Mode_GetAssetFile)
	//	if our new asset file needs to be written back to a normal file, do that
	if ( pAssetFile->GetNeedsExport() )
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
	TPtr<TLFileSys::TFileAsset>& pAssetFile = GetAssetFile();
	if ( !pAssetFile )
	{
		TLDebug_Break("Asset file expected");
		return "Failed";
	}

	//	attempt to write our new file back into our filesytem
	//	convert the asset file to a plain file first, then write that
	SyncBool ImportResult = pAssetFile->Import();
	if ( ImportResult == SyncWait )
		return TRef();

	if ( ImportResult == SyncFalse )
	{
		TTempString Debug_String("Failed to import asset file ");
		Debug_String.Append( pAssetFile->GetFilename() );
		TLDebug_Break( Debug_String );
		return "Failed";
	}

	//	imported, now create asset from this file
	return "ACreate";
}



//------------------------------------------------------------
//	save asset file back to file sys
//------------------------------------------------------------
TRef Mode_AssetFileCreate::Update(float Timestep)
{
	TPtr<TLFileSys::TFileAsset>& pTempAssetFile = GetTempAssetFile();
	if ( !pTempAssetFile )
	{
		TLDebug_Break("Temporary asset file expected");
		return "Failed";
	}

	//	create a real asset file
	TPtrArray<TLFileSys::TFileSys> FileSystems;
	//	first try and put asset file into the file system that the file came from...
	TLFileSys::GetFileSys( FileSystems, GetPlainFile()->GetFileSysRef(), TRef() );
	//	then any local file sys
	TLFileSys::GetFileSys( FileSystems, TRef(), "Local" );
	//	then virtual as a last resort
	TLFileSys::GetFileSys( FileSystems, "Virtual", "Virtual" );

	//	create real asset file
	GetAssetFile() = TLFileSys::CreateAssetFileInFileSys( GetAssetRef(), FileSystems );

	//	failed to create the real file, so just continue with our temporary asset file and create the asset
	if ( !GetAssetFile() )
	{
		TLDebug_Break("Should never get this case? should always at least write to the virtual file sys...");
		return "ACreate";
	}

	//	copy contents of temp asset file to the real asset file
	GetAssetFile()->CopyAssetFileData( *pTempAssetFile );

	//	delete temp asset file
	pTempAssetFile = NULL;

	//	write asset file back to file sys
	return "AFExport";
}


//------------------------------------------------------------
//	turn asset file back to plain file
//------------------------------------------------------------
TRef Mode_AssetFileExport::Update(float Timestep)
{
	Debug_PrintStep("Exporting asset file to plain file");
	TPtr<TLFileSys::TFileAsset>& pAssetFile = GetAssetFile();
	if ( !pAssetFile )
	{
		TLDebug_Break("Asset file expected");
		return "Failed";
	}

	//	attempt to write our new file back into our filesytem
	//	convert the asset file to a plain file first, then write that
	SyncBool ExportResult = pAssetFile->Export();
	if ( ExportResult == SyncWait )
		return TRef();

	if ( ExportResult == SyncFalse )
	{
		TTempString Debug_String("Failed to export asset file ");
		Debug_String.Append( pAssetFile->GetFilename() );
		TLDebug_Break( Debug_String );
		return "Failed";
	}

	//	now write the asset file to the file sys
	return "AFWrite";
}

//------------------------------------------------------------
//	save asset file back to file sys
//------------------------------------------------------------
TRef Mode_AssetFileWrite::Update(float Timestep)
{
	TPtr<TLFileSys::TFile> pAssetFile = GetAssetFile();
	if ( !pAssetFile )
	{
		TLDebug_Break("asset file expected");
		return "Failed";
	}

	//	write the contents back to our file sys
	TPtr<TLFileSys::TFileSys> pFileSys = TLFileSys::GetFileSys( pAssetFile->GetFileSysRef() );
	if ( !pFileSys )
	{
		TLDebug_Break("new asset file is in lost file system");
		return "Acreate";
	}

	//	write file
	SyncBool WriteResult = pFileSys->WriteFile( pAssetFile );
	if ( WriteResult == SyncWait )
		return TRef();

	//	failed or didnt, just continue
	return "Acreate";
}


//------------------------------------------------------------
//	create new asset
//------------------------------------------------------------
TRef Mode_CreateAsset::Update(float Timestep)
{
	Debug_PrintStep("Creating asset");

	TPtr<TLFileSys::TFileAsset>& pAssetFile = GetAssetFile();
	if ( !pAssetFile )
	{
		TLDebug_Break("Asset file expected");
		return TRef();
	}

	//	check in case this asset file needs importing
	if ( pAssetFile->GetNeedsImport() )
	{
		return "AFImport";
	}

	TRefRef NewAssetType = pAssetFile->GetAssetTypeRef();

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
		{
			TTempString Debug_String("Failed to create asset type ");
			NewAssetType.GetString( Debug_String );
			Debug_String.Append(" when creating asset ");
			GetAssetRef().GetString( Debug_String );
			TLDebug_Break( Debug_String );
			return "Failed";
		}
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
	if ( !pAsset )
	{
		TLDebug_Break("Asset missing for import");
		return "failed";
	}

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

