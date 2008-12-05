#include "TFile.h"
#include <TootleCore/TLMemory.h>
#include "TFileAsset.h"
#include "TLFileSys.h"
#include <TootleAsset/TAsset.h>




TLFileSys::TFile::TFile(TRefRef FileRef,TRefRef FileTypeRef) :
	m_FileSize		( -1 ),
	m_FileRef		( FileRef ),
	m_IsLoaded		( SyncFalse ),
	m_FileTypeRef	( FileTypeRef )
{
}


//-----------------------------------------------------------
//	initialise the file - this should check the file header
//-----------------------------------------------------------
SyncBool TLFileSys::TFile::Init(TRefRef FileSysRef,const TString& Filename)
{
	m_FileSysRef = FileSysRef;
	m_Filename = Filename;

	return SyncTrue;
}


//-----------------------------------------------------------
//	update timestamp
//-----------------------------------------------------------
void TLFileSys::TFile::SetTimestamp(const TLTime::TTimestamp& NewTimestamp)	
{
	//	if the old timestamp was invalid then just set new timestamp
	if ( !m_Timestamp.IsValid() )
	{
		m_Timestamp = NewTimestamp;
		return;
	}

	//	timestamp hasnt changed... do nothing
	if ( m_Timestamp == NewTimestamp )
		return;

	//	timestamp is older??
	if ( NewTimestamp < m_Timestamp )
	{
		if ( !TLDebug_Break("File's timestamp is OLDER than what it was before?") )
			return;
	}

	//	if the file is laoded, it's now out of date
	if ( IsLoaded() != SyncFalse )
		GetFlags().Set( TFile::OutOfDate );

	//	set new timestamp
	m_Timestamp = NewTimestamp;	
}


//-----------------------------------------------------------
//	update timestamp
//-----------------------------------------------------------
void TLFileSys::TFile::SetTimestampNow()
{
	//	update timestamp
	m_Timestamp.SetTimestampNow();

	//	if the file is laoded, it's now out of date
	if ( IsLoaded() != SyncFalse )
		GetFlags().Set( TFile::OutOfDate );
}


//-----------------------------------------------------------
//	turn this file into an asset file, when we create it, put it into this file system
//-----------------------------------------------------------
SyncBool TLFileSys::TFile::Export(TPtr<TFileAsset>& pAssetFile)
{
	if ( !pAssetFile )
	{
		TLDebug_Break("Asset file expected");
		return SyncFalse;
	}

	//	does this file convert to an asset? if so genericlly create assetfile from asset that we create
	Bool DoExportAsset = m_pExportAsset ? (m_pExportAsset->GetLoadingState() == SyncWait) : TRUE;
	if ( DoExportAsset )
	{
		Bool Supported = FALSE;
		SyncBool ExportAssetResult = ExportAsset( m_pExportAsset, Supported );

		//	is supported so see how it went... and convert
		if ( Supported )
		{
			//	new mesh asset is now "loaded"
			if ( m_pExportAsset )
				m_pExportAsset->SetLoadingState( ExportAssetResult );

			//	supported but still processing
			if ( ExportAssetResult == SyncWait )
				return SyncWait;

			//	failed to export to asset
			if ( ExportAssetResult == SyncFalse )
			{
				m_pExportAsset = NULL;
				return SyncFalse;
			}
		}
		else
		{
			if ( m_pExportAsset )
			{
				if ( !TLDebug_Break("ExportAsset() unsupported... but generated asset...") )
				{
					m_pExportAsset = NULL;
					return SyncFalse;
				}
			}

			m_pExportAsset = NULL;
		}
	}

	//	convert asset to asset file
	if ( m_pExportAsset )
	{
		//	loading state should be loaded
		if ( !m_pExportAsset->IsLoaded() )
		{
			TLDebug_Break("Asset should be loaded at this point");
			return SyncWait;
		}

		//	export asset to asset file
		SyncBool ExportResult = m_pExportAsset->Export( pAssetFile );
		if ( ExportResult == SyncWait )
			return SyncWait;

		//	failed, cleanup
		if ( ExportResult == SyncFalse )
		{
			m_pExportAsset = NULL;
			return SyncFalse;
		}
	}
	else
	{
		//	masquerade as a generic binary file
		pAssetFile->GetHeader().m_TootFileRef = TLFileSys::g_TootFileRef;
		pAssetFile->GetHeader().m_AssetType = "Binary";

		//	base code just sticks all our binary data into the root of the asset file 
		TBinaryTree& BinaryTree = pAssetFile->GetData();
		BinaryTree.Write( this->GetData() );

		//	set root of binary data to be called "data"
		BinaryTree.SetDataRef("Data");

		//	just set data info in header to what it is
		pAssetFile->GetHeader().m_DataLength = this->GetSize();
		pAssetFile->GetHeader().m_DataCheckSum = this->GetChecksum();

		//	data is not compressed
		pAssetFile->GetHeader().m_Flags.Clear( TFileAsset::Compressed );
	}

	//	this file no longer needs to be imported, but the binary file itself is out of date
	//	(the binary part of the asset file isnt THIS, it's been created from scratch)
	pAssetFile->SetNeedsImport( FALSE );
	pAssetFile->SetNeedsExport( TRUE );

	return SyncTrue;
}


//-----------------------------------------------------------
//	get a pointer to the file sys this file is owned by (GetFileSysRef)
//-----------------------------------------------------------
TPtr<TLFileSys::TFileSys> TLFileSys::TFile::GetFileSys() const
{
	return TLFileSys::GetFileSys( GetFileSysRef() );
}


//-----------------------------------------------------------
//	copy file data and attributes (timestamp, flags)
//-----------------------------------------------------------
Bool TLFileSys::TFile::Copy(TPtr<TFile>& pFile,Bool CopyFilename)
{
	if ( !pFile )
	{
		TLDebug_Break("File expected");
		return FALSE;
	}

	//	copy data
	GetData().Copy( pFile->GetData() );

	//	copy attributes
	m_IsLoaded = pFile->IsLoaded();
	m_Flags = pFile->GetFlags();
	m_Timestamp = pFile->GetTimestamp();

	//	copy refs?

	//	copy filename
	if ( CopyFilename )
	{
		m_Filename = pFile->GetFilename();
	}

	return TRUE;
}


