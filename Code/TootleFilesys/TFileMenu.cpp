#include "TFileMenu.h"
#include <TootleAsset/TMenu.h>
#include <TootleCore/TBinaryTree.h>
#include <TootleGame/TMenu.h>





TLFileSys::TFileMenu::TFileMenu(TRefRef FileRef,TRefRef FileTypeRef) :
	TFileXml				( FileRef, FileTypeRef )
{
}



//--------------------------------------------------------
//	import the XML
//--------------------------------------------------------
SyncBool TLFileSys::TFileMenu::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported)
{
	if ( pAsset )
	{
		TLDebug_Break("Async export not supported yet. asset should be NULL");
		return SyncFalse;
	}

	Supported = TRUE;

	//	import xml
	SyncBool ImportResult = TFileXml::Import();
	if ( ImportResult != SyncTrue )
		return ImportResult;

	//	get the root tag
	TPtr<TXmlTag> pRootTag = m_XmlData.GetChild("Menu");

	//	malformed XML
	if ( !pRootTag )
	{
		TLDebug_Print("Scheme file missing root <Menu> tag");
		return SyncFalse;
	}

	//	make up new storage asset type
	TPtr<TLAsset::TMenu> pNewAsset = new TLAsset::TMenu( GetFileRef() );
	ImportResult = ImportMenu( pRootTag, pNewAsset );

	//	failed to import
	if ( ImportResult != SyncTrue )
	{
		return SyncFalse;
	}

	//	assign resulting asset
	pAsset = pNewAsset;

	return SyncTrue;
}




//--------------------------------------------------------
//	
//--------------------------------------------------------
SyncBool TLFileSys::TFileMenu::ImportMenu(TPtr<TXmlTag>& pTag,TPtr<TLAsset::TMenu>& pMenu)
{
	/*
<Menu SchemeRef="ms_pause"> 
	<Item ItemRef="Close" MenuString="Close" CommandRef="close" ButtonMeshRef="m_close" />
</Menu>

	*/
	//	get scheme ref associated with menu
	const TString* pSchemeRefString = pTag->GetProperty("SchemeRef");
	TRef SchemeRef( pSchemeRefString );
	pMenu->SetSchemeRef( SchemeRef );

	
	for ( u32 c=0;	c<pTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pTag->GetChildren().ElementAt(c);

		SyncBool TagImportResult = SyncFalse;
		if ( pChildTag->GetTagName() == "item" )
		{
			TagImportResult = ImportMenuItem( pChildTag, pMenu );
		}
		else
		{
			TLDebug_Break("Unsupported tag in Menu import");
		}

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async Menu import");
			return SyncFalse;
		}
	}
	
	return SyncTrue;
}

//--------------------------------------------------------
//	
//--------------------------------------------------------
SyncBool TLFileSys::TFileMenu::ImportMenuItem(TPtr<TXmlTag>& pTag,TPtr<TLAsset::TMenu>& pMenu)
{
	/*
	<Item ItemRef="Close" MenuText="Close" CommandRef="close" GuiRenderNodeRef="m_close" />
	*/

	//	todo:
	//		- Move string into a seperate (data) tag so we can do widestring text
	//		- OR, change to ref for the text database

	//	read the graph ref
	const TString* pRefString = pTag->GetProperty("ItemRef");
	if ( !pRefString )
	{
		TLDebug_Break("Expected ItemRef property in <Item> tag");
		return SyncFalse;
	}

	//	create item
	TRef ItemRef( *pRefString );

	TPtr<TLMenu::TMenuItem> pItem = pMenu->AddMenuItem( ItemRef );
	
	//	read off other properties
	const TString* pString = NULL;
	
	//	string
	pString = pTag->GetProperty("Text");
	if ( pString )
		pItem->SetText( *pString );

	//	command ref
	pString = pTag->GetProperty("CommandRef");
	if ( pString )
		pItem->SetMenuCommand( TRef(*pString) );

	//	mesh/gui node ref
	pString = pTag->GetProperty("GuiRenderNodeRef");
	if ( pString )
		pItem->SetMeshRef( TRef(*pString) );

	return SyncTrue;
}



