#include "TRenderNodeQuadTree.h"
#include <TootlePhysics/TPhysicsgraph.h>
#include <TootleRender/TRenderTarget.h>
#include <TootleRender/TScreenManager.h>
#include <TootleScene/TScenegraph.h>




void CreateZoneMesh(TLAsset::TMesh& Mesh,const TPtr<TLMaths::TQuadTreeZone>& pZone,u32 ZoneDepth)
{
	//	gr: change the z so we get a 3D perspective which should help us see the gaps a bit better
	float z = 4.f - ( (float)ZoneDepth * 0.5f );
	TColour ZoneColour = TColour::Debug_GetColour( ZoneDepth );
	
	//	create a sphere for this zone's shape
	//	we use a sphere because if we did the proper quads then they'd meet at the edges and we wouldnt know where they end
	const TLMaths::TBox2D& ZoneShapeBox = pZone->GetShape();
	TLMaths::TSphere2D ZoneShapeSphere;
	ZoneShapeSphere.Set( ZoneShapeBox );

	//	filled if active
	SyncBool IsActive = pZone->IsActive();
	if ( IsActive == SyncTrue )
	{
		ZoneColour.GetAlpha() = 0.5f;
		Mesh.GenerateSphere( ZoneShapeSphere, &ZoneColour, z );
	}
	else if ( IsActive == SyncWait )
	{
		TColour ZoneColour = TColour::Debug_GetColour( 0 );
		ZoneColour.GetAlpha() = 0.4f;
		//Mesh.GenerateSphereOutline( ZoneShapeSphere, &ZoneColour, z );
		Mesh.GenerateSphere( ZoneShapeSphere, &ZoneColour, z );
	}
	else
	{
		TColour ZoneColour = TColour::Debug_GetColour( 0 );
		ZoneColour.GetAlpha() = 0.1f;
		//Mesh.GenerateSphereOutline( ZoneShapeSphere, &ZoneColour, z );
		Mesh.GenerateSphere( ZoneShapeSphere, &ZoneColour, z );
	}

	//	draw box outline still
	ZoneColour.GetAlpha() = 1.0f;
//	Mesh.GenerateQuadOutline( ZoneShapeBox, &ZoneColour, z );

	//	do children
	const TPtrArray<TLMaths::TQuadTreeZone>& ChildZones = pZone->GetChildZones();
	for ( u32 c=0;	c<ChildZones.GetSize();	c++ )
	{
		CreateZoneMesh( Mesh, ChildZones[c], ZoneDepth+1 );
	}
}




TLRender::TRenderNodeQuadTreeZone::TRenderNodeQuadTreeZone(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNodeDebugMesh	( RenderNodeRef, TypeRef )
{
}


//---------------------------------------------------------
//	init 
//---------------------------------------------------------
void TLRender::TRenderNodeQuadTreeZone::Initialise(TLMessaging::TMessage& Message)
{
	//	do inherited init first to create mesh etc
	TRenderNodeDebugMesh::Initialise( Message );

	//	draw shape in wireframe
	GetRenderFlags().Clear( TLRender::TRenderNode::RenderFlags::EnableCull );

	//	draw world space
	GetRenderFlags().Set( TLRender::TRenderNode::RenderFlags::ResetScene );

	GetRenderFlags().Clear( TLRender::TRenderNode::RenderFlags::DepthWrite );
	GetRenderFlags().Clear( TLRender::TRenderNode::RenderFlags::DepthRead );

	//	initialise to certain zone
	TRef ZoneRef;
	if ( Message.ImportData("Zone", ZoneRef ) )
	{
		if ( ZoneRef == "Physics" )
		{
			//	physic's graph's root zone
			TPtr<TLMaths::TQuadTreeZone> pZone = TLPhysics::g_pPhysicsgraph->GetRootCollisionZone();
			SetQuadTreeZone( pZone );
		}
		else if ( ZoneRef == "Scene" )
		{
			//	physic's graph's root zone
			TPtr<TLMaths::TQuadTreeZone> pZone = TLScene::g_pScenegraph->GetRootZone();
			SetQuadTreeZone( pZone );
		}
		else
		{
			//	find a render target with this ref and use it's zone
			TPtr<TLRender::TRenderTarget>& pRenderTarget = TLRender::g_pScreenManager->GetRenderTarget( ZoneRef );
			if ( pRenderTarget )
			{
				TPtr<TLMaths::TQuadTreeZone> pZone = pRenderTarget->GetRootQuadTreeZone();
				SetQuadTreeZone( pZone );
			}
		}
	}


}



//---------------------------------------------------------
//	change the physics node we're monitoring
//---------------------------------------------------------
void TLRender::TRenderNodeQuadTreeZone::SetQuadTreeZone(TPtr<TLMaths::TQuadTreeZone>& pZone)
{
	//	no change
	if ( m_pZone == pZone )
		return;

	//	clean up old stuff
	GetMeshAsset()->Empty();

	//	unsubscribe from old zone
	if ( m_pZone )
		this->UnsubscribeFrom( m_pZone );

	m_pZone = pZone;
	if ( !m_pZone )
		return;

	//	subscribe to new zone
	this->SubscribeTo( m_pZone );

	//	create new mesh stuff - recursively create shapes to represent zones
	CreateZoneMesh( *GetMeshAsset(), m_pZone, 0 );
}


//---------------------------------------------------------
//	catch changes to the zone we're tracking
//---------------------------------------------------------
void TLRender::TRenderNodeQuadTreeZone::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	have to assume is from m_pZone
	if ( Message.GetMessageRef() == "OnChanged" )
	{
		//	rebuild mesh
		GetMeshAsset()->Empty();
		CreateZoneMesh( *GetMeshAsset(), m_pZone, 0 );
		return;
	}

	//	do inherited
	TLRender::TRenderNodeDebugMesh::ProcessMessage( Message );
}
