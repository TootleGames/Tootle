#include "TFile.h"
#include <TootleCore/TLMemory.h>
#include "TFileAsset.h"
#include "TLFileSys.h"
#include <TootleAsset/TAsset.h>




TLFileSys::TFile::TFile(TRefRef InstanceRef,TRefRef TypeRef) :
	m_FileSize			( -1 ),
	m_InstanceRef		( InstanceRef ),
	m_FileAndTypeRef	( TRef(), TypeRef ),
	m_IsLoaded			( SyncFalse )
{
}


//-----------------------------------------------------------
//	initialise the file
//-----------------------------------------------------------
SyncBool TLFileSys::TFile::Init(TRefRef FileRef,TRefRef FileSysRef,const TString& Filename)
{
	//	the file ref should be invalid at this point
	if ( m_FileAndTypeRef.GetRef().IsValid() )
	{
		TLDebug_Break("Expected file's FileRef to be invalid, as it is NOT assigned at construction");
	}

	//	set params
	m_FileAndTypeRef.SetRef( FileRef );
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

	//	check in case timestamp is older
	//	gr: check seconds, some file systems dont record millisecond timestamps (only virtual ones do!)
	s32 SecondsDiff = m_Timestamp.GetSecondsDiff( NewTimestamp );
	//	timestamp is older??
	if ( SecondsDiff < 0 )
	{
		s32 MilliDiff = m_Timestamp.GetMilliSecondsDiff( NewTimestamp );
		TTempString Debug_String("File ");
		this->Debug_GetString( Debug_String );
		Debug_String.Appendf("'s timestamp is OLDER than what it was before? (%d hours, %d seconds, %d milliseconds diff)", SecondsDiff/60, SecondsDiff%60, MilliDiff );
		TLDebug_Print( Debug_String );
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
SyncBool TLFileSys::TFile::Export(TPtr<TFileAsset>& pAssetFile,TRefRef ExportAssetType)
{
	if ( !pAssetFile )
	{
		TLDebug_Break("Asset file expected");
		return SyncFalse;
	}

	//	does this file convert to an asset? if so genericlly create assetfile from asset that we create
	Bool DoExportAsset = TRUE;

	//	fetch a pointer to the Ptr where we're keeping our exported asset
	TPtr<TLAsset::TAsset>* ppExportAsset = m_ExportedAssets.Find( ExportAssetType );

	//	if there isn't already an entry for an asset ptr, add one
	if ( !ppExportAsset )
	{
		ppExportAsset = m_ExportedAssets.AddNew( ExportAssetType );
		if ( !ppExportAsset )
		{
			TDebugString Debug_String;
			Debug_String << "Failed to add key entry for exporting asset of type " << ExportAssetType;
			TLDebug_Break( Debug_String );
			return SyncFalse;
		}
	}

	//	get regular ptr
	TPtr<TLAsset::TAsset>& pExportAsset = *ppExportAsset;

	//	do we need to continue loading an existing asset?
	if ( pExportAsset )
	{
		//	if a valid asset already exists, it doesn't need exporting
		switch ( pExportAsset->GetLoadingState() )
		{
			case TLAsset::LoadingState_Loaded:
			case TLAsset::LoadingState_Failed:
				DoExportAsset = FALSE;
				break;
		};
	}

	//	need to start/do some more exporting
	if ( DoExportAsset )
	{
		//	export the asset
		SyncBool ExportAssetResult = ExportAsset( pExportAsset, ExportAssetType );

		//	success... but no asset?
		if ( ExportAssetResult == SyncTrue && !pExportAsset )
		{
			TDebugString Debug_String;
			Debug_String << "File " << this->GetFilename() << " succeeded export of " << ExportAssetType << " but returned no asset";
			TLDebug_Break( Debug_String );
			ExportAssetResult = SyncFalse;
		}

		//	update state of asset
		if ( pExportAsset )
		{
			if ( ExportAssetResult == SyncFalse )
				pExportAsset->SetLoadingState( TLAsset::LoadingState_Failed );
			else if ( ExportAssetResult == SyncWait )
				pExportAsset->SetLoadingState( TLAsset::LoadingState_Loading );
			else if ( ExportAssetResult == SyncTrue )
				pExportAsset->SetLoadingState( TLAsset::LoadingState_Loaded );
		}

		//	still processing
		if ( ExportAssetResult == SyncWait )
			return SyncWait;

		//	failed to export to asset - NULL resulting asset for flow below
		if ( ExportAssetResult == SyncFalse )
		{
			pExportAsset = NULL;
		}
	}

	//	check the file exported the right asset type (if we have an asset here, the export must have succeeeded)
	//	gr: I've done this here, outside the export stuff in case this file keeps a pointer to a previously-output asset [of a different type]
	//		so we can catch in case we need to clean up pointers somewhere in this routine
	if ( pExportAsset && ExportAssetType.IsValid() )
	{
		if ( pExportAsset->GetAssetType() != ExportAssetType )
		{
			TDebugString Debug_String;
			Debug_String << "File " << this->GetFilename() << " exported a " << pExportAsset->GetAssetType() << " asset type. We expected " << ExportAssetType;
			TLDebug_Break( Debug_String );

			//	gr: should we/do we need to null this asset?
			pExportAsset = NULL;

			return SyncFalse;
		}
	}

	//	failed to export
	if ( !pExportAsset )
	{
		TTempString Debug_String;
		Debug_String << "Failed to export asset type " << ExportAssetType << " from file " << this->GetFilename();
		TLDebug_Break( Debug_String );
		return SyncFalse;
	}

	//	convert asset to asset file...
	//	loading state should be loaded
	if ( !pExportAsset->IsLoaded() )
	{
		TLDebug_Break("Asset should be loaded at this point");
		return SyncWait;
	}

	//	export asset to asset file
	SyncBool ExportResult = pExportAsset->Export( pAssetFile );
	if ( ExportResult == SyncWait )
		return SyncWait;

	//	failed, cleanup
	if ( ExportResult == SyncFalse )
	{
		pExportAsset = NULL;
		return SyncFalse;
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


//-----------------------------------------------------------
//	copy data into file - this sets new timestamp, file size, and marks file as out of date
//-----------------------------------------------------------
Bool TLFileSys::TFile::Load(TBinary& Data)
{
	//	copy data
	TBinary::Copy( Data );
	SetFileSize( m_Data.GetSize() );				

	//	update timestamp on file
	SetTimestampNow();

	//	notify is now loaded
	OnFileLoaded();

	return TRUE;
}


//-----------------------------------------------------------
//	is this an asset type supported by the asset export?
//-----------------------------------------------------------
Bool TLFileSys::TFile::IsSupportedExportAssetType(TRefRef AssetType) const			
{	
	TFixedArray<TRef,100> SupportedTypes;
	GetSupportedExportAssetTypes( SupportedTypes );

	//	this asset type is explicitly supported
	if ( SupportedTypes.Exists( AssetType ) )
		return true;

	//	see if this file supports any/unknown type
	if ( SupportedTypes.Exists( TRef() ) )
		return true;

	//	not supported
	return false;
}

