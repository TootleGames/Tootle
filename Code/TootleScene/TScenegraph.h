

#pragma once

#include <TootleCore/TPtr.h>
#include <TootleCore/TLGraph.h>
#include <TootleCore/TManager.h>
#include <TootleCore/TClassFactory.h>

#include <TootleCore/TLMaths.h>

#include "TSceneNode.h"

namespace TLScene
{
	class TSceneNodeFactory;
	class TScenegraph;

	extern TPtr<TScenegraph> g_pScenegraph;

	class TSceneNode_Transform;		// Forward declaration
};

// Factory for creating scene nodes
class TLScene::TSceneNodeFactory : public TClassFactory<TSceneNode, FALSE>
{
protected:
	virtual TSceneNode*		CreateObject(TRefRef InstanceRef,TRefRef TypeRef);

};


/*
	TScenegraph	class
*/
class TLScene::TScenegraph : public TLGraph::TGraph<TSceneNode>
{
public:
	TScenegraph(TRefRef refManagerID) :
		TLGraph::TGraph<TSceneNode>		(refManagerID)
	{
	}

	// Node searching
	Bool	GetNearestNodes(const TLMaths::TLine& Line, const float& fDistance, TPtrArray<TSceneNode_Transform>& pArray);
	void	GetNearestNodes(TPtr<TSceneNode>& pNode, const TLMaths::TLine& Line, const float& fDistance, TPtrArray<TSceneNode_Transform>& pArray);

	Bool	IsNodeWithinRange(TPtr<TSceneNode>& pNode, const TLMaths::TLine& Line, const float& fDistance);

	TPtr<TSceneNode>	GetInstance(TRef refNodeID, TRef refTypeID);	//	gr: CREATE instance

	Bool	AddFactory(TPtr<TLScene::TSceneNodeFactory> pFactory)	{ return (m_pFactories.Add(pFactory) != -1); }

protected:
	virtual SyncBool			Initialise();

private:

	TPtrArray<TLScene::TSceneNodeFactory>	m_pFactories;	// List of scene node factories
};




