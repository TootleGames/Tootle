#include "TPath.h"



namespace TLPath
{
	TLArray::SortResult	NodeSort_ByRef(const TPtr<TPathNode>& a,const TPtr<TPathNode>& b,const void* pTestRef);	//	node sorting by ref
};


//----------------------------------------------------------
//	node sorting by ref
//----------------------------------------------------------
TLArray::SortResult	TLPath::NodeSort_ByRef(const TPtr<TLPath::TPathNode>& a,const TPtr<TLPath::TPathNode>& b,const void* pTestRef)
{
	const TRef& aRef = a->GetNodeRef();
	const TRef& bRef = pTestRef ? *(const TRef*)pTestRef : b->GetNodeRef();
	
	//	== turns into 0 (is greater) or 1(equals)
	return aRef < bRef ? TLArray::IsLess : (TLArray::SortResult)(aRef==bRef);	
}




TLPath::TPathNode::TPathNode(TRefRef NodeRef,const float2& NodePos) :
	TBinaryTree	( NodeRef ),
	m_Position	( NodePos )
{

}


//--------------------------------------------------
//	
//--------------------------------------------------
Bool TLPath::TPathNode::ImportData(TBinaryTree& Data)
{
	if ( !Data.Read( GetNodeRef() ) )	
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
	Data.Write( GetNodeRef() );
	Data.ExportData( "Pos", m_Position );
	Data.ExportArray( "Links", m_Links );

	return TRUE;
}




TLAsset::TPathNetwork::TPathNetwork(TRefRef AssetRef) :
	TAsset	( "PathNetwork", AssetRef ),
	m_Nodes	( &TLPath::NodeSort_ByRef )
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
//	non-const version so sorting is availible
//--------------------------------------------------
TRef TLAsset::TPathNetwork::GetFreeNodeRef(TRef FromRef)
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
TPtr<TLPath::TPathNode>& TLAsset::TPathNetwork::AddNode(TRef NodeRef,const float2& NodePos)
{
	//	no ref specified, find an unused noderef
	if ( !NodeRef.IsValid() )
	{
		NodeRef = GetFreeNodeRef();
	}
	else if ( m_Nodes.Exists( NodeRef ) ) //	check if this ref is already used
	{
		TLDebug_Break("NodeRef already used in PathNetwork");
		return TLPtr::GetNullPtr<TLPath::TPathNode>();
	}

	TPtr<TLPath::TPathNode> pNewNode = new TLPath::TPathNode( NodeRef, NodePos );
	TLDebug::CheckFloatType( NodePos, __FUNCTION__ );

	//	add to list
	TPtr<TLPath::TPathNode>& pNewNodeRef = m_Nodes.AddPtr( pNewNode );
	if ( pNewNodeRef )
	{
		OnNodeAdded( pNewNodeRef );
	}

	return pNewNodeRef;
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
void TLAsset::TPathNetwork::OnNodeAdded(TPtr<TLPath::TPathNode>& pNewNode)
{
	m_BoundsBox.Accumulate( pNewNode->GetPosition() );
	m_BoundsSphere.Accumulate( pNewNode->GetPosition() );
}


//--------------------------------------------------
//	
//--------------------------------------------------
void TLAsset::TPathNetwork::LinkNodes(TRefRef NodeARef,TRefRef NodeBRef)
{
	if ( NodeARef == NodeBRef )
	{
		TLDebug_Break("Attempting to link node to self");
		return;
	}

	//	get nodes
	TPtr<TLPath::TPathNode>& pNodeA = GetNode( NodeARef );
	TPtr<TLPath::TPathNode>& pNodeB = GetNode( NodeBRef );

	if ( !pNodeA || !pNodeB )
	{
		TLDebug_Break("Missing Node when linking nodes");
		return;
	}

	//	do link
	LinkNodes( *pNodeA, *pNodeB );
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
TPtr<TLPath::TPathNode>& TLAsset::TPathNetwork::DivideLink(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB,float2* pDividePos)
{
	//	unlink the nodes
	if ( !UnlinkNodes( NodeA, NodeB ) )
	{
		TLDebug_Break("Trying to divide link between two nodes that aren't linked");
		return TLPtr::GetNullPtr<TLPath::TPathNode>();
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
		return TLPtr::GetNullPtr<TLPath::TPathNode>();

	//	link the new node to the other two
	LinkNodes( NodeA, *pNewNode );
	LinkNodes( *pNewNode, NodeB );

	//	call back
	OnNodeLinkDivided( NodeA, *pNewNode, NodeB );

	//	need to get ref back to the new node
	//return pNewNode;
	return GetNode( pNewNode->GetNodeRef() );
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
		TPtr<TLPath::TPathNode> pNewNode = new TLPath::TPathNode();

		//	import data
		TPtr<TBinaryTree>& pNodeData = NodeDataArray[n];
		pNodeData->ResetReadPos();
		if ( !pNewNode->ImportData( *pNodeData ) )
			return SyncFalse;

		//	add to array of nodes
		m_Nodes.Add( pNewNode );
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
		TLPath::TPathNode& Node = *m_Nodes[n];

		//	add data for node
		TPtr<TBinaryTree>& pNodeData = Data.AddChild("Node");

		if ( !Node.ExportData( *pNodeData ) )
			return SyncFalse;
	}

	//	import other data
	ExportUnknownData( Data );

	return SyncTrue;
}


//--------------------------------------------------
//	find the nearest node to this position
//--------------------------------------------------
TPtr<TLPath::TPathNode>& TLAsset::TPathNetwork::GetNearestNode(const float2& Position)
{
	TLDebug_Break("Todo");
	return m_Nodes[0];
}


//--------------------------------------------------
//	change position of a node, returns TRUE if changed and invokes a changed message
//--------------------------------------------------
Bool TLAsset::TPathNetwork::SetNodePosition(TRefRef NodeRef,const float2& NewPos)
{
	TPtr<TLPath::TPathNode>& pPathNode = GetNode( NodeRef );
	if ( !pPathNode )
		return FALSE;
	
	return SetNodePosition( *pPathNode, NewPos );
}


//--------------------------------------------------
//	change position of a node, returns TRUE if changed and invokes a changed message
//--------------------------------------------------
Bool TLAsset::TPathNetwork::SetNodePosition(TLPath::TPathNode& Node,const float2& NewPos)
{
	//	not much of a change
	if ( Node.GetPosition() == NewPos )
		return FALSE;

	//	change pos
	Node.SetPosition( NewPos );

	//	do notification
	OnNodePosChanged( Node );

	return TRUE;
}

