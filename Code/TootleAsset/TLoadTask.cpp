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
	GetAssetAndTypeRef().GetString( Debug_String );
	Debug_String.Appendf(": %s", pStepString );
	TLDebug_Print( Debug_String );
}


//------------------------------------------------------------
//	
//------------------------------------------------------------
TLAsset::TLoadTask::TLoadTask(const TTypedRef& AssetAndTypeRef) :
	m_AssetAndTypeRef	( AssetAndTypeRef )
{
#ifdef _DEBUG
	TTempString Debug_String("Creating new load task for ");
	AssetAndTypeRef.GetString( Debug_String );
	TLDebug_Print( Debug_String );
#endif

	//	get asset file - if it exists, it loads it (AFLoad) or fetches plain file if we need to convert a plain file (PFGet)
	AddMode<Mode_GetAssetFile>("AFGet");
	AddMode<Mode_AssetFileLoad>("AFLoad");

	//	get plain file, load, export into asset file
	AddMode<Mode_PlainFileLoad>("PFLoad");
	AddMode<Mode_PlainFileCreateTempAssetFile>("PFcAF");
	AddMode<Mode_PlainFileExport>("PFexport");
	
	//	import temporary asset file from plain file, create final asset file (in Filesys) on success
	AddMode<Mode_AssetFileImport>("AFImport");
	AddMode<Mode_AssetFileCreate>("AFCreate");
	
	//	create & import asset from asset file
	AddMode<Mode_CreateAsset>("ACreate");
	AddMode<Mode_AssetImport>("AImport");
	
	//	import success, write asset file back to file system
	AddMode<Mode_AssetFileExport>("AFExport");
	AddMode<Mode_AssetFileWrite>("AFWrite");

	AddMode<Mode_Finished>("Finished");
	AddMode<Mode_Finished>("Failed");
}

//------------------------------------------------------------
//	depending on the state we can tell if it's loading, failed or loaded okay
//------------------------------------------------------------
SyncBool TLAsset::TLoadTask::GetLoadingState() const
{
	TRef CurrentModeRef = GetCurrentModeRef();
	switch ( CurrentModeRef.GetData() )
	{
		//	failed/no mode
		case TRef_InvalidValue:
		case TRef_Static(F,a,i,l,e):
			return SyncFalse;

		//	finished!
		case TRef_Static(F,i,n,i,s):
			return SyncTrue;

		//	some other mode, so still loading
		default:
			return SyncWait;
	}
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
					TLAsset::g_pManager->OnAssetLoad( GetAssetAndTypeRef(), TRUE);
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
			GetAssetAndTypeRef().GetString( Debug_String );
			TLDebug_Warning( Debug_String );
			#endif

			//	notification of failure
			//TRef AssetType = (m_pAssetFile ? m_pAssetFile->GetAssetTypeRef() : TRef_Invalid);
			TLAsset::g_pManager->OnAssetLoad( GetAssetAndTypeRef(), FALSE);

			return SyncFalse;
		}
	}
	while ( Blocking );

	//	otherwise still going
	return SyncWait;
}


//------------------------------------------------------------
//	load plain file from file sys
//------------------------------------------------------------
TRef Mode_PlainFileLoad::Update(float Timestep)
{
	Debug_PrintStep("Loading plain file");
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
	Debug_PrintStep("Creating temp asset file");
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
	GetTempAssetFile() = new TLFileSys::TFileAsset( GetAssetAndTypeRef().GetRef(), "Asset" );

	//	export plain file
	return "PFExport";
}



