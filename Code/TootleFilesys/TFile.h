/*------------------------------------------------------
	
	Plain binary file type

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TString.h>
#include <TootleCore/TLTime.h>
#include <TootleCore/TRef.h>
#include <TootleCore/TBinary.h>
#include <TootleCore/TFlags.h>
#include <TootleCore/TPtr.h>
#include <TootleAsset/TAsset.h>

namespace TLFileSys
{
	class TFileSys;
	class TFile;		//	base file type - same as the old Binary data class
	class TFileAsset;	
	class TFileFactory;
};

namespace TLAsset
{
	class TAsset;
}







//---------------------------------------------------------
//	basic binary file. Functionality shouldnt need to be overloaded
//	file systems may overload to attach additional variables etc
//	engine should not overload though, instead keep a smart pointer to
//	the file to keep it in memory (even if the filesys has ditched it)
//	then release when it's been converted/read etc
//---------------------------------------------------------
class TLFileSys::TFile : public TBinary
{
	friend class TFileFactory;	//	only the file factory has access to the InstanceRef - nothing else should use it
public:
	enum Flags
	{
		OutOfDate=0,	//	timestamp has changed since this file was loaded (should only apply when loaded state is true)
		TooBig,			//	file is bigger than u32 (>4gb) so we don't allow this to be loaded (or file size is inaccurate
		Lost,			//	file is no longer there and will be flushed next time the file list is updated
		UnknownType,	//	if we have tried to load this file and don't recognise the type, we mark it so we can ignore it in future
	};

protected:
	TFile(TRefRef InstanceRef,TRefRef TypeRef);

public:
	virtual ~TFile()				{	};
	
	SyncBool						Init(TRefRef FileRef,TRefRef FileSysRef,const TString& Filename);	//	assign values we couldn't do via CreateObject/constructor

	virtual SyncBool				Export(TPtr<TFileAsset>& pAssetFile);	//	turn this file into an asset file
	virtual SyncBool				ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported)		{	Supported = FALSE;	return SyncFalse;	}	//	turn this file into an asset - set Supported to FALSE if this file doesnt convert to an asset (i.e. SyncFalse doesnt mean ERROR)

	Bool							Copy(TPtr<TFile>& pFile,Bool CopyFilename=FALSE);		//	copy file data and attributes (timestamp, flags)

	SyncBool						IsLoaded() const				{	return m_IsLoaded;	}
	void							SetIsLoaded(SyncBool Loaded)	{	m_IsLoaded = Loaded;	}
	const TLTime::TTimestamp&		GetTimestamp() const			{	return m_Timestamp;	}
	void							SetTimestamp(const TLTime::TTimestamp& NewTimestamp);	//	update timestamp
	void							SetTimestampNow();				//	update timestamp

	s32								GetFileSize() const				{	return m_FileSize;	}		
	void							SetFileSize(s32 FileSize,Bool IsTooBig=FALSE)	{	m_FileSize = FileSize;	GetFlags().Set( TooBig, IsTooBig );	}
	TRefRef							GetFileSysRef() const			{	return m_FileSysRef;	}
	TPtr<TFileSys>					GetFileSys() const;				//	get a pointer to the file sys this file is owned by (GetFileSysRef)
	const TString&					GetFilename() const				{	return m_Filename;	}
	
	TRefRef							GetFileRef() const				{	return m_FileAndTypeRef.GetRef();	}
	TRefRef							GetTypeRef() const				{	return m_FileAndTypeRef.GetTypeRef();	}
	const TTypedRef&				GetFileAndTypeRef() const		{	return m_FileAndTypeRef;	}
	virtual TRef					GetFileExportAssetType() const	{	return TRef();	}	//	if the file type knows what type of Asset it exports to, return it. (aids loading)
	
	TBinary&						GetData()						{	return *this;	}
	TFlags<TFile::Flags>&			GetFlags()						{	return m_Flags;	}
	const TFlags<TFile::Flags>&		GetFlags() const				{	return m_Flags;	}

	FORCEINLINE void				SetUnknownType(Bool IsUnknown=TRUE)	{	m_Flags.Set( UnknownType, IsUnknown );	}
	FORCEINLINE Bool				IsUnknownType() const				{	return m_Flags.IsSet( UnknownType );	}

	virtual void					OnFileLoaded()					{	SetIsLoaded(SyncTrue);	TBinary::Compact();	m_Flags.Clear( OutOfDate );	};		//	binary file data has finished loading from file sys

	FORCEINLINE Bool				operator==(const TString& Filename) const			{	return GetFilename() == Filename;	}
	FORCEINLINE Bool				operator==(const TTypedRef& FileAndTypeRef) const	{	return GetFileAndTypeRef() == FileAndTypeRef;	}
	FORCEINLINE Bool				operator==(const TFile& File) const					{	return GetFileAndTypeRef() == File.GetFileAndTypeRef();	}
	FORCEINLINE Bool				operator==(TRefRef InstanceRef) const				{	return GetInstanceRef() == InstanceRef;	}	//	gr: better if this was protected, but we can't without changing TPtr :(

protected:
	TRefRef							GetInstanceRef() const					{	return m_InstanceRef;	}

private:
	TRef							m_InstanceRef;		//	unique instance ref of file - this does NOT correspond to the file name. Never ever store this!

protected:
	SyncBool						m_IsLoaded;			//	FALSE if not loaded, WAIT if still loading, TRUE if loaded
	s32								m_FileSize;			//	this is the size of the file set by the file sys. -1 if unknown
	TRef							m_FileSysRef;		//	what file system did this come from?
	TTypedRef						m_FileAndTypeRef;	//	ref(name) & type of file
	TString							m_Filename;			//	original filename
	TLTime::TTimestamp				m_Timestamp;		//	last-modified timestamp
	TFlags<TFile::Flags>			m_Flags;			//	file flags

	TPtr<TLAsset::TAsset>			m_pExportAsset;		//	if ExportAsset() is supported then this is the asset that's being exported
};


