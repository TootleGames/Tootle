#include <TootleCore/TLMaths.h>	//	gr: I don't know why but this needs to be included before "TFileFreetype.h" or "rand" isn't found
#include <TootleCore/TEventChannel.h>

#include "TLFileSys.h"
#include "TVirtualFileSys.h"
#include "TLocalFileSys.h"

#include "TFileFreetype.h"
#include "TFileAsset.h"
#include "TFileSimpleVector.h"
#include "TFileAssetMarkup.h"
#include "TFileXml.h"
#include "TFileWAV.h"
#include "TFileScheme.h"
#include "TFileCollada.h"
#include "TFileMenu.h"

#if defined(TL_TARGET_IPOD)
	#include "IPod/IPodLocalFileSys.h"
#endif

#if defined(_MSC_EXTENSIONS) && defined(TL_TARGET_PC)
	#include "PC/PCLocalFileSys.h"
#endif

namespace TLFileSys
{
	TPtr<TLFileSys::TFileSysFactory>	g_pFactory = NULL;
}






//----------------------------------------------------------
//	instance a file system
//----------------------------------------------------------
TLFileSys::TFileSys* TLFileSys::TFileSysFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	TRef VirtualRef("Virtual");
	TRef LocalRef("Local");

	if ( TypeRef == VirtualRef || InstanceRef == VirtualRef )
	{
		return new TLFileSys::TVirtualFileSys( InstanceRef, TypeRef );
	}

	if ( TypeRef == LocalRef || InstanceRef == LocalRef )
	{
		return new TLFileSys::TLocalFileSys( InstanceRef, TypeRef );
	}

	return NULL;
}


SyncBool TLFileSys::TFileSysFactory::Initialise()
{
	//	list of file systems to create (maybe take this from the app as a param)
	TArray<TRef> FileSystemRefs;
	FileSystemRefs.Add("Virtual");

	SyncBool Result = SyncTrue;

	//	create file systems
	for ( u32 i=0;	i<FileSystemRefs.GetSize();	i++ )
	{
		//	create/get file system
		TPtr<TFileSys> pFileSys = g_pFactory->GetInstance( FileSystemRefs[i], TRUE, FileSystemRefs[i] );
		if ( !pFileSys )
			return SyncFalse;

		//	init
		Result = pFileSys->Init();

		//	if failed, abort
		if ( Result == SyncFalse )
			return Result;

		//	success/wait just go to next one
	}
	
	return SyncTrue;
}

void TLFileSys::TFileSysFactory::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
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


SyncBool TLFileSys::TFileSysFactory::Update()
{
	UpdateObjects();

	return SyncTrue;
}

SyncBool TLFileSys::TFileSysFactory::Shutdown()
{
	return ShutdownObjects();
}


//----------------------------------------------------------
//	return a file system
//----------------------------------------------------------
TPtr<TLFileSys::TFileSys> TLFileSys::GetFileSys(TRefRef FileSysRef)
{
	//	missing factory
	if ( !g_pFactory )
	{
		TLDebug_Break("FileSysFactory expected");
		return NULL;
	}

	return g_pFactory->GetInstance( FileSysRef );
}


//----------------------------------------------------------
//	return all matching file systems to these refs/types
//----------------------------------------------------------
void TLFileSys::GetFileSys(TPtrArray<TLFileSys::TFileSys>& FileSysList,TRefRef FileSysRef,TRefRef FileSysTypeRef)
{
	//	missing factory
	if ( !g_pFactory )
	{
		TLDebug_Break("FileSysFactory expected");
		return;
	}

	//	if ref is valid, just get that one
	if ( FileSysRef.IsValid() )
	{
		FileSysList.Add( g_pFactory->GetInstance( FileSysRef ) );
		return;
	}

	//	if type is specified get all the file systems of that type
	if ( FileSysTypeRef.IsValid() )
	{
		for ( u32 f=0;	f<g_pFactory->GetSize();	f++ )
		{
			TPtr<TLFileSys::TFileSys>& pFileSys = g_pFactory->ElementAt(f);
			if ( !pFileSys )
				continue;

			if ( pFileSys->GetFileSysTypeRef() == FileSysTypeRef )
				FileSysList.Add( pFileSys );
		}
		return;
	}

	//	file sys ref and filesys type ref are invalid, so match any
	FileSysList.Add( g_pFactory->GetInstanceArray() );
}


