#include "TAsset.h"
#include <TootleFileSys/TLFileSys.h>
#include <TootleFileSys/TFileAsset.h>
#include "TLAsset.h"



//----------------------------------------------------------
//	
//----------------------------------------------------------
TLAsset::TAsset::TAsset(TRefRef AssetType,TRefRef AssetRef) :
	m_AssetAndTypeRef	( AssetRef, AssetType ),
	m_LoadingState		( TLAsset::LoadingState_Init ),
	m_Data				( STRef4(D,a,t,a) )
{
	m_AssetAndTypeRef.GetString( m_Debug_AssetRefString );
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

/*
	TTempString Debug_String("Asset ");
	this->GetAssetAndTypeRef().GetString( Debug_String );
	Debug_String.Append(" importing asset file data below");
	TLDebug_Print( Debug_String );
	pAssetFile->GetData().Debug_PrintTree();
*/

	//	reset read pos
	pAssetFile->GetAssetData().ResetReadPos();

	//	import the data
	SyncBool ImportState = ImportData( pAssetFile->GetAssetData() );
	
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
	TLFileSys::TFileAsset& AssetFile = *pAssetFile;

	//	ensure the asset is loaded before we can export it
	TLAsset::TLoadingState AssetLoadingState = GetLoadingState();
		
	//	if not loaded yet, cannot export
	if ( AssetLoadingState == TLAsset::LoadingState_Loading )
		return SyncWait;

	//	failed to load, cannot export
	if ( AssetLoadingState == TLAsset::LoadingState_Failed || AssetLoadingState == TLAsset::LoadingState_Init )
		return SyncFalse;

	//	setup asset file header
	TLFileSys::TFileAsset::Header& Header = AssetFile.GetHeader();
	Header.m_AssetType = GetAssetType();
	Header.m_TootFileRef = TLFileSys::g_TootFileRef;

	//	clear existing data (this should always be empty on a fresh export)
	AssetFile.GetAssetData().Empty();

	//	export data
	SyncBool ExportResult = ExportData( AssetFile.GetAssetData()  );

	//	asset file's tree is updated
	if ( ExportResult == SyncTrue )
	{
		AssetFile.SetNeedsExport(TRUE);
	}

	return ExportResult;
}


//----------------------------------------------------
//	take any data in this binary tree that we didn't read after importing and put it into this asset's data (m_Data)
//----------------------------------------------------
void TLAsset::TAsset::ImportUnknownData(TBinaryTree& Data)
{
	m_Data.AddUnreadChildren( Data, FALSE );
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
	if ( !Data.ReferenceDataTree( m_Data ) )
		return SyncFalse;

	return SyncTrue;
}



//----------------------------------------------------
//	load asset data out binary data - base type just imports dumb m_Data
//----------------------------------------------------
SyncBool TLAsset::TAsset::ImportData(TBinaryTree& Data)
{
	//	copy all our lost/dumb data into the data to be exported
	if ( !m_Data.ReferenceDataTree( Data ) )
		return SyncFalse;

	return SyncTrue;
}


//----------------------------------------------------
//	asset is being removed from the system, send notification to our subscribers
//----------------------------------------------------
void TLAsset::TAsset::OnRemoved()
{
	if ( !HasSubscribers() )
		return;

	//	send the "ASSetReMoved" message
	TLMessaging::TMessage Message( TRef_Static(A,s,s,R,m), GetAssetRef() );
	
	//	send asset type info so we can distinguish between two assets with the same ref
	Message.ExportData( TRef_Static4(T,y,p,e), GetAssetType() );

	PublishMessage( Message );
}


//----------------------------------------------------
//	contents of this asset have changed. send out a notification, then tell the asset manager to send out a message too. 
//	todo: expand to pass in TBinaryTree data to send with the message
//----------------------------------------------------
void TLAsset::TAsset::OnChanged()
{
	if ( HasSubscribers() )
	{
		//	send the "ASSetCHanged" message
		TLMessaging::TMessage Message( TRef_Static(A,s,s,C,h), GetAssetRef() );

		//	send asset type info so we can distinguish between two assets with the same ref
		Message.ExportData( TRef_Static4(T,y,p,e), GetAssetType() );

		PublishMessage( Message );
	}

	//	tell asset manager to send out notification too
	TLAsset::g_pManager->OnAssetChanged( *this );
}

