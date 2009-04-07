#include "TRenderNodeClear.h"



TLRender::TRenderNodeClear::TRenderNodeClear(TRefRef NodeRef,TRefRef TypeRef) :
	TRenderNode	( NodeRef, TypeRef )
{
	//	set specific flags
	m_RenderFlags.Clear( RenderFlags::DepthRead );
	m_RenderFlags.Clear( RenderFlags::DepthWrite );
	
	m_RenderFlags.Set( RenderFlags::ResetScene );
}


//-------------------------------------------------------
//	resize the mesh (also creates as required)
//-------------------------------------------------------
void TLRender::TRenderNodeClear::SetSize(const TLMaths::TBox2D& ClearBox,float NearZ)
{
	TLAsset::TMesh* pClearMesh = m_pClearMesh.GetObject();

	//	create mesh
	if ( !pClearMesh )
	{
		m_pClearMesh = TLAsset::CreateAsset( TLAsset::GetFreeAssetRef("Clear"), "Mesh" );
		pClearMesh = m_pClearMesh.GetObject();

		float3 Zero3( 0,0,0 );
		//	initialise clear quad
		pClearMesh->GenerateQuad( Zero3, Zero3, Zero3, Zero3 );
	}

	//	stretch past borders to cope with float precision
	float Overlap = 1.f;

	//	update vert positions
	float Top = ClearBox.GetTop() - Overlap;
	float Left = ClearBox.GetLeft() - Overlap;
	float Bottom = ClearBox.GetBottom() + Overlap;
	float Right = ClearBox.GetRight() + Overlap;

	pClearMesh->GetVertex(0).Set( Left, Bottom, NearZ );
	pClearMesh->GetVertex(1).Set( Left, Top, NearZ );
	pClearMesh->GetVertex(2).Set( Right, Bottom, NearZ );
	pClearMesh->GetVertex(3).Set( Right, Top, NearZ );
}