//------------------------------------------------------------
//	convert plain file to asset file
//------------------------------------------------------------
TRef Mode_PlainFileExport::Update(float Timestep)
{
	Debug_PrintStep("Exporting plain file to asset file");
	TPtr<TLFileSys::TFile> pPlainFile = GetPlainFile();
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
	SyncBool ExportResult = pPlainFile->Export( pAssetFile, GetAssetAndTypeRef().GetTypeRef() );
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

			//	gr: need to cast down to TFile ptr
			TPtr<TLFileSys::TFile> pAssetFileFile = pAssetFile;
			if ( pFileSys->DeleteFile( pAssetFileFile ) == SyncFalse )
				Debug_String.Append(" Failed!");

			TLDebug_Break( Debug_String );
		}

		//	add to list of files we failed to convert
		GetLoadTask()->AddFailedToConvertFile( pPlainFile );

		//	reset task vars
		GetAssetFile() = NULL;
		GetPlainFile() = NULL;
		GetTempAssetFile() = NULL;

		//	go back to finding the asset file
		return "AFGet";
	}

	//	created an asset file, check if it's creating the right kind of asset we want
	if ( pAssetFile->GetAssetTypeRef() != GetAssetAndTypeRef().GetTypeRef() )
	{
		TTempString Debug_String("Exported plain file ");
		pPlainFile->Debug_GetString( Debug_String );
		Debug_String << " but asset file's asset type is " << pAssetFile->GetAssetTypeRef() << ", looking for asset ref " << GetAssetAndTypeRef().GetTypeRef();
		TLDebug_Print( Debug_String );

		//	add to list of files we tried to convert but failed and try again
		GetLoadTask()->AddFailedToConvertFile( pPlainFile );
		GetLoadTask()->AddFailedToConvertFile( pAssetFile );

		//	reset task vars
		GetAssetFile() = NULL;
		GetPlainFile() = NULL;
		GetTempAssetFile() = NULL;

		//	go back to finding the asset file
		return "AFGet";
	}

	//	created temporary asset file, now create the real one and start writing to file system
	return "AFCreate";
}


