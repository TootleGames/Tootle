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
public:
	enum Flags
	{
		OutOfDate,	//	timestamp has changed since this file was loaded (should only apply when loaded state is true)
		TooBig,		//	file is bigger than u32 (>4gb) so we don't allow this to be loaded (or file size is inaccurate
		Lost,		//	file is no longer there and will be flushed next time the file list is updated
	};

public:
	TFile(TRefRef FileRef,TRefRef FileTypeRef);
	virtual ~TFile()			{	};
	
	SyncBool						Init(TRefRef FileSysRef,const TString& Filename);

	virtual SyncBool				Export(TPtr<TFileAsset>& pAssetFile);	//	turn this file into an asset file
	virtual SyncBool				ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported)		{	Supported = FALSE;	return SyncFalse;	}	//	turn this file into an asset - set Supported to FALSE if this file doesnt convert to an asset (i.e. SyncFalse doesnt mean ERROR)

	Bool							Copy(TPtr<TFile>& pFile,Bool CopyFilename=FALSE);		//	copy file data and attributes (timestamp, flags)

	SyncBool						IsLoaded() const				{	return m_IsLoaded;	}
	void							SetIsLoaded(SyncBool Loaded)	{	m_IsLoaded = Loaded;	}
	s32								GetFileSize() const				{	return m_FileSize;	}		
	TRefRef							GetFileSysRef() const			{	return m_FileSysRef;	}
	TPtr<TFileSys>					GetFileSys() const;				//	get a pointer to the file sys this file is owned by (GetFileSysRef)
	TRefRef							GetFileRef() const				{	return m_FileRef;	}
	TRefRef							GetFileTypeRef() const			{	return m_FileTypeRef;	}
	const TString&					GetFilename() const				{	return m_Filename;	}
	TBinary&						GetData()						{	return *this;	}
	const TLTime::TTimestamp&		GetTimestamp() const			{	return m_Timestamp;	}
	void							SetTimestamp(const TLTime::TTimestamp& NewTimestamp);	//	update timestamp
	void							SetTimestampNow();				//	update timestamp
	TFlags<TFile::Flags>&			GetFlags()						{	return m_Flags;	}
	const TFlags<TFile::Flags>&		GetFlags() const				{	return m_Flags;	}
	void							SetFileSize(s32 FileSize,Bool IsTooBig=FALSE)	{	m_FileSize = FileSize;	GetFlags().Set( TooBig, IsTooBig );	}

	virtual void					OnFileLoaded()					{	SetIsLoaded(SyncTrue);	TBinary::Compact();	m_Flags.Clear( OutOfDate );	};		//	binary file data has finished loading from file sys

	inline Bool						operator==(const TString& Filename) const	{	return GetFilename() == Filename;	}
	inline Bool						operator==(const TRef& FileRef) const		{	return GetFileRef() == FileRef;	}

protected:
	SyncBool						m_IsLoaded;			//	FALSE if not loaded, WAIT if still loading, TRUE if loaded
	s32								m_FileSize;			//	this is the size of the file set by the file sys. -1 if unknown
	TRef							m_FileSysRef;		//	what file system did this come from?
	TRef							m_FileRef;			//	ref of file
	TRef							m_FileTypeRef;		//	ref of type of file (eg. .mesh .ttf
	TString							m_Filename;			//	original filename
	TLTime::TTimestamp				m_Timestamp;		//	last-modified timestamp
	TFlags<TFile::Flags>			m_Flags;			//	file flags

	TPtr<TLAsset::TAsset>			m_pExportAsset;		//	if ExportAsset() is supported then this is the asset that's being exported
};


