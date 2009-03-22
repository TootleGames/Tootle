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
#include "TFileAssetScript.h"
#include "TFileTextDatabase.h"
#include "TFilePng.h"
#include "TFileFnt.h"

#if defined(TL_TARGET_IPOD)
	#include "IPod/IPodLocalFileSys.h"
#endif

#if defined(TL_TARGET_MAC)
	#include "Mac/MacLocalFileSys.h"
#endif

#if defined(_MSC_EXTENSIONS) && defined(TL_TARGET_PC)
	#include "PC/PCLocalFileSys.h"
#endif

namespace TLFileSys
{
	TPtr<TLFileSys::TFileSysFactory>	g_pFactory;		//	factory for filesystems
	TPtr<TLFileSys::TFileFactory>		g_pFileFactory;	//	factory for all the files we have, seperated from individual file systems now
}






//----------------------------------------------------------
//	async create a local filesystem for the specified path. 
//	FileSysRef is set to the new file system's ref for the purposes of asynchronousness so keep using it when async calling this func
//----------------------------------------------------------
SyncBool TLFileSys::CreateLocalFileSys(TRef& FileSysRef,const TString& Directory,Bool IsWritable)
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

	//	get existing file sys
	TPtr<TFileSys> pFileSys = g_pFactory->GetInstance( FileSysRef );
	Bool NewFileSys = !pFileSys.IsValid();
	
	if ( NewFileSys )
	{
		//	create new file system
		pFileSys = g_pFactory->GetInstance( FileSysRef, TRUE, "Local" );

		//	failed to create
		if ( !pFileSys )
		{
			TLDebug_Print("Failed to create filesystem");		
			return SyncFalse;
		}

		//	set directory
		TLFileSys::TLocalFileSys* pLocalFileSys = pFileSys.GetObject<TLFileSys::TLocalFileSys>();
		if ( pLocalFileSys )
		{
			pLocalFileSys->SetDirectory( Directory );
			pLocalFileSys->SetIsWritable( IsWritable );
		}
	}
		
	//	[continue] init
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
//	generate file ref and type ref from filename
//----------------------------------------------------------
TLFileSys::TFileRef TLFileSys::GetFileRef(const TString& Filename,TRef TypeRef)
{	
	//	extract a file type from the extension of the filename if it's not been provided
	TArray<TString> FilenameParts;
	TRef FileRef;
	
	//	no .'s in the filename, so use invalid file type and generate filename in the normal way
	if ( !Filename.Split('.',FilenameParts) )
	{
		FileRef.Set( Filename );
		//	gr: dont invalidate, just leave it as provided
		//TypeRef.SetInvalid();	
	}
	else 
	{
		//	need a type ref...
		if ( !TypeRef.IsValid() && FilenameParts.GetSize() > 1 )
		{
			TypeRef.Set( FilenameParts.ElementLastConst() );
			FilenameParts.RemoveLast();
		}

		//	get filename from first part only - world.scheme.asset becomes just "world"
		FileRef.Set( FilenameParts[0] );
	}

	return TFileRef( FileRef, TypeRef );
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
TPtr<TLFileSys::TFile>& TLFileSys::GetFile(TRefRef FileRef,TRefRef FileTypeRef)
{
	//	no factory
	if ( !TLFileSys::g_pFileFactory )
	{
		TLDebug_Break("FileSys factory expected - maybe looking for a file too soon :)");
		return TLPtr::GetNullPtr<TLFileSys::TFile>();
	}

	//	get file group for this file
	TPtr<TFileGroup>& pFileGroup = TLFileSys::g_pFileFactory->GetFileGroup( FileRef );
	if ( pFileGroup )
	{
		//	find file
		TPtr<TLFileSys::TFile>& pFile = pFileGroup->GetNewestFile( FileTypeRef );
		if ( pFile )
			return pFile;
	}

	//	didnt find, update the file lists
	Bool FileListsChanged = TLFileSys::g_pFactory->UpdateFileLists();

	//	nothing changed with file lists, obviously no file!
	if ( !FileListsChanged )
		return TLPtr::GetNullPtr<TLFileSys::TFile>();

	//	gr: maybe some kinda counter limit to stop recursive calls...
	return GetFile( FileRef, FileTypeRef );
}


//----------------------------------------------------------
//	create file factory in file sys constructor
//----------------------------------------------------------
TLFileSys::TFileSysFactory::TFileSysFactory(TRefRef ManagerRef) :
	TManager	( ManagerRef )
{
	if ( TLFileSys::g_pFileFactory )
	{
		TLDebug_Break("File factory should be null here");
	}
	else
	{
		TLFileSys::g_pFileFactory = new TLFileSys::TFileFactory();
	}
}

		
//----------------------------------------------------------
//	clean up
//----------------------------------------------------------
SyncBool TLFileSys::TFileSysFactory::Shutdown()
{
	if ( TLFileSys::g_pFileFactory )
	{
		//if ( TLFileSys::g_pFileFactory->Shutdown() == SyncWait )
		//	return SyncWait;
		
		TLFileSys::g_pFileFactory = NULL;
	}

	return SyncTrue;
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





//----------------------------------------------------------
//	update file lists of the file systems, return TRUE if any changed
//----------------------------------------------------------
Bool TLFileSys::TFileSysFactory::UpdateFileLists()
{
	Bool AnyChanged = FALSE;

	//	update all file systems
	for ( u32 f=0;	f<GetSize();	f++ )
	{
		TPtr<TFileSys>& pFileSys = ElementAt(f);
		
		if ( pFileSys->UpdateFileList() == SyncTrue)
			AnyChanged = TRUE;
	}

	return AnyChanged;
}


//----------------------------------------------------------
//	return a file system
//----------------------------------------------------------
TPtr<TLFileSys::TFileSys>& TLFileSys::GetFileSys(TRefRef FileSysRef)
{
	//	missing factory
	if ( !g_pFactory )
	{
		TLDebug_Break("FileSysFactory expected");
		return TLPtr::GetNullPtr<TLFileSys::TFileSys>();
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
		FileSysList.AddUnique( g_pFactory->GetInstance( FileSysRef ) );
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
				FileSysList.AddUnique( pFileSys );
		}
		return;
	}

	//	file sys ref and filesys type ref are invalid, so match any
	//FileSysList.Add( g_pFactory->GetInstanceArray() );
	for ( u32 f=0;	f<g_pFactory->GetInstanceArray().GetSize();	f++ )
	{
		FileSysList.AddUnique( g_pFactory->GetInstanceArray().ElementAt(f) );
	}
}





//------------------------------------------------------------
//
//------------------------------------------------------------
TLFileSys::TFileGroup::TFileGroup(TRefRef FileRef) :
	m_FileRef	( FileRef )
{
}


//------------------------------------------------------------
//	add file to group
//------------------------------------------------------------
void TLFileSys::TFileGroup::Add(TPtr<TLFileSys::TFile>& pFile)
{
	//	wrong file ref!
	if ( pFile->GetFileRef() != m_FileRef )
	{
		TLDebug_Break("Adding file of wrong file ref to group");
		return;
	}

	//	add to list
	m_Files.AddUnique( pFile );
}


//------------------------------------------------------------
//	remove file from group
//------------------------------------------------------------
void TLFileSys::TFileGroup::Remove(TPtr<TLFileSys::TFile>& pFile)
{
	m_Files.Remove( pFile );
}


//------------------------------------------------------------
//	get file with newest timestamp
//------------------------------------------------------------
TPtr<TLFileSys::TFile>& TLFileSys::TFileGroup::GetNewestFile(TRefRef FileType)
{
	s32 NewestIndex = -1;
	TLTime::TTimestamp NewestTimestamp;

	//	search through the files to find the one with the newest timestamp
	for ( u32 i=0;	i<m_Files.GetSize();	i++ )
	{
		TFile& File = *m_Files[i];

		//	looking for specific file type
		if ( FileType.IsValid() && File.GetTypeRef() != FileType )
			continue;

		//	is it newer? (or first hit)
		if ( NewestIndex == -1 || File.GetTimestamp() > NewestTimestamp )
		{
			NewestIndex = i;
			NewestTimestamp = File.GetTimestamp();
		}
	}

	//	failed to find any files
	if ( NewestIndex == -1 )
		return TLPtr::GetNullPtr<TFile>();

	return m_Files[NewestIndex];
}




//------------------------------------------------------------
//	create file
//------------------------------------------------------------
TLFileSys::TFile* TLFileSys::TFileFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
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
	
	//	tootle asset script xml file
	if ( TypeRef == TRef("tas") )
	{
		pFile = new TLFileSys::TFileAssetScript( InstanceRef, TypeRef );
		return pFile;
	}
	
	
	//	tootle text database xml file
	if ( TypeRef == TRef("ttd") )
	{
		pFile = new TLFileSys::TFileTextDatabase( InstanceRef, TypeRef );
		return pFile;
	}
	
	//	png texture
	if ( TypeRef == TRef("png") )
	{
		pFile = new TLFileSys::TFilePng( InstanceRef, TypeRef );
		return pFile;
	}
	
	//	font atlas
	if ( TypeRef == TRef("fnt") )
	{
		pFile = new TLFileSys::TFileFnt( InstanceRef, TypeRef );
		return pFile;
	}
	

	//	generic binary file
	pFile = new TLFileSys::TFile( InstanceRef, TypeRef );
	return pFile;
}


