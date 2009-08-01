#include "TScheme.h"





TLAsset::TSchemeNode::TSchemeNode(TRefRef NodeRef,TRefRef GraphRef,TRefRef TypeRef) :
	TBinaryTree	( NodeRef ),
	m_GraphRef	( GraphRef ),
	m_TypeRef	( TypeRef )
{
}


//----------------------------------------------------
//	load asset data out binary data
//----------------------------------------------------
SyncBool TLAsset::TSchemeNode::ImportData(TBinaryTree& Data)
{
	Data.ResetReadPos();

	//	root data contains ref's
	TRef NodeRef;
	if ( !Data.Read( NodeRef ) )	return SyncFalse;
	SetNodeRef( NodeRef );

	if ( !Data.Read( m_TypeRef ) )	return SyncFalse;
	if ( !Data.Read( m_GraphRef ) )	return SyncFalse;

	//	go through the tree and import child nodes and data
	TPtrArray<TBinaryTree>& DataChildren = Data.GetChildren();
	for ( u32 c=0;	c<DataChildren.GetSize();	c++ )
	{
		TPtr<TBinaryTree>& pChildData = DataChildren[c];
		if ( pChildData->GetDataRef() == "Node" )
		{
			//	import this node
			TPtr<TSchemeNode> pChildNode = new TSchemeNode();
			SyncBool ImportResult = pChildNode->ImportData( *pChildData.GetObject() );
			if ( ImportResult == SyncWait )
			{
				if ( !TLDebug_Break("Scheme Async import not yet supported") )
					return SyncFalse;
			}

			if ( ImportResult == SyncFalse )
				return SyncFalse;

			//	imported okay, add as child
			m_Children.Add( pChildNode );
		}
		else
		{
			//	just data, add to data tree
			GetData().AddChild( pChildData );
		}
	}

	return SyncTrue;
}
	
		
//----------------------------------------------------
//	save asset data to binary data
//----------------------------------------------------
SyncBool TLAsset::TSchemeNode::ExportData(TBinaryTree& Data)
{
	//	write ref's to root data
	Data.Write( GetNodeRef() );
	Data.Write( m_TypeRef );
	Data.Write( m_GraphRef );

	//	write our data
	u32 c;
	TPtrArray<TBinaryTree>& DataChildren = GetData().GetChildren();
	for ( c=0;	c<DataChildren.GetSize();	c++ )
	{
		Data.AddChild( DataChildren[c] );
	}

	//	write our child nodes to the data
	for ( c=0;	c<m_Children.GetSize();	c++ )
	{
		TPtr<TSchemeNode>& pChildNode = m_Children[c];
		TPtr<TBinaryTree> pChildNodeData = new TBinaryTree("Node");

		SyncBool ExportResult = pChildNode->ExportData( *pChildNodeData.GetObject() );
		if ( ExportResult == SyncWait )
		{
			if ( !TLDebug_Break("Scheme Async export not yet supported") )
				return SyncFalse;
		}

		if ( ExportResult == SyncFalse )
			return SyncFalse;

		Data.AddChild( pChildNodeData );
	}

	return SyncTrue;
}



TLAsset::TScheme::TScheme(TRefRef AssetRef) :
	TLAsset::TAsset( GetAssetType_Static(), AssetRef )
{
}


//----------------------------------------------------
//	save asset data to binary data
//----------------------------------------------------
SyncBool TLAsset::TScheme::ExportData(TBinaryTree& Data)
{
	//	write our child nodes to the data
	for ( u32 c=0;	c<m_Nodes.GetSize();	c++ )
	{
		TPtr<TSchemeNode>& pChildNode = m_Nodes[c];
		TPtr<TBinaryTree> pChildNodeData = new TBinaryTree("Node");

		SyncBool ExportResult = pChildNode->ExportData( *pChildNodeData.GetObject() );
		if ( ExportResult == SyncWait )
		{
			if ( !TLDebug_Break("Scheme Async export not yet supported") )
				return SyncFalse;
		}

		if ( ExportResult == SyncFalse )
			return SyncFalse;

		Data.AddChild( pChildNodeData );
	}

	//	write back any data we didn't recognise
	ExportUnknownData( Data );

	return SyncTrue;
}


//----------------------------------------------------
//	load asset data out binary data
//----------------------------------------------------
SyncBool TLAsset::TScheme::ImportData(TBinaryTree& Data)
{
	//	get an array of all the scheme nodes
	TPtrArray<TBinaryTree> SchemeNodes;
	Data.GetChildren("Node", SchemeNodes );

	//	import nodes
	for ( u32 n=0;	n<SchemeNodes.GetSize();	n++ )
	{
		TPtr<TSchemeNode> pChildNode = new TSchemeNode();
		SyncBool ImportResult = pChildNode->ImportData( *SchemeNodes[n].GetObject() );

		if ( ImportResult == SyncWait )
		{
			if ( !TLDebug_Break("Scheme Async import not yet supported") )
				return SyncFalse;
		}

		if ( ImportResult == SyncFalse )
			return SyncFalse;

		//	imported okay, add to list
		m_Nodes.Add( pChildNode );
	}

	//	store off any data we haven't read to keep this data intact
	ImportUnknownData( Data );

	return SyncTrue;
}
