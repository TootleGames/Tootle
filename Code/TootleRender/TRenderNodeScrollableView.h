/*------------------------------------------------------
	
	A render node which has a world-space box on it which
	clips(scissors) all children underneath, creating a pseudo
	scroll box. A scroll(a trasnform, so we could rotate/scale
	the children) variable exists so we can scroll the contents (the children)
	around inside our clipping box

-------------------------------------------------------*/
#pragma once


#include "TRenderNode.h"

namespace TLRender
{
	class TRenderNodeScrollableView;
}

class TLRender::TRenderNodeScrollableView : public TLRender::TRenderNode
{
public:
	TRenderNodeScrollableView(TRefRef RenderNodeRef=TRef(),TRefRef TypeRef=TRef()) :
		TRenderNode(RenderNodeRef, TypeRef),
		m_DatumRef(TLRender_TRenderNode_DatumBoundsBox2D)
	{
	}

	//virtual Bool							Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList)		{ return FALSE; }

	virtual void							PreDrawChildren(TLRender::TRenderTarget& RenderTarget,TLMaths::TTransform& SceneTransform);
	virtual void							PostDrawChildren(TLRender::TRenderTarget& RenderTarget);

protected:
	virtual void							SetProperty(TLMessaging::TMessage& Message);	//	generic render node init
	virtual void							ProcessMessage(TLMessaging::TMessage& Message);

	FORCEINLINE Bool						HasScroll() const		{	return m_ScrollTransform.HasAnyTransform();	}
	FORCEINLINE float2&						GetScroll()				{	return m_ScrollTransform.GetTranslate().xy();	}	//	accessor straight to the 2D scroll

private:
	void									OnRenderTargetRefChange();

private:
	TLMaths::TTransform			m_ScrollTransform;		//	gr: keep the scroll in a transform so we don't need to create a transform twice every render
	TLMaths::TBox2D				m_ViewBox;

	TRef						m_DatumRef;
	TRef						m_RenderTargetRef;

	Bool						m_bVerticalScroll;
	Bool						m_bHorizontalScroll;
};

