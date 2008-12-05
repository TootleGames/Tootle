
#pragma once

#include <TootleCore/TLGraph.h>

namespace TLScene
{
		class TSceneNode;
};

/*
	TSceneNode class
*/
class TLScene::TSceneNode : public TLGraph::TGraphNode<TLScene::TSceneNode>
{
public:
	TSceneNode(TRefRef NodeRef,TRefRef TypeRef=TRef());

	// Virtual properties that are only available on inherited classes
	// Saves u having to either add unnecessary properties to base classes or
	// the use of RTTI IsKindOf etc
	virtual Bool		HasTransform()			{	return FALSE; }
	virtual Bool		HasRender()				{	return FALSE; }
	virtual Bool		HasPhysics()			{	return FALSE; }

	FORCEINLINE TRefRef	GetNodeTypeRef() const	{	return m_NodeTypeRef;	}

protected:
	// Base scene node update routine - calls preupadte methods, then obejct doupdate, then any post update
	virtual void 	Update(float fTimestep);

	// Object virtual update routine - overload this if you want an object to perform some update
	virtual void 	DoUpdate(float fTimestep)	{}

	virtual void	ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);

protected:
	TRef			m_NodeTypeRef;		//	node type ref used in factory
};
