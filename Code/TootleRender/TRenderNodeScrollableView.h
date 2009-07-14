

#pragma once


#include "TRenderNode.h"

namespace TLRender
{
	class TRenderNodeScrollableView;
}

class TLRender::TRenderNodeScrollableView : public TRenderNode
{
public:
	TRenderNodeScrollableView(TRefRef RenderNodeRef=TRef(),TRefRef TypeRef=TRef()) :
		TRenderNode(RenderNodeRef, TypeRef)
	{
		m_Scroll.x = 0;
		m_Scroll.y = 0;
	}

	//virtual Bool							Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList)		{ return FALSE; }

	virtual void							PreDrawChildren(TLMaths::TTransform& SceneTransform);
	virtual void							PostDrawChildren(TLMaths::TTransform& SceneTransform);
protected:

	virtual void							Initialise(TLMessaging::TMessage& Message);	//	generic render node init

	virtual void							ProcessMessage(TLMessaging::TMessage& Message);

private:

	TLMaths::TBox2D				m_ViewBox;
	float2						m_Scroll;
};