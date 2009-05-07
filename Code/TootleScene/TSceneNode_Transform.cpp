#include "TSceneNode_Transform.h"
#include <TootleMaths/TLine.h>
#include "TScenegraph.h"



TLScene::TSceneNode_Transform::TSceneNode_Transform(TRefRef NodeRef,TRefRef TypeRef) :
	TSceneNode			( NodeRef, TypeRef ),
	m_ZoneInitialised	( FALSE )
{
	//	gr: initialise the transform so that the translate (and therefore Position of Transform node) is always valid
	m_Transform.SetTranslate( float3( 0.f, 0.f, 0.f ) );
}



//---------------------------------------------------------
//	generic initialise data
//---------------------------------------------------------
void TLScene::TSceneNode_Transform::Initialise(TLMessaging::TMessage& Message)
{
	// Add event channel for the transform message
	TPtr<TLMessaging::TEventChannel> pEventChannel = RegisterEventChannel( TRef_Static(O,n,T,r,a) );	

	if(pEventChannel)
		pEventChannel->SubscribeTo(this);

	//	gr: moved to be done FIRST to initialise flags (ie. if disabled)
	//	do inherited initialise
	TLScene::TSceneNode::Initialise( Message );

	//	read transform info (same as render node's init)
	u8 TransformChangedBits = m_Transform.ImportData( Message );

	//	Has transform been changed?
	if( TransformChangedBits )
		OnTransformChanged( TransformChangedBits );

	//	initialise zone
	InitialiseZone();
}


void TLScene::TSceneNode_Transform::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	gr: only apply change if explicitly sent to change
	if(Message.GetMessageRef() == TRef_Static(D,o,T,r,a) )
	{
		u8 TransformChangedBits = m_Transform.ImportData( Message );
		OnTransformChanged( TransformChangedBits );
	}
	else if(Message.GetMessageRef() == TRef_Static(R,e,q,T,r) )
	{
		float3 vVector;
		TLMaths::TQuaternion qRot;

		// Delta position/rotation/scale that are requested from other things i.e. editor
		if(Message.ImportData(TRef_Static(T,r,a,n,s), vVector))
		{
			// Apply translation
			Translate(vVector);
		}

		if(Message.ImportData("Rotate", qRot))
		{
			//Apply rotation
			TLDebug_Break("Rotate message received - needs implementing");
		}
		
		if(Message.ImportData(TRef_Static(S,c,a,l,e), vVector))
		{
			// Apply scale
			TLDebug_Break("Scale message received - needs implementing");
		}

		return;
	}

	// Super class process message
	TSceneNode::ProcessMessage(Message);
}


// Transform changed
void TLScene::TSceneNode_Transform::OnTransformChanged(u8 TransformChangedBits)
{
	//	if translation changed then set zone out of date
	if ( TransformChangedBits & TLMaths_TransformBitTranslate )
		TLMaths::TQuadTreeNode::SetZoneOutOfDate();	

	//	no changes
	if ( !TransformChangedBits )
		return;

	//	no one to send a message to 
	if ( !HasSubscribers() )
		return;

	TLMessaging::TMessage Message( TRef_Static(O,n,T,r,a), GetNodeRef());

	//	write trasform data, only publish if something was written.
	if ( m_Transform.ExportData( Message, TransformChangedBits ) != 0x0 )
	{
		PublishMessage(Message);
	}
}


void TLScene::TSceneNode_Transform::Translate(float3 vTranslation)
{
	//	no change
	if ( vTranslation.LengthSq() == 0.f )
		return;

	//	if the current translate is valid, move it, else explicitly set new translate.
	if ( GetTransform().HasTranslate() )
	{
		vTranslation += GetTranslate();
	}
	
	SetTranslate( vTranslation );
}


float TLScene::TSceneNode_Transform::GetDistanceTo(const TLMaths::TLine& Line)
{
	float3 vPos = GetPosition();

	// Do distance check from node to line
	return Line.GetDistanceSq(vPos);
}


//------------------------------------------------------
//	
//------------------------------------------------------
void TLScene::TSceneNode_Transform::PostUpdate(float fTimestep)
{
	//	update zone if out of date
	if ( IsZoneOutOfDate() )
	{
		TPtr<TLMaths::TQuadTreeZone>& pRootZone = TLScene::g_pScenegraph->GetRootZone();
		if ( pRootZone )
		{
			//	get our ptr
			TPtr<TLScene::TSceneNode>& pThisSceneNode = TLScene::g_pScenegraph->FindNode( GetNodeRef() );
			if ( pThisSceneNode )
			{
				TPtr<TLScene::TSceneNode_Transform> pThisSceneNodeTransform = pThisSceneNode;
				TPtr<TLMaths::TQuadTreeNode> pThisQuadTreeNode = pThisSceneNodeTransform;
				UpdateZone( pThisQuadTreeNode, pRootZone );
			}
			else
			{
				TLDebug_Break("Failed to find TPtr for this!");
			}
		}
	}
}


//------------------------------------------------------
//	our zone has changed - if we're the node being tracked in the graph, change the active zone
//------------------------------------------------------
void TLScene::TSceneNode_Transform::OnZoneChanged(TLMaths::TQuadTreeZone* pOldZone)
{
	TPtr<TLMaths::TQuadTreeZone>& pNewZone = GetZone();

	//	are we the tracked zone node
	if ( TLScene::g_pScenegraph->GetActiveZoneTrackNode() == GetNodeRef() )
	{
		TLScene::g_pScenegraph->SetActiveZone( pNewZone );
	}

	//	check for change of zone activity now that we've changed zone
	SyncBool OldZoneActive = pOldZone ? pOldZone->IsActive() : SyncFalse;
	SyncBool NewZoneActive = pNewZone ? pNewZone->IsActive() : SyncFalse;

	if ( OldZoneActive != NewZoneActive )
	{
		if ( NewZoneActive == SyncFalse )
		{
			OnZoneSleep();
		}
		else
		{
			OnZoneWake( NewZoneActive );
		}
	}
}


//------------------------------------------------------
//	
//------------------------------------------------------
SyncBool TLScene::TSceneNode_Transform::IsInShape(const TLMaths::TBox2D& Shape)
{
	if ( Shape.GetIntersection( GetPosition() ) )
		return SyncTrue;
	else
		return SyncFalse;
}


//------------------------------------------------------
//	if zone isn't initialised, initialise it
//------------------------------------------------------
void TLScene::TSceneNode_Transform::InitialiseZone()
{
	//	transform has been set for the first time, initialise zone
	if ( m_ZoneInitialised )
		return;

	//	cannot initialise zone without a root zone...
	TPtr<TLMaths::TQuadTreeZone>& pRootZone = TLScene::g_pScenegraph->GetRootZone();
	if ( !pRootZone )
		return;

	TPtr<TLScene::TSceneNode_Transform> pThis = TLScene::g_pScenegraph->FindNode( GetNodeRef() );
	TPtr<TLMaths::TQuadTreeNode> pQuadTreeThis = pThis;
	TLMaths::TQuadTreeNode::UpdateZone( pQuadTreeThis, pRootZone );

	//	gr: replaced with IsZoneAwake() usage to cope with non-zoned scenegraphs
	SyncBool ZoneActive = IsZoneAwake();

	//	if our initial zone is inactive, sleep
	if ( ZoneActive == SyncFalse )
		OnZoneSleep();

	//	gr: set as not-initialised if no zone?
	m_ZoneInitialised = TRUE;
}


