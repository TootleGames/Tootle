#include "TWidgetScrollbar.h"
#include <TootleRender/TRenderGraph.h>
#include <TootleRender/TScreenManager.h>
#include <TootleCore/TCoreManager.h>



TLGui::TWidgetScrollbar::TWidgetScrollbar(TRefRef RenderTargetRef,TRefRef ScrollBarRenderNode,TRefRef SliderRenderNode,TRefRef UserRef,TRefRef ActionOut,float InitialScrollValue) :
	m_ScrollValue			( InitialScrollValue ),
	m_ScrollBarRenderNode	( ScrollBarRenderNode ),
	m_SliderRenderNode		( SliderRenderNode ),
	m_SliderPosValid		( FALSE ),
	TGui					( RenderTargetRef, TRef(), UserRef, ActionOut )
{
	//	check an invalid scroll value provided is valid
	if ( m_ScrollValue < 0.f || m_ScrollValue > 1.f )
	{
		TLDebug_Break("Scrollbar value should be between 0 and 1");
		TLMaths::Limit( m_ScrollValue, 0.f, 1.f );
	}
}


//-------------------------------------------------
//	process a click and detect clicks on/off our render node. return SyncWait if we didnt process it and want to process again
//-------------------------------------------------
SyncBool TLGui::TWidgetScrollbar::ProcessClick(const TClick& Click,TLRender::TScreen& Screen,TLRender::TRenderTarget& RenderTarget,TPtr<TLRender::TRenderNode>& pRenderNode)
{
	//	mouse up, dont need to do anything
	if ( Click.m_ActionValue == 0.f )
		return SyncFalse;

	//	get render node for the scroll bar
	TPtr<TLRender::TRenderNode>& pScrollBarRenderNode = TLRender::g_pRendergraph->FindNode( m_ScrollBarRenderNode );
	if ( !pScrollBarRenderNode )
		return SyncWait;

	//	bounds aren't valid, wait till next time
	if ( !pScrollBarRenderNode->GetWorldBoundsBox().IsValid() )
	{
		return SyncWait;
	}

	//	get z center of bounds box to use as the z for the world pos from cursor
	float worldz = pScrollBarRenderNode->GetWorldBoundsBox().GetCenter().z;

	//	get a world position from the cursor
	float3 WorldPos;
	if ( !Screen.GetWorldPosFromScreenPos( RenderTarget, WorldPos, worldz, Click.m_CursorPos ) )
	{
		//	click was out of the render target so we couldnt get a pos
		return SyncFalse;
	}

	//	flatten bounds to 2D shape
	//	todo: 3D->2D proper conversion via screen/camera
	TLMaths::TBox2D BoundsBox( pScrollBarRenderNode->GetWorldBoundsBox().GetMin().xy(), pScrollBarRenderNode->GetWorldBoundsBox().GetMax().xy() );

	//	check the click-y is inside the box
	if ( WorldPos.y < BoundsBox.GetTop() || WorldPos.y > BoundsBox.GetBottom() )
		return SyncFalse;

	//	now get where along the x is, on a scale of 0..1
	WorldPos.x -= BoundsBox.GetLeft();

	//	divide by width (will turn X to 0..1)
	WorldPos.x /= BoundsBox.GetWidth();

	//	outside of box
	if ( WorldPos.x < 0.f || WorldPos.x > 1.f )
		return SyncFalse;

	//	inside box! new value!
	SetScrollValue( WorldPos.x );

	return SyncTrue;
}


//-------------------------------------------------
//	get array of all the render nodes we're using
//-------------------------------------------------
Bool TLGui::TWidgetScrollbar::Update()
{
	Bool NeedsUpdate = TGui::Update();

	//	update slider pos
	if ( !m_SliderPosValid )
		UpdateSliderPos();

	//	slider pos still needs update
	if ( !m_SliderPosValid )
		NeedsUpdate = TRUE;

	return NeedsUpdate;
}


//-------------------------------------------------
//	get array of all the render nodes we're using
//-------------------------------------------------
void TLGui::TWidgetScrollbar::GetRenderNodes(TArray<TRef>& RenderNodeArray)
{
	RenderNodeArray.Add( m_ScrollBarRenderNode );
	RenderNodeArray.Add( m_SliderRenderNode );
}

	
//-------------------------------------------------
//	set value and send out message if it changes
//-------------------------------------------------
void TLGui::TWidgetScrollbar::SetScrollValue(float NewValue)
{
	if ( m_ScrollValue != NewValue )
	{
		//	update value
		m_ScrollValue = NewValue;

		//	send out message
		SendActionMessage( TRUE, m_ScrollValue );

		//	update graphics
		m_SliderPosValid = FALSE;
	}
	
	if ( !m_SliderPosValid )
		UpdateSliderPos();
}


//-------------------------------------------------
//	when init has finished set the position of the slider
//-------------------------------------------------
void TLGui::TWidgetScrollbar::OnInitialised()
{
	UpdateSliderPos();

	//	need to update slider pos
	if ( !m_SliderPosValid )
	{
		this->SubscribeTo( TLCore::g_pCoreManager );
	}
}


//-------------------------------------------------
//	update graphical position of slider
//-------------------------------------------------
void TLGui::TWidgetScrollbar::UpdateSliderPos()
{
	//	this is a kind of reverse of the ProcessClick code... get the box, then find point along it, and turn it 3D

	//	get render node for the slider and scroll bar
	TPtr<TLRender::TRenderNode>& pSliderRenderNode = TLRender::g_pRendergraph->FindNode( m_SliderRenderNode );
	if ( !pSliderRenderNode )
		return;

	TPtr<TLRender::TRenderNode>& pScrollBarRenderNode = TLRender::g_pRendergraph->FindNode( m_ScrollBarRenderNode );
	if ( !pScrollBarRenderNode )
		return;

	//	bounds aren't valid, wait till next time
	const TLMaths::TBox& BoundsBox3 = pScrollBarRenderNode->GetWorldBoundsBox();
	if ( !BoundsBox3.IsValid() )
		return;

	//	get the center line (x) through the box
	float3 BoxCenter = BoundsBox3.GetCenter();
	TLMaths::TLine BoxCenterLine( float3( BoundsBox3.GetMin().x, BoundsBox3.GetMin().y, BoxCenter.z ), float3( BoundsBox3.GetMax().x, BoundsBox3.GetMin().y, BoxCenter.z ) );

	float3 SliderPos = BoxCenterLine.GetStart() + ( BoxCenterLine.GetDirection() * m_ScrollValue );
	pSliderRenderNode->SetTranslate( SliderPos );

	m_SliderPosValid = TRUE;
}


