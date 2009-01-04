#include "TFileScheme.h"
#include <TootleAsset/TScheme.h>
#include <TootleCore/TBinaryTree.h>



namespace TLString
{
	Bool	ReadNextFloatArray(const TString& String,u32& CharIndex,float* pFloats,u32 FloatSize);
};


namespace TLFileScheme
{
	enum DataType
	{
		DataType_Invalid = 0,	//	invalid type
		DataType_Float,
		DataType_Float2,
		DataType_Float3,
		DataType_Float4,
		DataType_Quaternion = DataType_Float4,
		DataType_Colour = DataType_Float4,
		DataType_String,		//	plain string
		DataType_WideString,	//	plain string in wide chars, represented in hex
		DataType_TRef,			//	string -> ref
		DataType_Hex,			//	raw binary data in hex
	};

	DataType	GetDataTypeFromString(const TString& String);
	SyncBool	ImportBinaryData(TPtr<TXmlTag>& pTag,TBinary& BinaryData,TLFileScheme::DataType Type);
}



TLFileScheme::DataType TLFileScheme::GetDataTypeFromString(const TString& String)
{
	if ( String == "float" )		return TLFileScheme::DataType_Float;
	if ( String == "float2" )		return TLFileScheme::DataType_Float2;
	if ( String == "float3" )		return TLFileScheme::DataType_Float3;
	if ( String == "float4" )		return TLFileScheme::DataType_Float4;
	if ( String == "quaternion" )	return TLFileScheme::DataType_Quaternion;
	if ( String == "colour" )		return TLFileScheme::DataType_Colour;
	if ( String == "string" )		return TLFileScheme::DataType_String;
	if ( String == "widestring" )	return TLFileScheme::DataType_WideString;
	if ( String == "tref" )			return TLFileScheme::DataType_TRef;
	if ( String == "hex" )			return TLFileScheme::DataType_Hex;
	
	//	unknown type
	TLDebug_Break("Unsupported data type in scheme file");

	return TLFileScheme::DataType_Invalid;
}


//--------------------------------------------------------
//	
//--------------------------------------------------------
SyncBool TLFileScheme::ImportBinaryData(TPtr<TXmlTag>& pTag,TBinary& BinaryData,TLFileScheme::DataType Type)
{
	//	grab data string
	const TString& DataString = pTag->GetDataString();
	u32 CharIndex = 0;

	switch ( Type )
	{
		case DataType_Float:
		{
			float f;
			if ( !TLString::ReadNextFloatArray( DataString, CharIndex, &f, 1 ) )
				return SyncFalse;
			BinaryData.Write( f );
			return SyncTrue;
		}

		case DataType_Float2:
		{
			float2 f;
			if ( !TLString::ReadNextFloatArray( DataString, CharIndex, f.GetData(), f.GetSize() ) )
				return SyncFalse;
			BinaryData.Write( f );
			return SyncTrue;
		}

		case DataType_Float3:
		{
			float3 f;
			if ( !TLString::ReadNextFloatArray( DataString, CharIndex, f.GetData(), f.GetSize() ) )
				return SyncFalse;
			BinaryData.Write( f );
			return SyncTrue;
		}

		case DataType_Float4:
		{
			float4 f;
			if ( !TLString::ReadNextFloatArray( DataString, CharIndex, f.GetData(), f.GetSize() ) )
				return SyncFalse;
			BinaryData.Write( f );
			return SyncTrue;
		}

		case DataType_TRef:
		{
			TRef Ref( DataString );
			BinaryData.Write( Ref );
			return SyncTrue;
		}

		default:
			TLDebug_Break("Unsupported data type in scheme file");
			return SyncFalse;
	}
}




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
		return ImportResult;

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
				<Data DataRef="Translate"><float3>0,40,0</float3></Data>
				<Data DataRef="MeshRef"><TRef>LeftArrow</TRef></Data>
			</Node>
			<Node NodeRef="RightArrow">
				<Data DataRef="Translate"><float3>40,40,0</float3></Data>
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
				<Data DataRef="Translate"><float3>0,40,0</float3></Data>
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

		//	ignore comments
		if ( pChildTag->GetTagType() == TLXml::TagType_Hidden )
			continue;

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
				<Data DataRef="Translate"><float3>0,40,0</float3></Data>
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
		<Data DataRef="Translate"><float3>0,40,0</float3></Data>
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
			TagImportResult = TLFileScheme::ImportBinaryData( pChildTag, NodeData, TLFileScheme::GetDataTypeFromString( pChildTag->GetTagName() ) );
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








