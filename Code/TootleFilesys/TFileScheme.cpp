#include "TFileScheme.h"
#include <TootleAsset/TScheme.h>
#include <TootleCore/TBinaryTree.h>
#include "TLFile.h"



TLFileSys::TFileScheme::TFileScheme(TRefRef FileRef,TRefRef FileTypeRef) :
	TFileXml				( FileRef, FileTypeRef )
{
}



//--------------------------------------------------------
//	import the XML
//--------------------------------------------------------
SyncBool TLFileSys::TFileScheme::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported)
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
	{
		if ( ImportResult == SyncFalse )
		{
			TLDebug_Break("Failed to parse xml file");
		}
		return ImportResult;
	}

	//	get the root tag
	TPtr<TXmlTag> pRootTag = m_XmlData.GetChild("Scheme");

	//	malformed XML
	if ( !pRootTag )
	{
		TLDebug_Print("Scheme file missing root <Scheme> tag");
		return SyncFalse;
	}

	//	make up new storage asset type
	TPtr<TLAsset::TScheme> pNewAsset = new TLAsset::TScheme( GetFileRef() );
	ImportResult = ImportScheme( pRootTag, pNewAsset );

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
SyncBool TLFileSys::TFileScheme::ImportScheme(TPtr<TXmlTag>& pTag,TPtr<TLAsset::TScheme>& pScheme)
{
	/*
<Scheme>
	<Graph GraphRef="Render">
		<Node NodeRef="GuiRoot">
			<Node NodeRef="LeftArrow">
				<Data DataRef=TRef_Static(T,r,a,n,s)><float3>0,40,0</float3></Data>
				<Data DataRef="MeshRef"><TRef>LeftArrow</TRef></Data>
			</Node>
			<Node NodeRef="RightArrow">
				<Data DataRef=TRef_Static(T,r,a,n,s)><float3>40,40,0</float3></Data>
				<Data DataRef="MeshRef"><TRef>RightArrow</TRef></Data>
			</Node>
		</Node>
	</Graph>
</Scheme>
	*/
	
	//	deal with child tags
	for ( u32 c=0;	c<pTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pTag->GetChildren().ElementAt(c);

		SyncBool TagImportResult = SyncFalse;
		if ( pChildTag->GetTagName() == "graph" )
		{
			TagImportResult = ImportGraph( pChildTag, pScheme );
		}
		else
		{
			TLDebug_Break("Unsupported tag in Scheme import");
		}

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async Scheme import");
			return SyncFalse;
		}
	}
	
	return SyncTrue;
}

//--------------------------------------------------------
//	
//--------------------------------------------------------
SyncBool TLFileSys::TFileScheme::ImportGraph(TPtr<TXmlTag>& pTag,TPtr<TLAsset::TScheme>& pScheme)
{
	/*
	<Graph GraphRef="Render">
		<Node NodeRef="GuiRoot">
			<Node NodeRef="LeftArrow">
				<Data DataRef=TRef_Static(T,r,a,n,s)><float3>0,40,0</float3></Data>
				<Data DataRef="MeshRef"><TRef>LeftArrow</TRef></Data>
			</Node>
		</Node>
	</Graph>
	*/
	//	read the graph ref
	const TString* pRefString = pTag->GetProperty("GraphRef");
	if ( !pRefString )
	{
		TLDebug_Break("Expected GraphRef property in <graph> tag");
		return SyncFalse;
	}

	TRef GraphRef( *pRefString );
	
	//	deal with child tags
	for ( u32 c=0;	c<pTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pTag->GetChildren().ElementAt(c);

		SyncBool TagImportResult = SyncFalse;
		if ( pChildTag->GetTagName() == "node" )
		{
			TagImportResult = ImportNode( pChildTag, GraphRef, pScheme, NULL );
		}
		else
		{
			TLDebug_Break("Unsupported tag in Scheme import");
		}

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async Scheme import");
			return SyncFalse;
		}
	}
	
	return SyncTrue;
}

