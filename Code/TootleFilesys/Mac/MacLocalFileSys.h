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

	virtual bool			IsWritable() const					{	return m_IsWritable;	}
	virtual SyncBool		Init();								//	check directory exists
	virtual SyncBool		LoadFileList();						//	search for all files
	virtual SyncBool		LoadFile(TPtr<TFile>& pFile);		//	load a file
	virtual TPtr<TFile>		CreateNewFile(const TString& Filename);
	virtual SyncBool		WriteFile(TPtr<TFile>& pFile);
	virtual SyncBool		Shutdown();

	void					SetDirectory(const TString& Directory);
	void					SetIsWritable(Bool IsWritable)		{	m_IsWritable = IsWritable;	}

protected:
	Bool					IsDirectoryValid();					//	returns FALSE if m_Directory isn't a directory

	Bool					LoadFileList(const char* pFileSearch);	//	load files with a filter, returns number of files found. -1 on error

protected:
	Bool					m_IsWritable;						//	overriding readonly setting
	TString					m_Directory;
};