//------------------------------------------------------------
//	create instance, init, add to group
//------------------------------------------------------------
TPtr<TLFileSys::TFile>& TLFileSys::TFileFactory::CreateFileInstance(const TFileRef& FileRef,TRefRef FileSysRef,const TString& Filename)
{
	//	get a unique instance ref (based on filename)
	//	gr: currently NOT based on filename to stay away from confusion
	//TRef InstanceRef = TClassFactory::GetFreeInstanceRef( FileRef.GetFileRef() );
	TRef InstanceRef = GetFreeInstanceRef();

	TPtr<TLFileSys::TFile>& pNewFile = GetInstance( InstanceRef, TRUE, FileRef.GetTypeRef() );
	if ( !pNewFile )
		return pNewFile;

	//	init
	if ( !pNewFile->Init( FileRef.GetFileRef(), FileSysRef, Filename ) )
	{
		RemoveInstance( InstanceRef );
		return TLPtr::GetNullPtr<TLFileSys::TFile>();
	}

	//	add to group
	OnFileAdded( pNewFile );

	return pNewFile;
}


//------------------------------------------------------------
//	delete instance, remove from group
//------------------------------------------------------------
Bool TLFileSys::TFileFactory::RemoveFileInstance(TPtr<TLFileSys::TFile>& pFile)
{
	if ( !pFile )
	{
		TLDebug_Break("File expected");
		return FALSE;
	}

	//	remove from groups
	OnFileRemoved( pFile );

	//	remove instance using it's instance ref
	return RemoveInstance( pFile->GetInstanceRef() );
}