//----------------------------------------------------------
//	constructor
//----------------------------------------------------------
TLFileSys::TFileSys::TFileSys(TRefRef FileSysRef,TRefRef FileSysTypeRef) :
	m_FileSysRef		( FileSysRef ),
	m_FileSysTypeRef	( FileSysTypeRef )
{
}


//----------------------------------------------------------
//	fetch file based on filename
//----------------------------------------------------------
TPtr<TLFileSys::TFile> TLFileSys::TFileSys::GetFile(const TString& Filename,Bool Load)
{
	TPtr<TLFileSys::TFile> pFile = GetInstance( Filename );

	//	no such file
	if ( !pFile )
		return pFile;

	//	invoke load if required
	if ( Load && pFile->IsLoaded() != SyncTrue )
	{
		SyncBool LoadState = LoadFile( pFile );

		//	loading has failed - still return the file?
		if ( LoadState == SyncFalse )
			return pFile;
	}
	
	return pFile;
}


//----------------------------------------------------------
//	return a file from the file list
//----------------------------------------------------------
TPtr<TLFileSys::TFile> TLFileSys::TFileSys::GetFile(TRefRef FileRef,Bool Load)
{
	//	find the file
	TPtr<TLFileSys::TFile> pFile = GetInstance(FileRef);

	//	no such file
	if ( !pFile )
		return pFile;

	//	invoke load if required
	if ( Load && pFile->IsLoaded() != SyncTrue )
	{
		SyncBool LoadState = LoadFile( pFile );

		//	loading has failed - still return the file?
		if ( LoadState == SyncFalse )
			return pFile;
	}
		
	return pFile;
}


//----------------------------------------------------------
//	overloaded from class factory
//----------------------------------------------------------
TLFileSys::TFile* TLFileSys::TFileSys::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	TLFileSys::TFile* pFile = NULL;

	//	tootle asset type
	if ( TypeRef == TRef("asset") )
	{
		pFile = new TLFileSys::TFileAsset( InstanceRef, TypeRef );
		return pFile;
	}

	//	freetype compatible font
	if ( TypeRef == TRef("ttf") )
	{
		pFile = new TLFileSys::TFileFreetype( InstanceRef, TypeRef );
		return pFile;
	}

	//	xml file
	if ( TypeRef == TRef("xml") )
	{
		pFile = new TLFileSys::TFileXml( InstanceRef, TypeRef );
		return pFile;
	}

	//	svg file
	if ( TypeRef == TRef("svg") )
	{
		pFile = new TLFileSys::TFileSimpleVector( InstanceRef, TypeRef );
		return pFile;
	}

	//	TAM file
	if ( TypeRef == TRef("tam") )
	{
		pFile = new TLFileSys::TFileAssetMarkup( InstanceRef, TypeRef );
		return pFile;
	}
	
	//	WAV file
	if ( TypeRef == TRef("wav") )
	{
		pFile = new TLFileSys::TFileWAV( InstanceRef, TypeRef );
		return pFile;
	}
	
	//	scheme file
	if ( TypeRef == TRef("scheme") )
	{
		pFile = new TLFileSys::TFileScheme( InstanceRef, TypeRef );
		return pFile;
	}
	
	//	collada xml file
	if ( TypeRef == TRef("dae") )
	{
		pFile = new TLFileSys::TFileCollada( InstanceRef, TypeRef );
		return pFile;
	}
	
	//	menu xml file
	if ( TypeRef == TRef("menu") )
	{
		pFile = new TLFileSys::TFileMenu( InstanceRef, TypeRef );
		return pFile;
	}
	

	//	generic binary file
	pFile = new TLFileSys::TFile( InstanceRef, TypeRef );
	return pFile;
}


