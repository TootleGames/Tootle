
#pragma once

#include <TootleCore/TLGraph.h>

namespace TLScene
{
		class TSceneNode;
};

//---------------------------------------------------------
//	TSceneNode class
//---------------------------------------------------------
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
	virtual void 	Update(float fTimestep);	//	base scene node update

	virtual void	ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);

protected:
	TRef			m_NodeTypeRef;		//	node type ref used in factory
};
