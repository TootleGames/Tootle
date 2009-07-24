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

	//	initialise zone
	InitialiseZone();
}


//---------------------------------------------------------
//	set node properties
//---------------------------------------------------------
void TLScene::TSceneNode_Transform::SetProperty(TLMessaging::TMessage& Message)
{
	//	read transform info (same as render node's init)
	u8 TransformChangedBits = m_Transform.ImportData( Message );

	//	Has transform been changed?
	if( TransformChangedBits )
		OnTransformChanged( TransformChangedBits );

	//	super setproperty
	TLScene::TSceneNode::SetProperty( Message );
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
void TLScene::TSceneNode_Transform::UpdateNodeData()
{
	//	update transform by clearing out old values and adding new ones
	GetNodeData().RemoveChild(TRef_Static(T,r,a,n,s));
	GetNodeData().RemoveChild(TRef_Static(S,c,a,l,e));
	GetNodeData().RemoveChild(TRef_Static(R,o,t,a,t));
	GetTransform().ExportData( GetNodeData() );

	//	inherited update
	TLScene::TSceneNode::UpdateNodeData();
}


void TLScene::TSceneNode_Transform::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	gr: same format as TRenderNode; "SetTransform" to overwrite, "DoTransform" to change
	if(Message.GetMessageRef() == TRef_Static(S,e,t,T,r) )
	{
		//	overwrite our transform
		u8 TransformChangedBits = m_Transform.ImportData( Message );
		OnTransformChanged(TransformChangedBits);

		return;
	}
	else if(Message.GetMessageRef() == TRef_Static(L,o,c,T,r))	//	local transform
	{
		//	read sent transform
		TLMaths::TTransform Transform;
		Transform.ImportData( Message );
		
		//	modify our existing transform by this transform
		//	gr: this takes Transform, localises the changes (eg. rotate and scale the translate) and then sets the values. 
		//	This is kinda okay for rotations and scales, but wrong for translations. This is like a Matrix multiply
		u8 TransformChangedBits = m_Transform.Transform_HasChanged( Transform );
		OnTransformChanged( TransformChangedBits );
		return;
	}
	else if(Message.GetMessageRef() == TRef_Static(D,o,T,r,a))
	{
		//	read sent transform
		TLMaths::TTransform Transform;
		Transform.ImportData( Message );
		
		//	modify our existing transform by this transform
		u8 TransformChangedBits = m_Transform.AddTransform_HasChanged( Transform );
		OnTransformChanged( TransformChangedBits );
		return;
	}

	// Super class process message
	TSceneNode::ProcessMessage(Message);
}


// Transform changed
void TLScene::TSceneNode_Transform::OnTransformChanged(u8 TransformChangedBits)
{
	//	no changes
	if ( !TransformChangedBits )
		return;

	//	if translation changed then set zone out of date
	if ( TransformChangedBits & TLMaths_TransformBitTranslate )
		TLMaths::TQuadTreeNode::SetZoneOutOfDate();	

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


float TLScene::TSceneNode_Transform::GetDistanceTo(const TLMaths::TLine& Line)
{
	TLDebug_Break("gr; note; this returns SQuared distance, not distance");
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


