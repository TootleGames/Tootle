#include "TFileMenu.h"
#include <TootleAsset/TMenu.h>
#include <TootleCore/TBinaryTree.h>
#include <TootleGame/TMenu.h>
#include "TLFile.h"





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
	
	//	gr: shouldnt be using the "text=" property. not sure if this was deprecated or if some XML was wrong and it was just duplicated
	pString = pTag->GetProperty("Text");
	if ( pString )
	{
		TLDebug_Break("The Text= property of a menu item's XML should be String=");
	}
	else
	{
		//	string
		pString = pTag->GetProperty("String");
	}

	if ( pString )
		pItem->SetText( *pString );

	//	command ref
	pString = pTag->GetProperty("CommandRef");
	if ( pString )
		pItem->SetMenuCommand( TRef(*pString) );

	//	nextmenu
	pString = pTag->GetProperty("NextMenu");
	if ( pString )
		pItem->SetNextMenu( TRef(*pString) );

	//	mesh/gui node ref
	pString = pTag->GetProperty("GuiRenderNodeRef");
	if ( pString )
	{
		TTempString Debug_String("The GuiRendernodeRef is now deprecated in menu item XML, please store the ref in menu item data - ");
		GetFileAndTypeRef().GetString( Debug_String );
		TLDebug_Break( Debug_String );
		pItem->SetMeshRef( TRef(*pString) );
	}

	pString = pTag->GetProperty("AudioRef");
	if ( pString )
	{
		//TLDebug_Break("The audio ref is now deprecated in menu item XML, please store the ref in menu item data");
		pItem->SetAudioRef( TRef(*pString) );
	}

	//	import child data
	TPtrArray<TXmlTag> DataTags;
	pTag->GetChildren( "Data", DataTags );
	for ( u32 i=0;	i<DataTags.GetSize();	i++ )
	{
		TPtr<TXmlTag>& pDataTag = DataTags[i];

		if ( !TLFile::ParseXMLDataTree( pDataTag, pItem->GetData() ) )
			return SyncFalse;
	}

	/*
	//	find out what we need to do
	for ( u32 c=0;	c<pTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pTag->GetChildren().ElementAt(c);

		SyncBool TagImportResult = ImportMenuItem_ImportData( pMenu, pChildTag);

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async menu import");
			return SyncFalse;
		}
	}
	*/

	return SyncTrue;
}

/*
SyncBool TLFileSys::TFileTimeline::ImporTTimeline_ImportCommandData(TPtr<TLAsset::TMenu>& pMenu, TPtr<TXmlTag>& pImportTag)
{
	// Get data tag and all data properties
	for ( u32 c=0;	c<pImportTag->GetPropertyCount();	c++ )
	{
		const TXmlTag::TProperty& Property = pImportTag->GetPropertyAt(c);

		const TStringLowercase<TTempString>& PropertyName = Property.m_Key;
		const TString& PropertyData = Property.m_Item;

		if ( PropertyName == "DataRef" )
		{
		}
		else if(PropertyName == "State")
		{
		}

		// Now get any extra data once we reach the end of the tag info
		if(pCommandChildData.IsValid() && (c == (pImportTag->GetPropertyCount() - 1)))
		{
			TPtr<TXmlTag>& pChildTag = pImportTag->GetChildren().ElementAt(0);

			if(pChildTag)
			{
				TRef DataTypeRef = TLFile::GetDataTypeFromString( pChildTag->GetTagName() );

				//	update type of data
				pCommandChildData->SetDataTypeHint( DataTypeRef );

				SyncBool TagImportResult = SyncFalse;

				TagImportResult = TLFile::ImportBinaryData( pChildTag, *pCommandChildData, DataTypeRef );

				if(TagImportResult != SyncTrue)
				{
					// Failed to copy the data form the XML file
					TLDebug_Print("Failed to get command data from TTL file");
					return SyncFalse;
				}
			}
		}
	}
	
	return SyncTrue;
}
*/



