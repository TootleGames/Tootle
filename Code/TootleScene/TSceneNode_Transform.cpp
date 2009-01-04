#include "TSceneNode_Transform.h"
#include <TootleMaths/TLine.h>

using namespace TLScene;

void TSceneNode_Transform::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	if(pMessage->GetMessageRef() == "TRANSFORM")
	{
		float3 vTransform;

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


