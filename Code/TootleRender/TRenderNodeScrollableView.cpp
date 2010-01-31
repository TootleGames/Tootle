#include "TRenderNodeScrollableView.h"
#include "TRenderTarget.h"
#include "TScreenManager.h"
#include <TootleMaths/TShapeBox.h>
#include <TootleCore/TLMaths.h>

using namespace TLRender;


TRenderNodeScrollableView::TRenderNodeScrollableView(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNode					( RenderNodeRef, TypeRef ),
	m_LimitBox					( -TLMaths_FloatMax, -TLMaths_FloatMax, TLMaths_FloatMax, TLMaths_FloatMax),
	m_ClipDatumRef				( TLRender_TRenderNode_DatumBoundsBox2D ),
	m_uTempMomentumUpdateCount	( 0 ),
	m_bVerticalScroll			( TRUE ),
	m_bHorizontalScroll			( TRUE ),
	m_bDepthScroll				( FALSE ),
	m_AlignChildrenToClipDatum	( TRUE ),
	m_bUseMomentum				( FALSE ),
	m_bLimitToBounds			( FALSE ),
	m_ChildWorldTransformValid	( SyncFalse )
{
}



void TRenderNodeScrollableView::SetProperty(TLMessaging::TMessage& Message)
{
	TRenderNode::SetProperty(Message);

	if ( Message.ImportData("Datum", m_ClipDatumRef) )
		OnDatumChanged();

	if(Message.ImportData("RTarget", m_RenderTargetRef))
		OnRenderTargetRefChange();

	Message.ImportData("Horizontal", m_bHorizontalScroll);
	Message.ImportData("Vertical", m_bVerticalScroll);
	Message.ImportData("Depth", m_bDepthScroll);

	if ( Message.ImportData("AlignChildren", m_AlignChildrenToClipDatum ) )
		OnOffsetChanged();
	
	Message.ImportData("Momentum", m_bUseMomentum);	

	m_bLimitToBounds |= Message.ImportData("XMinLimit", m_LimitBox.GetMin().x);
	m_bLimitToBounds |= Message.ImportData("YMinLimit", m_LimitBox.GetMin().y);
	
	m_bLimitToBounds |= Message.ImportData("XMaxLimit", m_LimitBox.GetMax().x);
	m_bLimitToBounds |= Message.ImportData("YMaxLimit", m_LimitBox.GetMax().y);
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
				u8 uLimited = 0;
				ChangeScroll(Change, uLimited);
				
				if(m_bUseMomentum)
				{
					// Update the temp momentum
					if(m_bVerticalScroll)	m_fTempMomentum.y += Change.y;
					if(m_bHorizontalScroll)	m_fTempMomentum.x += Change.x;
					if(m_bDepthScroll)		m_fTempMomentum.z += Change.z;
					
					m_uTempMomentumUpdateCount++;
				}				
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
			u8 uLimited = 0;
			ChangeScroll(Change, uLimited);
			
			if(m_bUseMomentum)
			{
				// Update the temp momentum
				if(m_bVerticalScroll)	m_fTempMomentum.y += Change.y;
				if(m_bHorizontalScroll)	m_fTempMomentum.x += Change.x;
				if(m_bDepthScroll)		m_fTempMomentum.z += Change.z;
				
				m_uTempMomentumUpdateCount++;
				
			}
			
		}
	}
	else if ( Message.GetMessageRef() == TRef_Static(E,n,d,S,c) )	// EndScroll
	{
		// Scroll has finished - trigger the momentum scrolling
		if(m_bUseMomentum && (m_uTempMomentumUpdateCount > 0))
		{
			// Set the final momentum
			m_fMomentum = m_fTempMomentum / m_uTempMomentumUpdateCount;
			
			// Reset the temp momentum data
			m_fTempMomentum.Set(0.0f, 0.0f, 0.0f);
			m_uTempMomentumUpdateCount = 0;
		}
	}
	else if ( Message.GetMessageRef() == TRef_Static(B,e,g,S,c) )	// BegScroll (begin scroll)
	{
		// Stop any momentum motion
		m_fMomentum.Set(0.0f, 0.0f, 0.0f);
	}


	TRenderNode::ProcessMessage(Message);
}


