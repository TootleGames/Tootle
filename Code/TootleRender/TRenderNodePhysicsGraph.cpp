#include "TRenderNodePhysicsGraph.h"
#include <TootlePhysics/TPhysicsGraph.h>




void CreateCollisionZoneRenderNode(TPtr<TLPhysics::TCollisionZone>& pCollisionZone,u32 ZoneDepth,TPtrArray<TLRender::TRenderNode>& ZoneRenderList,TPtrArray<TLRender::TRenderNode>& NodeRenderList)
{
	if ( !pCollisionZone )
		return;

	if ( !pCollisionZone->HasAnyNodesTotal() )
		return;

	TPtrArray<TLPhysics::TPhysicsNode>& CollisionZoneNodes = pCollisionZone->GetNodes();
	TColour Colour = TColour::Debug_GetColour( ZoneDepth );
	Colour.GetAlpha() = 0.2f;

	{
		float3 BoxOffset( 0.5f, 0.5f, 0 );
		TPtr<TLRender::TRenderNode> pRenderNode = new TLRender::TRenderNode();
		pRenderNode->SetMeshRef("d_cube");
		const TLMaths::TBox2D& CollisionBox = pCollisionZone->GetCollisionShape().GetBox();
		float3 BoxMin3( CollisionBox.GetMin(), 0.f );
		float3 BoxMax3( CollisionBox.GetMax(), 0.f );
		pRenderNode->SetScale( BoxMax3 - BoxMin3 - BoxOffset );
		pRenderNode->SetTranslate( BoxMin3 + BoxOffset );

		if ( CollisionZoneNodes.GetSize() == 0 )
			pRenderNode->GetRenderFlags().Set( TLRender::TRenderNode::RenderFlags::Debug_Wireframe );

		pRenderNode->GetRenderFlags().Clear( TLRender::TRenderNode::RenderFlags::DepthRead );
		pRenderNode->GetRenderFlags().Clear( TLRender::TRenderNode::RenderFlags::MergeColour );
		pRenderNode->SetColour( Colour );

		ZoneRenderList.Add( pRenderNode );
	}

	//	add a render node for every node in this zone
	for ( u32 c=0;	c<CollisionZoneNodes.GetSize();	c++ )
	{
		TPtr<TLPhysics::TPhysicsNode>& pPhysicsNode = CollisionZoneNodes[c];

		TPtr<TLRender::TRenderNode> pRenderNode = new TLRender::TRenderNode();
		pRenderNode->SetMeshRef("d_cube");
		pRenderNode->SetTranslate( pPhysicsNode->GetPosition() );
		pRenderNode->GetRenderFlags().Clear( TLRender::TRenderNode::RenderFlags::DepthRead );
		pRenderNode->GetRenderFlags().Clear( TLRender::TRenderNode::RenderFlags::MergeColour );
	
		float Scale = 2.f;
		if ( pPhysicsNode->m_Debug_StaticCollisions > 0 )
		{
			Scale = 3.f;
			TColour TempColour = Colour;
			TempColour.GetAlpha() = 1.f;
			pRenderNode->SetColour( TempColour );
		}
		else
		{
			pRenderNode->SetColour( Colour );
		}

		pRenderNode->SetScale( float3(Scale,Scale,Scale) );

		NodeRenderList.Add( pRenderNode );
	}

}


void CreateAllCollisionZoneRenderNodes(TPtr<TLPhysics::TCollisionZone>& pCollisionZone,u32 ZoneDepth,TPtrArray<TLRender::TRenderNode>& ZoneRenderList,TPtrArray<TLRender::TRenderNode>& NodeRenderList)
{
	if ( !pCollisionZone )
		return;

	//	create render node for this node
	CreateCollisionZoneRenderNode( pCollisionZone, ZoneDepth, ZoneRenderList, NodeRenderList );

	//	add children
	TPtrArray<TLPhysics::TCollisionZone>& ChildZones = pCollisionZone->GetChildZones();

	for ( u32 c=0;	c<ChildZones.GetSize();	c++ )
	{
		CreateAllCollisionZoneRenderNodes( ChildZones[c], ZoneDepth+1, ZoneRenderList, NodeRenderList );
	}
}






TLRender::TRenderNodePhysicsGraph::TRenderNodePhysicsGraph(TRefRef RenderNodeRef) :
	TLRender::TRenderNode	( RenderNodeRef )
{
}


//---------------------------------------------------------------
//	pre-draw routine for a render object
//---------------------------------------------------------------
Bool TLRender::TRenderNodePhysicsGraph::Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList)
{
	TLPhysics::TPhysicsgraph* pPhysicsGraph = TLPhysics::g_pPhysicsgraph.GetObject();
	if ( !pPhysicsGraph )
		return FALSE;

	//	draw zones
	TPtr<TLPhysics::TCollisionZone>& pRootCollisionZone = pPhysicsGraph->GetRootCollisionZone();

	TPtrArray<TRenderNode>& ZoneRenderList = PostRenderList;
	TPtrArray<TRenderNode> NodeRenderList;

	CreateAllCollisionZoneRenderNodes( pRootCollisionZone, 0, PostRenderList, NodeRenderList );

	//	add nodes after zones
	PostRenderList.Add( NodeRenderList );

	return TRUE;
}


