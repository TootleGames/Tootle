#include "TSceneNode_Transform.h"
#include <TootleMaths/TLine.h>

using namespace TLScene;


	
//---------------------------------------------------------
//	generic initialise data
//---------------------------------------------------------
void TLScene::TSceneNode_Transform::Initialise(TLMessaging::TMessage& Message)
{
	// Add event channel for the transform message
	TPtr<TLMessaging::TEventChannel> pEventChannel = RegisterEventChannel("OnTransform");	

	if(pEventChannel)
		pEventChannel->SubscribeTo(this);


	//	read transform info (same as render node's init)
	Bool bTranslation, bRotation, bScale;
	bTranslation = bRotation = bScale = FALSE;

	if ( Message.ImportData("Translate", m_Transform.GetTranslate() ) == SyncTrue )
	{
		m_Transform.SetTranslateValid();
		bTranslation = TRUE;
	}

	if ( Message.ImportData("Rotation", m_Transform.GetRotation() ) == SyncTrue )
	{
		m_Transform.SetRotationValid();
		bRotation = TRUE;
	}

	if ( Message.ImportData("Scale", m_Transform.GetScale() ) == SyncTrue )
	{
		m_Transform.SetScaleValid();
		bScale = TRUE;
	}

	//	Has transform been changed?
	if( bTranslation || bRotation || bScale )
		OnTransformChanged(bTranslation, bRotation, bScale);

	//	do inherited initialise
	TLScene::TSceneNode::Initialise( Message );
}


void TSceneNode_Transform::ProcessMessage(TLMessaging::TMessage& Message)
{
	if(Message.GetMessageRef() == "Physics")
	{
		float3 vVector;
		TLMaths::TQuaternion qRot;
		Bool bTranslation, bRotation, bScale;
		bTranslation = bRotation = bScale = FALSE;

		// Absolute position/rotation/scale setting
		if(Message.ImportData("Translate", vVector))
		{
			m_Transform.SetTranslate(vVector);
			bTranslation = TRUE;
		}

		if(Message.ImportData("Rotation", qRot))
		{
			m_Transform.SetRotation(qRot);
			bRotation = TRUE;
		}

		if(Message.ImportData("Scale", vVector))
		{
			m_Transform.SetScale(vVector);
			bScale = TRUE;
		}

		// If any part of the transform changed forward the message on
		if(bTranslation || bRotation || bScale)
		{
			// Publish a new message with 'Node' ID
			OnTransformChanged(bTranslation, bRotation, bScale);
		}
	}
	else if(Message.GetMessageRef() == "Editor")
	{
		float3 vVector;
		TLMaths::TQuaternion qRot;

		// Delta position/rotation/scale that are requested from other things i.e. editor
		if(Message.ImportData("Translate", vVector))
		{
			// Apply translation
			Translate(vVector);
		}

		if(Message.ImportData("Rotate", qRot))
		{
			//Apply rotation
			TLDebug_Break("Rotate message received - needs implementing");
		}
		
		if(Message.ImportData("Scale", vVector))
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
void TSceneNode_Transform::OnTransformChanged(Bool bTranslation, Bool bRotation, Bool bScale)
{
	TLMessaging::TMessage Message("Node");
	Message.AddChannelID("OnTransform");

	if(bTranslation && m_Transform.HasTranslate() )
		Message.ExportData("Translate", GetTranslate());

	if(bRotation && m_Transform.HasRotation() )
		Message.ExportData("Rotation", GetRotation());

	if(bScale && m_Transform.HasScale() )
		Message.ExportData("Scale", GetScale());

	//	gr: inject our ref so subscriber knows what node has changed
	Message.AddChildAndData("SNRef", GetNodeRef() );
	Message.AddChildAndData("SNType", GetNodeTypeRef() );

	PublishMessage(Message);
}


void TSceneNode_Transform::Translate(float3 vTranslation)
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


float TSceneNode_Transform::GetDistanceTo(const TLMaths::TLine& Line)
{
	float3 vPos = GetPosition();

	// Do distance check from node to line
	return Line.GetDistanceSq(vPos);
}


