/*------------------------------------------------------
	
	Base filesystem type & factory

-------------------------------------------------------*/
#pragma once
#include <TootleCore/TLTypes.h>
#include <TootleCore/TClassFactory.h>
#include <TootleCore/TLTime.h>
#include <TootleCore/TManager.h>
#include "TFile.h"


namespace TLCore
{
	class TApplication;
}

namespace TLFileSys
{
	class TFileRef;			//	file ref AND type ref in one type, essentially the filename in the format xxxxx.yyyyy
	class TFile;			//	base file type - same as the old Binary data class
	class TFileSys;			//	base file system type
	class TFileSysFactory;	//	file system class factory
	class TFileGroup;		//	groups files with the same ref (same filename) together so we can sort out which to load/convert/etc
};




//------------------------------------------------------------
//	base file system - which is secretly a class factory! (list of files)
//	just re-uses some code and makes a few things easier :)
//------------------------------------------------------------
class TLFileSys::TFileSys
{
	friend class TLFileSys::TFileSysFactory;
public:
	TFileSys(TRefRef FileSysRef,TRefRef FileSysTypeRef);
	virtual ~TFileSys()															{	Shutdown();	}

	virtual SyncBool			Init()											{	return SyncTrue;	}
	virtual SyncBool			Shutdown()										{	return SyncTrue;	}

	inline TRefRef				GetFileSysRef() const							{	return m_FileSysRef;	}
	inline TRefRef				GetFileSysTypeRef() const						{	return m_FileSysTypeRef;	}

	virtual SyncBool			LoadFile(TPtr<TFile>& pFile)					{	return SyncFalse;	}		//	read-in file
	virtual SyncBool			WriteFile(TPtr<TFile>& pFile)					{	return SyncFalse;	}		//	write file into file system if possible - if the filesys is read-only we cannot add external files and this fails
	virtual TPtr<TFile>			CreateFile(const TString& Filename,TRef TypeRef)	{	return NULL;	}		//	create a new empty file into file system if possible - if the filesys is read-only we cannot add external files and this fails
	virtual SyncBool			DeleteFile(const TFileRef& FileRef)				{	return SyncFalse;	}		//	delete file from file sys

	//inline Bool				operator==(const TFileSys& FileSys) const		{	return (this == &FileSys);	}
	inline Bool					operator==(const TRef& FileSysRef) const		{	return (GetFileSysRef() == FileSysRef);	}

protected:
	virtual SyncBool			LoadFileList()									{	return SyncFalse;	}		//	update list of files in our filelist
	virtual SyncBool			UpdateFileList();								//	update file list if it's out of date. returns FALSE if no changes, WAIT if possible changes, TRUE if there was changes
	
	Bool						CheckIsFileFromThisFileSys(TPtr<TFile>& pFile);	//	check this file belongs to this file system, if not break
	Bool						GetFileExists(const TString& Filename) const	{	return GetFileList().Exists( Filename );	}	
	Bool						GetFileExists(const TFileRef& FileRef) const	{	return GetFileList().Exists( FileRef );	}	
	TPtr<TFile>&				GetFile(const TFileRef& FileRef)				{	return m_Files.FindPtr( FileRef );	}

	TPtrArray<TFile>&			GetFileList()									{	return m_Files;	}
	const TPtrArray<TFile>&		GetFileList() const								{	return m_Files;	}

protected:
	TPtr<TFile>					CreateFileInstance(const TString& Filename,TRef TypeRef=TRef());	//	create new file into the file list - returns existing file if it already exists
	Bool						RemoveFileInstance(TPtr<TFile> pFile);								//	remove file - NULL's ptr too
	TFileRef					GetFileRef(const TString& Filename,TRef TypeRef);					//	generate file ref with explicit type

	virtual u32					GetFilelistTimeoutSecs() const										{	return 10;	}	//	update file list every N seconds
	void						FinaliseFileList();													//	update timestamp and flush missing files

protected:
	TLTime::TTimestamp			m_LastFileListUpdate;					//	timestamp of when we last updated the filelist

private:
	TPtrArray<TFile>			m_Files;								//	array of files in this file sys...
	TRef						m_FileSysRef;							//	ref for this file system
	TRef						m_FileSysTypeRef;						//	ref for this type of file system
};
