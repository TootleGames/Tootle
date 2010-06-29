#include "TScenegraph.h"
#include "TSceneNode_Transform.h"
#include <TootleCore/TLTime.h>


// include files for the node factory
#include "TSceneNode_Camera.h"
#include "TSceneNode_Timeline.h"
#include "TSceneNode_Emitter.h"
#include "TSceneNode_Object.h"
#include "TSchemeNode.h"


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

	if(TypeRef == "Timeline")
		return new TSceneNode_Timeline(InstanceRef,TypeRef);

	if(TypeRef == "Emitter")
		return new TSceneNode_Emitter(InstanceRef,TypeRef);

	if(TypeRef == "Scheme")
		return new TSceneNode_Scheme(InstanceRef,TypeRef);

	if(TypeRef == "object")
		return new TSceneNode_Object(InstanceRef,TypeRef);

	return NULL;
}


SyncBool TScenegraph::Initialise()
{

	// Attach the base scene node factory by default
	TPtr<TNodeFactory<TSceneNode> > pFactory = new TSceneNodeFactory();

	if(pFactory)
		AddFactory(pFactory);

	return TLGraph::TGraph<TSceneNode>::Initialise();
}



SyncBool TScenegraph::Shutdown()
{
	TLDebug_Print("Scenegraph shutdown");

	//	remove active zone
	m_pActiveZone = NULL;
	m_ActiveZoneList.Empty();
	m_HalfActiveZoneList.Empty();

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



Bool TScenegraph::GetNearestNodes(const TLMaths::TLine& Line, const float& fDistance, TArray<TSceneNode_Transform*>& Array)
{
	TSceneNode& RootNode = GetRootNode();
	GetNearestNodes( RootNode, Line, fDistance, Array);

	// Return TRUE if we found any nodes ith range of the line
	return (Array.GetSize() > 0);
}

void TScenegraph::GetNearestNodes(TSceneNode& Node, const TLMaths::TLine& Line, const float& fDistance, TArray<TSceneNode_Transform*>& Array)
{
	// Check the node itself - if within range add to the array
	if(IsNodeWithinRange(Node, Line, fDistance))
	{
		TSceneNode_Transform& TransformNode = static_cast<TSceneNode_Transform&>( Node );
		Array.Add( &TransformNode );
	}
	

	TPointerArray<TSceneNode>& NodeChildren = Node.GetChildren();
	for ( u32 c=0;	c<NodeChildren.GetSize();	c++ )
	{
		TSceneNode& Child = *NodeChildren[c];
		GetNearestNodes( Child, Line, fDistance, Array );
	}

}

Bool TScenegraph::IsNodeWithinRange(TSceneNode& Node, const TLMaths::TLine& Line, const float& fDistance)
{
	if(Node.HasTransform())
	{
		// If the node has transform then we can assume it is the transform node type
		// Cast to a transform node
		TSceneNode_Transform& TransformNode = static_cast<TSceneNode_Transform&>( Node );

		// Do distance check from node to line
		float fDistanceToLine = TransformNode.GetDistanceTo(Line);

		// Is the distance to the line less than the threshold?
		if(fDistanceToLine < fDistance)
			return TRUE;
	}

	return FALSE;
}


//---------------------------------------------------
//	set a new root zone	
//---------------------------------------------------
void TLScene::TScenegraph::SetRootZone(TLMaths::TQuadTreeZone* pZone)
{
	m_pRootZone = pZone;

	//	predivide all the zones to their smallest level
	m_pRootZone->DivideAll();
	m_pRootZone->FindNeighboursAll();

	//	set active zone as the root by default so everything is active (it's default state
	SetActiveZone( m_pRootZone );
}


//---------------------------------------------------
//	change active zone
//---------------------------------------------------
void TLScene::TScenegraph::SetActiveZone(TLMaths::TQuadTreeZone* pZone)	
{
	u32 z;

	//	collect zones to enable and disable so that we don't turn a zone off and on
	TFixedArray<TLMaths::TQuadTreeZone*,100> ZonesOff;
	TFixedArray<TLMaths::TQuadTreeZone*,100> ZonesOnWait;
	TFixedArray<TLMaths::TQuadTreeZone*,100> ZonesOn;

	//	get list of zones to turn off...
	ZonesOff.Add( m_ActiveZoneList );

	//	assign new zone...
	m_pActiveZone = pZone;

	//	clear old list
	m_ActiveZoneList.Empty();
	m_HalfActiveZoneList.Empty();


	//	collect nodes from new zone
	if ( m_pActiveZone )
	{
		ZonesOff.Remove( m_pActiveZone );
		ZonesOn.Add( m_pActiveZone );

		//	set neighbours as active...
		TArray<TLMaths::TQuadTreeZone*>& ZoneNeighbours = m_pActiveZone->GetNeighbourZones();
		for ( z=0;	z<ZoneNeighbours.GetSize();	z++ )
		{
			TLMaths::TQuadTreeZone* pNeighbourZone = ZoneNeighbours[z];
			ZonesOff.Remove( pNeighbourZone );
			ZonesOn.AddUnique( pNeighbourZone );
			ZonesOnWait.Remove( pNeighbourZone );

			//	and neighbours's neighbours as half-on
			TArray<TLMaths::TQuadTreeZone*>& NeighbourNeighbours = pNeighbourZone->GetNeighbourZones();
			for ( u32 i=0;	i<NeighbourNeighbours.GetSize();	i++ )
			{
				TLMaths::TQuadTreeZone* pNeighbourNeighbourZone = NeighbourNeighbours[i];

				//	if already in the on/onwait list, just skip
				if ( ZonesOn.Exists( pNeighbourNeighbourZone ) )
					continue;

				if ( ZonesOnWait.Exists( pNeighbourNeighbourZone ) )
					continue;

				//	add to half-on list
				ZonesOnWait.Add( pNeighbourNeighbourZone );
				ZonesOff.Remove( pNeighbourNeighbourZone );
			}
		}
	}


	//	gr: because this is not atomic (or because the Sleep/Wake of the nodes in the zones isn't async)
	//	we have to do this in a specific order... ENABLE zones first, THEN sleep old ones. I have cases
	//	with recycling cars when zones are made inactive... but there are no active zones to respawn into

	//	activate new zones
	for ( z=0;	z<ZonesOn.GetSize();	z++ )
	{
		m_ActiveZoneList.Add( ZonesOn[z] );
		ZonesOn[z]->SetActive( SyncTrue, TRUE );
	}

	//	activate half asleep zones
	for ( z=0;	z<ZonesOnWait.GetSize();	z++ )
	{
		m_ActiveZoneList.Add( ZonesOnWait[z] );
		m_HalfActiveZoneList.Add( ZonesOnWait[z] );
		ZonesOnWait[z]->SetActive( SyncWait, TRUE );
	}	
	
	//	deactivate old zones
	for ( z=0;	z<ZonesOff.GetSize();	z++ )
	{
		ZonesOff[z]->SetActive( SyncFalse, TRUE );
	}

	//	send out "active zone changed" message
	TLMessaging::TMessage Message("ActZoneChange");
	PublishMessage( Message );
}


//----------------------------------------------------
//	special scene graph update
//----------------------------------------------------
void TLScene::TScenegraph::UpdateGraph(float TimeStep)
{
	TLTime::TScopeTimer Timer( TRef_Static(s,c,e,n,e) );

	// Process all queued messages first
	ProcessMessageQueue();

	//	no zoning? just update all nodes
	if ( !m_pRootZone )
	{
		// Update the graph nodes
		GetRootNode().UpdateAll(TimeStep);
	}
	else
	{
		//	update always-updated nodes
		for ( u32 n=0;	n<m_AlwaysUpdateNodes.GetSize();	n++ )
		{
			//	get node
			TSceneNode* pNode = FindNode( m_AlwaysUpdateNodes[n] );
			if ( !pNode )
			{
				TDebugString Debug_String;
				Debug_String << "Node " << m_AlwaysUpdateNodes[n] << " expected";
				TLDebug_Warning( Debug_String );
				continue;
			}
			pNode->UpdateAll( TimeStep );
		}

		//	update active zones
		for ( u32 z=0;	z<m_ActiveZoneList.GetSize();	z++ )
		{
			UpdateNodesByZone( TimeStep, *(m_ActiveZoneList[z]) );
		}
	}

	// Update the graph structure with any changes
	UpdateGraphStructure();
}

	
//----------------------------------------------------
//	update all the nodes in a zone. then update that's zones neighbours if required
//----------------------------------------------------
void TLScene::TScenegraph::UpdateNodesByZone(float TimeStep,TLMaths::TQuadTreeZone& Zone)
{
	TArray<TLMaths::TQuadTreeNode*>& ZoneNodes = Zone.GetNodes();
	for ( u32 n=0;	n<ZoneNodes.GetSize();	n++ )
	{
		TLMaths::TQuadTreeNode& ZoneNode = *ZoneNodes[n];
		TSceneNode_Transform& SceneNode = static_cast<TSceneNode_Transform&>( ZoneNode );

		//	node that is already updated
		if ( IsAlwaysUpdateNode( SceneNode.GetNodeRef() ) )
			continue;

		//	update this scene node and it's children
		SceneNode.UpdateAll( TimeStep );		
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
	TLScene::TSceneNode* pSceneNode = FindNode( m_ActiveZoneTrackNode );
	
	//	node doesnt [currently] exist - NULL active zone and assume it'll initialise when the node does
	if ( !pSceneNode )
	{
		SetActiveZone( NULL );
		return;
	}

	//	not a zoned node
	if ( !pSceneNode->HasZone() )
	{
		m_ActiveZoneTrackNode.SetInvalid();
		SetActiveZone( NULL );
		return;
	}

	//	grab current zone of node and make it active
	TSceneNode_Transform& SceneNode = static_cast<TSceneNode_Transform&>( *pSceneNode );
	SetActiveZone( SceneNode.GetZone() );
}

