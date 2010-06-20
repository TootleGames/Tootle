#include "TPath.h"



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




TLPath::TPathNodeLink::TPathNodeLink() :
	m_Direction	( TLPath::TDirection::Any )
{
}

TLPath::TPathNodeLink::TPathNodeLink(TRefRef LinkNodeRef,TLPath::TDirection::Type Direction) :
	m_LinkNodeRef	( LinkNodeRef ),
	m_Direction		( Direction )
{
}


	


TLAsset::TPathNetwork::TPathNetwork(TRefRef AssetRef) :
	TAsset	( GetAssetType_Static(), AssetRef )
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
		TLPath::TPathNode* pLinkNode = GetNode( pNode->GetLinkRef(l) );
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
//	if OneWayDirection is specified, the one-way direction goes from A to B
//--------------------------------------------------
void TLAsset::TPathNetwork::LinkNodes(TRefRef NodeARef,TRefRef NodeBRef,Bool OneWayDirection)
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
	LinkNodes( *pNodeA, *pNodeB, OneWayDirection );
}


//--------------------------------------------------
//	if OneWayDirection is specified, the one-way direction goes from A to B
//--------------------------------------------------
void TLAsset::TPathNetwork::LinkNodes(TLPath::TPathNode& NodeA,TLPath::TPathNode& NodeB,Bool OneWayDirection)
{
	if ( NodeA == NodeB )
	{
		TLDebug_Break("Attempting to link node to self");
		return;
	}

	Bool Changed = FALSE;
	Changed |= NodeA.AddLink( NodeB, OneWayDirection ? TLPath::TDirection::Forward : TLPath::TDirection::Any );		//	A -> B == Foward
	Changed |= NodeB.AddLink( NodeA, OneWayDirection ? TLPath::TDirection::Backward : TLPath::TDirection::Any );	//	B -> A == Backward

	//	create a path link - ALWAYS in this order - if there is a OneWayDirection it'll be a->b. If not, it doesn't matter
	TLPath::TPathLink NewLink( NodeA, NodeB );
	TLDebug_Assert( NewLink.IsValid(), "New link isn't valid");
	m_Links.AddUnique( NewLink );

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

	//	remove matching link
	TLPath::TPathLink NewLink( NodeA.GetNodeRef(), NodeB.GetNodeRef() );
	m_Links.Remove( NewLink );

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
	//	store the original link
	const TLPath::TPathNodeLink* pLink = NodeA.GetLink( NodeB.GetNodeRef() );
	if ( !pLink )
	{
		TLDebug_Break("Trying to divide link between two nodes that aren't linked");
		return TLPtr::GetNullPtr<TLPath::TPathNode>();
	}

	//	copy it as we're about to delete the link
	TLPath::TPathNodeLink OldLinkAB = *pLink;

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
	//	work out the old direction, and continue it
	if ( OldLinkAB.GetDirection() == TLPath::TDirection::Forward )
	{
		LinkNodes( NodeA, *pNewNode, TRUE );
		LinkNodes( *pNewNode, NodeB, TRUE );
	}
	else if ( OldLinkAB.GetDirection() == TLPath::TDirection::Backward )
	{
		//	reverse order
		LinkNodes( NodeB, *pNewNode, TRUE );
		LinkNodes( *pNewNode, NodeA, TRUE );
	}
	else
	{
		//	no directional link
		LinkNodes( NodeA, *pNewNode, FALSE );
		LinkNodes( *pNewNode, NodeB, FALSE );
	}


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


//--------------------------------------------------
//	
//--------------------------------------------------
void TLAsset::TPathNetwork::OnNodePosChanged(TLPath::TPathNode& Node)
{
	//	re-init data of path links
	for ( u32 i=0;	i<m_Links.GetSize();	i++ )
	{
		TLPath::TPathLink& Link = m_Links[i];
		if ( !Link.HasNode( Node.GetNodeRef() ) )
			continue;

		//	get the "other" node
		//	gr: order of OnNodePosChanged doesnt matter, but in case it matters in future...
		if ( Link.GetNodeA() == Node.GetNodeRef() )
		{
			TPtr<TLPath::TPathNode>& pOtherNode = GetNode( Link.GetNodeB() );
			Link.OnNodePosChanged( Node, *pOtherNode );
		}
		else
		{
			TPtr<TLPath::TPathNode>& pOtherNode = GetNode( Link.GetNodeA() );
			Link.OnNodePosChanged( *pOtherNode, Node );
		}
	}
}






TLPath::TPathLink::TPathLink() :
	m_CacheValid	( FALSE )
{
}


TLPath::TPathLink::TPathLink(TRefRef NodeA,TRefRef NodeB) :
	m_CacheValid	( FALSE )
{
	m_Nodes.Add( NodeA );
	m_Nodes.Add( NodeB );

	TLDebug_Assert( IsValid(), "Nodes provided to TPathLink are invalid" );
}


TLPath::TPathLink::TPathLink(const TPathNode& NodeA,const TPathNode& NodeB) :
	m_CacheValid	( FALSE )
{
	m_Nodes.Add( NodeA.GetNodeRef() );
	m_Nodes.Add( NodeB.GetNodeRef() );

	//	calc cache stuff
	OnNodePosChanged( NodeA, NodeB );
}


//--------------------------------------------------
//	if either node position changes recalc cache info (line/sphere/etc)
//--------------------------------------------------
void TLPath::TPathLink::OnNodePosChanged(const TPathNode& NodeA,const TPathNode& NodeB)
{
	//	set line
	m_LinkLine.Set( NodeA.GetPosition(), NodeB.GetPosition() );

	//	set bounds sphere
	float Radius = m_LinkLine.GetLength() / 2.f;
	m_LinkBoundsSphere.Set( m_LinkLine.GetCenter(), Radius );

	//	all up to date
	m_CacheValid = TRUE;
}


//--------------------------------------------------
//	
//--------------------------------------------------
void TLPath::TPathLink::GetTransformOnLink(TLMaths::TTransform& Transform,float AlongLine)
{
	//	work out translate
	GetLinkLine().GetPointAlongLine( Transform.GetTranslate().xy(), AlongLine );
	Transform.GetTranslate().z = 0.f;
	Transform.SetTranslateValid();

	//	reposition node along the path
	TLMaths::TAngle DirectionAngle = GetLinkLine().GetAngle();
	Transform.SetRotation( TLMaths::TQuaternion( float3( 0.f, 0.f, 1.f ), DirectionAngle.GetRadians() ) );
}


