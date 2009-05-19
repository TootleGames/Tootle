#include "IPodLocalFileSys.h"
#include <stdio.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSString.h>
#import <Foundation/Foundation.h>


using namespace TLFileSys;


Platform::LocalFileSys::LocalFileSys(TRefRef FileSysRef,TRefRef FileSysTypeRef) :
	TFileSys			( FileSysRef, FileSysTypeRef ),
	m_IsWritable		( TRUE )
{
}


//---------------------------------------------------------------
//	check directory exists
//---------------------------------------------------------------
SyncBool Platform::LocalFileSys::Init()
{
	//	check directory exists
	if ( !IsDirectoryValid() )
		return SyncFalse;
	
	return SyncTrue;
}


//---------------------------------------------------------------
//
//---------------------------------------------------------------
SyncBool Platform::LocalFileSys::Shutdown()
{
	return SyncTrue;
}

//---------------------------------------------------------------
//	search for all files
//---------------------------------------------------------------
SyncBool Platform::LocalFileSys::LoadFileList()
{
	//	mark all the current files as missing, then any files we dont 
	//	re-find when scanning the dir we remove
	for ( u32 f=0;	f < GetFileList().GetSize();	f++ )
	{
		TPtr<TFile>& pFile = GetFileList().ElementAt( f );
		if ( pFile )
			pFile->GetFlags().Set( TFile::Lost );
	}
	
	//	now search for all the other file types
	if ( !Platform::LocalFileSys::DoLoadFileList() )
	{
		return SyncFalse;
	}
	
	//	finalise file list (updates timestamp and flushes missing files)
	FinaliseFileList();
	
	return SyncTrue;
}


//---------------------------------------------------------------
//	load files with a filter, returns number of files found. -1 on error
//---------------------------------------------------------------
Bool Platform::LocalFileSys::DoLoadFileList()
{
	NSString *pDirString = [[NSString alloc] initWithUTF8String:m_Directory.GetData()];
	 
	//	make a directory enumerator
	NSDirectoryEnumerator *direnum = [[NSFileManager defaultManager] enumeratorAtPath: pDirString];
	
	NSString *pFilename;
	
	while ( pFilename = [direnum nextObject] )
	{
		if ( [[pFilename pathExtension] isEqualToString:@"rtfd"] )
		{
			//	skip tree under "rtfd"
			[direnum skipDescendents];
		}
		else
		{
			//	found a file! - check it's not a dir
			NSDictionary *pFileAttribs = [direnum fileAttributes];
			NSString *FileType = [pFileAttribs objectForKey:@"NSFileType"];
	
			//	is a directory
			if ( FileType == NSFileTypeDirectory )
			{
#ifdef _DEBUG
				const char* pRealFilename = (const char*)[pFilename fileSystemRepresentation];
				//TTempString Filename = pRealFilename;
	
				TTempString DebugString("Ignoring directory ");
				//pFile->GetFileRef().GetString( DebugString );
				DebugString.Append(pRealFilename);
				TLDebug_Print( DebugString );
#endif
				continue;
			}
			
			const char* pRealFilename = (const char*)[pFilename fileSystemRepresentation];
			TTempString Filename = pRealFilename;
			
			TPtr<TFile> pFile = CreateFileInstance( Filename );
			
			if ( !pFile )
			{
				TLDebug_Print( TString("Failed to create file instance for %s", Filename.GetData() ) );
			}
			else
			{
				TTempString DebugString("Created new file instance ");
				pFile->GetFileRef().GetString( DebugString );
				DebugString.Append(", type: ");
				pFile->GetTypeRef().GetString( DebugString );
				TLDebug_Print( DebugString );
			}		
			
		}
	}
	
	[pDirString release];

	return TRUE;
}


