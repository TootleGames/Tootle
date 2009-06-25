#include "TWidgetThumbStick.h"
#include <TootleMaths/TShapeSphere.h>
#include <TootleRender/TRenderNode.h>

#include "TWidgetManager.h"

TLGui::TWidgetThumbStick::TWidgetThumbStick(TRefRef RenderTargetRef,TRefRef RenderNodeRef,TRefRef UserRef,TRefRef ActionOutDown,TRefRef ActionOutUp,float DeadZone) : 
	TLInput::TInputInterface	( RenderTargetRef, RenderNodeRef, UserRef, ActionOutDown, ActionOutUp ),
	m_DeadZone					( DeadZone )
{
}



//-------------------------------------------------
//	process a click and detect clicks on/off our render node. return SyncWait if we didnt process it and want to process again
//-------------------------------------------------
SyncBool TLGui::TWidgetThumbStick::ProcessClick(TClick& Click,TLRender::TScreen& Screen,TLRender::TRenderTarget& RenderTarget,TLRender::TRenderNode& RenderNode)
{
	Bool bClickAction = TLGui::g_pWidgetManager->IsClickActionRef(Click.GetActionRef());
	Bool bMoveAction = TLGui::g_pWidgetManager->IsMoveActionRef(Click.GetActionRef());
	Bool bCurrentAction = FALSE;
	Bool bActionPair = FALSE;
	
	if(m_ActionBeingProcessedRef.IsValid())
	{
		bCurrentAction = (Click.GetActionRef() == m_ActionBeingProcessedRef);
		bActionPair = TLGui::g_pWidgetManager->IsActionPair(m_ActionBeingProcessedRef, Click.GetActionRef());
	}
	
	Bool bTestIntersection = FALSE;
	
	if(m_ActionBeingProcessedRef.IsValid())
	{
		if(bClickAction && bCurrentAction)
		{
			if(Click.GetActionValue() == 0.f)
			{
				OnClickEnd( Click );
				m_ActionBeingProcessedRef.SetInvalid();
				return SyncTrue;
			}
		}
		else if(bMoveAction && bActionPair)
		{
			bTestIntersection = TRUE;
		}
	}
	else if(bClickAction)
	{
		if(Click.GetActionValue() == 0.f)
		{
			if(bCurrentAction)
			{
				OnClickEnd( Click );
				m_ActionBeingProcessedRef.SetInvalid();
				return SyncTrue;
			}
		}
		else 
		{
			// No 'click' action in progress yet.  Test intersection
			bTestIntersection = TRUE;
		}
	}
	
	
	if(!bTestIntersection)
		return SyncFalse;

	//	see if ray intersects our object, and creates a valid ray
	SyncBool Intersection = IsIntersecting(Screen, RenderTarget, RenderNode, Click );

	if ( Intersection == SyncTrue )
	{
		if(bClickAction)
			m_ActionBeingProcessedRef = Click.GetActionRef();
	}
	else if ( Intersection == SyncFalse )
	{
		OnClickEnd( Click );
		
		if(bClickAction)
			m_ActionBeingProcessedRef.SetInvalid();
		
		return SyncFalse;
	}
	else if ( Intersection == SyncWait )
	{
		return SyncWait;
	}

	 /*
	SyncBool Result = TInputInterface::ProcessClick(Click, Screen, RenderTarget, RenderNode);
	
	if(Result != SyncTrue)
		return Result;
	*/
	
	// Custom OnClick effectively...

	//	get the 2d circle bounds as the thumbstick
	//	if not valid, wait till next frame (off-screen?)
	const TLMaths::TShapeSphere2D& BoundsSphere = RenderNode.GetWorldBoundsSphere2D();
	if ( !BoundsSphere.IsValid() )
		return SyncWait;

	//	get vector from center of the sphere to the click point
	float2 ClickVector = Click.GetWorldRay().GetStart().xy() - BoundsSphere.GetCenter().xy();

	//	normalise vector to 0..1 in the sphere
	ClickVector /= BoundsSphere.GetSphere().GetRadius();

	//	get length to work out normalised vector and if we're in the deadzone
	float VectorLen = ClickVector.Length();

	//	in the dead zone, return TRUE (processed click) but don't send out a message
	//	this "ignores" the click
	if ( VectorLen <= m_DeadZone )
		return SyncTrue;

	//	send out thumb stick message
	TRef ActionOutRef = m_ActionOutDown;


	if ( ActionOutRef.IsValid() )
	{
	#ifdef _DEBUG
		TTempString Debug_String("WidgetThumbstick (");
		m_RenderNodeRef.GetString( Debug_String );
		Debug_String.Append(") sending thumbstick message: ");
		ActionOutRef.GetString( Debug_String );
		Debug_String.Appendf(": %.2f, %.2f", ClickVector.x, ClickVector.y );
		TLDebug_Print( Debug_String );
	#endif		
		
		//	make up fake input message
		TLMessaging::TMessage Message(TRef_Static(A,c,t,i,o));
		Message.Write( ActionOutRef );
		Message.ExportData("RawData", ClickVector );

		//	send message
		PublishMessage( Message );
	}

	return SyncTrue;
}

