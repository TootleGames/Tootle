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
void TLRender::TRenderNodeClear::SetSize(const Type4<s32>& ClearSize,float NearZ,const TColour& ClearColour)
{
	TLAsset::TMesh* pClearMesh = m_pClearMesh.GetObject();

	//	create mesh
	if ( !pClearMesh )
	{
		m_pClearMesh = TLAsset::CreateAsset( TLAsset::GetFreeAssetRef("Clear"), "Mesh" );
		pClearMesh = m_pClearMesh.GetObject();

		//	initialise clear quad
		TLAsset::TMesh::Tristrip* pClearTristrip = pClearMesh->GetTristrips().AddNew();
		if ( !pClearTristrip )
		{
			m_pClearMesh = NULL;
			return;
		}
		TLAsset::TMesh::Tristrip& ClearTristrip = *pClearTristrip;
		ClearTristrip.SetSize(4);

		//	add vert for each corner
		ClearTristrip[0] = pClearMesh->AddVertex( float3(0,0,0), ClearColour );
		ClearTristrip[1] = pClearMesh->AddVertex( float3(0,0,0), ClearColour );
		ClearTristrip[2] = pClearMesh->AddVertex( float3(0,0,0), ClearColour );
		ClearTristrip[3] = pClearMesh->AddVertex( float3(0,0,0), ClearColour );
	}

	//	update vert positions
	float Top = (float)ClearSize.Top() - 10.f;
	float Left = (float)ClearSize.Left() - 10.f;
	float Bottom = (float)ClearSize.Bottom() + 10.f;
	float Right = (float)ClearSize.Right() + 10.f;

	pClearMesh->GetVertex(0).Set( Left, Bottom, NearZ );
	pClearMesh->GetVertex(1).Set( Left, Top, NearZ );
	pClearMesh->GetVertex(2).Set( Right, Bottom, NearZ );
	pClearMesh->GetVertex(3).Set( Right, Top, NearZ );

	pClearMesh->GetColours().SetAll( ClearColour );
}

