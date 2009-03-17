
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
	//	remove active zone
	m_pActiveZone = NULL;

	//	remove zones
	if ( m_pRootZone )
	{
		m_pRootZone->Shutdown();
		m_pRootZone = NULL;
	}

	//	clean node array
	m_AlwaysUpdateNodes.Empty();

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
	m_pRootZone->FindNeighboursAll( m_pRootZone );

	//	set active zone as the root by default so everything is active (it's default state
	SetActiveZone( m_pRootZone );
}


//---------------------------------------------------
//	change active zone
//---------------------------------------------------
void TLScene::TScenegraph::SetActiveZone(TPtr<TLMaths::TQuadTreeZone>& pZone)	
{
	u32 z;

	//	collect zones to enable and disable so that we don't turn a zone off and on
	TFixedArray<TLMaths::TQuadTreeZone*,9> ZonesOff(0);
	TFixedArray<TLMaths::TQuadTreeZone*,9> ZonesOnWait(0);
	TFixedArray<TLMaths::TQuadTreeZone*,9> ZonesOn(0);

	//	collect nodes to disable from old zone
	if ( m_pActiveZone )
	{
		ZonesOff.Add( m_pActiveZone );

		//	and it's neighbours
		TPtrArray<TLMaths::TQuadTreeZone>& ZoneNeighbours = m_pActiveZone->GetNeighbourZones();
		for ( z=0;	z<ZoneNeighbours.GetSize();	z++ )
		{
			TLMaths::TQuadTreeZone* pNeighbourZone = ZoneNeighbours[z];
			ZonesOff.Add( pNeighbourZone );
		}
	}

	//	un-assign zone (just so we don't accidently use it below)
	m_pActiveZone = NULL;

	//	collect nodes from new zone
	if ( pZone )
	{
		ZonesOff.Remove( pZone );
		ZonesOn.Add( pZone );

		//	and it's neighbours
		TPtrArray<TLMaths::TQuadTreeZone>& ZoneNeighbours = pZone->GetNeighbourZones();
		for ( z=0;	z<ZoneNeighbours.GetSize();	z++ )
		{
			TLMaths::TQuadTreeZone* pNeighbourZone = ZoneNeighbours[z];
			ZonesOff.Remove( pNeighbourZone );
			ZonesOnWait.Add( pNeighbourZone );
		}
	}

	//	assign new zone...
	m_pActiveZone = pZone;

	//	deactivate old zones
	for ( z=0;	z<ZonesOff.GetSize();	z++ )
		ZonesOff[z]->SetActive( SyncFalse, TRUE );

	//	activate new zones
	for ( z=0;	z<ZonesOn.GetSize();	z++ )
		ZonesOn[z]->SetActive( SyncTrue, TRUE );

	//	activate not-fully-on zones
	for ( z=0;	z<ZonesOnWait.GetSize();	z++ )
		ZonesOnWait[z]->SetActive( SyncWait, TRUE );
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
		//	update always-updated nodes
		for ( u32 n=0;	n<m_AlwaysUpdateNodes.GetSize();	n++ )
		{
			//	get node
			TPtr<TSceneNode>& pNode = FindNode( m_AlwaysUpdateNodes[n] );
			pNode->UpdateAll( TimeStep );
		}

		//	update from active zone
		if ( m_pActiveZone )
		{
			UpdateNodesByZone( TimeStep, *m_pActiveZone, TRUE );
		}
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

		//	node that is already updated
		if ( IsAlwaysUpdateNode( SceneNode.GetNodeRef() ) )
			continue;

		//	update this scene node and it's children
		SceneNode.UpdateAll( TimeStep );		
	}

	//	update neighbour zones
	if ( UpdateNeighbours )
	{
		TPtrArray<TLMaths::TQuadTreeZone>& NeighbourZones = Zone.GetNeighbourZones();
		for ( u32 z=0;	z<NeighbourZones.GetSize();	z++ )
		{
			//	update neighbour, but not it's neighbours otherwise we'll get stuck in a loop
			UpdateNodesByZone( TimeStep, *NeighbourZones[z], FALSE );
		}
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

