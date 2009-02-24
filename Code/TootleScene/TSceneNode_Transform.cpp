#include "TSceneNode_Transform.h"
#include <TootleMaths/TLine.h>

using namespace TLScene;


	
//---------------------------------------------------------
//	generic initialise data
//---------------------------------------------------------
void TLScene::TSceneNode_Transform::Initialise(TPtr<TLMessaging::TMessage>& pMessage)
{
	if ( pMessage )
	{
		//	read transform info (same as render node's init)
		Bool TransformChanged = FALSE;

		if ( pMessage->ImportData("Translate", m_Transform.GetTranslate() ) == SyncTrue )
		{
			m_Transform.SetTranslateValid();
			TransformChanged = TRUE;
		}

		if ( pMessage->ImportData("Scale", m_Transform.GetScale() ) == SyncTrue )
		{
			m_Transform.SetScaleValid();
			TransformChanged = TRUE;
		}

		if ( pMessage->ImportData("Rotation", m_Transform.GetRotation() ) == SyncTrue )
		{
			m_Transform.SetRotationValid();
			TransformChanged = TRUE;
		}

		//	transform has been set
		if ( TransformChanged )
			OnTransformChanged();
	}

	//	do inherited initialise
	TLScene::TSceneNode::Initialise( pMessage );
}


void TSceneNode_Transform::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	if(pMessage->GetMessageRef() == "TRANSFORM")
	{
		float3 vTransform;

		//	gr: note "translate" in init messages is explicit rather than change. change this to MoveTranslate, MoveRotate, MoveScale?
		if(pMessage->ImportData("TRANSLATE", vTransform))
		{
			// Apply translation
			Translate(vTransform);
		}
		
		if(pMessage->ImportData("ROTATE", vTransform))
		{
			//Apply rotation
			TLDebug_Break("Rotate message received - needs implementing");
		}
		
		if(pMessage->ImportData("SCALE", vTransform))
		{
			// Apply scale
			TLDebug_Break("Scale message received - needs implementing");
		}

	}

	// Super class process message
	TSceneNode::ProcessMessage(pMessage);
}


void TSceneNode_Transform::OnTransformChanged()
{
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