void TRenderNodeScrollableView::ChangeScroll(const float3& delta, u8& uLimited)
{
	//	 change scroll
	if(m_bVerticalScroll)	GetScroll().y += delta.y;
	if(m_bHorizontalScroll)	GetScroll().x += delta.x;
	if(m_bDepthScroll)		GetScroll().z += delta.z;
	
	
	// Get the bounds box for the child node(s)
	/*
	 
	// NOTE: Bounds are going to be in 3D world space. Need transforming into screen space but not clipped to world view
	//		 Also, the bounds needs to be of the children not this node which doesn't *appear* to include
	//		 the children.  This could be because they don't get drawn in the same way or something?  Not sure.
	//		 Either way this is wrong.  When sorted should precalc everything and store in the m_LimitBox
	//		 so we don't have to do too much work to get this info.
	 
	TPtr<TLRender::TScreen> pScreen;
	TLRender::TRenderTarget* pRenderTarget = TLRender::g_pScreenManager->GetRenderTarget( m_RenderTargetRef, pScreen );

	const TLMaths::TBox2D& box = GetLocalBoundsBox2D().GetBox();
	
		 
	Type2<s32> ScreenPos;
	float fHeight = 10000.0f;
	
	float z = GetTransform().HasTranslate() ? GetTranslate().z : 0.f;
	float3 BoxBottomLeft( box.GetMin().x, box.GetMax().y, z );
	
	// Get bottom left corner of box as min point
	if(pScreen->GetScreenPosFromWorldPos(*pRenderTarget, BoxBottomLeft, ScreenPos))
		fHeight = ScreenPos.y;
	*/
	
	if(m_bLimitToBounds)
	{
		// Limit the scroll changes to within the bounds of the child nodes
		if(m_bVerticalScroll)
		{
			//NOTE: These checks are inverted due to how the menu on Rocket uses them for now.
			if(GetScroll().y > m_LimitBox.GetMin().y)
			{
				GetScroll().y = m_LimitBox.GetMin().y;
				uLimited |= TLRender::TRenderNodeScrollableView_LimitY;
			}
			else if(GetScroll().y < -m_LimitBox.GetHeight())
			{
				GetScroll().y = -m_LimitBox.GetHeight();
				uLimited |= TLRender::TRenderNodeScrollableView_LimitY;		
			}
		}
		
		if(m_bHorizontalScroll)
		{
			//NOTE: These checks are inverted due to how the menu on Rocket uses them for now.
			if(GetScroll().x > m_LimitBox.GetMin().x)
			{
				GetScroll().x = m_LimitBox.GetMin().x;
				uLimited |= TLRender::TRenderNodeScrollableView_LimitX;
			}
			else if(GetScroll().x < -m_LimitBox.GetWidth())
			{
				GetScroll().x = -m_LimitBox.GetWidth();
				uLimited |= TLRender::TRenderNodeScrollableView_LimitX;		
			}
		}
		
		//TODO: Depth scroll limiting
	}
	
	
	OnScrollChanged();	
}



