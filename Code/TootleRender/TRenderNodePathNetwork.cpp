#include "TRenderNodePathNetwork.h"
#include <TootleAsset/TPath.h>




TLRender::TRenderNodePathNetwork::TRenderNodePathNetwork(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNodeDebugMesh	( RenderNodeRef, TypeRef ),
	m_PathColour			( 1.f, 1.f, 1.f, 1.f )
{
}


//---------------------------------------------------------
//	init 
//---------------------------------------------------------
void TLRender::TRenderNodePathNetwork::Initialise(TLMessaging::TMessage& Message)
{
	//	do inherited init first to create mesh etc
	TRenderNodeDebugMesh::Initialise( Message );

	//	debug the points on the path
	GetRenderFlags().Set( TLRender::TRenderNode::RenderFlags::Debug_Points );
	GetRenderFlags().Clear( TLRender::TRenderNode::RenderFlags::EnableCull );

	//	read out colour
	Message.ImportData("Colour", m_PathColour );

	//	see if a path network asset has been specified (do this last!)
	TRef PathNetworkRef;
	if ( Message.ImportData("PathNetwork", PathNetworkRef ) )
		SetPathNetwork( PathNetworkRef );
}



//---------------------------------------------------------
//	change the path network asset
//---------------------------------------------------------
void TLRender::TRenderNodePathNetwork::SetPathNetwork(TRefRef PathNetworkRef)
{
	//	no change
	if ( m_PathNetworkRef == PathNetworkRef )
		return;

	//	clean up old stuff
	GetMeshAsset()->Empty();
	m_PathNodeVertex.Empty();

	//	set new ref
	m_PathNetworkRef = PathNetworkRef;

	//	been set to no-path, so nothing to do
	if ( !m_PathNetworkRef.IsValid() )
		return;

	//	get the path network asset
	TPtr<TLAsset::TPathNetwork> pPathNetwork = TLAsset::GetAsset( m_PathNetworkRef, TRUE );
	if ( !pPathNetwork )
	{
		TLDebug_Break("Missing Path asset for TRenderNodePathNetwork");
		return;
	}

	//	check type
	if ( pPathNetwork->GetAssetType() != "PathNetwork" )
	{
		TLDebug_Break("Path asset for TRenderNodePathNetwork is not a path network");
		return;
	}

	//	create the debug mesh for the asset
	InitMeshFromPathNetwork( *pPathNetwork );
}


//---------------------------------------------------------
//	catch asset changes
//---------------------------------------------------------
void TLRender::TRenderNodePathNetwork::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	handle inherited messages
	TRenderNodeDebugMesh::ProcessMessage( Message );
}


//---------------------------------------------------------
//	create the debug mesh for the asset
//---------------------------------------------------------
void TLRender::TRenderNodePathNetwork::InitMeshFromPathNetwork(TLAsset::TPathNetwork& PathNetwork)
{
	TLAsset::TMesh& Mesh = *GetMeshAsset();

	TPtrArray<TLPath::TPathNode>& PathNodes = PathNetwork.GetNodeArray();
	for ( u32 n=0;	n<PathNodes.GetSize();	n++ )
	{
		TLPath::TPathNode& PathNode = *PathNodes[n];

		//	get/add vertex for this node...
		s32 Vertex = GetPathNodeVertex( PathNetwork, PathNode );
		if ( Vertex < 0 )
			continue;

		//	create lines to the links
		TArray<TRef>& LinkRefs = PathNode.GetLinks();
		for ( u32 i=0;	i<LinkRefs.GetSize();	i++ )
		{
			//	get the index of the link, if it's less than N then we've already processed this node, and therefore
			//	we already have a line connecting them both.
			s32 LinkNodeIndex = PathNodes.FindIndex( LinkRefs[i] );
			if ( LinkNodeIndex < (s32)n )
				continue;

			//	get vertex of node
			TLPath::TPathNode& LinkPathNode = *PathNodes[LinkNodeIndex];
			s32 LinkVertex = GetPathNodeVertex( PathNetwork, LinkPathNode );
			if ( LinkVertex < 0 )
				continue;

			//	create line between nodes
			TArray<u16>* pLine = Mesh.GetLines().AddNew();
			pLine->Add( Vertex );
			pLine->Add( LinkVertex );
		}
	}

}


//---------------------------------------------------------
//	return vertex for this path node - if it doesn't exist, create it
//---------------------------------------------------------
s32 TLRender::TRenderNodePathNetwork::GetPathNodeVertex(TLAsset::TPathNetwork& PathNetwork,TLPath::TPathNode& PathNode)
{
	TRefRef PathNodeRef = PathNode.GetNodeRef();

	//	lookup existing vertex
	u16* pVertexIndex = m_PathNodeVertex.Find( PathNodeRef );
	if ( pVertexIndex )
		return (s32)( *pVertexIndex );

	//	doesn't exist, create new vertex...
	//	add vertex to mesh
	s32 VertexIndex = GetMeshAsset()->AddVertex( PathNode.GetPosition().xyz(0.f), m_PathColour );

	//	failed to add vertex?
	if ( VertexIndex == -1 )
		return -1;

	//	add index to the lookup table so we keep a record of it
	m_PathNodeVertex.Add( PathNodeRef, (u16)VertexIndex );

	return VertexIndex;
}


