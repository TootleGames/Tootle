
#pragma once

#include <TootleCore/TLMaths.h>

#include "TSceneNode.h"

namespace TLScene
{
		class TSceneNode_Transform;
};

/*
	TSceneNode_Transform class
*/
class TLScene::TSceneNode_Transform : public TLScene::TSceneNode
{
public:
	TSceneNode_Transform(TRefRef NodeRef,TRefRef TypeRef) :
	  TSceneNode(NodeRef,TypeRef)
	{
	}

	virtual void				Initialise(TPtr<TLMessaging::TMessage>& pMessage);
	virtual Bool				HasTransform()	{ return TRUE; }

	FORCEINLINE float3			GetPosition() const						{	float3 Pos( 0,0,0 );	GetPosition( Pos );	return Pos;	}
	FORCEINLINE void			GetPosition(float3& Pos) const			{	GetTransform().TransformVector( Pos );	}	//	you can use this to get a relative offset from this node by initialising Pos to the [local] offset you want
	FORCEINLINE	const float3&	GetTranslate() const					{	return m_Transform.GetTranslate();	}
	FORCEINLINE void			SetTranslate(const float3& fPos)		{	m_Transform.SetTranslate(fPos);	OnTransformChanged();	}

	const TLMaths::TTransform&	GetTransform() const								{	return m_Transform;	}
	void						SetTransform(const TLMaths::TTransform& Transform)	{	m_Transform = Transform;	}	

	// Distance checks
	virtual float				GetDistanceTo(const TLMaths::TLine& Line);

protected:
	virtual void				ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);

	virtual void				Translate(float3 vTranslation);
	//virtual void				Rotate(float3 vRotation);
	//virtual void				Scale(float3 vScale);
	virtual void				OnTransformChanged();

private:
	TLMaths::TTransform			m_Transform;
};

