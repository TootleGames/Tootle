/*------------------------------------------------------

	Simple render object used to clear screen when hardware
	can't do what we want

-------------------------------------------------------*/
#pragma once

#include "TRenderNode.h"

namespace TLRender
{
	class TRenderNodeTile;
}


class TLRender::TRenderNodeTile : public TLRender::TRenderNode
{
public:
	TRenderNodeTile(TRefRef RenderNodeRef,TRefRef TypeRef);

	virtual Bool		Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList);	//	generate render nodes on the fly to render this mesh as tiled relative to the camera

protected:
	TPtrArray<TLRender::TRenderNode>	m_TileRenderNodeBuffer;	//	re-use render nodes when drawing
};