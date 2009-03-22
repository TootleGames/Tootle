#include "TLGame.h"
#include <TootleAsset/TMesh.h>
#include <TootleRender/TRenderNode.h>
#include <TootleRender/TRenderGraph.h>
#include <TootleRender/TScreen.h>
#include <TootleRender/TScreenManager.h>





//-------------------------------------------------------
//	
//-------------------------------------------------------
TLGame::TScreenRayTest::TScreenRayTest(TRefRef RenderTargetRef) :
	m_RenderTargetRef		( RenderTargetRef )
{
	//	
}


//-------------------------------------------------------
//	delete mesh and render node created
//-------------------------------------------------------
TLGame::TScreenRayTest::~TScreenRayTest()
{
	//	delete asset
	if ( m_pMesh )
	{
		TLAsset::DeleteAsset( m_pMesh->GetAssetRef() );
		m_pMesh = NULL;
	}

	//	delete render node
	TLRender::g_pRendergraph->RemoveNode( m_RenderNodeRef );
}



//-------------------------------------------------------
//	
//-------------------------------------------------------
void TLGame::TScreenRayTest::ProcessMessage(TLMessaging::TMessage& Message)
{
}


//-------------------------------------------------------
//	screen pos has changed, get new projection Ray and update mesh
//-------------------------------------------------------
Bool TLGame::TScreenRayTest::SetScreenPos(const Type2<s32>& ScreenPos)
{
	//	get ray
	TPtr<TLRender::TScreen> pScreen;
	TPtr<TLRender::TRenderTarget> pRenderTarget = TLRender::g_pScreenManager->GetRenderTarget( m_RenderTargetRef, pScreen );

	//	render target doesnt exist? cant get ray
	if ( !pRenderTarget )
		return FALSE;

	TLMaths::TLine WorldRay;
	if ( !pScreen->GetWorldRayFromScreenPos( pRenderTarget, WorldRay, ScreenPos ) )
		return FALSE;

	TTempString DebugString;
	DebugString.Appendf("%d,%d extends to %.2f,%.2f,%.2f", ScreenPos.x, ScreenPos.y, WorldRay.GetEnd().x, WorldRay.GetEnd().y, WorldRay.GetEnd().z );
	TLDebug_Print( DebugString );

	//	create render node
	if ( !CreateRenderNode( pRenderTarget ) )
		return FALSE;

	//	update mesh
	SetMeshLine( WorldRay );

	return TRUE;
}


//-------------------------------------------------------
//	creates the mesh and render node if they dont exist
//-------------------------------------------------------
Bool TLGame::TScreenRayTest::CreateRenderNode(TPtr<TLRender::TRenderTarget>& pRenderTarget)
{
	//	create mesh
	if ( !m_pMesh )
	{
		m_pMesh = TLAsset::CreateAsset("Ray", "Mesh");
		if ( !m_pMesh )
			return FALSE;
	}

	//	create render node
	if ( !m_RenderNodeRef.IsValid() )
	{
		TLMessaging::TMessage Message( TLCore::InitialiseRef );
		Message.AddChildAndData("MeshRef", m_pMesh->GetAssetRef() );
		Message.AddChildAndData("RFSet", (u32)TLRender::TRenderNode::RenderFlags::ResetScene );
		Message.AddChildAndData("RFSet", (u32)TLRender::TRenderNode::RenderFlags::Debug_Points );
		Message.AddChildAndData("LineWidth", 4.f );
		m_RenderNodeRef = TLRender::g_pRendergraph->CreateNode("ray", TRef(), pRenderTarget->GetRootRenderNodeRef(), &Message );
	}

	return TRUE;
}



//-------------------------------------------------------
//	update the line on the mesh to match our ray
//-------------------------------------------------------
void TLGame::TScreenRayTest::SetMeshLine(const TLMaths::TLine& Line)
{
	if ( !m_pMesh )
		return;

	//	delete old lines
	m_pMesh->Empty();

	//	create line
	TLAsset::TMesh::Linestrip* pNewLine = m_pMesh->GetLinestrips().AddNew();
	
	//	split into segments
#define STEPS	10
	for ( u32 i=0;	i<STEPS;	i++ )
	{
		float Step = 1.f / (float)STEPS;
		float3 Pos = TLMaths::Interp( Line.GetStart(), Line.GetEnd(), Step * (float)i );

		if ( Pos.z < 0.f )
			break;

		s32 VertexIndex = m_pMesh->AddVertex( Pos, TColour( 1.f, 1.f, 0.f, 1.f ) );
		pNewLine->Add( VertexIndex );
	}
	
	//	and an end step
	pNewLine->Add( m_pMesh->AddVertex( Line.GetEnd(), TColour( 1.f, 1.f, 1.f, 1.f ) ) );
	
	/*
	//	add a vertex at z=0 for completeness sake
	float3 PosA = Line.GetStart() * float3( 1.f, 1.f, 0.f );
	float3 PosB = Line.GetEnd() * float3( 1.f, 1.f, 0.f );
	m_pMesh->AddVertex( PosA, TColour( 0.f, 0.f, 0.f, 1.f ) );
	m_pMesh->AddVertex( PosB, TColour( 0.f, 0.f, 0.f, 1.f ) );
*/
	//	mark as loaded now it has some data in it
	m_pMesh->SetLoadingState( TLAsset::LoadingState_Loaded );
}

