#include "TRenderNodeScrollableView.h"
#include "TLRender.h"
#include "TRenderTarget.h"

using namespace TLRender;

void TRenderNodeScrollableView::Initialise(TLMessaging::TMessage& Message)
{
	TRenderNode::Initialise(Message);

	float2 fmin, fmax;


	//TODO: Calculate the box screen coords from the bounds box
	// NOTE: 0,0 is bottom left of screen, not top left
	
	// DB - Test values for test box.  These are roughly the values needed after conversion.
	fmin.x = 50;
	fmin.y = 150;

	fmax.x = 500;
	fmax.y = 600;

	m_ViewBox = TLMaths::TBox2D(fmin, fmax);
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
