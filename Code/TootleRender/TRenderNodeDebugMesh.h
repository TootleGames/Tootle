/*------------------------------------------------------

	Render node that contains it's own mesh. Designed
	to be overloaded to draw debug items for a node.

-------------------------------------------------------*/
#pragma once

#include "TRenderNode.h"


namespace TLRender
{
	class TRenderNodeDebugMesh;
}



//----------------------------------------------------
//	
//----------------------------------------------------
class TLRender::TRenderNodeDebugMesh : public TLRender::TRenderNode
{
public:
	TRenderNodeDebugMesh(TRefRef RenderNodeRef,TRefRef TypeRef);

	virtual TPtr<TLAsset::TMesh>&	GetMeshAsset()		{	return m_pDebugMesh;	}

protected:
	virtual void					Initialise(TLMessaging::TMessage& Message);
	virtual Bool					Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList)	{	return TRUE;	}

public:
	TPtr<TLAsset::TMesh>	m_pDebugMesh;
};


