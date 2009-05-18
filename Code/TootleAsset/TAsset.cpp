#include "TAsset.h"
#include "TAsset.h"
#include <TootleFileSys/TLFileSys.h>
#include <TootleFileSys/TFileAsset.h>



//----------------------------------------------------------
//	
//----------------------------------------------------------
TLAsset::TAsset::TAsset(const TRef& AssetType,const TRef& AssetRef) :
	m_AssetType		( AssetType ),
	m_AssetRef		( AssetRef ),
	m_LoadingState	( TLAsset::LoadingState_Init ),
	m_Data			( STRef4(D,a,t,a) )
{
	m_AssetRef.GetString( m_Debug_AssetRefString );
}


//----------------------------------------------------
//	load asset data out of file
//----------------------------------------------------
void TLAsset::TAsset::Import(TPtr<TLFileSys::TFileAsset>& pAssetFile)
{
	if ( !pAssetFile ) 
	{
		TLDebug_Break("Asset file expected");
		SetLoadingState( TLAsset::LoadingState_Failed );
		return;
	}

	//	wrong file type for this asset
	if ( pAssetFile->GetAssetTypeRef() != GetAssetType() )
	{
		if ( TLDebug_Break("Asset file is different type to self") )
		{
			SetLoadingState( TLAsset::LoadingState_Failed );
			return;
		}
	}

	//	print out tree
	//TLDebug_Print("Importing asset file...");
	//pAssetFile->GetData().Debug_PrintTree();

	//	reset read pos
	pAssetFile->GetData().ResetReadPos();

	//	import the data
	SyncBool ImportState = ImportData( pAssetFile->GetData() );
	
	if ( ImportState == SyncFalse )
		SetLoadingState( TLAsset::LoadingState_Failed );
	else if ( ImportState == SyncWait )
		SetLoadingState( TLAsset::LoadingState_Loading );
	else if ( ImportState == SyncTrue )
		SetLoadingState( TLAsset::LoadingState_Loaded );
}


//----------------------------------------------------
//	save asset to file
//----------------------------------------------------
SyncBool TLAsset::TAsset::Export(TPtr<TLFileSys::TFileAsset>& pAssetFile)
{
	if ( !pAssetFile )
	{
		TLDebug_Break("Asset file expected");
		return SyncFalse;
	}

	//	ensure the asset is loaded before we can export it
	TLAsset::TLoadingState AssetLoadingState = GetLoadingState();
		
	//	if not loaded yet, cannot export
	if ( AssetLoadingState == TLAsset::LoadingState_Loading )
		return SyncWait;

	//	failed to load, cannot export
	if ( AssetLoadingState == TLAsset::LoadingState_Failed || AssetLoadingState == TLAsset::LoadingState_Init )
		return SyncFalse;

	//	setup asset file header
	TLFileSys::TFileAsset::Header& Header = pAssetFile->GetHeader();
	Header.m_AssetType = GetAssetType();
	Header.m_TootFileRef = TLFileSys::g_TootFileRef;

	//	export data
	SyncBool ExportResult = ExportData( pAssetFile->GetData()  );

	//	asset file's tree is updated
	if ( ExportResult == SyncTrue )
	{
		pAssetFile->SetNeedsExport(TRUE);
	}

	//	print out tree
	//TLDebug_Print("Exported asset to asset file...");
	//pAssetFile->GetData().Debug_PrintTree();

	return ExportResult;
}


//----------------------------------------------------
//	take any data in this binary tree that we didn't read after importing and put it into this asset's data (m_Data)
//----------------------------------------------------
void TLAsset::TAsset::ImportUnknownData(TBinaryTree& Data)
{
	for ( u32 i=0;	i<Data.GetChildren().GetSize();	i++ )
	{
		TPtr<TBinaryTree>& pChild = Data.GetChildren().ElementAt(i);
		
		//	if read pos isnt reset we can assume we didn't read the data in
		//	gr: now if any children HAVE been read we don't store this data
		if ( pChild->IsDataTreeRead() )
			continue;

		//	gr: does this need to clone?
		//	save this child
		m_Data.AddChild( pChild );

		#ifdef _DEBUG
		TTempString Debug_String("Storing non-imported data in asset ");
		GetAssetRef().GetString( Debug_String );
		Debug_String.Append("(");
		GetAssetType().GetString( Debug_String );
		Debug_String.Append("): ");
		pChild->GetDataRef().GetString( Debug_String );
		if ( pChild->GetDataTypeHint().IsValid() )
		{
			Debug_String.Append("(type: ");
			pChild->GetDataTypeHint().GetString( Debug_String );
			Debug_String.Append(")");
		}
		TLDebug_Print( Debug_String );
		#endif
	}

}


//----------------------------------------------------
//	write out our unknown data
//----------------------------------------------------
void TLAsset::TAsset::ExportUnknownData(TBinaryTree& Data)
{
	//	our root data should not have data in it
	if ( m_Data.GetSize() )
	{
		TLDebug_Break("warning: the root m_Data in an asset should NOT have data in it - it is not exported and will be lost!");
	}

	TPtrArray<TBinaryTree>& DataChildren = m_Data.GetChildren();

	//	add the asset's child data to the exporting data
	for ( u32 i=0;	i<DataChildren.GetSize();	i++ )
	{
		TPtr<TBinaryTree>& pChild = DataChildren[i];
		Data.AddChild( pChild );
	}

}


//----------------------------------------------------
//	fetch data of a specific ref
//----------------------------------------------------
TPtr<TBinaryTree>& TLAsset::TAsset::GetData(TRefRef DataRef,Bool CreateNew)
{
	TPtr<TBinaryTree>& pData = m_Data.GetChild(DataRef);
	if ( pData || !CreateNew )
		return pData;

	//	doesn't exist and we need to create it
	TPtr<TBinaryTree> pNewData = new TBinaryTree( DataRef );
	return m_Data.AddChild( pNewData );
}



//----------------------------------------------------
//	save asset data to binary data - base type just exports dumb m_Data
//----------------------------------------------------
SyncBool TLAsset::TAsset::ExportData(TBinaryTree& Data)
{
	//	copy all our lost/dumb data into the data to be exported
	if ( !Data.ReferenceDataTree( m_Data, FALSE ) )
		return SyncFalse;

	return SyncTrue;
}



//----------------------------------------------------
//	load asset data out binary data - base type just imports dumb m_Data
//----------------------------------------------------
SyncBool TLAsset::TAsset::ImportData(TBinaryTree& Data)
{
	//	copy all our lost/dumb data into the data to be exported
	if ( !m_Data.ReferenceDataTree( Data, FALSE ) )
		return SyncFalse;

	return SyncTrue;
}

