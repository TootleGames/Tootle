
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

	FORCEINLINE Bool	IsEnabled() const				{	return TRUE;	}	//	for the graph templating - scene nodes aren't explicitly disabled... (yet)

	virtual void		UpdateAll(float Timestep);		//	overloaded to force a PostUpdate() after the normal Update()

	// Virtual properties that are only available on inherited classes
	// Saves u having to either add unnecessary properties to base classes or
	// the use of RTTI IsKindOf etc
	virtual Bool		HasTransform()			{	return FALSE; }
	virtual Bool		HasRender()				{	return FALSE; }
	virtual Bool		HasPhysics()			{	return FALSE; }

protected:
	virtual void 		Update(float fTimestep);		//	base scene node update
	virtual void		PostUpdate(float fTimestep);	//

	virtual void		ProcessMessage(TLMessaging::TMessage& Message);
};


