#include "TRenderNodeScrollableView.h"
#include "TLRender.h"
#include "TRenderTarget.h"
#include "TScreenManager.h"
#include <TootleMaths/TShapeBox.h>

using namespace TLRender;

void TRenderNodeScrollableView::SetProperty(TLMessaging::TMessage& Message)
{
	TRenderNode::SetProperty(Message);

	Message.ImportData("Datum", m_ClipDatumRef);

	if(Message.ImportData("RTarget", m_RenderTargetRef))
		OnRenderTargetRefChange();

	Message.ImportData("Horizontal", m_bHorizontalScroll);
	Message.ImportData("Vertical", m_bVerticalScroll);
	Message.ImportData("Depth", m_bDepthScroll);

	Message.ImportData("AlignChildren", m_AlignChildrenToClipDatum );
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
				if(m_bVerticalScroll)		GetScroll().y += Change.y;
				if(m_bHorizontalScroll)		GetScroll().x += Change.x;
				if(m_bDepthScroll)			GetScroll().z += Change.z;

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
	else if ( Message.GetMessageRef() == TRef_Static(D,o,S,c,r) )
	{
		//	apply a scroll change
		float3 Change;
		Message.ResetReadPos();
		if ( Message.Read(Change) )
		{
			//	mark as having a scroll now
			m_ScrollTransform.SetTranslateValid();

			//	 change scroll
			if(m_bVerticalScroll)	GetScroll().y += Change.y;
			if(m_bHorizontalScroll)	GetScroll().x += Change.x;
			if(m_bDepthScroll)		GetScroll().z += Change.z;
		}
	}

	TRenderNode::ProcessMessage(Message);
}


void TRenderNodeScrollableView::OnRenderTargetRefChange(TLRender::TRenderTarget* pRenderTarget)
{
	//	reset viewbox
	m_ViewBox.SetInvalid();
	m_ClipDatumOffset.SetInvalid();
	TLRender::TScreen* pScreen = NULL;

	//	get render target if no provided
	if ( !pRenderTarget )
	{
		TPtr<TLRender::TScreen> ppScreen;
		pRenderTarget = TLRender::g_pScreenManager->GetRenderTarget( m_RenderTargetRef, ppScreen );
		pScreen = ppScreen;
	}
	else
	{
		//	todo: get proper screen of render target provided
		pScreen = TLRender::g_pScreenManager->GetDefaultScreen();
	}

	//	missing render target
	if ( !pRenderTarget || !pScreen )
	{
		TLDebug_Break("Render target/screen expected");
		return;
	}
	
	//	get the local datum first so we can get the local offset
	const TLMaths::TShapeBox2D* pLocalDatum = GetLocalDatum<TLMaths::TShapeBox2D>( m_ClipDatumRef );
	if ( !pLocalDatum )
	{
		TTempString Debug_String("Datum ");
		m_ClipDatumRef.GetString( Debug_String );
		Debug_String.Append(" not found (or not box2d shape) for scroll render node ");
		this->GetNodeRef().GetString( Debug_String );
		TLDebug_Print( Debug_String );
		return;
	}

	//	gr: even though we've fetched the local datum, use the world one to take advantage of
	//	future caching
	TPtr<TLMaths::TShape> pWorldDatum = GetWorldDatum( m_ClipDatumRef );
	if( !pWorldDatum )
		return;

	if ( pWorldDatum->GetShapeType() != TLMaths_ShapeRef(TBox2D) )
	{
		TLDebug_Break("Currently only box2D shapes are supported for scroll-view render node datums");
		return;
	}

	//	cache the datum's min point/child offset
	m_ClipDatumOffset.SetTranslate( pLocalDatum->GetBox().GetMin().xyz(0.f) );

	//	get box shape
	TLMaths::TShapeBox2D* pBox = pWorldDatum.GetObject<TLMaths::TShapeBox2D>();
	const TLMaths::TBox2D& box = pBox->GetBox();

	Type2<s32> ScreenPos[2];
	Bool bSuccess = FALSE;

	//NOTE: For the OpengGL handling 0,0 is the left,bottom of the screen not left,top
	//		so we need to pass in these points to the calculation for the screen pos
	// 		and we need to flip the Y as well

	float z = GetTransform().HasTranslate() ? GetTranslate().z : 0.f;
	float3 WorldBoxBottomLeft( box.GetMin().x, box.GetMax().y, z );
	float3 WorldBoxTopRight( box.GetMax().x, box.GetMin().y, z );

	// Get bottom left corner of box as min point
	if(!pScreen->GetScreenPosFromWorldPos(*pRenderTarget, WorldBoxBottomLeft, ScreenPos[0]))
		return;

	// Get top right corner of box as max point
	if(!pScreen->GetScreenPosFromWorldPos(*pRenderTarget, WorldBoxTopRight, ScreenPos[1]))
		return;

	float2 fmin, fmax;

	// Use lowest values for max and min extents of the scissor box
	if(ScreenPos[0].x > ScreenPos[1].x)
	{
		fmin.x = (float)ScreenPos[1].x;
		fmax.x = (float)ScreenPos[0].x;
	}
	else
	{
		fmin.x = (float)ScreenPos[0].x;
		fmax.x = (float)ScreenPos[1].x;
	}

	if(ScreenPos[0].y > ScreenPos[1].y)
	{
		fmin.y = (float)ScreenPos[1].y;
		fmax.y = (float)ScreenPos[0].y;
	}
	else
	{
		fmin.y = (float)ScreenPos[0].y;
		fmax.y = (float)ScreenPos[1].y;
	}

	m_ViewBox = TLMaths::TBox2D(fmin, fmax);
}