//----------------------------------------------------------
//	create new file into the file list - returns existing file if it already exists
//----------------------------------------------------------
TPtr<TLFileSys::TFile> TLFileSys::TFileSys::CreateFileInstance(const TString& Filename,TRef TypeRef)
{
	//	see if this file already exists
	TPtr<TFile> pFile = GetFile( Filename, FALSE );

	//	already created just return
	if ( pFile )
		return pFile;

	//	generate a ref from the file name
	TRef FileRef;

	//	if we're extracting the extension for a typeref, then dont include that in the filename
	if ( !TypeRef.IsValid() )
	{
		GetFileRef( Filename, FileRef, TypeRef );
	}
	else
	{
		GetFileRef( Filename, FileRef );
	}

	//	as this is a new file, we want to make sure we're not overwriting the fileref of
	//	another file. 
	//	this means LongFileName1 and LongFileName2 can both exist and generate different refs
	if ( !pFile )
	{
		//	see if there's a file with this file ref
		TPtr<TFile> pDupeFile = GetFile( FileRef, FALSE );
		while ( pDupeFile )
		{
			FileRef.Increment();
			pDupeFile = GetFile( FileRef, FALSE );
		}
	}

	//	create new file
	CreateInstance( pFile, FileRef, TypeRef );

	//	failed to create
	if ( !pFile )
		return pFile;

	//	init
	if ( !pFile->Init( GetFileSysRef(), Filename ) )
	{
		RemoveInstance( FileRef );
		return NULL;
	}

	return pFile;
}


//----------------------------------------------------------
//	update file list if it's out of date.
//	returns FALSE if no changes, WAIT if possible changes,
//	TRUE if there were any changes
//----------------------------------------------------------
SyncBool TLFileSys::TFileSys::UpdateFileList()
{
	Bool ReloadFilelist = FALSE;

	//	if timestamp is valid compare with current time
	if ( m_LastFileListUpdate.IsValid() )
	{
		TLTime::TTimestamp TimeNow = TLTime::GetTimeNow();
		if ( m_LastFileListUpdate.GetSecondsDiff( TimeNow ) > (s32)GetFilelistTimeoutSecs() )
		{
			ReloadFilelist = TRUE;
		}
	}
	else
	{
		//	timestamp is invalid, get filelist
		ReloadFilelist = TRUE;
	}

	//	file list doesnt need reloading, return no changes
	if ( !ReloadFilelist )
		return SyncFalse;
	
	//	reload file list, if failed return no change
	SyncBool LoadResult = LoadFileList();

	if ( LoadResult == SyncFalse )
		return SyncFalse;

	return SyncWait;
}


//----------------------------------------------------------
//	update timestamp and flush missing files
//----------------------------------------------------------
void TLFileSys::TFileSys::FinaliseFileList()
{
	//	update time stamp of file list
	m_LastFileListUpdate.SetTimestampNow();

	//	flush missing/null files
	for ( s32 f=GetSize()-1;	f>=0;	f-- )
	{
		TPtr<TFile>& pFile = ElementAt(f);

		//	if flagged missing, null (will then be removed)
		if ( pFile->GetFlags()( TFile::Lost ) )
			pFile = NULL;

		//	file gone, remove
		if ( !pFile )
			RemoveAt( f );
	}
}


//----------------------------------------------------------
//	generate file ref and type ref from filename
//----------------------------------------------------------
void TLFileSys::TFileSys::GetFileRef(const TString& Filename,TRef& FileRef,TRef& TypeRef)
{	
	//	extract a file type from the extension of the filename if it's not been provided
	TArray<TString> FilenameParts;
	
	//	no .'s in the filename, so use invalid file type and generate filename in the normal way
	if ( !Filename.Split('.',FilenameParts) )
	{
		TypeRef.SetInvalid();
		GetFileRef( Filename, FileRef );
		return;
	}

	//	only one entry (probably named File.<nothing>) so treat as no split
	if ( FilenameParts.GetSize() <= 1 )
	{
		TypeRef.SetInvalid();
		GetFileRef( FilenameParts[0], FileRef );
		return;
	}

	//	turn last part (the file extension) into a ref
	TypeRef.Set( FilenameParts.ElementLastConst() );
	FilenameParts.RemoveLast();
	
	//	join the rest of the filename parts together and generate a file ref from that
	/*
	// [27 11 08] DB - Removed as this caused smaller than 5 letter filenames to have the svg as part fo their name
	for ( u32 i=1;	i<FilenameParts.GetSize();	i++ )
	{
		FilenameParts[0].Append( FilenameParts[i] );
	}
	 */

	//	now set file ref from our filename (without the extension)
	GetFileRef( FilenameParts[0], FileRef );
}


