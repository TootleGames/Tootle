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
#include "TFileTimeline.h"
#include "TFileTextDatabase.h"
#include "TFilePng.h"
#include "TFileFnt.h"
#include "TFileParticle.h"

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


//--------------------------------------------------
//	get the group of files for a file ref
//--------------------------------------------------
TPtr<TLFileSys::TFileGroup>& TLFileSys::GetFileGroup(TRefRef FileRef)
{
	return TLFileSys::g_pFileFactory->GetFileGroup( FileRef );
}


//--------------------------------------------------
//	update the file lists on the file systems, returns TRUE if any file sys' has change
//--------------------------------------------------
Bool TLFileSys::UpdateFileLists()
{
	if ( !TLFileSys::g_pFactory )
		return FALSE;
	
	return TLFileSys::g_pFactory->UpdateFileLists();
}


//--------------------------------------------------
//	from a list of files, return the one with the most recent timestamp
//--------------------------------------------------
TPtr<TLFileSys::TFile>& TLFileSys::GetLatestFile(TPtrArray<TLFileSys::TFile>& Files,TRef FileType)
{
	s32 NewestIndex = -1;
	TLTime::TTimestamp NewestTimestamp;

	//	search through the files to find the one with the newest timestamp
	for ( u32 i=0;	i<Files.GetSize();	i++ )
	{
		TFile& File = *Files[i];

		//	looking for specific file type
		if ( FileType.IsValid() && File.GetTypeRef() != FileType )
			continue;

		//	ignore files that cannot be loaded
		if ( File.IsUnknownType() )
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

	return Files[NewestIndex];
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
//	get a list of all files in all the file systems (gets refs out of the groups)
//----------------------------------------------------------
void TLFileSys::GetFileList(TArray<TRef>& FileList)
{
	if ( !g_pFileFactory )
		return;

	//	gr: update all file system file listings?

	//	get the file groups
	const TPtrArray<TFileGroup>& FileGroups = g_pFileFactory->GetFileGroups();
	for ( u32 g=0;	g<FileGroups.GetSize();	g++ )
	{
		FileList.Add( FileGroups[g]->GetFileRef() );
	}

	//	just in case we don't find any files, maybe we need to add file list updates
	if ( FileList.GetSize() == 0 )
	{
		TLDebug_Print("No files found in all file systems... file systems need file list updates?");
	}
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
TTypedRef TLFileSys::GetFileAndTypeRef(const TString& Filename)
{	
	//	extract a file type from the extension of the filename if it's not been provided
	TArray<TString> FilenameParts;
	TRef FileRef,TypeRef;

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

	return TTypedRef( FileRef, TypeRef );
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
//	create file factory in file sys constructor
//----------------------------------------------------------
TLFileSys::TFileSysFactory::TFileSysFactory(TRefRef ManagerRef) :
	TLCore::TManager	( ManagerRef )
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


TLFileSys::TFileSysFactory::~TFileSysFactory()
{
	// Delete the file factory when the filesystem factory is deleted.
	TLFileSys::g_pFileFactory = NULL;

}




		
//----------------------------------------------------------
//	clean up
//----------------------------------------------------------
SyncBool TLFileSys::TFileSysFactory::Shutdown()
{
	// [18/08/09] The factory may still be used during the shutdown so leave it active and remove it during the destructor
	/*
	if ( TLFileSys::g_pFileFactory )
	{
		//if ( TLFileSys::g_pFileFactory->Shutdown() == SyncWait )
		//	return SyncWait;
		
		TLFileSys::g_pFileFactory = NULL;
	}
	 */

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
//	wrapper to create a file for a .asset file (to ensure consistent filenames)
//------------------------------------------------------------
TPtr<TLFileSys::TFile> TLFileSys::CreateAssetFileInFileSys(const TTypedRef& AssetAndTypeRef,TPtrArray<TLFileSys::TFileSys>& FileSysList)
{
	//	asset file name format is Name.Type.Asset
	TTempString Filename;
	AssetAndTypeRef.GetRef().GetString( Filename );
	Filename.Append('.');
	AssetAndTypeRef.GetTypeRef().GetString( Filename );
	Filename.Append('.');
	TRef( TRef_Static(A,s,s,e,t) ).GetString( Filename );

	//	create file sys TFile
	return CreateFileInFileSys( Filename, FileSysList );
}


//------------------------------------------------------------
//	try to create a file in one of the file systems provided. FileType dictates the TFile type (not extension or anything)
//	todo: omit this and use the extension of the filename?
//------------------------------------------------------------
TPtr<TLFileSys::TFile> TLFileSys::CreateFileInFileSys(const TString& Filename,TPtrArray<TLFileSys::TFileSys>& FileSysList)
{
	TPtr<TLFileSys::TFile> pNewFile;

	//	loop through file systems and try and create file
	for ( u32 i=0;	i<FileSysList.GetSize();	i++ )
	{
		TLFileSys::TFileSys& FileSys = *FileSysList[i];

		//	try and create file
		pNewFile = FileSys.CreateFile( Filename );

		//	created file, break out of loop
		if ( pNewFile )
		{
			//	debug info
			#ifdef _DEBUG
			{
				TTempString Debug_String("Created new file ");
				Debug_String.Append( pNewFile->GetFilename() );
				Debug_String.Append(" in file sys ");
				FileSys.GetFileSysRef().GetString( Debug_String );
				Debug_String.Append(" (");
				FileSys.GetFileSysTypeRef().GetString( Debug_String );
				Debug_String.Append(")");
				TLDebug_Print( Debug_String );
			}
			#endif
			return pNewFile;
		}
	}

	return NULL;
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
	return TLFileSys::GetLatestFile( m_Files, FileType );
}




//------------------------------------------------------------
//	create file
//------------------------------------------------------------
TLFileSys::TFile* TLFileSys::TFileFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	//	tootle asset type
	switch ( TypeRef.GetData() )
	{
	case TRef_Static(a,s,s,e,t):	return new TLFileSys::TFileAsset( InstanceRef, TypeRef );			//	tootle asset type
	case TRef_Static3(t,t,f):		return new TLFileSys::TFileFreetype( InstanceRef, TypeRef );		//	freetype compatible font
	case TRef_Static3(x,m,l):		return new TLFileSys::TFileXml( InstanceRef, TypeRef );				//	non-specific xml file
	case TRef_Static3(s,v,g):		return new TLFileSys::TFileSimpleVector( InstanceRef, TypeRef );	//	svg file
	case TRef_Static(m,a,r,k,u):
	case TRef_Static3(t,a,m):		return new TLFileSys::TFileAssetMarkup( InstanceRef, TypeRef );		//	TAM file
	case TRef_Static3(w,a,v):		return new TLFileSys::TFileWAV( InstanceRef, TypeRef );				//	Wave file
	case TRef_Static(s,c,h,e,m):	return new TLFileSys::TFileScheme( InstanceRef, TypeRef );			//	scheme file
	case TRef_Static3(d,a,e):		return new TLFileSys::TFileCollada( InstanceRef, TypeRef );			//	"dae" collada xml file
	case TRef_Static4(m,e,n,u):		return new TLFileSys::TFileMenu( InstanceRef, TypeRef );			//	"menu" menu xml file
	case TRef_Static(t,i,m,e,l):
	case TRef_Static3(t,t,l):		return new TLFileSys::TFileTimeline( InstanceRef, TypeRef );		//	"timeline"/"ttl" tootle timeline xml file
	case TRef_Static4(t,e,x,t):
	case TRef_Static3(t,t,d):		return new TLFileSys::TFileTextDatabase( InstanceRef, TypeRef );	//	"text"/"ttd" tootle text database xml file
	case TRef_Static3(p,n,g):		return new TLFileSys::TFilePng( InstanceRef, TypeRef );				//	png texture
	case TRef_Static3(f,n,t):		return new TLFileSys::TFileFnt( InstanceRef, TypeRef );				//	"fnt" font atlas
	case TRef_Static(P,a,r,t,i):	return new TLFileSys::TFileParticle( InstanceRef, TypeRef );		//	"Particle" markup
	
	default:
		//	generic binary file
		return new TLFileSys::TFile( InstanceRef, TypeRef );
	}
}


//------------------------------------------------------------
//	create instance, init, add to group
//------------------------------------------------------------
TPtr<TLFileSys::TFile>& TLFileSys::TFileFactory::CreateFileInstance(const TString& Filename,TRefRef FileSysRef)
{
	//	get the file type from the filename
	TTypedRef FileNameAndTypeRef = TLFileSys::GetFileAndTypeRef( Filename );

	//	fail to make instances with no type (eg. executable name on ipod) 
	if ( !FileNameAndTypeRef.IsValid() )
	{
#ifdef _DEBUG
		TTempString Debug_String("Ignoring file with no type; ");
		Debug_String.Append( Filename );
		TLDebug_Print( Debug_String );
#endif
		return TLPtr::GetNullPtr<TLFileSys::TFile>();
	}

	//	get a unique instance ref (based on filename)
	//	gr: currently NOT based on filename to stay away from confusion
	//TRef InstanceRef = TClassFactory::GetFreeInstanceRef( FileRef.GetFileRef() );
	TRef InstanceRef = GetFreeInstanceRef();

	TPtr<TLFileSys::TFile>& pNewFile = GetInstance( InstanceRef, TRUE, FileNameAndTypeRef.GetTypeRef() );
	if ( !pNewFile )
		return pNewFile;

	//	init
	if ( !pNewFile->Init( FileNameAndTypeRef.GetRef(), FileSysRef, Filename ) )
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

	//	remove instance
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