void TRenderNodeScrollableView::PreDrawChildren(TLRender::TRenderTarget& RenderTarget,TLMaths::TTransform& SceneTransform)
{
	//	if we havent calculated a view box for this render target, then do it
	if ( m_RenderTargetRef != RenderTarget.GetRef() )
	{
		m_RenderTargetRef = RenderTarget.GetRef();
		OnRenderTargetRefChange( &RenderTarget );
	}

	// Enable scissoring
	//Opengl::EnableScissor(TRUE);

	// Set the scissor box
	//TODO: copy the current scissor box?
	if ( m_ViewBox.IsValid() )
		Opengl::SetScissor( (u32)m_ViewBox.GetLeft(), (u32)m_ViewBox.GetTop(), (u32)m_ViewBox.GetWidth(), (u32)m_ViewBox.GetHeight() );

	//	Only alter the scene transform if we have a valid scroll
	//	or there is some child alignment we need to apply
	if ( HasScroll() || ( m_AlignChildrenToClipDatum && m_ClipDatumOffset.HasAnyTransform() ) )
	{
		//	gr: rather than manipulate the transform in post draw, we can save the scene, then restore it
		//	note: ALWAYS match a BeginScene and EndScene (will assert if you don't as it corrupts scene)
		RenderTarget.BeginScene();

		//	offset children by modifying the transform
		if ( m_AlignChildrenToClipDatum )
		{
			SceneTransform.Transform( m_ClipDatumOffset );
			Opengl::SceneTransform( m_ClipDatumOffset );
		}

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
	if ( HasScroll() || ( m_AlignChildrenToClipDatum && m_ClipDatumOffset.HasAnyTransform() ) )
	{
		// Undo changes to transform
		RenderTarget.EndScene();
	}
}


void TLRender::TRenderNodeScrollableView::OnTransformChanged(u8 TransformChangedBits)
{
	//	transform changed, so world datum may have changed, need to recalculate view box
	if ( TransformChangedBits != 0x0 )
	{
		OnRenderTargetRefChange(NULL);
	}

	TLRender::TRenderNode::OnTransformChanged( TransformChangedBits );
}


