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
		TRenderNode					( RenderNodeRef, TypeRef ),
		m_ClipDatumRef				( TLRender_TRenderNode_DatumBoundsBox2D ),
		m_uTempMomentumUpdateCount	( 0 ),
		m_bVerticalScroll			( TRUE ),
		m_bHorizontalScroll			( TRUE ),
		m_bDepthScroll				( FALSE ),
		m_AlignChildrenToClipDatum	( TRUE ),
		m_bUseMomentum				( FALSE ),
		m_ChildWorldTransformValid	( SyncFalse )
	{
	}

	//virtual Bool							Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList)		{ return FALSE; }

protected:
	virtual void							SetProperty(TLMessaging::TMessage& Message);	//	generic render node init
	virtual void							ProcessMessage(TLMessaging::TMessage& Message);

	virtual void							OnTransformChanged(u8 TransformChangedBits=TLMaths_TransformBitAll);			//	gr

	virtual TPtrArray<TRenderNode>&			GetLocalBoundsChildren()						{	static TPtrArray<TRenderNode> NoChildren;	return NoChildren;	}	//	we don't include our children when calculating the local bounds as they're going to be clipped somewhere inside the node anyway
	virtual const TLMaths::TTransform&		GetChildWorldTransform(TRenderNode* pRootNode=NULL,Bool ForceCalculation=FALSE);
	virtual Bool							SetWorldTransformOld(Bool SetPosOld,Bool SetTransformOld,Bool SetShapesOld);	//	world transform has changed, invalidate child world transform

	virtual void							PreDrawChildren(TLRender::TRenderTarget& RenderTarget,TLMaths::TTransform& SceneTransform);
	virtual void							PostDrawChildren(TLRender::TRenderTarget& RenderTarget);

	FORCEINLINE Bool						HasScroll() const		{	return m_ScrollTransform.HasAnyTransform();	}
	FORCEINLINE float3&						GetScroll()				{	return m_ScrollTransform.GetTranslate();	}	//	accessor straight to the 2D scroll

private:
	void									OnRenderTargetRefChange(TLRender::TRenderTarget* pRenderTarget=NULL);
	void									OnScrollChanged();														//	called when scroll changes
	void									OnOffsetChanged()														{	OnScrollChanged();	}
	void									OnDatumChanged()														{	OnRenderTargetRefChange(NULL);	}		//	recalc view box if datum changes or moves

private:
	TLMaths::TTransform			m_ScrollTransform;		//	gr: keep the scroll in a transform so we don't need to create a transform twice every render
	TLMaths::TBox2D				m_ViewBox;				//	gr: if we end up having multiple render targets then turn this into a keyarray so we have a view box for each render target

	TRef						m_ClipDatumRef;
	TLMaths::TTransform			m_ClipDatumOffset;		//	"cached" offset for aligning children to the clip datum in local space (the offset will be local)
	TRef						m_RenderTargetRef;
	
	float3						m_fMomentum;					// Momentum of the scroll
	float3						m_fTempMomentum;				// Temp momentum from touch/mouse movements
	u8							m_uTempMomentumUpdateCount;		// Number of times the temp momentum is updated - avoids using an array

	Bool						m_bVerticalScroll;
	Bool						m_bHorizontalScroll;
	Bool						m_bDepthScroll;				//	allow scroll on z
	Bool						m_AlignChildrenToClipDatum;	//	if true, 0,0,0 position on a child will be at the top left of the clipping box
	Bool						m_bUseMomentum;				//	allow momentum movement

	TLMaths::TTransform			m_ChildWorldTransform;		//	cache of the children's world transform
	SyncBool					m_ChildWorldTransformValid;	//	
};

