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
			SyncBool ImportResult = pChildNode->ImportData( *pChildData.GetObjectPointer() );
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

		SyncBool ExportResult = pChildNode->ExportData( *pChildNodeData.GetObjectPointer() );
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



TLAsset::TScheme::TScheme(TRefRef AssetRef,TRefRef TypeRef) :
	TLAsset::TAsset( TypeRef, AssetRef )
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

		SyncBool ExportResult = pChildNode->ExportData( *pChildNodeData.GetObjectPointer() );
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
	//	pre-alloc number of nodes if availible
	u16 NodeCount = 0;
	if ( !Data.ImportData("NodeCount", NodeCount ) )
		NodeCount = Data.GetChildCount();

	//	pre-alloc
	m_Nodes.AddAllocSize( NodeCount );

	//	rather than fetching the children (which is slow when there's a lot) 
	//	we just walk through the children and do the nodes we want
	TPtrArray<TBinaryTree>& ChildDatas = Data.GetChildren();
	for ( u32 d=0;	d<ChildDatas.GetSize();	d++ )
	{
		//	only interested in nodes...
		TBinaryTree& NodeData = *ChildDatas[d];
		if ( NodeData.GetDataRef() != TRef_Static4(N,o,d,e) )
			continue;

		TPtr<TSchemeNode> pChildNode = new TSchemeNode();
		SyncBool ImportResult = pChildNode->ImportData( NodeData );

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
