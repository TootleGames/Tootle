
#include "TRenderNodeScrollableView.h"
#include "TLRender.h"

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
				m_Scroll.x += Change.x;
				m_Scroll.y += Change.y;
			}
		}
	}

	TRenderNode::ProcessMessage(Message);
}


void TRenderNodeScrollableView::PreDrawChildren(TLMaths::TTransform& SceneTransform)
{
	// Don't change anything if we don't have any children
	if(!HasChildren())
		return;

	// Enable scissoring
	//Opengl::EnableScissor(TRUE);

	// Set the scissor box
	//TODO: copy the current scissor box?
	Opengl::SetScissor( (u32)m_ViewBox.GetLeft(), (u32)m_ViewBox.GetTop(), (u32)m_ViewBox.GetWidth(), (u32)m_ViewBox.GetHeight() );

	// Only alter the scene transform if we have a valid scroll
	if(m_Scroll.x != 0.0f && m_Scroll.y != 0.0f)
	{
		// Update the transform based on our scroll values
		TLMaths::TTransform	ScrollTransform;

		ScrollTransform.SetTranslate(float3(m_Scroll.x, m_Scroll.y, 0.0f));

		SceneTransform.Transform( ScrollTransform );

		Opengl::SceneTransform(ScrollTransform);
	}
}

void TRenderNodeScrollableView::PostDrawChildren(TLMaths::TTransform& SceneTransform)
{
	if(!HasChildren())
		return;

	//Disable scissoring
	//Opengl::EnableScissor(FALSE);

	// Only alter the scene transform if we have a valid scroll
	if(m_Scroll.x != 0.0f && m_Scroll.y != 0.0f)
	{
		// Undo changes to transform

		TLMaths::TTransform	ScrollTransform;

		ScrollTransform.SetTranslate(float3(-m_Scroll.x, -m_Scroll.y, 0.0f));

		SceneTransform.Transform( ScrollTransform );

		Opengl::SceneTransform(ScrollTransform);
	}
}