//------------------------------------------------------------
//	fetch asset file
//------------------------------------------------------------
TRef Mode_GetAssetFile::Update(float Timestep)
{
	Debug_PrintStep("Finding asset file");

	//	get the file group with this ref (eg. tree.asset, tree.png, tree.mesh)
	TRefRef FileRef = GetAssetAndTypeRef().GetRef();
	TLFileSys::UpdateFileLists();
	TPtr<TLFileSys::TFileGroup>& pFileGroup = TLFileSys::GetFileGroup( FileRef );
	
	//	no files at all starting with the asset's name
	if ( !pFileGroup )
	{
		//	no file at all with a matching name
		TTempString Debug_String("Failed to find any file with a file name/ref matching ");
		GetAssetAndTypeRef().GetRef().GetString( Debug_String );
		TLDebug_Print( Debug_String );
		return "Failed";
	}

	//	get list of possible files..
	TPtrArray<TLFileSys::TFile> FileGroupFiles;
	FileGroupFiles.Copy( pFileGroup->GetFiles() );

	//	remove files which we know won't convert to the right asset type
	for ( s32 f=FileGroupFiles.GetLastIndex();	f>=0;	f-- )
	{
		TLFileSys::TFile& File = *FileGroupFiles[f];

		//	remove files we know will convert to wrong asset type
		if ( !File.IsSupportedExportAssetType( GetAssetAndTypeRef().GetTypeRef() ) )
		{
			FileGroupFiles.RemoveAt( f );
			continue;
		}
		else
		{
			//	make sure it's not in our already-tried list
			if ( GetLoadTask()->HasFailedToConvertFile( File ) )
			{
#ifdef _DEBUG
				TTempString Debug_String;
				Debug_String << "Failed to convert file " << GetAssetAndTypeRef().GetRef();
				TLDebug_Print( Debug_String );
#endif			
				FileGroupFiles.RemoveAt( f );
				continue;
			}
		}
	}

	//	no files to try and convert?
	if ( FileGroupFiles.GetSize() == 0 )
	{
		//	no file at all with a matching name
		TTempString Debug_String;
		Debug_String << "Failed to find any more files with a file name/ref matching " << GetAssetAndTypeRef() << " to try and load/convert";
		TLDebug_Print( Debug_String );
		return "Failed";
	}

	//	got a list of potential files now, get the last-modified one
	TPtr<TLFileSys::TFile>& pLatestFile = TLFileSys::GetLatestFile( FileGroupFiles );
	if ( !pLatestFile )
	{
		TLDebug_Break("File expected, TLFileSys::GetLatestFile failed to return a file from non-empty list");
		return "failed";
	}

	//	if the latest file is not an asset file, then convert
	if ( pLatestFile->GetTypeRef() != "Asset" )
	{
		TTempString Debug_String("Found newest file for ");
		GetAssetAndTypeRef().GetRef().GetString( Debug_String );
		Debug_String.Append(", but newest is type ");
		pLatestFile->GetTypeRef().GetString( Debug_String );
		Debug_String.Append(", converting...");
		TLDebug_Print( Debug_String );

		//	save fetching it again
		GetPlainFile() = pLatestFile;

		//	if the plain file needs loading/reloading then load it...
		if ( GetPlainFile()->IsLoaded() != SyncTrue || GetPlainFile()->GetFlags()( TLFileSys::TFile::OutOfDate ) )
		{
			return "PFLoad";
		}

		//	plain file is loaded and ready to convert, convert to asset file
		return "PFcAF";
	}

	//	latest file is an asset file, so assign and load/convert
	GetAssetFile() = pLatestFile;

	//	need to load/reload asset file
	if ( GetAssetFile()->IsLoaded() != SyncTrue || GetAssetFile()->GetFlags()( TLFileSys::TFile::OutOfDate ) )
	{
		return "AFLoad";
	}

	//	gr: asset file may need importing at this point, (even if the plain file is loaded)
	//	if the GetFileExportAssetType returns invalid, it doesn't know what type of asset it holds [yet]
	//	this can only be true if it's not yet loaded the header, so we need to load & import the asset file
	if ( GetAssetFile()->GetNeedsImport() )
	{
		return "AFImport";
	}

	//	check type... this asset file should be of our desired type... we shouldn't really get into this situation
	if ( GetAssetFile()->GetAssetTypeRef() != GetAssetAndTypeRef().GetTypeRef() )
	{
		//	gr: should probably abort here? (or get to a previous step to start again)
		TTempString Debug_String;
		Debug_String << "latest file asset type: " << GetAssetFile()->GetAssetTypeRef() << " doesn't convert to desired asset type " << GetAssetAndTypeRef().GetTypeRef();
		TLDebug_Break( Debug_String );
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
	Debug_PrintStep("Loading asset file");

	TPtr<TLFileSys::TFileAsset>& pAssetFile = GetAssetFile();
	if ( !pAssetFile )
	{
		TLDebug_Break("Asset file expected");
		return "Failed";
	}

	//	load the file as required
	if ( pAssetFile->IsLoaded() != SyncTrue )
	{
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
	}

	//	asset file needs converting from plain file to asset file before we can make an asset
	if ( pAssetFile->GetNeedsImport() )
	{
		return "AFImport";
	}
	else
	{
		//	ready to create an asset
		return "Acreate";
	}
}


//------------------------------------------------------------
//	import asset file data from [itself] plain file
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

	//	import file from plain to AssetFile data
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
	Debug_PrintStep("Creating final asset file");
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
	GetAssetFile() = TLFileSys::CreateAssetFileInFileSys( GetAssetAndTypeRef(), FileSystems );

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
	if ( GetAssetFile()->GetNeedsExport() )
		return "AFExport";
	else
		return "Finished";
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
	Debug_PrintStep("Writing new asset file to file system");
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
		return "Failed";
	}

	//	write file
	SyncBool WriteResult = pFileSys->WriteFile( pAssetFile );
	if ( WriteResult == SyncWait )
		return TRef();

	//	old method would now create asset, but we should have already done that so we can finish now
	if ( !GetAsset() )
	{
		return "Acreate";
	}
	else
	{
		return "Finished";
	}
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

	//	check the asset file is ready to be turned into an asset
	if ( pAssetFile->GetNeedsImport() )
	{
		TLDebug_Break("Asset file still needs import, should have already caught this");
		return "AFImport";
	}

	//	check the asset file contains the right type of asset
	//	gr: this may not have been checked yet as this stage could be right after the assetfile import
	TRefRef AssetFileAssetType = pAssetFile->GetAssetTypeRef();
	if ( AssetFileAssetType != GetAssetAndTypeRef().GetTypeRef() )
	{
		TTempString Debug_String("Importing asset ");
		GetAssetAndTypeRef().GetString( Debug_String );
		Debug_String.Append(" failed: AssetFile's asset type is ");
		AssetFileAssetType.GetString( Debug_String );
		TLDebug_Print( Debug_String );

		//	gr: note at this point, we're ditching the temp asset file which has a perfectly good asset in it's importer
		//		bit of a waste

		//	add to list of files we tried to convert but failed and try again
		GetLoadTask()->AddFailedToConvertFile( GetPlainFile() );
		GetLoadTask()->AddFailedToConvertFile( pAssetFile );

		//	reset task vars
		GetAssetFile() = NULL;
		GetPlainFile() = NULL;
		GetTempAssetFile() = NULL;

		//	go back to finding the asset file
		return "AFGet";
	}

	//	create asset if it doesn't exist
	TPtr<TLAsset::TAsset> pAsset = GetAsset();
	if ( !pAsset )
	{
		//	create new asset from the factory
		pAsset = TLAsset::CreateAsset( GetAssetAndTypeRef() );
		//	failed to create
		if ( !pAsset )
		{
			TTempString Debug_String("Failed to create asset instance ");
			GetAssetAndTypeRef().GetString( Debug_String );
			TLDebug_Break( Debug_String );
			return "Failed";
		}

		//	quick debug check
		pAsset = GetAsset();
		if ( !pAsset )
		{
			TTempString Debug_String("Asset unexpectedly NULL (CreateAsset return an asset okay): ");
			GetAssetAndTypeRef().GetString( Debug_String );
			TLDebug_Break( Debug_String );
			return "Failed";
		}
	}
	
	//	gr: mark asset as importing so we don't think it's failed...
	//	this is to fix the system on the ipod (dunno why it's just the ipod that struggles like this)
	//	but the pre-load system was checking for assets being loaded, and caught the asset at this state
	//	ready to be imported, correct type created, but loading state still in default (which is FALSE)
	//	maybe make default state wait? maybe that's confusing, may need to change to Init,Fail,Wait,True...
	//	gr: 31 July - does this need adding?
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
	TPtr<TLFileSys::TFileAsset>& pAssetFile = GetAssetFile();
	if ( !pAsset || !pAssetFile )
	{
		TLDebug_Break("Asset and Asset file missing for import");
		return "failed";
	}

	//	import asset from asset file
	pAsset->Import( GetAssetFile() );

	//	change mode depending on loading state
	TLAsset::TLoadingState LoadingState = pAsset->GetLoadingState();
	switch ( LoadingState )
	{
		case TLAsset::LoadingState_Loaded:
		{
			#ifdef _DEBUG
			{
				TTempString Debug_String("Imported asset okay ");
				pAsset->GetAssetRef().GetString( Debug_String );
				Debug_String.Append(" (");
				pAsset->GetAssetType().GetString( Debug_String );
				Debug_String.Appendf(") %x", pAsset.GetObjectPointer() );
				TLDebug_Print( Debug_String );
			}
			#endif

			//	finished and converted okay, if the asset file needs exporting back to a plain file (and writing back to file sys)
			//	do that before we finished
			if ( pAssetFile->GetNeedsExport() )
				return "AFExport";

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

