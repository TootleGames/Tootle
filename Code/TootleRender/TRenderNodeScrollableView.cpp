#include "TRenderNodeScrollableView.h"
#include "TLRender.h"
#include "TRenderTarget.h"
#include "TScreenManager.h"
#include <TootleMaths/TShapeBox.h>

using namespace TLRender;

void TRenderNodeScrollableView::Initialise(TLMessaging::TMessage& Message)
{
	TRenderNode::Initialise(Message);

	if(Message.ImportData("RTarget", m_RenderTargetRef))
		OnRenderTargetRefChange();
}


void TRenderNodeScrollableView::ProcessMessage(TLMessaging::TMessage& Message)
{
	if(Message.GetMessageRef() == STRef(A,c,t,i,o))
	{
		TRef ActionRef;
		Message.Read(ActionRef);

		if(ActionRef == STRef(S,c,r,o,l)) // "Scroll"
		{
			float3 Change;
			if(Message.ImportData("Move3", Change ))
			{
				//	mark as having a scroll now
				m_ScrollTransform.SetTranslateValid();

				//	 change scroll
				GetScroll() += Change.xy();

				//	gr: when we change the scroll (ie. changing the world transform for the children) then really
				//		we need to invalidate our children otherwise their world bounds/transforms are going to be
				//		out of date and when we try and get the bounds (eg. for widget clicking) they're going to
				//		be in the wrong place - missing the scroll offset.
				//	note: also need to think about the visibility of the bounds for the widget... either by clipping
				//		datums, or some kinda "check point isn't clipped" functionality when ray casting into it...
				//		not sure of the best (ie. still efficient) way of doing this...
	
				//	gr: untested
				//SetBoundsInvalid( TInvalidateFlags( InvalidateChildWorldBounds, InvalidateChildWorldPos ) );
			}
		}
	}

	TRenderNode::ProcessMessage(Message);
}


void TRenderNodeScrollableView::OnRenderTargetRefChange()
{
	TPtr<TLRender::TScreen> pScreen;
	TPtr<TLRender::TRenderTarget>& pRenderTarget = TLRender::g_pScreenManager->GetRenderTarget( m_RenderTargetRef, pScreen );

	if(pRenderTarget)
	{
		Type4<s32> RenderTargetSize;
		pScreen->GetRenderTargetSize( RenderTargetSize, pRenderTarget );


		TPtr<TLMaths::TShape> pDatum = GetWorldDatum( m_DatumRef );

		if(!pDatum || (pDatum->GetShapeType() != TLMaths_ShapeRef(TBox)))
			return;

		TLMaths::TShapeBox* pBox = dynamic_cast<TLMaths::TShapeBox*>(pDatum.GetObject());

		const TLMaths::TBox& box = pBox->GetBox();

		Type2<s32> ScreenPos[2];
		Bool bSuccess = FALSE;

		float3 Point[2];
		Point[0] = float3(box.GetMin().x,box.GetMax().y, GetTranslate().z);
		Point[1] = float3(box.GetMax().x,box.GetMin().y, GetTranslate().z);

		// Get bottom left corner of box as min point
		bSuccess |= pRenderTarget->GetScreenPos(ScreenPos[0], Point[0], RenderTargetSize, pScreen->GetScreenShape(), TRUE);

		// Get top right corner of box as max point
		bSuccess |= pRenderTarget->GetScreenPos(ScreenPos[1], Point[1], RenderTargetSize, pScreen->GetScreenShape(), TRUE);

		if(bSuccess)
		{
			float2 fmin, fmax;
			// NOTE: 0,0 is bottom left of screen, not top left

			// DB - Test values for test box.  These are roughly the values needed after conversion.
			fmin.x = (float)ScreenPos[0].x; //50;
			fmin.y = (float)ScreenPos[0].y; //150;

			fmax.x = TLMaths::Absf((float)ScreenPos[1].x - (float)ScreenPos[0].x); //500;
			fmax.y = TLMaths::Absf((float)ScreenPos[1].y - (float)ScreenPos[0].y); //600;

			m_ViewBox = TLMaths::TBox2D(fmin, fmax);
		}
	}
	else
		TLDebug_Break("Failed to find render target");
}


void TRenderNodeScrollableView::PreDrawChildren(TLRender::TRenderTarget& RenderTarget,TLMaths::TTransform& SceneTransform)
{
	// Enable scissoring
	//Opengl::EnableScissor(TRUE);

	// Set the scissor box
	//TODO: copy the current scissor box?
	Opengl::SetScissor( (u32)m_ViewBox.GetLeft(), (u32)m_ViewBox.GetTop(), (u32)m_ViewBox.GetWidth(), (u32)m_ViewBox.GetHeight() );

	// Only alter the scene transform if we have a valid scroll
	if ( HasScroll() )
	{
		//	gr: rather than manipulate the transform in post draw, we can save the scene, then restore it
		//	note: ALWAYS match a BeginScene and EndScene (will assert if you don't as it corrupts scene)
		RenderTarget.BeginScene();

		// Update the transform based on our scroll values
		SceneTransform.Transform( m_ScrollTransform );
		Opengl::SceneTransform( m_ScrollTransform );
	}
}

void TRenderNodeScrollableView::PostDrawChildren(TLRender::TRenderTarget& RenderTarget)
{
	//restore scissoring
	//Opengl::EnableScissor(FALSE);

	// Only alter the scene transform if we have a valid scroll
	if ( HasScroll() )
	{
		// Undo changes to transform
		RenderTarget.EndScene();
	}
}
