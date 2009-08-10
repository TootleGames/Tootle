/*------------------------------------------------------
	
	Local file system... reads files out of a specific directory

-------------------------------------------------------*/
#pragma once
#include "../TLFileSys.h"


namespace TLFileSys
{
	namespace Platform
	{
		class LocalFileSys;	
	}
};



//------------------------------------------------------------
//	
//------------------------------------------------------------
class TLFileSys::Platform::LocalFileSys : public TLFileSys::TFileSys
{
public:
	LocalFileSys(TRefRef FileSysRef,TRefRef FileSysTypeRef);

	virtual SyncBool		Init();								//	check directory exists
	virtual SyncBool		LoadFileList();						//	search for all files
	virtual SyncBool		LoadFile(TPtr<TFile>& pFile);		//	load a file
	virtual TPtr<TFile>		CreateFile(const TString& Filename);
	virtual SyncBool		WriteFile(TPtr<TFile>& pFile);
	virtual SyncBool		Shutdown();

	void					SetDirectory(const TString& Directory);
	void					SetIsWritable(Bool IsWritable)		{	m_IsWritable = IsWritable;	}

protected:
	Bool					IsDirectoryValid();					//	returns FALSE if m_Directory isn't a directory
	TPtr<TFile>				CreateFileInstance(const WIN32_FIND_DATA& FileData,Bool LostIfEmpty);						//	create a new file from find data
	void					UpdateFileInstance(TPtr<TFile> pFile,Bool LostIfEmpty);										//	update file info by doing a win32 search for it and fetching its file details
	void					UpdateFileInstance(TPtr<TFile> pFile,const WIN32_FIND_DATA* pFileData,Bool LostIfEmpty);	//	update file info by doing a win32 search for it and fetching its file details

	Bool					LoadFileList(const char* pFileSearch);	//	load files with a filter, returns number of files found. -1 on error

protected:
	Bool					m_IsWritable;						//	overriding readonly setting
	TString					m_Directory;						//	path to dir
	HANDLE					m_FileFindHandle;					//	current handle for the file list
	WIN32_FIND_DATA			m_FileFindData;						//	current file find data
};

