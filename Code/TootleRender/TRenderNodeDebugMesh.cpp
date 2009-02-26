#include "TRenderNodeDebugMesh.h"




TLRender::TRenderNodeDebugMesh::TRenderNodeDebugMesh(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNode		( RenderNodeRef, TypeRef )
{
}


//---------------------------------------------------------
//	init - create our mesh
//---------------------------------------------------------
void TLRender::TRenderNodeDebugMesh::Initialise(TPtr<TLMessaging::TMessage>& pMessage)
{
	//	do inherited init
	TRenderNode::Initialise( pMessage );

	//	make up mesh
	m_pDebugMesh = new TLAsset::TMesh("Debug");
	m_pDebugMesh->SetLoadingState( TLAsset::LoadingState_Loaded );
}


