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
	class TFile;		//	base file type 
	class TFileBinary;	//	plain file containing just binary data. Doesn't support any asset exporting
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

	virtual SyncBool				Export(TPtr<TFileAsset>& pAssetFile,TRefRef ExportAssetType);			//	turn this file into an asset file
	virtual SyncBool				ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType)=0;	//	extract an asset of the specified type. return SyncTrue on success, SyncWait for async processing, SyncFalse if unsupported or parsing failed
	Bool							IsSupportedExportAssetType(TRefRef AssetType) const;					//	is this an asset type supported by the asset export?
	virtual void					GetSupportedExportAssetTypes(TArray<TRef>& SupportedTypes) const=0;		//	append all the asset types this file supports export of

	Bool							Copy(TPtr<TFile>& pFile,Bool CopyFilename=FALSE);		//	copy file data and attributes (timestamp, flags)
	Bool							Load(TBinary& Data);			//	copy data into file - this sets new timestamp, file size, and marks file as out of date

	bool							IsWritable() const;				//	can we overwrite this file?
	SyncBool						IsLoaded() const				{	return m_IsLoaded;	}
	void							SetIsLoaded(SyncBool Loaded)	{	m_IsLoaded = Loaded;	}
	const TLTime::TTimestamp&		GetTimestamp() const			{	return m_Timestamp;	}
	void							SetTimestamp(const TLTime::TTimestamp& NewTimestamp);	//	update timestamp
	void							SetTimestampNow();				//	update timestamp
	bool							IsOutOfDate() const				{	return GetFlags()( OutOfDate );	}
	void							SetOutOfDate(bool OutOfDate=true);		//	mark file as out of date

	s32								GetFileSize() const				{	return m_FileSize;	}		
	void							SetFileSize(s32 FileSize,Bool IsTooBig=FALSE)	{	m_FileSize = FileSize;	GetFlags().Set( TooBig, IsTooBig );	}
	TRefRef							GetFileSysRef() const			{	return m_FileSysRef;	}
	TPtr<TFileSys>					GetFileSys() const;				//	get a pointer to the file sys this file is owned by (GetFileSysRef)
	const TString&					GetFilename() const				{	return m_Filename;	}

	TRefRef							GetFileRef() const				{	return m_FileAndTypeRef.GetRef();	}		
	TRefRef							GetTypeRef() const				{	return m_FileAndTypeRef.GetTypeRef();	}
	const TTypedRef&				GetFileAndTypeRef() const		{	return m_FileAndTypeRef;	}

	TBinary&						GetData()						{	return *this;	}
	const TBinary&					GetData() const					{	return *this;	}
	TFlags<TFile::Flags>&			GetFlags()						{	return m_Flags;	}
	const TFlags<TFile::Flags>&		GetFlags() const				{	return m_Flags;	}

	FORCEINLINE void				SetUnknownType(Bool IsUnknown=TRUE)	{	m_Flags.Set( UnknownType, IsUnknown );	}
	FORCEINLINE Bool				IsUnknownType() const				{	return m_Flags.IsSet( UnknownType );	}

	virtual void					OnFileLoaded();						//	file has been loaded and it's contents changed

	FORCEINLINE Bool				operator==(const TString& Filename) const			{	return GetFilename().IsEqual( Filename, FALSE );	}	//	gr: although files are stored as case-sensitive (CS on unix/mac/ipod, not windows) we do case-insenstive comparisons. do not STORE case insenstive though
	FORCEINLINE Bool				operator==(const TFile& File) const					{	return GetInstanceRef() == File.GetInstanceRef();	}
	FORCEINLINE Bool				operator==(const TRef& InstanceRef) const			{	return GetInstanceRef() == InstanceRef;	}		//	note: NOT a match to the file ref

	void							Debug_GetString(TString& String)		{	m_FileAndTypeRef.GetString( String );	}

protected:
	TRefRef							GetInstanceRef() const					{	return m_InstanceRef;	}

private:
	TRef							m_InstanceRef;		//	unique instance ref of file - this does NOT correspond to the file name. Never ever store this!

protected:
	SyncBool						m_IsLoaded;			//	FALSE if not loaded, WAIT if still loading, TRUE if loaded
	s32								m_FileSize;			//	this is the size of the file set by the file sys. -1 if unknown
	TRef							m_FileSysRef;		//	what file system did this come from?
	TTypedRef						m_FileAndTypeRef;	//	ref(name) & type of file - NOT UNIQUE PER FILE SYS
	TString							m_Filename;			//	original filename
	TLTime::TTimestamp				m_Timestamp;		//	last-modified timestamp
	TFlags<TFile::Flags>			m_Flags;			//	file flags

	TPtrKeyArray<TRef,TLAsset::TAsset>	m_ExportedAssets;	//	this is a pointer to the asset currently being exported (or has been exported) per asset type
};





//---------------------------------------------------------
//	gr: to keep the base TFile type abstract, this TFileBinary type
//		is just a plain file with no asset exporting
//---------------------------------------------------------
class TLFileSys::TFileBinary : public TLFileSys::TFile
{
public:
	TFileBinary(TRefRef FileRef,TRefRef FileTypeRef) :
		TFile( FileRef, FileTypeRef )
	{
	}
	
	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType)	{	return SyncFalse;	}
	virtual void		GetSupportedExportAssetTypes(TArray<TRef>& SupportedTypes) const	{	}
};