//----------------------------------------------------------
//	file was found in a file system, put it into it's group
//----------------------------------------------------------
void TLFileSys::TFileFactory::OnFileAdded(TPtr<TFile>& pFile)
{
	//	get existing group index
	s32 GroupIndex = m_FileGroups.FindIndex( pFile->GetFileRef() );

	//	create new group
	if ( GroupIndex == -1 )
	{
		TPtr<TFileGroup> pNewFileGroup = new TFileGroup( pFile->GetFileRef() );
		GroupIndex = m_FileGroups.Add( pNewFileGroup );
	}

	//	get group
	TPtr<TFileGroup>& pFileGroup = m_FileGroups[GroupIndex];
	pFileGroup->Add( pFile );
}

 
//----------------------------------------------------------
//	file removed from a file system, remove from it's group
//----------------------------------------------------------
void TLFileSys::TFileFactory::OnFileRemoved(TPtr<TFile>& pFile)
{
	//	get group for file
	s32 GroupIndex = m_FileGroups.FindIndex( pFile->GetFileRef() );
	if ( GroupIndex == -1 )
		return;

	TPtr<TFileGroup>& pFileGroup = m_FileGroups[GroupIndex];
	if ( !pFileGroup )
	{
		TLDebug_Break("File group shouldnt be null");
		return;
	}

	//	remove from group
	pFileGroup->Remove( pFile );

	//	if group is empty, remove the group
	if ( pFileGroup->IsEmpty() )
		m_FileGroups.RemoveAt( GroupIndex );
}





