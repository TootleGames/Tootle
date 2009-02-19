/*------------------------------------------------------
	
	Base filesystem type & factory

-------------------------------------------------------*/
#pragma once
#include <TootleCore/TLTypes.h>
#include <TootleCore/TClassFactory.h>
#include <TootleCore/TLTime.h>
#include <TootleCore/TManager.h>
#include "TFile.h"

namespace TLFileSys
{
	class TFile;			//	base file type - same as the old Binary data class
	class TFileSys;			//	base file system type
	class TFileSysFactory;	//	file system class factory

	TPtr<TLFileSys::TFileSys>&	GetFileSys(TRefRef FileSysRef);		//	return a file system
	void						GetFileSys(TPtrArray<TLFileSys::TFileSys>& FileSysList,TRefRef FileSysRef,TRefRef FileSysTypeRef);	//	return all matching file systems to these refs/types

	//	find the newest file [of a specific type] in the specified file systems
	TPtr<TFile>					FindFile(TPtrArray<TLFileSys::TFileSys>& FileSysList,TRefRef FileRef,TRefRef FileTypeRef=TRef());	//	invalid type ref matches any type

	//	create a local file system
	SyncBool					CreateLocalFileSys(TRef& FileSysRef,const TString& Directory,Bool IsWritable);	//	async create a local filesystem for the specified path. FileSysRef is set to the new file system's ref for the purposes of asynchronousness so keep using it when async calling this func

	Bool						GetParentDir(TString& Directory);	//	directory manipulation - turns filename to dir, then chops off a dir at a time

	extern TPtr<TLFileSys::TFileSysFactory>	g_pFactory;
};



//------------------------------------------------------------
//	class factory for file systems
//------------------------------------------------------------
class TLFileSys::TFileSysFactory : public TManager, public TObjectFactory<TLFileSys::TFileSys>
{
public:
	TFileSysFactory(TRef refManagerID) :
	  TManager(refManagerID)
	{
	}
  
protected:
	virtual TLFileSys::TFileSys*	CreateObject(TRefRef InstanceRef,TRefRef TypeRef);

	virtual SyncBool Initialise();
	virtual SyncBool Update();
	virtual SyncBool Shutdown();
	
	virtual void	OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);
	
};


//------------------------------------------------------------
//	base file system - which is secretly a class factory! (list of files)
//	just re-uses some code and makes a few things easier :)
//------------------------------------------------------------
class TLFileSys::TFileSys : public TClassFactory<TLFileSys::TFile>
{
public:
	TFileSys(TRefRef FileSysRef,TRefRef FileSysTypeRef);
	virtual ~TFileSys()													{	Shutdown();	}

	virtual SyncBool			Init()									{	return SyncTrue;	}
	virtual SyncBool			Update()								{	return SyncWait;	}
	virtual SyncBool			Shutdown()								{	return SyncTrue;	}

	inline TRefRef				GetFileSysRef() const					{	return m_FileSysRef;	}
	inline TRefRef				GetFileSysTypeRef() const				{	return m_FileSysTypeRef;	}

	virtual SyncBool			LoadFileList()							{	return SyncFalse;	}		//	update list of files in our filelist
	virtual SyncBool			UpdateFileList();						//	update file list if it's out of date. returns FALSE if no changes, WAIT if possible changes, TRUE if there was changes
	virtual SyncBool			LoadFile(TPtr<TFile>& pFile)			{	return SyncFalse;	}		//	read-in file
	virtual SyncBool			WriteFile(TPtr<TFile>& pFile)			{	return SyncFalse;	}		//	write file into file system if possible - if the filesys is read-only we cannot add external files and this fails
	virtual TPtr<TFile>			CreateFile(const TString& Filename,TRefRef FileTypeRef)		{	return NULL;	}		//	create a new empty file into file system if possible - if the filesys is read-only we cannot add external files and this fails
	virtual SyncBool			DeleteFile(TRefRef FileRef)				{	return SyncFalse;	}		//	delete file from file sys
	
	Bool						GetFileExists(const TString& Filename) const	{	return GetFileList().Exists( Filename );	}	
	Bool						GetFileExists(TRefRef FileRef) const			{	return GetFileList().Exists( FileRef );	}	
	TPtr<TFile>					GetFile(const TString& Filename,Bool Load);		//	fetch file based on filename
	TPtr<TFile>					GetFile(TRefRef FileRef,Bool Load);				//	fetch file and load if required

	const TPtrArray<TFile>&		GetFileList() const								{	return *this;	}

	//inline Bool				operator==(const TFileSys& FileSys) const		{	return (this == &FileSys);	}
	inline Bool					operator==(const TRef& FileSysRef) const		{	return (GetFileSysRef() == FileSysRef);	}

protected:
	virtual TPtr<TFile>			CreateFileInstance(const TString& Filename,TRef TypeRef=TRef());	//	create new file into the file list - returns existing file if it already exists
	virtual void				GetFileRef(const TString& Filename,TRef& FileRef,TRef& TypeRef);	//	generate file ref and type ref from filename
	virtual void				GetFileRef(const TString& Filename,TRef& FileRef);					//	generate file ref from filename
	virtual TFile*				CreateObject(TRefRef InstanceRef,TRefRef TypeRef);					//	overloaded from class factory
	virtual u32					GetFilelistTimeoutSecs() const										{	return 10;	}	//	update file list every N seconds
	void						FinaliseFileList();													//	update timestamp and flush missing files

protected:
	TRef						m_FileSysRef;							//	ref for this file system
	TRef						m_FileSysTypeRef;						//	ref for this type of file system
	TLTime::TTimestamp			m_LastFileListUpdate;					//	timestamp of when we last updated the filelist
};