void TRenderNodeScrollableView::OnRenderTargetRefChange(TLRender::TRenderTarget* pRenderTarget)
{
	//	reset viewbox
	m_ViewBox.SetInvalid();
	m_ClipDatumOffset.SetInvalid();
	OnOffsetChanged();
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
		if ( m_RenderTargetRef.IsValid() )
		{
			TLDebug_Break("Render target/screen expected");
		}
		return;
	}
	
	//	get the local datum first so we can get the local offset
	const TLMaths::TShapeBox2D* pLocalDatum = GetLocalDatum<TLMaths::TShapeBox2D>( m_ClipDatumRef );
	if ( !pLocalDatum )
	{
		TTempString Debug_String;
		Debug_String << "Datum " << m_ClipDatumRef << " not found (or not box2d shape) for scroll render node " << this->GetNodeRef();
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
	OnOffsetChanged();

	//	get box shape
	TLMaths::TShapeBox2D* pBox = pWorldDatum.GetObjectPointer<TLMaths::TShapeBox2D>();
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
	// Update the momentum
	if(m_bUseMomentum && m_fMomentum.LengthSq() > 0.0f)
	{
		m_fMomentum = TLMaths::Interp(m_fMomentum, float3(0.0f,0.0f,0.0f), 0.07f);
	
		u8 uLimited = 0;
		ChangeScroll(m_fMomentum, uLimited);
		/*
		// NOTE: Gives a bounce-off-wall type of effect which could be useful at some point
		// If limited, invert the momentum.
		if(uLimited & TLRender::TRenderNodeScrollableView_LimitY)
			m_fMomentum.y *= -1.0f;

		if(uLimited & TLRender::TRenderNodeScrollableView_LimitX)
			m_fMomentum.x *= -1.0f;
		
		if(uLimited & TLRender::TRenderNodeScrollableView_LimitZ)
			m_fMomentum.z *= -1.0f;
		 */

	}
	
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



//---------------------------------------------------------
//	return the world transform for our children. called by the child from GetWorldTransform. Overload if you transform your children in a specific way without using the base trasnform
//---------------------------------------------------------
const TLMaths::TTransform& TLRender::TRenderNodeScrollableView::GetChildWorldTransform(TRenderNode* pRootNode,Bool ForceCalculation)
{
	//	up to date
	//	gr: because this function isn't always neccessary (if the world transform is still valid post-render)
	//		then our m_ChildWorldTransform could be out of sync with the world transform validity.
	if ( !ForceCalculation && m_ChildWorldTransformValid == SyncTrue )
		return m_ChildWorldTransform;

	//	calculate our own world transform first
	const TLMaths::TTransform& ThisWorldTransform = GetWorldTransform( pRootNode, ForceCalculation );

	//	copy our world transform...
	m_ChildWorldTransform = ThisWorldTransform;
	
	//	apply our own changes for the children
	if ( HasScroll() || ( m_AlignChildrenToClipDatum && m_ClipDatumOffset.HasAnyTransform() ) )
	{
		if ( m_AlignChildrenToClipDatum )
			m_ChildWorldTransform.Transform( m_ClipDatumOffset );
		m_ChildWorldTransform.Transform( m_ScrollTransform );
	}

	//	child transform now up to date
	m_ChildWorldTransformValid = SyncTrue;
	
	return m_ChildWorldTransform;
}

//---------------------------------------------------------
//	world transform has changed, invalidate child world transform
//---------------------------------------------------------
Bool TLRender::TRenderNodeScrollableView::SetWorldTransformOld(Bool SetPosOld,Bool SetTransformOld,Bool SetShapesOld)
{
	Bool Changed = FALSE;

	if ( SetTransformOld && m_ChildWorldTransformValid == SyncTrue )
	{
		m_ChildWorldTransformValid = SyncWait;
		Changed = TRUE;
	}

	Changed |= TLRender::TRenderNode::SetWorldTransformOld( SetPosOld, SetTransformOld, SetShapesOld );
	return Changed;
}


//---------------------------------------------------------
//	called when scroll changes
//---------------------------------------------------------
void TLRender::TRenderNodeScrollableView::OnScrollChanged()
{
	//	gr: when we change the scroll (ie. changing the world transform for the children) then really
	//		we need to invalidate our children otherwise their world bounds/transforms are going to be
	//		out of date and when we try and get the bounds (eg. for widget clicking) they're going to
	//		be in the wrong place - missing the scroll offset.
	//	note: also need to think about the visibility of the bounds for the widget... either by clipping
	//		datums, or some kinda "check point isn't clipped" functionality when ray casting into it...
	//		not sure of the best (ie. still efficient) way of doing this...

	//	gr: untested
	SetBoundsInvalid( TInvalidateFlags( ForceInvalidateChildren, InvalidateChildWorldBounds, InvalidateChildWorldPos ) );

}

