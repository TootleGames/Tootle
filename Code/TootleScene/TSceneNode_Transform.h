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
	TSceneNode_Transform(TRefRef NodeRef,TRefRef TypeRef);

	virtual Bool				HasTransform()	{ return TRUE; }

	FORCEINLINE const float3&	GetPosition() const												{	return GetTranslate();	}					//	our position will be 
	FORCEINLINE void			GetLocalPosition(float3& Pos) const								{	GetTransform().Transform( Pos );	}	//	get a position relative to the scene node

	const TLMaths::TTransform&	GetTransform() const											{	return m_Transform;	}
	FORCEINLINE	const float3&	GetTranslate() const											{	return m_Transform.GetTranslate();	}
	FORCEINLINE	const TLMaths::TQuaternion&		GetRotation() const								{	return m_Transform.GetRotation();	}
	FORCEINLINE	const float3&	GetScale() const												{	return m_Transform.GetScale();	}

	//	explicit changes
	virtual void				SetTransform(const TLMaths::TTransform& Transform)				{	m_Transform = Transform; OnTransformChanged(TRUE, TRUE, TRUE); }	
	virtual void				SetTranslate(const float3& Translate)							{	m_Transform.SetTranslate(Translate);	OnTranslationChanged();	}
	virtual void				SetRotation(const TLMaths::TQuaternion& Rotation)				{	m_Transform.SetRotation(Rotation);	OnRotationChanged();	}
	virtual void				SetScale(const float3& Scale)									{	m_Transform.SetScale(Scale);	OnScaleChanged();	}

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

	FORCEINLINE void			OnTranslationChanged()					{	OnTransformChanged( TLMaths_TransformBitTranslate );	}
	FORCEINLINE void			OnRotationChanged()						{	OnTransformChanged( TLMaths_TransformBitRotation );	}
	FORCEINLINE void			OnScaleChanged()						{	OnTransformChanged( TLMaths_TransformBitScale );	}
	FORCEINLINE void			OnTransformChanged(Bool bTranslation, Bool bRotation, Bool bScale);
	virtual void				OnTransformChanged(u8 TransformChangedBits);

	virtual void				OnZoneWake(SyncBool ZoneActive)			{	}	//	notifcation when zone is set to active (from non-active). SceneNode will now be updated
	virtual void				OnZoneSleep()							{	}	//	notifcation when zone is set to non-active (from active). SceneNode will now NOT be updated
	virtual void				OnZoneChanged(TLMaths::TQuadTreeZone* pOldZone);	//	our zone has changed - if we're the node being tracked in the graph, change the active zone
	virtual SyncBool			IsInShape(const TLMaths::TBox2D& Shape);
	virtual Bool				HasZoneShape()							{	return TRUE;	}	//	return validity of shape for early bail out tests.
	FORCEINLINE SyncBool		IsZoneAwake() const;					//	get zone's active state
	virtual SyncBool			IsAwake() const							{	return IsZoneAwake();	}	//	checks to see if this node is awake (NOT THE SAME AS IsEnabled!) - currently gets our zone and checks it's active state - shouldnt be a need to store the sleep/awake state
	void						InitialiseZone();						//	if zone isn't initialised, initialise it

private:
	TLMaths::TTransform			m_Transform;
	Bool						m_ZoneInitialised;					//	first time we initialise, (or change transform) if the zone hasn't been initialised then we do an initial zone calculation
};




//--------------------------------------------
//	get zone's active state
//--------------------------------------------
FORCEINLINE SyncBool TLScene::TSceneNode_Transform::IsZoneAwake() const
{
	TLMaths::TQuadTreeZone* pZone = GetZone();
	return pZone ? pZone->IsActive() : SyncFalse;
}


//--------------------------------------------
//	
//--------------------------------------------
FORCEINLINE void TLScene::TSceneNode_Transform::OnTransformChanged(Bool bTranslation, Bool bRotation, Bool bScale)
{
	u8 TransformBits = (TLMaths_TransformBitTranslate*bTranslation);
	TransformBits |= (TLMaths_TransformBitRotation*bRotation);
	TransformBits |= (TLMaths_TransformBitScale*bScale);

	//	go onto real routine if there were any changes
	if ( TransformBits != 0x0 )
	{
		OnTransformChanged( TransformBits );
	}
}