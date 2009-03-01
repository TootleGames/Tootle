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
	m_LoadingState	( TLAsset::LoadingState_Init )
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
		if ( pChild->GetReadPos() >= 0 )
			continue;

		//	gr: does this need to clone?
		//	save this child
		m_Data.Add( pChild );
	}

}


//----------------------------------------------------
//	write out our unknown data
//----------------------------------------------------
void TLAsset::TAsset::ExportUnknownData(TBinaryTree& Data)
{
	for ( u32 i=0;	i<m_Data.GetSize();	i++ )
	{
		TPtr<TBinaryTree>& pChild = m_Data[i];

		TPtr<TBinaryTree> pNewChild = Data.AddChild( pChild->GetDataRef() );

		pNewChild->ReferenceDataTree( pChild );
	}

}


//----------------------------------------------------
//	fetch data of a specific ref
//----------------------------------------------------
TPtr<TBinaryTree>& TLAsset::TAsset::GetData(TRefRef DataRef,Bool CreateNew)
{
	TPtr<TBinaryTree>& pData = m_Data.FindPtr( DataRef );
	if ( pData || !CreateNew )
		return pData;

	//	doesn't exist and we need to create it
	TPtr<TBinaryTree> pNewData = new TBinaryTree( DataRef );
	return m_Data.AddPtr( pNewData );
}
