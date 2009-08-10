#include "PCLocalFileSys.h"

#if defined(_MSC_EXTENSIONS)
#include <TootleCore/PC/PCTime.h>
#endif


using namespace TLFileSys;


Platform::LocalFileSys::LocalFileSys(TRefRef FileSysRef,TRefRef FileSysTypeRef) :
	TFileSys			( FileSysRef, FileSysTypeRef ),
	m_FileFindHandle	( INVALID_HANDLE_VALUE ),
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
	//	close file file handle
	if ( m_FileFindHandle != INVALID_HANDLE_VALUE )
	{
		FindClose( m_FileFindHandle );
		m_FileFindHandle = INVALID_HANDLE_VALUE;
	}

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
	FILE* pFileHandle = NULL;
	errno_t Result = fopen_s( &pFileHandle, FullFilename.GetData(), "rb" );

	//	failed to open
	if ( Result != 0 )
	{
		UpdateFileInstance( pFile, NULL );
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
		UpdateFileInstance( pFile, NULL );
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
	u32 DirAttribs = GetFileAttributes( m_Directory.GetData() );

	//	no such dir/file
	if ( DirAttribs == INVALID_FILE_ATTRIBUTES )
		return FALSE;

	//	check is a directory and not a file
	if ( (DirAttribs & FILE_ATTRIBUTE_DIRECTORY) == 0x0 )
		return FALSE;

	//	check if the directory is read-only, if it is, disable writing
	if ( (DirAttribs & FILE_ATTRIBUTE_READONLY) != 0x0 )
		SetIsWritable( FALSE );

	return TRUE;
}


//---------------------------------------------------------------
//	set the directory
//---------------------------------------------------------------
void Platform::LocalFileSys::SetDirectory(const TString& Directory)	
{
	//	prepend with the root directory all directories come from
	//	note: currently vunerable to .. attacks
	TString NewDirectory = TLCore::Platform::GetAppExe();
	TLFileSys::GetParentDir( NewDirectory );		
	NewDirectory.Append( Directory );

	if ( NewDirectory.GetLength() != 0 )
	{
		char BackSlash = '\\';
		char ForwardSlash = '/';

		//	if the directory doesnt end with a slash then go up a level (should cut off filename)
		char LastChar = NewDirectory.GetCharLast();
		if ( LastChar != BackSlash && LastChar != ForwardSlash )
			TLFileSys::GetParentDir( NewDirectory );
	}

	//	if directory name has changed reset the file list and invalidate the time stamp
	if ( m_Directory != NewDirectory )
	{
		m_Directory = NewDirectory;

		//	remove current files
		for ( s32 f=GetFileList().GetLastIndex();	f>=0;	f-- )
		{
			RemoveFileInstance( GetFileList().ElementAt(f) );
		}
		
		//	should be empty anyway
		if ( GetFileList().GetSize() > 0 )
		{
			TLDebug_Break("Expected instance array to be empty now");
			GetFileList().Empty(TRUE);
		}

		//	reset last-update timestamp
		m_LastFileListUpdate.SetInvalid();
	}
}


//---------------------------------------------------------------
//	create a new file from find data
//---------------------------------------------------------------
TPtr<TFile> Platform::LocalFileSys::CreateFileInstance(const WIN32_FIND_DATA& FileData,Bool LostIfEmpty)
{
	//	if a directory, dont create a file
	if ( (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0x0 )
		return NULL;

	//	pull out the file name to create/fetch existing file
	TString Filename = FileData.cFileName;

	//	create/get existing file
	TPtr<TFile> pFile = TFileSys::CreateFileInstance( Filename );

	//	error creating file
	if ( !pFile )
		return pFile;

	//	update info on file
	UpdateFileInstance( pFile, &FileData, LostIfEmpty );

	//	if this file was found to be lost straight away (either zero size, or some problem with it)
	//	then remove it straight away
	if ( pFile->GetFlags().IsSet( TFile::Lost ) )
	{
		if ( TFileSys::RemoveFileInstance( pFile ) )
		{
			//	file removed
			pFile = NULL;
			return NULL;
		}
	}	

	return pFile;
}


	
//---------------------------------------------------------
//	update file info by doing a win32 search for it and fetching its file details
//---------------------------------------------------------
void Platform::LocalFileSys::UpdateFileInstance(TPtr<TFile> pFile,Bool LostIfEmpty)
{
	if ( !pFile )
	{
		TLDebug_Break("File expected");
		return;
	}

	TTempString FullFilename = m_Directory;
	FullFilename.Append( pFile->GetFilename() );

	//	get file data by searching for it
	HANDLE FileFindHandle;
	WIN32_FIND_DATA FileFindData;
	FileFindHandle = FindFirstFile( FullFilename.GetData(), &FileFindData );

	//	failed to find our new file
	if ( FileFindHandle == INVALID_HANDLE_VALUE )
	{
		UpdateFileInstance( pFile, NULL );
		return;
	}

	//	close file-find 
	FindClose( FileFindHandle );
	FileFindHandle = INVALID_HANDLE_VALUE;

	//	update info
	UpdateFileInstance( pFile, &FileFindData, LostIfEmpty );
}


//---------------------------------------------------------
//	update file info by doing a win32 search for it and fetching its file details
//---------------------------------------------------------
void Platform::LocalFileSys::UpdateFileInstance(TPtr<TFile> pFile,const WIN32_FIND_DATA* pFileData,Bool LostIfEmpty)
{
	if ( !pFile )
	{
		TLDebug_Break("File expected");
		return;
	}
	
	//	lost file
	if ( !pFileData )
	{
		pFile->GetFlags().Set( TFile::Lost );
		return;
	}

	//	if file size is zero, treat as lost
	if ( LostIfEmpty )
	{
		if ( pFileData->nFileSizeHigh == 0 && pFileData->nFileSizeLow == 0 )
		{
			UpdateFileInstance( pFile, NULL, LostIfEmpty );
			return;
		}
	}

	//	clear missing flag in case it was set when we we're updating the file list... we know it exists now
	pFile->GetFlags().Clear( TFile::Lost );

	//	update timestamp
	TLTime::TTimestamp NewTimestamp;
	TLTime::Platform::GetTimestamp( NewTimestamp, pFileData->ftLastWriteTime );
	pFile->SetTimestamp( NewTimestamp );

	//	update file size
	//	file is too big for our engine :( if the high part is set (high part of u64) 
	//	then its size is bigger than u32 and really, we dont need 4gb assets. maybe in the distant future when we're loading dvds into memory... or not
	if ( pFileData->nFileSizeHigh > 0 )
	{
		pFile->SetFileSize( -1, TRUE );
	}
	else
	{
		pFile->SetFileSize( pFileData->nFileSizeLow );
	}

}


//---------------------------------------------------------
//	create a new empty file into file system if possible - if the filesys is read-only we cannot add external files and this fails
//---------------------------------------------------------
TPtr<TLFileSys::TFile> Platform::LocalFileSys::CreateFile(const TString& Filename)
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
	FILE* pFileHandle = NULL;
	errno_t Result = fopen_s( &pFileHandle, FullFilename.GetData(), "wb" );

	//	failed to open
	if ( Result != 0 )
	{
		TLDebug_Break("gr: update file instance null should set loaded to false anyway? follow this code");
		UpdateFileInstance( pFile, NULL );
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
	UpdateFileInstance( pFile, FALSE );

	return SyncTrue;
}
