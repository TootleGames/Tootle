#include "TPath.h"






TLPath::TPathNode::TPathNode(TRefRef NodeRef,const float2& NodePos) :
	m_NodeRef	( NodeRef ),
	m_Position	( NodePos )
{

}


//--------------------------------------------------
//	
//--------------------------------------------------
Bool TLPath::TPathNode::ImportData(TBinaryTree& Data)
{
	if ( !Data.Read( m_NodeRef ) )	
		return FALSE;

	if ( !Data.ImportData( "Pos", m_Position ) )	
		return FALSE;

	if ( Data.ImportArrays( "Links", m_Links ) == SyncFalse )
		return FALSE;

	return TRUE;
}



//--------------------------------------------------
//	
//--------------------------------------------------
Bool TLPath::TPathNode::ExportData(TBinaryTree& Data)
{
	Data.Write( m_NodeRef );
	Data.ExportData( "Pos", m_Position );
	Data.ExportArray( "Links", m_Links );

	return TRUE;
}




TLAsset::TPathNetwork::TPathNetwork(TRefRef AssetRef) :
	TAsset	( "PathNetwork", AssetRef )
{
}


//--------------------------------------------------
//	return a ref for a node that isn't currently used
//--------------------------------------------------
TRef TLAsset::TPathNetwork::GetFreeNodeRef(TRef FromRef) const
{
	if ( !FromRef.IsValid() )
		FromRef.Increment();

	while ( m_Nodes.Exists(FromRef) )
		FromRef.Increment();

	return FromRef;
}

//--------------------------------------------------
//	create a new node, returns NULL if it fails, e.g. if NodeRef already exists
//--------------------------------------------------
TLPath::TPathNode* TLAsset::TPathNetwork::AddNode(TRef NodeRef,const float2& NodePos)
{
	//	no ref specified, find an unused noderef
	if ( !NodeRef.IsValid() )
	{
		NodeRef = GetFreeNodeRef();
	}
	else if ( m_Nodes.Exists( NodeRef ) ) //	check if this ref is already used
	{
		TLDebug_Break("NodeRef already used in PathNetwork");
		return NULL;
	}

	TLPath::TPathNode NewNode( NodeRef, NodePos );
	TLDebug::CheckFloatType( NodePos, __FUNCTION__ );

	//	add to list
	s32 Index = m_Nodes.Add( NewNode );
	if ( Index == -1 )
		return NULL;

	OnNodeAdded( m_Nodes[Index] );


	return &m_Nodes[Index];
}


//--------------------------------------------------
//	remove node and clear up links
//--------------------------------------------------
void TLAsset::TPathNetwork::RemoveNode(TRef NodeRef)
{
	TLPath::TPathNode* pNode = GetNode( NodeRef );
	if ( !pNode )
		return;

	//	clean up all the links
	for ( s32 l=pNode->GetLinks().GetLastIndex();	l>=0;	l-- )
	{
		TLPath::TPathNode* pLinkNode = GetNode( pNode->GetLinks().ElementAt(l) );
		UnlinkNodes( *pNode, *pLinkNode );
	}

	//	delete node
	m_Nodes.Remove( NodeRef );
}


//--------------------------------------------------
//	callback when node added
//--------------------------------------------------
void TLAsset::TPathNetwork::OnNodeAdded(TLPath::TPathNode& Node)
{
	m_BoundsBox.Accumulate( Node.GetPosition() );
	m_BoundsSphere.Accumulate( Node.GetPosition() );
}


//--------------------------------------------------
//	
//--------------------------------------------------
void TLAsset::TPathNetwork::LinkNodes(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB)
{
	if ( NodeA == NodeB )
	{
		TLDebug_Break("Attempting to link node to self");
		return;
	}

	Bool Changed = FALSE;
	Changed |= NodeA.AddLink( NodeB );
	Changed |= NodeB.AddLink( NodeA );

	//	callback
	if ( Changed )
		OnNodesLinked( NodeA, NodeB );
}