//---------------------------------------------------------------
//	load a file
//---------------------------------------------------------------
SyncBool Platform::LocalFileSys::LoadFile(TPtr<TFile>& pFile)
{
	//	file expected
	if ( !pFile )
	{
		TLDebug_Break("File expected");
		return SyncFalse;
	}
	
	//	already loaded
	if ( pFile->IsLoaded() == SyncTrue )
		return SyncTrue;
	
	//	get full path and filename
	TString FullFilename = m_Directory;
	FullFilename.Append( pFile->GetFilename() );
	
	//	open
	FILE* pFileHandle = fopen( FullFilename.GetData(), "rb" );
	
	//	failed to open
	if ( !pFileHandle )
	{
		//UpdateFileInstance( pFile, NULL );
		pFile->SetIsLoaded( SyncFalse );
		return SyncFalse;
	}
	else
	{
		pFile->GetFlags().Clear( TFile::OutOfDate );
	}
	
	//	move cursor to end and get position
	fseek( pFileHandle, 0, SEEK_END );
	s32 FileSize = ftell( pFileHandle );
	
	//	file size is zero, treat as non-existant
	if ( FileSize == 0 )
	{
		fclose( pFileHandle );
		//UpdateFileInstance( pFile, NULL );
		pFile->SetIsLoaded( SyncFalse );
		return SyncFalse;
	}
	
	
	//	file data
	TBinary& Data = pFile->GetData();
	
	//	alloc data for file
	if ( !Data.SetSize( FileSize ) )
	{
		//	failed to alloc data - but file isn't neccesarily corrupt
		fclose( pFileHandle );
		pFile->SetIsLoaded( SyncWait );
		return SyncWait;
	}
	
	//	go back to the start of the file
	fseek( pFileHandle, 0, SEEK_SET );
	
	//	read data
	Bool FileTooBig = FALSE;
	u32 DataRead = 0;
	while ( DataRead < Data.GetSize() )
	{
		u64 ThisRead = fread( Data.GetData(DataRead), 1, Data.GetSize()-DataRead, pFileHandle );
		
		//	too much data!
		if ( ThisRead > (u64)0xffffffff )
		{
			TLDebug_Break("file has read more data than a u32, file is too big!");
			FileTooBig = TRUE;
			break;
		}
		
		if ( ThisRead <= 0 )
		{
			if ( TLDebug_Break("Read zero data from file") )
				break;
		}
		
		DataRead += (u32)ThisRead;
	}
	
	//	close file
	fclose( pFileHandle );
	pFileHandle = NULL;
	
	//	make sure file is marked as okay
	pFile->GetFlags().Clear( TFile::Lost );
	pFile->GetFlags().Clear( TFile::TooBig );
	pFile->GetFlags().Clear( TFile::OutOfDate );
	
	//	file was too big
	if ( FileTooBig )
	{
		pFile->GetFlags().Set( TFile::TooBig );
		pFile->SetIsLoaded( SyncFalse );	//	failed to load
		return SyncFalse;
	}
	
	//	is loaded now
	pFile->OnFileLoaded();
	
	//	reset read pos ready for first use
	//	gr: dont do this any more, we use the read pos to determine if we've attempted to read data post-import
	//pFile->ResetReadPos();
	
	return SyncTrue;
}


//---------------------------------------------------------------
//	returns FALSE if m_Directory isn't a directory
//---------------------------------------------------------------
Bool Platform::LocalFileSys::IsDirectoryValid()
{
	NSString *pDirString = [[NSString alloc] initWithUTF8String:m_Directory.GetData()];

	//	no such file/path
	if ( [[NSFileManager defaultManager] fileExistsAtPath:pDirString] == NO )
	{
		[pDirString release];
		return FALSE;
	}

	//	gr: todo, see if we can check if this directory is read-only... see the PC local file sys IsDirectoryValid()
	/*	
	BOOL isDirectory = NO;

	//	no such file/path
	if ( [[NSFileManager defaultManager] fileExistsAtPath:pDirString :isDirectory] == NO )
	{
		[pDirString release];
		return FALSE;
	}
	 
	[pDirString release];

	
	//	not a directory
	if ( isDirectory == NO )
		return FALSE;
*/		
	[pDirString release];
	return TRUE;
}


//---------------------------------------------------------------
//	set the directory
//---------------------------------------------------------------
void Platform::LocalFileSys::SetDirectory(const TString& Directory)	
{
	TTempString NewDirectory = Directory;
	
	if ( NewDirectory.GetLength() != 0 )
	{
		char BackSlash = '\\';
		
		//	if the directory doesnt end with a slash then go up a level (should cut off filename)
		char LastChar = NewDirectory.GetCharLast();
		if ( LastChar != BackSlash )
			TLFileSys::GetParentDir( NewDirectory );
	}

	//	gr: should really be using this; but with our own paths somehow
	/*
	NSString *documentsDirectory;
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	if ([paths count] > 0)  
	{
	    documentsDirectory = [paths objectAtIndex:0];
	}
	*/

	//	get the root directory all directories come from
	NSString *HomeDir = NSHomeDirectory();
	const char* pAppHomeDir = (const char*)[HomeDir UTF8String];
	TTempString RootDirectory = pAppHomeDir;
	RootDirectory.Append("/");

	RootDirectory.Append( NewDirectory );
	
	//	if directory name has changed reset the file list and invalidate the time stamp
	if ( m_Directory != RootDirectory )
	{
		m_Directory = RootDirectory;
		GetFileList().Empty(TRUE);
		m_LastFileListUpdate.SetInvalid();
	}
}

