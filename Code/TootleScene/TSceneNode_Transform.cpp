#include "TSceneNode_Transform.h"
#include <TootleMaths/TLine.h>

using namespace TLScene;


	
//---------------------------------------------------------
//	generic initialise data
//---------------------------------------------------------
void TLScene::TSceneNode_Transform::Initialise(TLMessaging::TMessage& Message)
{
	//	read transform info (same as render node's init)
	Bool TransformChanged = FALSE;

	if ( Message.ImportData("Translate", m_Transform.GetTranslate() ) == SyncTrue )
	{
		m_Transform.SetTranslateValid();
		TransformChanged = TRUE;
	}

	if ( Message.ImportData("Scale", m_Transform.GetScale() ) == SyncTrue )
	{
		m_Transform.SetScaleValid();
		TransformChanged = TRUE;
	}

	if ( Message.ImportData("Rotation", m_Transform.GetRotation() ) == SyncTrue )
	{
		m_Transform.SetRotationValid();
		TransformChanged = TRUE;
	}

	//	transform has been set
	if ( TransformChanged )
		OnTransformChanged();

	//	do inherited initialise
	TLScene::TSceneNode::Initialise( Message );
}


void TSceneNode_Transform::ProcessMessage(TLMessaging::TMessage& Message)
{
	if(Message.GetMessageRef() == "TRANSFORM")
	{

		float3 vVector;
		TLMaths::TQuaternion qRot;
		Bool bTranformChanged = FALSE;

		// Absolute position/rotation/scale setting
		if(Message.ImportData("Translate", vVector))
		{
			m_Transform.SetTranslate(vVector);
			bTranformChanged = TRUE;
			//SetTranslate(vVector);
		}

		if(Message.ImportData("Rotation", qRot))
		{
			m_Transform.SetRotation(qRot);
			bTranformChanged = TRUE;
			//SetRotation(qRot);
		}

		if(Message.ImportData("Scale", vVector))
		{
			m_Transform.SetScale(vVector);
			bTranformChanged = TRUE;
			//SetScale(vVector);
		}

		// If any part of the transform changed forward the message on
		if(bTranformChanged)
		{
			//	gr: inject our ref so subscriber knows what node has changed
			Message.AddChildAndData("SNRef", this->GetNodeRef() );
			Message.AddChildAndData("SNType", this->GetNodeTypeRef() );

			PublishMessage(Message);
		}


		// Delta position/rotation/scale that are requested from other things i.e. editor
		if(Message.ImportData("ReqTranslate", vVector))
		{
			// Apply translation
			Translate(vVector);
		}

		if(Message.ImportData("ReqRotate", qRot))
		{
			//Apply rotation
			TLDebug_Break("Rotate message received - needs implementing");
		}
		
		if(Message.ImportData("ReqScale", vVector))
		{
			// Apply scale
			TLDebug_Break("Scale message received - needs implementing");
		}

		return;
	}

	// Super class process message
	TSceneNode::ProcessMessage(Message);
}

// Translation changed
void TSceneNode_Transform::OnTranslationChanged()
{
	TLMessaging::TMessage Message("TRANSFORM");
	Message.ExportData("Translate", GetTranslate());
	PublishMessage(Message);
}

// Rotation changed
void TSceneNode_Transform::OnRotationChanged()
{
	TLMessaging::TMessage Message("TRANSFORM");
	Message.ExportData("Rotation", GetRotation());
	PublishMessage(Message);
}

// Scale changed
void TSceneNode_Transform::OnScaleChanged()
{
	TLMessaging::TMessage Message("TRANSFORM");
	Message.ExportData("Scale", GetScale());
	PublishMessage(Message);
}

// Entire transform changed
void TSceneNode_Transform::OnTransformChanged()
{
	TLMessaging::TMessage Message("TRANSFORM");
	Message.ExportData("Translate", GetTranslate());
	Message.ExportData("Rotation", GetRotation());
	Message.ExportData("Scale", GetScale());
	PublishMessage(Message);
}


void TSceneNode_Transform::Translate(float3 vTranslation)
{
	float3 fPos = GetTranslate();
	fPos += vTranslation;
	SetTranslate(fPos);
}


float TSceneNode_Transform::GetDistanceTo(const TLMaths::TLine& Line)
{
	float3 vPos = GetPosition();

	// Do distance check from node to line
	return Line.GetDistanceSq(vPos);
}


