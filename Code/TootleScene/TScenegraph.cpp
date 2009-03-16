
#include "TScenegraph.h"

#include "TSceneNode_Transform.h"


////////////////////////////////////////////////////
// include files for the node factory
////////////////////////////////////////////////////
#include "TSceneNode_Camera.h"
#include "TSceneNode_Emitter.h"
#include "TSchemeNode.h"
////////////////////////////////////////////////////


namespace TLScene
{
	TPtr<TScenegraph> g_pScenegraph = NULL;
};

using namespace TLScene;



TSceneNode* TSceneNodeFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	// Create engine/middleware side scene nodes
	if(TypeRef == "Camera")
		return new TSceneNode_Camera(InstanceRef,TypeRef);

	if(TypeRef == "Emitter")
		return new TSceneNode_Emitter(InstanceRef,TypeRef);

	if(TypeRef == "Scheme")
		return new TSceneNode_Scheme(InstanceRef,TypeRef);

	return NULL;
}


SyncBool TScenegraph::Initialise()
{

	// Attach the base scene node factory by default
	TPtr<TClassFactory<TSceneNode,FALSE> > pFactory = new TSceneNodeFactory();

	if(pFactory)
		AddFactory(pFactory);

	return TLGraph::TGraph<TSceneNode>::Initialise();
}



SyncBool TScenegraph::Shutdown()
{
	if ( m_pRootZone )
	{
		m_pRootZone->Shutdown();
		m_pRootZone = NULL;
	}

	return TLGraph::TGraph<TSceneNode>::Shutdown();
}



Bool TScenegraph::GetNearestNodes(const TLMaths::TLine& Line, const float& fDistance, TPtrArray<TSceneNode_Transform>& pArray)
{
	TPtr<TSceneNode> pRootNode = GetRootNode();

	if(pRootNode.IsValid())
		GetNearestNodes(pRootNode, Line, fDistance, pArray);

	// Return TRUE if we found any nodes ith range of the line
	return (pArray.GetSize() > 0);
}

void TScenegraph::GetNearestNodes(TPtr<TSceneNode>& pNode, const TLMaths::TLine& Line, const float& fDistance, TPtrArray<TSceneNode_Transform>& pArray)
{
	// Check the node itself - if within range add to the array
	if(IsNodeWithinRange(pNode, Line, fDistance))
		pArray.Add(pNode);

#ifdef TLGRAPH_OWN_CHILDREN

	TPtrArray<TSceneNode>& NodeChildren = pNode->GetChildren();
	for ( u32 c=0;	c<NodeChildren.GetSize();	c++ )
	{
		TPtr<TSceneNode>& pChild = NodeChildren[c];
		GetNearestNodes( pChild, Line, fDistance, pArray );
	}

#else

	// Check the nodes children - the child will propagate the check to all other children too
	TPtr<TSceneNode> pNextNode = pNode->GetChildFirst();
	if(pNextNode.IsValid())
		GetNearestNodes(pNextNode, Line, fDistance, pArray);

	// Check the nodes siblings - the next one will check the next sibling and so on...
	pNextNode = pNode->GetNext();
	if(pNextNode.IsValid())
		GetNearestNodes(pNextNode, Line, fDistance, pArray);

#endif
}

Bool TScenegraph::IsNodeWithinRange(TPtr<TSceneNode>& pNode, const TLMaths::TLine& Line, const float& fDistance)
{
	if(pNode->HasTransform())
	{
		// If the node has transform then we can assume it is the transform node type
		// Cast to a transform node
		TSceneNode_Transform* pTransformNode = reinterpret_cast<TSceneNode_Transform*>(pNode.GetObject());

		// Do distance check from node to line
		float fDistanceToLine = pTransformNode->GetDistanceTo(Line);

		// Is the distance to the line less than the threshold?
		if(fDistanceToLine < fDistance)
			return TRUE;
	}

	return FALSE;
}


//---------------------------------------------------
//	set a new root zone	
//---------------------------------------------------
void TLScene::TScenegraph::SetRootZone(TPtr<TLMaths::TQuadTreeZone>& pZone)
{
	m_pRootZone = pZone;

	//	predivide all the zones to their smallest level
	m_pRootZone->DivideAll( m_pRootZone );

	//	set active zone as the root by default so everything is active (it's default state
	SetActiveZone( m_pRootZone );
}


//---------------------------------------------------
//	change active zone
//---------------------------------------------------
void TLScene::TScenegraph::SetActiveZone(TPtr<TLMaths::TQuadTreeZone>& pZone)	
{
	//	de-activate old zone
	if ( m_pActiveZone )
	{
		m_pActiveZone->SetActive( FALSE, TRUE );
	}

	//	assign new zone...
	m_pActiveZone = pZone;

	//	...and activate
	if ( m_pActiveZone )
	{
		m_pActiveZone->SetActive( TRUE, TRUE );
	}
	
}


//----------------------------------------------------
//	special scene graph update
//----------------------------------------------------
void TLScene::TScenegraph::UpdateGraph(float TimeStep)
{
	// Process all queued messages first
	ProcessMessageQueue();

	//	no zoning? just update all nodes
	if ( !m_pRootZone )
	{
		// Update the graph nodes
		GetRootNode()->UpdateAll(TimeStep);
	}
	else
	{
		//	update from active zone
		if ( m_pActiveZone )
		{
			UpdateNodesByZone( TimeStep, *m_pActiveZone, TRUE );
		}

		//	update non-zoned nodes...
	}

	// Update the graph structure with any changes
	UpdateGraphStructure();
}

	
//----------------------------------------------------
//	update all the nodes in a zone. then update that's zones neighbours if required
//----------------------------------------------------
void TLScene::TScenegraph::UpdateNodesByZone(float TimeStep,TLMaths::TQuadTreeZone& Zone,Bool UpdateNeighbours)
{
	TPtrArray<TLMaths::TQuadTreeNode>& ZoneNodes = Zone.GetNodes();
	for ( u32 n=0;	n<ZoneNodes.GetSize();	n++ )
	{
		TPtr<TLMaths::TQuadTreeNode>& pQuadTreeNode = ZoneNodes[n];
		TLScene::TSceneNode_Transform& SceneNode = *pQuadTreeNode.GetObject<TLScene::TSceneNode_Transform>();

		//	update this scene node and it's children
		SceneNode.UpdateAll( TimeStep );		
	}

	//	update neighbour zones
	if ( UpdateNeighbours )
	{
		//for ( u32 z=0;
	}
}


//----------------------------------------------------
//	change (and re-initialise) the scene node we're tracking for the active zone
//----------------------------------------------------
void TLScene::TScenegraph::SetActiveZoneTrackNode(TRefRef SceneNodeRef)		
{	
	//	no change
	if ( m_ActiveZoneTrackNode == SceneNodeRef )
		return;

	//	change node
	m_ActiveZoneTrackNode = SceneNodeRef;	

	//	get node
	TPtr<TLScene::TSceneNode>& pSceneNode = FindNode( m_ActiveZoneTrackNode );
	
	//	node doesnt [currently] exist - NULL active zone and assume it'll initialise when the node does
	if ( !pSceneNode )
	{
		SetActiveZone( TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>() );
		return;
	}

	//	not a zoned node
	if ( !pSceneNode->HasTransform() )
	{
		m_ActiveZoneTrackNode.SetInvalid();
		SetActiveZone( TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>() );
		return;
	}

	//	grab current zone of node and make it active
	TLScene::TSceneNode_Transform& SceneNode = *(pSceneNode.GetObject<TLScene::TSceneNode_Transform>());
	SetActiveZone( SceneNode.GetZone() );
}

