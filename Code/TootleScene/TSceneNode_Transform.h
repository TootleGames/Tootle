
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

  	virtual Bool	HasTransform()	{ return TRUE; }


	FORCEINLINE	const float3&	GetPosition() const						{	return m_Transform.GetTranslate();	}
	FORCEINLINE void			SetPosition(const float3& fPos)			{	m_Transform.SetTranslate(fPos);	}
	FORCEINLINE	const float3&	GetTranslate() const					{	return m_Transform.GetTranslate();	}
	FORCEINLINE void			SetTranslate(const float3& fPos)		{	m_Transform.SetTranslate(fPos);	}

	const TLMaths::TTransform&	GetTransform() const								{	return m_Transform;	}
	void						SetTransform(const TLMaths::TTransform& Transform)	{	m_Transform = Transform;	}	

	// Distance checks
	virtual float				GetDistanceTo(const TLMaths::TLine& Line);

protected:
	virtual void				ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);

	virtual void				Translate(float3 vTranslation);
	//virtual void				Rotate(float3 vRotation);
	//virtual void				Scale(float3 vScale);

private:
	TLMaths::TTransform			m_Transform;
};

