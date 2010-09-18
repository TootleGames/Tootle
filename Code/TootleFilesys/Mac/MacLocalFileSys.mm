#include "MacLocalFileSys.h"
#include <stdio.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSString.h>
#import <Foundation/Foundation.h>

#include <TootleCore/Mac/MacString.h>

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
	Bool Changed = FinaliseFileList();

	return Changed ? SyncTrue : SyncWait;
}


//---------------------------------------------------------------
//	load files with a filter, returns number of files found. -1 on error
//---------------------------------------------------------------
Bool Platform::LocalFileSys::LoadFileList(const char* pFileSearch)
{
	NSString *pDirString = TLString::ConvertToUnicharString(m_Directory);

#ifdef _DEBUG
	//	fail to make instances with no type (eg. executable name on ipod) 
	TTempString Debug_String("LoadFileList checking directory: ");

	const char* pNSChars = [pDirString UTF8String];
	u32 NsLength = [pDirString length];
	
	Debug_String.Append( pNSChars, NsLength );
	
	TLDebug_Print( Debug_String );
#endif
	

	// Enumerate the files (and direcotories) within the root directory.  This is a shallow search so no sub-directory
	// files will be listed.
	NSArray* pArray = [[NSFileManager defaultManager]contentsOfDirectoryAtPath:pDirString error:NULL];

	
	if(!pArray)
	{
		[pDirString release];
		
		// Error getting file list for the directory
		TLDebug_Print( "Invalid filelist pointer" );
		return FALSE;
	}
	
	
	for(u32 uIndex = 0; uIndex < [pArray count]; uIndex++)
	{
		NSString* pFilename = [pArray objectAtIndex:uIndex];
		
		const char* pRealFilename = (const char*)[pFilename fileSystemRepresentation];

		TLDebug_Print( pRealFilename );

		TTempString filepath = m_Directory;
		filepath.Append(pRealFilename);		
		
		NSString* fullfilepath = TLString::ConvertToUnicharString(filepath);

		NSDictionary* pFileAttribs = [[NSFileManager defaultManager] attributesOfItemAtPath:fullfilepath error:NULL];
		
		[fullfilepath release];
		
		NSString *FileType = [pFileAttribs objectForKey:@"NSFileType"];
				
		TTempString Filename = pRealFilename;

		//	is a directory
		if ( FileType == NSFileTypeDirectory )
		{
#ifdef _DEBUG			
			TTempString DebugString("Ignoring directory ");
			//pFile->GetFileRef().GetString( DebugString );
			DebugString.Append(pRealFilename);
			TLDebug_Print( DebugString );
#endif
			continue;
		}
		
		
		// is hidden file? On the Mac the first character is a '.' to represent a hidden file.
		if(Filename.GetLength() && Filename[0] == '.')
		{
#ifdef _DEBUG
			
			TTempString DebugString("Ignoring hiden file/directory ");
			//pFile->GetFileRef().GetString( DebugString );
			DebugString.Append(pRealFilename);
			TLDebug_Print( DebugString );
#endif				
			continue;
		}
		
		
		TFile* pFile = CreateFileInstance( Filename );
		
		if ( !pFile )
		{
			TDebugString Debug_String;
			Debug_String << "Failed to create file instance for %s" << Filename.GetData();
			TLDebug_Print( Debug_String );
			continue;
		}

		TDebugString Debug_String("Created new file instance ");
		Debug_String << pFile->GetFileRef() << ", type: " << pFile->GetFileAndTypeRef();
		TLDebug_Print( Debug_String );
	
		// Get the timestamp of the file
		NSDate *Timestamp = [pFileAttribs objectForKey:@"NSFileModificationDate"];
		
		// Get time since reference date in seconds (double)
		NSTimeInterval time = [Timestamp timeIntervalSinceReferenceDate];
		
		u32 EpochSeconds = (u32) time;
		TLTime::TTimestamp FileTimestamp;
		FileTimestamp.SetEpochSeconds( EpochSeconds );
		
		// Set the file timestamp
		pFile->SetTimestamp(FileTimestamp);
		
		// Clear the Lost flag to ensure the file is subsequently removed from the system if 
		// this was called from the LoadFileList where it will set this flag assuming it will be reset 
		// when found.  The CreateFileInstance above will simply return if the file already exists.
		pFile->GetFlags().Clear( TFile::Lost );
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

	//	if already loaded and not out of date, skip the load
	if ( pFile->IsLoaded() == SyncTrue && !pFile->IsOutOfDate() )
	{
		TDebugString Debug_String;
		Debug_String << "LoadFile() with file " << pFile->GetFilename() << " which is already loaded and not out of date. Shouldn't be any need to call this...";
		TLDebug_Break( Debug_String );
		return SyncTrue;
	}

	//	get full path and filename
	TString FullFilename = m_Directory;
	FullFilename.Append( pFile->GetFilename() );

	//	open
	NSString *pFileString = TLString::ConvertToUnicharString(FullFilename);
	
	FILE* pFileHandle = fopen( [pFileString UTF8String], "rb" );
	
	[pFileString release];

	//	failed to open
	if ( !pFileHandle )
	{
		//UpdateFileInstance( pFile, NULL );
		pFile->SetIsLoaded( SyncFalse );
		return SyncFalse;
	}
	else
	{
		pFile->SetOutOfDate( false );;
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
	pFile->SetOutOfDate( false );;

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
	NSString *pDirString = TLString::ConvertToUnicharString(m_Directory);

	// Check to see if the directory exist at the path specified
	BOOL isDir = NO;
	BOOL result = [[NSFileManager defaultManager] fileExistsAtPath:pDirString isDirectory:&isDir];

	//	release string
	[pDirString release];
	
	//	failed - file/dir doesn't exist
	if ( result == NO )
		return false;
	
	//	not a dir (but file exists)
	if ( isDir == NO )
		return false;
	
	return true;
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
	NSString *path = TLString::ConvertToUnicharString(Directory);

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
	
	[path release];
}

//---------------------------------------------------------
//	create a new empty file into file system if possible - if the filesys is read-only we cannot add external files and this fails
//---------------------------------------------------------
TPtr<TLFileSys::TFile> Platform::LocalFileSys::CreateNewFile(const TString& Filename)
{
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

	NSString *pFileString = TLString::ConvertToUnicharString(FullFilename);

	//	attempt to open file before creating instance
	
	FILE* pFileHandle = fopen( [pFileString UTF8String], "wb" );

	[pFileString release];
	
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

	NSString *pFileString = TLString::ConvertToUnicharString(FullFilename);

	//	open file for writing
	FILE* pFileHandle = fopen( [pFileString UTF8String], "wb" );
	
	[pFileString release];
	
	//	failed to open
	if ( !pFileHandle )
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
	pFile->SetOutOfDate( false );;

	//	refresh file info
	//UpdateFileInstance( pFile, FALSE );
	
	return SyncTrue;
}