//---------------------------------------------------------
//	create a new empty file into file system if possible - if the filesys is read-only we cannot add external files and this fails
//---------------------------------------------------------
TPtr<TLFileSys::TFile> Platform::LocalFileSys::CreateFile(const TString& Filename,TRef TypeRef)
{
	//	not allowed to write to this file sys
	if ( !m_IsWritable )
		return NULL;
		
	TFileRef FileRef = GetFileRef( Filename, TypeRef );

	//	look for existing file
	TPtr<TFile>& pFile = GetFile( FileRef );
	if ( pFile )
	{
		if ( !pFile->GetFilename().IsEqual( Filename, FALSE ) )
		{
			TLDebug_Break("Called CreateFile(), new FileRef matches existing file, BUT filename is different. Conflict here. Aborting file creation");
			//return TLPtr::GetNullPtr<TLFileSys::TFile>();
			return NULL;
		}

		return pFile;
	}

	//	cant create a file if directory is invalid
	if ( !IsDirectoryValid() )
		return NULL;

	//	get full path and filename
	TTempString FullFilename = m_Directory;
	FullFilename.Append( Filename );

	//	attempt to open file before creating instance
	FILE* pFileHandle = fopen( FullFilename.GetData(), "wb" );

	//	failed to open
	if ( !pFileHandle )
	{
		return NULL;
	}

	//	close file
	fclose( pFileHandle );
	pFileHandle = NULL;

	
	//	create instance
	TPtr<TLFileSys::TFile> pNewFile = CreateFileInstance( Filename );
	if ( !pNewFile )
	{
		TLDebug_Break( TString("Failed to create file instance for %s", Filename.GetData() ) );
		return NULL;
	}
	else
	{
		TTempString DebugString("Created new file instance ");
		pNewFile->GetFileRef().GetString( DebugString );
		DebugString.Append(", type: ");
		pNewFile->GetTypeRef().GetString( DebugString );
		TLDebug_Print( DebugString );
	}	
	
	//	check type returned matches up
	if ( pNewFile->GetFileRefObject() != FileRef )
	{
		TLDebug_Break("Created new file, but file ref doesn't match to what we expected");
	}

	//	return new instance if it worked
	return pNewFile;
}




//---------------------------------------------------------
//	add this file into the file system if it's not there
//---------------------------------------------------------
SyncBool Platform::LocalFileSys::WriteFile(TPtr<TFile>& pFile)
{
	if ( !pFile )
	{
		TLDebug_Break("File expected");
		return SyncFalse;
	}
	
	//	not allowed to write to this file sys
	if ( !m_IsWritable )
		return SyncFalse;

	TTempString FullFilename = m_Directory;
	FullFilename.Append( pFile->GetFilename() );
	
	//	open file for writing
	FILE* pFileHandle = fopen( FullFilename.GetData(), "wb" );
	
	//	failed to open
	if ( !pFileHandle )
	{
		//UpdateFileInstance( pFile, NULL );
		pFile->SetIsLoaded( SyncFalse );
		return SyncFalse;
	}
	
	//	go back to the start of the file
	fseek( pFileHandle, 0, SEEK_SET );
	
	//	file data
	TBinary& Data = pFile->GetData();
	
	//	write data
	u32 DataWritten = 0;
	while ( DataWritten < Data.GetSize() )
	{
		u64 ThisWritten = fwrite( Data.GetData(DataWritten), 1, Data.GetSize()-DataWritten, pFileHandle );
		
		if ( ThisWritten <= 0 )
		{
			if ( TLDebug_Break("Wrote zero data to file") )
				break;
		}
		
		//	too much data!
		if ( ThisWritten > (u64)0xffffffff )
		{
			TLDebug_Break("written more data than a u32? somethings gone wrong");
			break;
		}
		
		DataWritten += (u32)ThisWritten;
	}
	
	//	close file
	fclose( pFileHandle );
	pFileHandle = NULL;
	
	//	file can't be out of date any more!
	pFile->GetFlags().Clear( TFile::OutOfDate );
	
	//	refresh file info
	//UpdateFileInstance( pFile, FALSE );
	
	return SyncTrue;
}
