/*------------------------------------------------------
	
	Tootle asset file (BinaryTree) type

-------------------------------------------------------*/
#pragma once

#include "TFile.h"
#include <TootleCore/TBinaryTree.h>
#include <TootleCore/TStateMachine.h>


namespace TLFileSys
{
	class TFile;
	class TFileAsset;
	class TFileAssetImporter;
	
	extern const TRef		g_TootFileRef;


	namespace TLFileAssetImporter
	{
		class Mode_ImportChild;
	}
};



//---------------------------------------------------------
//	Tootle asset file (BinaryTree) type
//---------------------------------------------------------
class TLFileSys::TFileAsset : public TLFileSys::TFile
{
	friend class TLFileAssetImporter::Mode_ImportChild;
public:
	class Header;
	class SectionHeader;

	enum HeaderFlags
	{
		Broken = 0,		//	failed to parse - file is broken
		Compressed,		//	data is compressed
		NeedsImport,	//	file data has changed, binary tree is now out of date
		NeedsExport,	//	asset tree has changed, binary File is now out of date
	};

public:
	TFileAsset(TRefRef FileRef,TRefRef FileTypeRef);

	SyncBool			Import();					//	turn this TFile into a binary tree
	SyncBool			Export();					//	export the binary tree to this TFile

	TRefRef				GetAssetTypeRef() const		{	return m_Header.m_AssetType;	}
	TFileAsset::Header&	GetHeader()					{	return m_Header;	}
	TBinaryTree&		GetData()					{	return m_Data;	}
	const TBinaryTree&	GetData() const				{	return m_Data;	}

	void				SetNeedsImport(Bool Enable)	{	return m_Header.m_Flags.Set( NeedsImport, Enable );	}
	void				SetNeedsExport(Bool Enable)	{	return m_Header.m_Flags.Set( NeedsExport, Enable );	}
	Bool				GetNeedsImport() const		{	return m_Header.m_Flags( NeedsImport );	}
	Bool				GetNeedsExport() const		{	return m_Header.m_Flags( NeedsExport );	}

	virtual void		OnFileLoaded()				{	TFile::OnFileLoaded();	m_Header.m_Flags.Set( NeedsImport );	}
	virtual void		OnImportFinished(SyncBool ImportResult)	{	m_Data.Compact();	}

protected:
	Bool				ImportTree(TPtr<TBinaryTree>& pChild,SectionHeader& Header,TBinary& Data);	//	recursive import of tree
	Bool				ExportTree(TPtrArray<TBinaryTree>& Children,TBinary& Data);				//	recursive export of tree

public:
	class Header
	{
	public:
		Header();

		Bool						IsValid() const;	//	check if the header has been setup

	public:
		TRef						m_TootFileRef;		//	first part of a tootfile is a u32 to ensure it's a tootfile
		TFlags<HeaderFlags,u32>		m_Flags;			//	file flags
		u32							m_DataLength;		//	expected length of data, uncompressed (not including header)
		u32							m_DataCheckSum;		//	checksum of file data, uncompressed (exclusing header)
		TRef						m_AssetType;		//	asset type
	};

	//	section header stored in file
	class SectionHeader
	{
	public:
		enum Flags
		{
		};

	public:
		TRef				m_SectionRef;
		u32					m_Length;		//	redundant now as blocks of data have the length written before them...
		u16					m_ChildCount;
		TFlags<Flags,u16>	m_Flags;
	};

protected:
	Header						m_Header;		//	toot header
	TBinaryTree					m_Data;			//	data turned into binary tree
	TPtr<TFileAssetImporter>	m_pImporter;	//	step-by-step importer
};



namespace TLFileSys
{
	class TFileAssetImporter;

	namespace TLFileAssetImporter
	{
		class Mode_Base : public TStateMode
		{
		protected:
			inline TFileAsset*	GetAssetFile();//		{	return GetStateMachine<TFileAssetImporter>()->m_pAssetFile;	}
			TFile*				GetFile()			{	return GetAssetFile();	}
			TFileAsset::Header&	GetHeader()			{	return GetAssetFile()->GetHeader();	}
		};

		class Mode_Init : public Mode_Base
		{
		protected:
			virtual TRef			Update(float Timestep);
		};

		class Mode_Decompress : public Mode_Base
		{
		protected:
			virtual TRef			Update(float Timestep);
		};

		class Mode_ImportChild : public Mode_Base
		{
		public:
			Mode_ImportChild() : m_CurrentChild (-1)	{}
		protected:
			virtual TRef			Update(float Timestep);
		protected:
			s32						m_CurrentChild;
		};

		class Mode_Finished : public Mode_Base
		{
		protected:
			virtual Bool			OnBegin(TRefRef PreviousMode);
		};

		class Mode_Failed : public Mode_Base
		{
		protected:
			virtual Bool			OnBegin(TRefRef PreviousMode);
		};
	}

	class TFileAssetImporter : public TStateMachine
	{
	public:
		TFileAssetImporter(TFileAsset* pAssetFile);

		SyncBool		GetResult();		//	work out what state of the process we're in
		TFileAsset*		GetAssetFile()		{	return m_pAssetFile;	}

	private:
		TFileAsset*		m_pAssetFile;
	};
}



//-------------------------------------------------------------
//	definition needs to be after TFileAssetImporter declaration
//-------------------------------------------------------------
inline TLFileSys::TFileAsset* TLFileSys::TLFileAssetImporter::Mode_Base::GetAssetFile()
{
	return GetStateMachine<TFileAssetImporter>()->GetAssetFile();
}
