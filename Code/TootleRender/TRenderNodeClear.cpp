#include "TRenderNodeClear.h"



TLRender::TRenderNodeClear::TRenderNodeClear(TRefRef RenderNodeRef) :
	TRenderNode	( RenderNodeRef )
{
	//	set specific flags
	m_RenderFlags.Clear( RenderFlags::DepthRead );
	m_RenderFlags.Clear( RenderFlags::DepthWrite );
	
	m_RenderFlags.Set( RenderFlags::ResetScene );
}


//-------------------------------------------------------
//	resize the mesh (also creates as required)
//-------------------------------------------------------
void TLRender::TRenderNodeClear::SetSize(const Type4<s32>& ClearSize,float NearZ)
{
	//	create mesh
	if ( !m_pClearMesh )
	{
		m_pClearMesh = new TLAsset::TMesh("Clear");

		//	initialise clear quad
		TLAsset::TMesh::Tristrip* pClearTristrip = m_pClearMesh->GetTristrips().AddNew();
		if ( !pClearTristrip )
		{
			m_pClearMesh = NULL;
			return;
		}
		TLAsset::TMesh::Tristrip& ClearTristrip = *pClearTristrip;
		ClearTristrip.SetSize(4);

		//	add vert for each corner
		ClearTristrip[0] = m_pClearMesh->AddVertex( float3(0,0,0) );
		ClearTristrip[1] = m_pClearMesh->AddVertex( float3(0,0,0) );
		ClearTristrip[2] = m_pClearMesh->AddVertex( float3(0,0,0) );
		ClearTristrip[3] = m_pClearMesh->AddVertex( float3(0,0,0) );
	}

	//	update vert positions
	float Top = (float)ClearSize.Top() - 10.f;
	float Left = (float)ClearSize.Left() - 10.f;
	float Bottom = (float)ClearSize.Bottom() + 10.f;
	float Right = (float)ClearSize.Right() + 10.f;

	m_pClearMesh->GetVertex(0).Set( Left, Bottom, NearZ );
	m_pClearMesh->GetVertex(1).Set( Left, Top, NearZ );
	m_pClearMesh->GetVertex(2).Set( Right, Bottom, NearZ );
	m_pClearMesh->GetVertex(3).Set( Right, Top, NearZ );
}

