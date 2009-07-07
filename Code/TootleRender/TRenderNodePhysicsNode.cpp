#include "TRenderNodePhysicsNode.h"
#include <TootlePhysics/TPhysicsGraph.h>
#include <TootlePhysics/TPhysicsNode.h>
#include <TootlePhysics/TCollisionShape.h>
#include <TootleMaths/TShapeCapsule.h>
#include <TootleMaths/TShapeSphere.h>
#include <TootleMaths/TShapeBox.h>


#define COLLISION_SHAPE_COLOUR	TColour( 1.f, 0.f, 0.f, 1.f )



TLRender::TRenderNodePhysicsNode::TRenderNodePhysicsNode(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNodeDebugMesh	( RenderNodeRef, TypeRef ),
	m_ShapeQualityScalar	( 1.f )
{
}


//---------------------------------------------------------
//	init 
//---------------------------------------------------------
void TLRender::TRenderNodePhysicsNode::Initialise(TLMessaging::TMessage& Message)
{
	//	do inherited init first to create mesh etc
	TRenderNodeDebugMesh::Initialise( Message );

	//	draw shape in wireframe
	GetRenderFlags().Clear( TLRender::TRenderNode::RenderFlags::EnableCull );
	
	//	default debuggy flags
//	GetRenderFlags().Set( TLRender::TRenderNode::RenderFlags::Debug_Wireframe );
	GetRenderFlags().Set( TLRender::TRenderNode::RenderFlags::Debug_ColourWireframe );
	GetRenderFlags().Set( TLRender::TRenderNode::RenderFlags::Debug_Position );

	//	physics info is in world space
	GetRenderFlags().Set( TLRender::TRenderNode::RenderFlags::ResetScene );

	//	see if a physics node has been specified
	TRef PhysicsNodeRef;
	if ( Message.ImportData("PhNode", PhysicsNodeRef ) )
		SetPhysicsNode( PhysicsNodeRef );

	Message.ImportData("quality", m_ShapeQualityScalar );
}



//---------------------------------------------------------
//	change the physics node we're monitoring
//---------------------------------------------------------
void TLRender::TRenderNodePhysicsNode::SetPhysicsNode(TRefRef PhysicsNodeRef)
{
	//	no change
	if ( m_PhysicsNodeRef == PhysicsNodeRef )
		return;

	//	clean up old stuff
	GetMeshAsset()->Empty();

	//	unsubscribe from old node
	TPtr<TLPhysics::TPhysicsNode>& pOldPhysicsNode = TLPhysics::g_pPhysicsgraph->FindNode( m_PhysicsNodeRef );
	if ( pOldPhysicsNode )
	{
		this->UnsubscribeFrom( pOldPhysicsNode );
	}

	//	set new ref
	m_PhysicsNodeRef = PhysicsNodeRef;

	//	subscribe to node
	SubscribeToPhysicsNode();

}


//---------------------------------------------------------
//	keep trying to subscribe to physics node if we need to - returns wait if we still need to
//---------------------------------------------------------
SyncBool TLRender::TRenderNodePhysicsNode::SubscribeToPhysicsNode()
{
	//	doesnt need subscription
	if ( !m_PhysicsNodeRef.IsValid() )
		return SyncFalse;

	//	find node - gr: don't search the request queue, we only want nodes that have been initialised
	TPtr<TLPhysics::TPhysicsNode>& pPhysicsNode = TLPhysics::g_pPhysicsgraph->FindNode( m_PhysicsNodeRef, FALSE );

	//	didnt find node, subscribe to the graph so we catch when it is created
	if ( !pPhysicsNode )
	{
		this->SubscribeTo( TLPhysics::g_pPhysicsgraph );
		return SyncWait;
	}

	//	found node, subscribe to it to catch changes
	this->SubscribeTo( pPhysicsNode );

	//	initialise mesh
	OnPhysicsNodeChanged( *pPhysicsNode );

	//	unsubscribe from graph as we dont need to catch the creation any more
	this->UnsubscribeFrom( TLPhysics::g_pPhysicsgraph );

	return SyncTrue;
}


//---------------------------------------------------------
//	catch asset changes
//---------------------------------------------------------
void TLRender::TRenderNodePhysicsNode::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	handle inherited messages
	TRenderNodeDebugMesh::ProcessMessage( Message );

	//	catch node creation
	if ( Message.GetMessageRef() == "NodeAdded" && Message.GetSenderRef() == "PhysicsGraph" )
	{
		//	is this the node we're looking for?
		TRef NodeRef;
		if ( Message.Read( NodeRef ) )
		{
			if ( NodeRef == m_PhysicsNodeRef )
			{
				//	created! - resubscribe
				SubscribeToPhysicsNode();
				return;
			}
		}
	}

	//	catch node destruction (gr: dont think nodes actually send out shutdowns yet)
	if ( Message.GetMessageRef() == "shutdown" && Message.GetSenderRef() == m_PhysicsNodeRef )
	{
		SetPhysicsNode( TRef() );
		return;
	}

	//	catch change in collision shape
	if ( Message.GetMessageRef() == TRef_Static(O,n,T,r,a) && Message.GetSenderRef() == m_PhysicsNodeRef )
	{
		TPtr<TLPhysics::TPhysicsNode>& pPhysicsNode = TLPhysics::g_pPhysicsgraph->FindNode( m_PhysicsNodeRef );
		if ( pPhysicsNode )
		{
			OnPhysicsNodeChanged( *pPhysicsNode );
		}
		else
		{
			TLDebug_Break("Physics node expected");
		}
	}
}



//---------------------------------------------------------
//	rebuild the mesh when the physics details change
//---------------------------------------------------------
void TLRender::TRenderNodePhysicsNode::OnPhysicsNodeChanged(TLPhysics::TPhysicsNode& PhysicsNode)
{
	//	clean up old mesh
	GetMeshAsset()->Empty();

	/*
	//	draw position
	float3 Pos = PhysicsNode.GetPosition();
#define CROSS_SIZE	3.f
	TLMaths::TLine LineA( float3( Pos.x - CROSS_SIZE, Pos.y - CROSS_SIZE, Pos.z ), float3( Pos.x + CROSS_SIZE, Pos.y + CROSS_SIZE, Pos.z ) );
	TLMaths::TLine LineB( float3( Pos.x + CROSS_SIZE, Pos.y - CROSS_SIZE, Pos.z ), float3( Pos.x - CROSS_SIZE, Pos.y + CROSS_SIZE, Pos.z ) );
	GetMeshAsset()->GenerateLine( LineA, COLLISION_SHAPE_COLOUR );
	GetMeshAsset()->GenerateLine( LineB, COLLISION_SHAPE_COLOUR );
	*/

	//	get world shapes 
	TPtrArray<TLMaths::TShape> ShapeArray;
	PhysicsNode.GetCollisionShapesWorld( ShapeArray );

	//	generate geometry for each shape
	for ( u32 i=0;	i<ShapeArray.GetSize();	i++ )
	{
		GetMeshAsset()->GenerateShape( *ShapeArray.ElementAt(i) );
	}

	//	mesh has changed
	OnMeshChanged();
}