//----------------------------------------------------------
//	generate file ref from filename
//----------------------------------------------------------
void TLFileSys::TFileSys::GetFileRef(const TString& Filename,TRef& FileRef)
{
	s32 DotCharIndex = Filename.GetCharIndex('.');
	if ( DotCharIndex != -1 )
	{
		TTempString NewFilename;
		NewFilename.Append( Filename, DotCharIndex );
		FileRef.Set( NewFilename );
	}
	else
	{
		FileRef.Set( Filename );
	}
	
	//	a valid fileref hasn't been generated
	//	filename is blank maybe? 
	//	make a last-resort ref
	if ( !FileRef.IsValid() )
	{
		if ( !TLDebug_Break( TString("Failed to generate a file ref from the filename %s", Filename.GetData() ) ) )
			FileRef.Set(1);
	}
}


//----------------------------------------------------------
//	async create a local filesystem for the specified path. 
//	FileSysRef is set to the new file system's ref for the purposes of asynchronousness so keep using it when async calling this func
//----------------------------------------------------------
SyncBool TLFileSys::CreateLocalFileSys(TRef& FileSysRef,const TString& Directory)
{
	//	factory (and therefore lib) haven't been setup yet
	if ( !g_pFactory )
	{
		TLDebug_Break("CreateLocalFileSys before TLFileSys lib has been initialised");
		return SyncFalse;
	}

	//	create ref for this file system if we havent already got one (could have been specific)
	if ( !FileSysRef.IsValid() )
	{
		FileSysRef = "Local";

		//	increment the ref if we have one with the same name already
		TPtr<TFileSys> pDupeFileSys = g_pFactory->GetInstance( FileSysRef, FALSE );
		while ( pDupeFileSys )
		{
			FileSysRef.Increment();
			pDupeFileSys = g_pFactory->GetInstance( FileSysRef, FALSE );
		}
	}

	//	create/get this file system
	TPtr<TFileSys> pFileSys = g_pFactory->GetInstance( FileSysRef, TRUE, "Local" );

	//	failed to create
	if ( !pFileSys )
	{
		TLDebug_Print("Failed to create filesystem");		
		return SyncFalse;
	}

	//	set directory
	TLFileSys::TLocalFileSys* pLocalFileSys = pFileSys.GetObject<TLFileSys::TLocalFileSys>();
	if ( pLocalFileSys )
		pLocalFileSys->SetDirectory( Directory );
		
	//	init
	SyncBool Result = pFileSys->Init();
	
#ifdef _DEBUG
	if(Result == SyncFalse)
	{
		TLDebug_Print("Failed to initialise file system");		
	}
#endif

	return Result;
}


//----------------------------------------------------------
//	directory manipulation - turns filename to dir, then chops off a dir at a time
//----------------------------------------------------------
Bool TLFileSys::GetParentDir(TString& Directory)
{
	char BackSlash = '\\';
	char ForwardSlash = '\\';

	//	get the last (non terminator) char
	s32 LastCharIndex = Directory.GetCharGetLastIndex();
	if ( LastCharIndex <= 0 )
		return FALSE;

	s32 SearchFrom = LastCharIndex;

	//	ends with a slash, so get the slash BEFORE the last one to go up a directory
	char& LastChar = Directory.GetCharAt( LastCharIndex );
	if ( LastChar == BackSlash || LastChar == ForwardSlash )
	{
		SearchFrom--;
	}

	//	go back to previous slash
	s32 LastBackSlashIndex = Directory.GetLastCharIndex( BackSlash, SearchFrom );
	s32 LastForwardSlashIndex = Directory.GetLastCharIndex( ForwardSlash, SearchFrom );
	s32 LastSlashIndex = (LastBackSlashIndex > LastForwardSlashIndex) ? LastBackSlashIndex : LastForwardSlashIndex;
	
	if ( LastSlashIndex == -1 )
		return FALSE;

	//	set new length
	Directory.SetLength( LastSlashIndex+1 );	//	+1 to include the trailing slash

	return TRUE;
}


