#include "MacLocalFileSys.h"
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
	//	directory not set - use root dir
	if ( !m_Directory.GetLength() )
	{
		SetDirectory("");
	}

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
	for ( u32 f=0;	f<GetFileList().GetSize();	f++ )
	{
		TPtr<TFile>& pFile = GetFileList().ElementAt( f );
		if ( pFile )
			pFile->GetFlags().Set( TFile::Lost );
	}

	//	search for all the file types
	if ( !Platform::LocalFileSys::LoadFileList("*.*") )
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
Bool Platform::LocalFileSys::LoadFileList(const char* pFileSearch)
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
				continue;
			
			
			const char* pRealFilename = (const char*)[pFilename fileSystemRepresentation];
			TTempString Filename = pRealFilename;
			
			// is hidden file? On the Mac the first character is a '.' to represent a hidden file.
			if(Filename.GetLength() && Filename[0] == '.')
				continue;

			
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
				pFile->GetFileAndTypeRef().GetString( DebugString );
				TLDebug_Print( DebugString );
			}		
			
		}
	}
	
	[pDirString release];

			
	/*
	//	no file list handle, start
	if ( m_FileFindHandle == INVALID_HANDLE_VALUE )
	{
		//	make a wildcard search from the directory name
		TTempString FileSearch = m_Directory;
		FileSearch.Append(pFileSearch);

		//	init the search
		m_FileFindHandle = FindFirstFile( FileSearch.GetData(), &m_FileFindData );

		//	failed
		if ( m_FileFindHandle == INVALID_HANDLE_VALUE )
		{
			//	just because dir is empty? that's okay. 
			if ( GetLastError() == ERROR_FILE_NOT_FOUND )
			{
				return TRUE;
			}

			//	failed for other reason
			return FALSE;
		}

		//	pull out info of first file
		CreateFileInstance( m_FileFindData, TRUE );
	}


	//	goto next file
	while ( FindNextFile( m_FileFindHandle, &m_FileFindData ) )
	{
		CreateFileInstance( m_FileFindData, TRUE );
	}

	//	find next file has failed... assume out of files?
	//if ( GetLastError() == ERROR_NO_MORE_FILES )

	//	close find handle
	FindClose( m_FileFindHandle );
	m_FileFindHandle = INVALID_HANDLE_VALUE;
	*/
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
	if ( pFileHandle == NULL )
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

	BOOL isDir = NO;
	
	// Check to see if the directory exist at the path specified
	BOOL result = [[NSFileManager defaultManager] fileExistsAtPath:pDirString isDirectory:&isDir];

	
#ifdef _DEBUG
	
	if(result == NO)
		TLDebug_Break("Directory is invalid");
#endif
	
	[pDirString release];
	
	return (result == YES && isDir == YES);
}


//---------------------------------------------------------------
//	set the directory
//---------------------------------------------------------------
void Platform::LocalFileSys::SetDirectory(const TString& Directory)	
{
	//	get the root directory all directories come from
	NSString *HomeDir = NSHomeDirectory();
	

	TTempString RootDirectory;
	
	// Check to see if the 'home' dir exists already in the directory being passed in
	// if so use it as-is otherwise we need to setup the home dir and then append the new directory path to it
	NSString* path = [[NSString alloc] initWithUTF8String:Directory.GetData()];
	
	NSRange range = [path rangeOfString:HomeDir];
	
	if(range.location == NSNotFound)
	{
		const char* pAppHomeDir = (const char*)[HomeDir UTF8String];
		RootDirectory = pAppHomeDir;
		RootDirectory.Append("/");
		RootDirectory.Append( Directory );
	}	
	else 
		RootDirectory = Directory;

	
	
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
TPtr<TLFileSys::TFile> Platform::LocalFileSys::CreateNewFile(const TString& Filename)
{
 return NULL;

/*
	//	not allowed to write to this file sys
	if ( !m_IsWritable )
		return NULL;

	//	look for existing file
	TPtr<TFile>& pFile = GetFile( Filename );
	if ( pFile )
		return pFile;

	//	cant create a file if directory is invalid
	if ( !IsDirectoryValid() )
		return NULL;

	//	get full path and filename
	TTempString FullFilename = m_Directory;
	FullFilename.Append( Filename );

	//	attempt to open file before creating instance
	FILE* pFileHandle = NULL;
	errno_t Result = fopen_s( &pFileHandle, FullFilename.GetData(), "wb" );

	//	failed to open
	if ( Result != 0 )
	{
		return NULL;
	}

	//	close file
	fclose( pFileHandle );
	pFileHandle = NULL;

	//	get file data by searching for it
	HANDLE FileFindHandle;
	WIN32_FIND_DATA FileFindData;
	FileFindHandle = FindFirstFile( FullFilename.GetData(), &FileFindData );

	//	failed to find our new file
	if ( FileFindHandle == INVALID_HANDLE_VALUE )
		return NULL;

	FindClose( FileFindHandle );
	FileFindHandle = INVALID_HANDLE_VALUE;

	//	create instance
	TPtr<TLFileSys::TFile> pNewFile = CreateFileInstance( FileFindData, FALSE );
	if ( !pNewFile )
	{
		TLDebug_Break("failed to find newly created file?");
		return NULL;
	}

	//	return new instance if it worked
	return pNewFile;
 */
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
	if ( pFileHandle == NULL )
	{
		TLDebug_Break("gr: update file instance null should set loaded to false anyway? follow this code");
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



