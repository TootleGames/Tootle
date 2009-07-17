
#pragma once

#include <TootleCore/TLGraph.h>

namespace TLScene
{
	class TSceneNode;

	namespace TSceneNodeFlags
	{
		enum Type
		{
			AlwaysAwake = 0,	//	when zone is asleep, we still do a SceneNodeUpdate
		};
	};
};



//---------------------------------------------------------
//	TSceneNode class
//---------------------------------------------------------
class TLScene::TSceneNode : public TLGraph::TGraphNode<TLScene::TSceneNode>
{
public:
	TSceneNode(TRefRef NodeRef,TRefRef TypeRef=TRef());

	FORCEINLINE Bool	IsEnabled() const				{	return m_bEnabled;	}	
	void				SetEnable(const Bool& bEnable);

	virtual void		UpdateAll(float Timestep);		//	overloaded to force a PostUpdate() after the normal Update()

	// Virtual properties that are only available on inherited classes
	// Saves u having to either add unnecessary properties to base classes or
	// the use of RTTI IsKindOf etc
	virtual Bool		HasTransform()			{	return FALSE; }
	virtual Bool		HasRender()				{	return FALSE; }
	virtual Bool		HasPhysics()			{	return FALSE; }

	//	gr: the functions above are useless without actual (and therefore forced to be virtual..) accessors
	//		otherwise you have to cast (and use RTTI) to ensure you're accessing the actual functions on a class...
	//		if a node HasRender(), I still can't assume it's an Object node...
	virtual TRef		GetPhysicsNodeRef() const		{	return TRef();	}
	virtual TRef		GetRenderNodeRef() const		{	return TRef();	}

protected:
	virtual void		PostUpdate(float fTimestep)		{	}	//	gr: an update that occurs immediately after normal update. 

	virtual void		OnEnable()		{}
	virtual void		OnDisable()		{}

private:
	Bool				m_bEnabled;
};