//----------------------------------------------------------
//	set pNewestFile to the newest of the two files. returns TRUE if pNewestFile is changed
//----------------------------------------------------------
Bool UpdateNewestFile(TPtr<TLFileSys::TFile>& pNewestFile,const TPtr<TLFileSys::TFile>& pTestFile)
{
	//	cannot be newer than current newest if it doesnt exist
	if ( !pTestFile )
		return FALSE;

	//	same file, no changes
	if ( pNewestFile == pTestFile )
		return FALSE;

	//	no current newest, this MUST be newer
	if ( !pNewestFile && pTestFile )
	{
		pNewestFile = pTestFile;
		return TRUE;
	}

	//	okay, both files exist. compare time stamps

	//	this file is newer, replace
	if ( pTestFile->GetTimestamp() < pNewestFile->GetTimestamp() )
	{
		//	is newer, replace
		pNewestFile = pTestFile;
		return TRUE;
	}

	return FALSE;
}


//----------------------------------------------------------
//	find the newest file [of a specific type] in the specified file systems
//----------------------------------------------------------
TPtr<TLFileSys::TFile> TLFileSys::FindFile(TPtrArray<TLFileSys::TFileSys>& FileSysList,TRefRef FileRef,TRefRef FileTypeRef)
{
	//	empty file list, fail immedietly
	if ( !FileSysList.GetSize() )
		return NULL;

	TTempString DebugString;
	DebugString.Append("FindFile ");
	FileRef.GetString( DebugString );
	DebugString.Append(" (");
	FileTypeRef.GetString( DebugString );
	DebugString.Append(")");
	TLDebug_Print(DebugString);

	//	loop through file systems to find matches - keep the newest one
	TPtr<TLFileSys::TFile> pNewestFile = NULL;

	//	check through matching file systems for the file
	for ( u32 f=0;	f<FileSysList.GetSize();	f++ )
	{
		TPtr<TLFileSys::TFileSys>& pFileSys = FileSysList[f];
		if ( !pFileSys )
			continue;

		//	grab the file
		TPtr<TLFileSys::TFile> pFile = pFileSys->GetFile( FileRef, TRUE );

		//	if the filetype is specified, make sure it's the right type
		if ( pFile && FileTypeRef.IsValid() )
		{
			//	wrong type, but the file exists, skip to next file sys
			if ( pFile->GetFileTypeRef() != FileTypeRef )
			{
				TTempString RefString;
				FileRef.GetString( RefString );
				TTempString TypeString;
				FileTypeRef.GetString( TypeString );

				TTempString FoundTypeString;
				pFile->GetFileTypeRef().GetString ( FoundTypeString );
				TLDebug_Print( TString("Found matching file for %s, but type is %s, we're looking for %s", RefString.GetData(), FoundTypeString.GetData(), TypeString.GetData() ) );
				continue;
			}
		}

		//	see if this file is newer...
		if ( UpdateNewestFile( pNewestFile, pFile ) )
			continue;

		//	it wasn't, or didn't exist, see if filesys needs updating and try again
		if ( pFileSys->UpdateFileList() == SyncFalse )
		{
			//	no changes to file sys
			continue;
		}

		//	try and grab file again
		pFile = pFileSys->GetFile( FileRef, TRUE );
		
		//	if the filetype is specified, make sure it's the right type
		if ( pFile && FileTypeRef.IsValid() )
		{
			//	wrong type, but the file exists, skip to next file sys
			if ( pFile->GetFileTypeRef() != FileTypeRef )
				continue;
		}

		//	see if this file is newer...
		if ( UpdateNewestFile( pNewestFile, pFile ) )
			continue;
	}

	//	return the newest matching file we could find
	return pNewestFile;
}

