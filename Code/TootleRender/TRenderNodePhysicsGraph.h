/*------------------------------------------------------

	Render node that renders a debug view of the physics graph

-------------------------------------------------------*/
#pragma once

#include "TRenderNode.h"

namespace TLRender
{
	class TRenderNodePhysicsGraph;
}


class TLRender::TRenderNodePhysicsGraph : public TLRender::TRenderNode
{
public:
	TRenderNodePhysicsGraph(TRefRef RenderNodeRef=TRef());

	virtual Bool				Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList);	//	pre-draw routine for a render object
};


