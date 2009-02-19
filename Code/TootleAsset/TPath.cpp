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
	TAsset	( AssetRef, "PathNetwork" )
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
//	create a new road node
//--------------------------------------------------
TLPath::TPathNode* TLAsset::TPathNetwork::AddNode(TRef NodeRef,const float2& NodePos)
{
	//	no ref specified, find an unused noderef
	if ( !NodeRef.IsValid() )
		NodeRef = GetFreeNodeRef();

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
//
//--------------------------------------------------
void TLAsset::TPathNetwork::UnlinkNodes(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB)
{
	Bool Changed = FALSE;
	Changed |= NodeA.RemoveLink( NodeB );
	Changed |= NodeB.RemoveLink( NodeA );

	//	callback if changed
	if ( Changed )
		OnNodesUnlinked( NodeA, NodeB );
}


//--------------------------------------------------
//	split a line, adds a new node in between these two - returns ref of new node
//--------------------------------------------------
TRef TLAsset::TPathNetwork::DivideLink(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB,float2* pDividePos)
{
	//	unlink the nodes
	UnlinkNodes( NodeA, NodeB );

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
		return TRef();

	//	link the new node to the other two
	LinkNodes( NodeA, *pNewNode );
	LinkNodes( *pNewNode, NodeB );

	//	call back
	OnNodeLinkDivided( NodeA, *pNewNode, NodeB );

	return pNewNode->GetNodeRef();
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

