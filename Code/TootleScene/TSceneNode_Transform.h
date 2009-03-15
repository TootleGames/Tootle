#pragma once

#include <TootleCore/TLMaths.h>
#include <TootleMaths/TQuadTree.h>
#include "TSceneNode.h"



namespace TLScene
{
	class TSceneNode_Transform;
};



//------------------------------------------------------------------
//	TSceneNode_Transform class
//	gr: this node type (with positional information) is derived from a quad 
//	tree node so it can be placed in a zone
//------------------------------------------------------------------
class TLScene::TSceneNode_Transform : public TLScene::TSceneNode, public TLMaths::TQuadTreeNode
{
public:
	TSceneNode_Transform(TRefRef NodeRef,TRefRef TypeRef) :
	  TSceneNode(NodeRef,TypeRef)
	{
	}

	virtual Bool				HasTransform()	{ return TRUE; }

	FORCEINLINE const float3&	GetPosition() const												{	return GetTranslate();	}					//	our position will be 
	FORCEINLINE void			GetLocalPosition(float3& Pos) const								{	GetTransform().TransformVector( Pos );	}	//	get a position relative to the scene node

	FORCEINLINE	const float3&	GetTranslate() const											{	return m_Transform.GetTranslate();	}
	FORCEINLINE void			SetTranslate(const float3& vPos)								{	m_Transform.SetTranslate(vPos);	OnTranslationChanged();	}

	FORCEINLINE	const TLMaths::TQuaternion&		GetRotation() const								{	return m_Transform.GetRotation();	}
	FORCEINLINE void							SetRotation(const TLMaths::TQuaternion& qRot)	{	m_Transform.SetRotation(qRot);	OnRotationChanged();	}

	FORCEINLINE	const float3&	GetScale() const												{	return m_Transform.GetScale();	}
	FORCEINLINE void			SetScale(const float3& vScale)									{	m_Transform.SetScale(vScale);	OnScaleChanged();	}

	const TLMaths::TTransform&	GetTransform() const								{	return m_Transform;	}
	void						SetTransform(const TLMaths::TTransform& Transform)	{	m_Transform = Transform; OnTransformChanged(TRUE, TRUE, TRUE); }	

	// Distance checks
	virtual float				GetDistanceTo(const TLMaths::TLine& Line);			//	gr: note, this returns SQUARED distance! bad function naming!

protected:
	virtual void				Initialise(TLMessaging::TMessage& Message);
	virtual void				PostUpdate(float fTimestep);

	virtual void				ProcessMessage(TLMessaging::TMessage& Message);

	virtual void				Translate(float3 vTranslation);
	//virtual void				Rotate(float3 vRotation);
	//virtual void				Scale(float3 vScale);
	TLMaths::TTransform&		GetTransform() 							{	return m_Transform;	}

	FORCEINLINE void			OnTranslationChanged()					{	TLMaths::TQuadTreeNode::SetZoneOutOfDate();	OnTransformChanged(TRUE, FALSE, FALSE);	}
	FORCEINLINE void			OnRotationChanged()						{	OnTransformChanged(FALSE, TRUE, FALSE);	}
	FORCEINLINE void			OnScaleChanged()						{	OnTransformChanged(FALSE, FALSE, TRUE);	}
	void						OnTransformChanged(Bool bTranslation, Bool bRotation, Bool bScale);

	virtual void				OnZoneWake()							{	}	//	notifcation when zone is set to active (from non-active). SceneNode will now be updated
	virtual void				OnZoneSleep()							{	}	//	notifcation when zone is set to non-active (from active). SceneNode will now NOT be updated

private:
	TLMaths::TTransform			m_Transform;
};

