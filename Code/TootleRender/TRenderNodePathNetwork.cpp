#include "TRenderNodePathNetwork.h"
#include <TootleAsset/TPath.h>



#define PATH_DIRECTION_ARROW_LENGTH		1.0f	//	how big are the triangles
#define PATH_DIRECTION_ARROW_WIDTH		1.0f	//	how big are the triangles
#define PATH_DIRECTION_ARROW_HALFWIDTH	(PATH_DIRECTION_ARROW_WIDTH*0.5f)
#define PATH_DIRECTION_ARROW_RATE		10.f		//	draw a triangle every N metres


TLRender::TRenderNodePathNetwork::TRenderNodePathNetwork(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNodeDebugMesh	( RenderNodeRef, TypeRef ),
	m_EnableMarkers			( TRUE )
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
	GetRenderFlags().Clear( TLRender::TRenderNode::RenderFlags::DepthRead );
	GetRenderFlags().Clear( TLRender::TRenderNode::RenderFlags::EnableCull );

	//	read marker setting
	Message.ImportData("Markers", m_EnableMarkers );

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
		TArray<TLPath::TPathLink>& NodeLinks = PathNode.GetLinks();
		for ( u32 i=0;	i<NodeLinks.GetSize();	i++ )
		{
			TLPath::TPathLink& PathLink = NodeLinks[i];

			//	get the index of the link, if it's less than N then we've already processed this node, and therefore
			//	we already have a line connecting them both.
			s32 LinkNodeIndex = PathNodes.FindIndex( PathLink.GetLinkNodeRef() );
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

			if ( m_EnableMarkers )
			{
				//	draw markers along the line
				//	if the link is one way, then add arrows along the line
				TLPath::TDirection::Type Direction = PathLink.GetDirection();
				
				//	reverse line direction if direction is backwards
				const float2& FromPos = (Direction != TLPath::TDirection::Backward) ? PathNode.GetPosition() : LinkPathNode.GetPosition();
				const float2& ToPos = (Direction != TLPath::TDirection::Backward) ? LinkPathNode.GetPosition() : PathNode.GetPosition();

				//	get dir and length
				float2 LineDir( ToPos - FromPos );
				float LineLength = LineDir.Length();
				LineDir.Normalise();

				TArray<float> ArrowOffsets;

				//	not gonna fit on more than 1 arrow, just add one in the middle
				if ( LineLength <= PATH_DIRECTION_ARROW_RATE )
				{
					ArrowOffsets.Add( LineLength * 0.5f );
				}
				else
				{
					//	add an arrow every so often...
					float ArrowCountf = LineLength / PATH_DIRECTION_ARROW_RATE;
					u32 ArrowCount = (u32)ArrowCountf;
					float Offset = (ArrowCountf - (float)ArrowCount) / 2.f;	//	use remainder to center arrows along line
					
					//	offset half way as well
					Offset += PATH_DIRECTION_ARROW_RATE / 2.f;

					for ( u32 i=0;	i<ArrowCount;	i++ )
						ArrowOffsets.Add( Offset + ((float)i*PATH_DIRECTION_ARROW_RATE) );
				}

				//	draw an arrow along the path
				for ( u32 i=0;	i<ArrowOffsets.GetSize();	i++ )
				{
					float2 ArrowHeadPos = FromPos + (LineDir * (ArrowOffsets[i] + PATH_DIRECTION_ARROW_LENGTH) );
					float2 ArrowTailPos = FromPos + (LineDir * ArrowOffsets[i] );
					float2 ArrowLeft = LineDir.GetAntiClockwise() * PATH_DIRECTION_ARROW_HALFWIDTH;
					float2 ArrowTailLeft = ArrowTailPos + ArrowLeft;
					float2 ArrowTailRight = ArrowTailPos - ArrowLeft;

					if ( Direction == TLPath::TDirection::Any )
					{
						//	draw a quad instead of an arrow when it's any direction
						float2 ArrowHeadLeft = ArrowHeadPos + ArrowLeft;
						float2 ArrowHeadRight = ArrowHeadPos - ArrowLeft;

						//	make up quad
						//	this line is corrupting mem!
						Mesh.GenerateQuad( ArrowHeadLeft.xyz(0.f), ArrowHeadRight.xyz(0.f), ArrowTailRight.xyz(0.f), ArrowTailLeft.xyz(0.f) );
					}
					else
					{
						//	make up triangle
						TLAsset::TMesh::Triangle* pTriangle = Mesh.GetTriangles().AddNew();
						pTriangle->x = Mesh.AddVertex( ArrowHeadPos.xyz(0.f) );
						pTriangle->y = Mesh.AddVertex( ArrowTailLeft.xyz(0.f) );
						pTriangle->z = Mesh.AddVertex( ArrowTailRight.xyz(0.f) );
					}
				}
			}
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
	s32 VertexIndex = GetMeshAsset()->AddVertex( PathNode.GetPosition().xyz(0.f) );

	//	failed to add vertex?
	if ( VertexIndex == -1 )
		return -1;

	//	add index to the lookup table so we keep a record of it
	m_PathNodeVertex.Add( PathNodeRef, (u16)VertexIndex );

	return VertexIndex;
}