//--------------------------------------------------------
//	
//--------------------------------------------------------
SyncBool TLFileSys::TFileScheme::ImportNode(TPtr<TXmlTag>& pTag,TRefRef GraphRef,TPtr<TLAsset::TScheme>& pScheme,TPtr<TLAsset::TSchemeNode> pParentNode)
{
	/*
		<Node NodeRef="GuiRoot">
			<Node NodeRef="LeftArrow">
				<Data DataRef=TRef_Static(T,r,a,n,s)><float3>0,40,0</float3></Data>
				<Data DataRef="MeshRef"><TRef>LeftArrow</TRef></Data>
			</Node>
		</Node>
	*/

	//	read the node ref
	const TString* pNodeRefString = pTag->GetProperty("NodeRef");
	TRef NodeRef( pNodeRefString ? *pNodeRefString : "" );

	//	read the node type ref
	const TString* pTypeRefString = pTag->GetProperty("TypeRef");
	TRef TypeRef( pTypeRefString ? *pTypeRefString : "" );

	//	create node
	TPtr<TLAsset::TSchemeNode> pNode = new TLAsset::TSchemeNode( NodeRef, GraphRef, TypeRef );
	
	//	add to parent if there is one, otherwise directly to scheme
	if ( pParentNode )
		pParentNode->AddChild( pNode );
	else
		pScheme->AddNode( pNode );

	//	deal with child tags
	for ( u32 c=0;	c<pTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pTag->GetChildren().ElementAt(c);
	
		SyncBool TagImportResult = SyncFalse;
		if ( pChildTag->GetTagName() == "node" )
		{
			TagImportResult = ImportNode( pChildTag, GraphRef, pScheme, pNode );
		}
		else if ( pChildTag->GetTagName() == "data" )
		{
			TagImportResult = ImportNode_Data( pChildTag, pNode );
		}
		else
		{
			TLDebug_Break("Unsupported tag in Scheme import");
		}

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async Scheme import");
			return SyncFalse;
		}
	}
	
	return SyncTrue;
}


//--------------------------------------------------------
//	
//--------------------------------------------------------
SyncBool TLFileSys::TFileScheme::ImportNode_Data(TPtr<TXmlTag>& pTag,TPtr<TLAsset::TSchemeNode>& pNode)
{
	/*
		<Data DataRef=TRef_Static(T,r,a,n,s)><float3>0,40,0</float3></Data>
	*/

	//	read the data ref
	const TString* pDataRefString = pTag->GetProperty("DataRef");
	TRef DataRef( pDataRefString ? *pDataRefString : "" );

	//	establish the data we're writing data to
	TBinaryTree& NodeRootData = pNode->GetData();
	TPtr<TBinaryTree> pDataChild;

	//	add a child to the node data if it has a ref, otherwise data is added to root of the node
	if ( DataRef.IsValid() )
	{
		pDataChild = NodeRootData.AddChild( DataRef );
		//	failed to add child data...
		if ( !pDataChild )
		{
			TLDebug_Break("failed to add child data");
			return SyncFalse;
		}
	}

	//	import contents of data
	TBinaryTree& NodeData = pDataChild ? *pDataChild.GetObject() : NodeRootData;


	//	deal with child tags
	for ( u32 c=0;	c<pTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pTag->GetChildren().ElementAt(c);

		SyncBool TagImportResult = SyncFalse;

		if ( pChildTag->GetTagName() == "data" )
		{
			//	import child data
			TLDebug_Break("Todo");
		}
		else
		{
			TRef DataTypeRef = TLFile::GetDataTypeFromString( pChildTag->GetTagName() );

			//	update type of data
			NodeData.SetDataTypeHint( DataTypeRef );

			TagImportResult = TLFile::ImportBinaryData( pChildTag, NodeData, DataTypeRef );
		}

		//	failed
		if ( TagImportResult == SyncFalse )
		{
			TLDebug_Break( TString("failed to import <data> tag \"%s\" in scheme", pChildTag->GetTagName() ) );
			return SyncFalse;
		}

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async Scheme import");
			return SyncFalse;
		}
	}
	
	return SyncTrue;
}








