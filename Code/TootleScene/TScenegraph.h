#pragma once

#include <TootleCore/TPtr.h>
#include <TootleCore/TLGraph.h>
#include <TootleCore/TManager.h>
#include <TootleCore/TClassFactory.h>
#include <TootleCore/TLMaths.h>
#include <TootleMaths/TQuadTree.h>

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
	TScenegraph() :
		TLGraph::TGraph<TSceneNode>	("Scene")
	{
	}

	// Node searching
	Bool							GetNearestNodes(const TLMaths::TLine& Line, const float& fDistance, TPtrArray<TSceneNode_Transform>& pArray);
	void							GetNearestNodes(TPtr<TSceneNode>& pNode, const TLMaths::TLine& Line, const float& fDistance, TPtrArray<TSceneNode_Transform>& pArray);

	Bool							IsNodeWithinRange(TPtr<TSceneNode>& pNode, const TLMaths::TLine& Line, const float& fDistance);

	void							SetRootZone(TPtr<TLMaths::TQuadTreeZone>& pZone);	//	set a new root zone
	TPtr<TLMaths::TQuadTreeZone>&	GetRootZone()										{	return m_pRootZone;	}

	const TLMaths::TQuadTreeZone*	GetActiveZone() const								{	return m_pActiveZone;	}
	const TArray<TLMaths::TQuadTreeZone*>&	GetActiveZones() const						{	return m_ActiveZoneList;	}
	const TArray<TLMaths::TQuadTreeZone*>&	GetHalfActiveZones() const					{	return m_HalfActiveZoneList;	}
	void							SetActiveZone(TPtr<TLMaths::TQuadTreeZone>& pZone);	//	change active zone
	FORCEINLINE TRefRef				GetActiveZoneTrackNode() const						{	return m_ActiveZoneTrackNode;	}
	void							SetActiveZoneTrackNode(TRefRef SceneNodeRef);		//	change (and re-initialise) the scene node we're tracking for the active zone

	FORCEINLINE Bool				IsAlwaysUpdateNode(TRefRef NodeRef) const			{	return m_AlwaysUpdateNodes.Exists( NodeRef );	}
	FORCEINLINE void				AddAlwaysUpdateNode(TRefRef NodeRef) 				{	m_AlwaysUpdateNodes.AddUnique( NodeRef );	}
	FORCEINLINE void				RemoveAlwaysUpdateNode(TRefRef NodeRef)				{	m_AlwaysUpdateNodes.Remove( NodeRef );	}

protected:
	virtual SyncBool				Initialise();
	virtual SyncBool				Shutdown();
	virtual void					UpdateGraph(float TimeStep);			//	special scene graph update

	void							UpdateNodesByZone(float TimeStep,TLMaths::TQuadTreeZone& Zone);	//	update all the nodes in a zone. then update that's zones neighbours if required

protected:
	TPtr<TLMaths::TQuadTreeZone>		m_pRootZone;			//	root zone for the zone quad tree
	TRef								m_ActiveZoneTrackNode;	//	if set this sets m_ActiveZone to follow this node and set the zone
	TArray<TRef>						m_AlwaysUpdateNodes;	//	list of nodes to always update, regardless of zone

private:
	TLMaths::TQuadTreeZone*				m_pActiveZone;			//	the currently active zone
	TArray<TLMaths::TQuadTreeZone*>		m_ActiveZoneList;		//	list of ALL the zones that are active (includes m_pActiveZone)
	TArray<TLMaths::TQuadTreeZone*>		m_HalfActiveZoneList;	//	list of half-awake zones in m_ActiveZoneList
};