//--------------------------------------------------
//	returns FALSE if they weren't linked
//--------------------------------------------------
Bool TLAsset::TPathNetwork::UnlinkNodes(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB)
{
	Bool Changed = FALSE;
	Changed |= NodeA.RemoveLink( NodeB );
	Changed |= NodeB.RemoveLink( NodeA );

	//	callback if changed
	if ( Changed )
		OnNodesUnlinked( NodeA, NodeB );
	
	return Changed;
}


//--------------------------------------------------
//	split a line, adds a new node in between these two - returns ref of new node
//--------------------------------------------------
TLPath::TPathNode* TLAsset::TPathNetwork::DivideLink(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB,float2* pDividePos)
{
	//	unlink the nodes
	if ( !UnlinkNodes( NodeA, NodeB ) )
	{
		TLDebug_Break("Trying to divide link between two nodes that aren't linked");
		return NULL;
	}

	TLPath::TPathNode* pNewNode = NULL;

	//	create new node if it doesnt exist
	if ( !pNewNode )
	{
		//	work out position
		float2 MidPos;
		if ( pDividePos )
			MidPos = *pDividePos;
		else
			MidPos = (NodeA.GetPosition() + NodeB.GetPosition()) * 0.5f;

		//	get ref of new node
		TRef NewNodeRef = GetFreeNodeRef( NodeA.GetNodeRef() );

		//	create new node
		pNewNode = AddNode( NewNodeRef, MidPos );
	}

	//	failed to create
	if ( !pNewNode )
		return NULL;

	//	link the new node to the other two
	LinkNodes( NodeA, *pNewNode );
	LinkNodes( *pNewNode, NodeB );

	//	call back
	OnNodeLinkDivided( NodeA, *pNewNode, NodeB );

	return pNewNode;
}


//--------------------------------------------------
//	
//--------------------------------------------------
SyncBool TLAsset::TPathNetwork::ImportData(TBinaryTree& Data)
{
	Data.ImportData("Box2D", m_BoundsBox );
	Data.ImportData("Sph2D", m_BoundsSphere );

	//	import children
	TPtrArray<TBinaryTree> NodeDataArray;
	Data.GetChildren("Node", NodeDataArray );
	m_Nodes.SetAllocSize( NodeDataArray.GetSize() );

	for ( u32 n=0;	n<NodeDataArray.GetSize();	n++ )
	{
		//	alloc node
		TLPath::TPathNode NewNode;

		//	import data
		TPtr<TBinaryTree>& pNodeData = NodeDataArray[n];
		pNodeData->ResetReadPos();
		if ( !NewNode.ImportData( *pNodeData ) )
			return SyncFalse;

		//	add to array of nodes
		m_Nodes.Add( NewNode );
	}

	//	import other data
	ImportUnknownData( Data );

	return SyncTrue;
}


//--------------------------------------------------
//	
//--------------------------------------------------
SyncBool TLAsset::TPathNetwork::ExportData(TBinaryTree& Data)
{
	if ( m_BoundsBox.IsValid() )
		Data.ExportData("Box2D", m_BoundsBox );

	if ( m_BoundsSphere.IsValid() )
		Data.ExportData("Sph2D", m_BoundsSphere );

	//	export nodes
	for ( u32 n=0;	n<m_Nodes.GetSize();	n++ )
	{
		TLPath::TPathNode& Node = m_Nodes[n];

		//	add data for node
		TPtr<TBinaryTree>& pNodeData = Data.AddChild("Node");

		if ( !Node.ExportData( *pNodeData ) )
			return SyncFalse;
	}

	//	import other data
	ExportUnknownData( Data );

	return SyncTrue;
}






TLPath::TPath::TPath(TRefRef PathNetwork) :
	m_PathNetwork	( PathNetwork )
{
}

