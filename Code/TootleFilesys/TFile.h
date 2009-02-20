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
	class TFileRef;
	class TFileAsset;	
	class TFileFactory;
};

namespace TLAsset
{
	class TAsset;
}




//------------------------------------------------------------
//	file ref AND type ref in one type, essentially the filename in the format xxxxx.yyyyy
//	we can have duplicates of these globally (in the FileFactory) but not per file system
//	the InstanceRef of the TFiles is what is globally unique 
//------------------------------------------------------------
class TLFileSys::TFileRef
{
	friend class TLFileSys::TFile;
public:
	TFileRef()					{	};
	TFileRef(const TFileRef& FileRef) :
		m_FileRef	( FileRef.GetFileRef() ),
		m_TypeRef	( FileRef.GetTypeRef() )
	{
	};
	TFileRef(TRefRef FileRef,TRefRef TypeRef) :
		m_FileRef	( FileRef ),
		m_TypeRef	( TypeRef )
	{
	};

	FORCEINLINE TRefRef		GetFileRef() const		{	return m_FileRef;	}
	FORCEINLINE TRefRef		GetTypeRef() const		{	return m_TypeRef;	}

	FORCEINLINE Bool		operator==(const TFileRef& FileRef) const	{	return (GetFileRef() == FileRef.GetFileRef()) && (GetTypeRef()==FileRef.GetTypeRef());	}
	FORCEINLINE Bool		operator!=(const TFileRef& FileRef) const	{	return (GetFileRef() != FileRef.GetFileRef()) || (GetTypeRef()!=FileRef.GetTypeRef());	}

protected:
	FORCEINLINE void		SetFileRef(TRefRef FileRef)	{	return m_FileRef = FileRef;	}
	//FORCEINLINE void		SetTypeRef(TRefRef TypeRef)	{	return m_TypeRef = TypeRef;	}	//	gr: shouldnt be needed
	
private:
	TRef		m_FileRef;
	TRef		m_TypeRef;
};



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
	s32								GetFileSize() const				{	return m_FileSize;	}		
	TRefRef							GetFileSysRef() const			{	return m_FileSysRef;	}
	TPtr<TFileSys>					GetFileSys() const;				//	get a pointer to the file sys this file is owned by (GetFileSysRef)
	TRefRef							GetFileRef() const				{	return m_FileRef.GetFileRef();	}
	TRefRef							GetTypeRef() const				{	return m_FileRef.GetTypeRef();	}
	const TFileRef&					GetFileRefObject() const		{	return m_FileRef;	}
	const TString&					GetFilename() const				{	return m_Filename;	}
	TBinary&						GetData()						{	return *this;	}
	const TLTime::TTimestamp&		GetTimestamp() const			{	return m_Timestamp;	}
	void							SetTimestamp(const TLTime::TTimestamp& NewTimestamp);	//	update timestamp
	void							SetTimestampNow();				//	update timestamp
	TFlags<TFile::Flags>&			GetFlags()						{	return m_Flags;	}
	const TFlags<TFile::Flags>&		GetFlags() const				{	return m_Flags;	}
	void							SetFileSize(s32 FileSize,Bool IsTooBig=FALSE)	{	m_FileSize = FileSize;	GetFlags().Set( TooBig, IsTooBig );	}

	virtual void					OnFileLoaded()					{	SetIsLoaded(SyncTrue);	TBinary::Compact();	m_Flags.Clear( OutOfDate );	};		//	binary file data has finished loading from file sys

	FORCEINLINE Bool				operator==(const TString& Filename) const	{	return GetFilename() == Filename;	}
	FORCEINLINE Bool				operator==(const TFileRef& FileRef) const	{	return GetFileRefObject() == FileRef;	}
	FORCEINLINE Bool				operator==(const TFile& File) const			{	return GetFileRefObject() == File.GetFileRefObject();	}
	FORCEINLINE Bool				operator==(TRefRef InstanceRef) const		{	return GetInstanceRef() == InstanceRef;	}	//	gr: better if this was protected, but we can't without changing TPtr :(

protected:
	TRefRef							GetInstanceRef() const					{	return m_InstanceRef;	}

private:
	TRef							m_InstanceRef;		//	unique instance ref of file

protected:
	SyncBool						m_IsLoaded;			//	FALSE if not loaded, WAIT if still loading, TRUE if loaded
	s32								m_FileSize;			//	this is the size of the file set by the file sys. -1 if unknown
	TRef							m_FileSysRef;		//	what file system did this come from?
	TFileRef						m_FileRef;			//	ref(name) & type of file
	TString							m_Filename;			//	original filename
	TLTime::TTimestamp				m_Timestamp;		//	last-modified timestamp
	TFlags<TFile::Flags>			m_Flags;			//	file flags

	TPtr<TLAsset::TAsset>			m_pExportAsset;		//	if ExportAsset() is supported then this is the asset that's being exported
};


