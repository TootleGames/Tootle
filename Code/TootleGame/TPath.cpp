#include "TPath.h"



namespace TLRef
{
	//	gr: extern this if you want to use it... sorry!
	TLArray::SortResult		RefSort(const TRef& aRef,const TRef& bRef,const void* pTestVal);	//	simple ref-sort func - for arrays of TRef's
}


//#define DEAD_END_CODE


TLPath::TPath::TPath(TRefRef PathNetworkRef) :
	m_PathNetworkRef	( PathNetworkRef )
{
}



//---------------------------------------------------------
//	create a find path spider
//---------------------------------------------------------
SyncBool TLPath::TPath::FindPath(TRefRef StartNode,TRefRef EndNode,TLPath::TPathMode::Type PathMode, Bool Blocking)
{
	//	get network path asset
	TPtr<TLAsset::TAsset>& pPathNetworkAsset = TLAsset::GetAsset( m_PathNetworkRef );
	if ( !pPathNetworkAsset )
	{
		TLDebug_Break("FindPath failed, missing path network");
		return SyncFalse;
	}

	if ( pPathNetworkAsset->GetAssetType() != "PathNetwork" )
	{
		TLDebug_Break("FindPath failed, asset is not path network");
		return SyncFalse;
	}
	
	//	cast ptr
	TPtr<TLAsset::TPathNetwork> pPathNetwork = pPathNetworkAsset;

	//	create new path spider
	m_pPathSpider = new TPathSpider_Path( pPathNetwork, StartNode, EndNode, PathMode );

	//	do one update, if blocking keep going until we get a result
	SyncBool SpiderResult = SyncWait;
	do
	{
		SpiderResult = UpdateFindPath();
	}
	while ( Blocking && SpiderResult==SyncWait );

	return SpiderResult;
}




//---------------------------------------------------------
//	just follow N random nodes to make a path
//---------------------------------------------------------
SyncBool TLPath::TPath::FindPathRandom(TRefRef StartNode,u32 NodesInRoute, Bool Blocking)
{
	if ( !Blocking )
	{
		TLDebug_Break("Note; no async version yet");
	}

	//	get network path asset
	TPtr<TLAsset::TAsset>& pPathNetworkAsset = TLAsset::GetAsset( m_PathNetworkRef );
	if ( !pPathNetworkAsset )
	{
		TLDebug_Break("FindPath failed, missing path network");
		return SyncFalse;
	}

	if ( pPathNetworkAsset->GetAssetType() != "PathNetwork" )
	{
		TLDebug_Break("FindPath failed, asset is not path network");
		return SyncFalse;
	}
	
	//	cast ptr
	TPtr<TLAsset::TPathNetwork> pPathNetwork = pPathNetworkAsset;

	//	get starting node
	TPtr<TLPath::TPathNode> pNode = pPathNetwork->GetNode( StartNode );
	if ( !pNode )
	{
		TLDebug_Break("failed to find start node");
		return SyncFalse;
	}

	//	add start node to the route
	m_Nodes.Add( pNode->GetNodeRef() );

	//	now follow random links until we've got enough nodes
	while ( m_Nodes.GetSize() < NodesInRoute )
	{
		//	just to make sure we don't get stuck, make a list of all the possible links then pick one at random
		TArray<TLPath::TPathNodeLink>& NodeLinks = pNode->GetLinks();
		TFixedArray<TLPath::TPathNodeLink*,100> ValidNodeLinks;
		TLPath::TPathNodeLink* pLastResort = NULL;
		for ( u32 i=0;	i<NodeLinks.GetSize();	i++ )
		{
			TLPath::TPathNodeLink& Link = NodeLinks[i];
			
			//	if this is a backwards link we cannot follow it
			if ( Link.GetDirection() == TLPath::TDirection::Backward )
				continue;

			//	try to avoid going back where we came from
			if ( m_Nodes.GetSize() >= 2 )
			{
				//	get the node before pNode
				TRefRef PrevNode = m_Nodes.ElementAt( m_Nodes.GetLastIndex() -1 );
				if ( PrevNode == Link.GetLinkNodeRef() )
				{
					pLastResort = &Link;
					continue;
				}
			}

			ValidNodeLinks.Add( &Link );
		}

		//	if no links, and we have a last resort, use that
		if ( pLastResort && ValidNodeLinks.GetSize() == 0 )
		{
			TLDebug_Print("Using last resort (going back down route) when generating random path");
			ValidNodeLinks.Add( pLastResort );
		}

		//	no valid links to follow
		if ( ValidNodeLinks.GetSize() == 0 )
		{
			TLDebug_Warning("Hit dead end when generating random path");
			break;
		}

		//	pick a link to follow
		TLPath::TPathNodeLink* pFollowLink = ValidNodeLinks.ElementRand();
		pNode = pPathNetwork->GetNode( pFollowLink->GetLinkNodeRef() );
		if ( !pNode )
		{
			TLDebug_Break("Link following node is missing from PathNetwork... Corrupt network/node?");
			break;
		}

		//	add this link to the route
		m_Nodes.Add( pNode->GetNodeRef() );
	}

	if ( m_Nodes.GetSize() < 2 )
	{
		TLDebug_Break("Generated a route that's too short");
		return SyncFalse;
	}

	return SyncTrue;
}


//---------------------------------------------------------
//	continue updating find path spider
//---------------------------------------------------------
SyncBool TLPath::TPath::UpdateFindPath()
{
	if ( !m_pPathSpider )
	{
		TLDebug_Break("Attempting to update non-existant path spider");
		return SyncFalse;
	}

	//	update spider
	SyncBool Result = m_pPathSpider->Update();

	//	if not finished just return result
	if ( Result != SyncTrue )
		return Result;

	//	finished, grab path
	TArray<TRef>* pFinalPath = m_pPathSpider->GetFinalPath();
	if ( !pFinalPath )
	{
		TLDebug_Break("Spider finished... but no path");
		return SyncFalse;
	}

	//	append path to our path
	m_Nodes.Add( *pFinalPath );

	//	delete now-redundant spider
	m_pPathSpider = NULL;

	return Result;
}



//---------------------------------------------------------
//	
//---------------------------------------------------------
TLPath::TPathSpider::TPathSpider(TPtr<TLAsset::TPathNetwork>& pPathNetwork,TRef StartingNodeRef) :
	m_pPathNetwork		( pPathNetwork ),
	m_EndNodes			( TLRef::RefSort, 200 ),
	m_CompletedNodes	( TLRef::RefSort, 200 ),
	m_StartingNodeRef	( StartingNodeRef )
{
	//	add first road node to start from
	if ( !m_StartingNodeRef.IsValid() )
		m_StartingNodeRef = m_pPathNetwork->GetRandomNode();

	m_EndNodes.Add( m_StartingNodeRef );
}


//---------------------------------------------------------
//	do next spidering
//---------------------------------------------------------
SyncBool TLPath::TPathSpider::Update()
{
	//	no end nodes to process - all done
	if ( !m_EndNodes.GetSize() )
		return SyncTrue;
		
	//	copy the array before we call the functions that change the array
	TArray<TRef> OldEndNodes;
	OldEndNodes.Copy( m_EndNodes );

	u32 e;
	u32 LastProcessed = 0;

	//	limit how muich we spider in one frame
	s32 MaxLinksToProcess = 10;

	//	spider out from all the ends
	for ( e=0;	e<OldEndNodes.GetSize();	e++ )
	{
		DoSpiderNode( OldEndNodes[e], MaxLinksToProcess );
		LastProcessed = e;

		if ( MaxLinksToProcess <= 0 )
			break;
	}

	//	remove the old end nodes
	for ( e=0;	e<=LastProcessed;	e++ )
	{
		m_EndNodes.Remove( OldEndNodes[e] );
	}

	return SyncWait;
}


//-----------------------------------------------------------
//	spider outwards from this node
//-----------------------------------------------------------
void TLPath::TPathSpider::DoSpiderNode(TRefRef RoadNodeRef,s32& MaxLinksToProcess)
{
	//	already processed this node?
	if ( m_CompletedNodes.Exists( RoadNodeRef ) )
		return;

	//	get node
	TPtr<TLPath::TPathNode> pRoadNode = m_pPathNetwork->GetNode( RoadNodeRef );

	//	spider out to this node's links
	for ( u32 i=0;	i<pRoadNode->GetLinks().GetSize();	i++ )
	{
		TPtr<TLPath::TPathNode>& pLinkNode = m_pPathNetwork->GetNode( pRoadNode->GetLinkRef(i) );
		
		//	add this link as a new end node to process
		if ( !m_CompletedNodes.Find( pLinkNode->GetNodeRef() ) )
		{
			m_EndNodes.AddUnique( pLinkNode->GetNodeRef() );
		}

		MaxLinksToProcess--;
	}

	//	mark this node as processed
	m_CompletedNodes.Add( RoadNodeRef );
}
	




TLPath::TPathSpider_Path::TPathSpider_Path(TPtr<TLAsset::TPathNetwork>& pPathNetwork,TRefRef PathFromNode,TRefRef PathToNode,TLPath::TPathMode::Type PathMode) :
	TPathSpider		( pPathNetwork, PathFromNode ),
	m_NodeFrom		( PathFromNode ),
	m_NodeTo		( PathToNode ),
	m_PathMode		( PathMode )
{
	TPtr<TLPath::TPathNode> pNodeFrom = m_pPathNetwork->GetNode( PathFromNode );
	if ( !pNodeFrom )
	{
		TLDebug_Break("SpiderPath missing from node");
		return;
	}

	TPtr<TLPath::TPathNode> pNodeTo = m_pPathNetwork->GetNode( PathToNode );
	if ( !pNodeTo )
	{
		TLDebug_Break("SpiderPath missing to node");
		return;
	}

	//	we added the starting node as the starting point, set the distance for it to zero
	m_EndNodeLength.Add( m_NodeFrom, 0.f );
	m_EndNodeDistance.Add( m_NodeFrom, (pNodeTo->GetPosition() - pNodeFrom->GetPosition()).Length() );

	TPtr<TArray<TRef> > pInitialPath = new TArray<TRef>();
	pInitialPath->Add( m_NodeFrom );
	m_EndNodePaths.Add( m_NodeFrom, pInitialPath );

	//	just an initial check for loop mode, if going from FromNode to ToNode results in a dead end, 
	//	there's no way they can form a loop
	if ( m_PathMode == TLPath::TPathMode::Loop )
	{
		TTempString DebugString;
		DebugString.Appendf("Looking for loop between %d and %d", PathFromNode.GetData(), PathToNode.GetData() );
		TLDebug_Print( DebugString );

#ifdef DEAD_END_CODE
		//	if either node is on a dead end, we're not gonna make a loop
		if ( pNodeFrom->IsOnDeadEnd() || pNodeTo->IsOnDeadEnd() )
		{
			//	empty end nodes and it'll fail
			m_EndNodes.Empty();
			m_EndNodeLength.Empty();
			m_EndNodeDistance.Empty();
		}
#endif
	}
}


SyncBool TLPath::TPathSpider_Path::Update()
{
	//	got a final path, success!
	if ( m_pFinalPath )
		return SyncTrue;

	//	no more end nodes to process - as we didnt get a path, we must have failed to find a path and exhausted all routes
	if ( !m_EndNodes.GetSize() )
		return SyncFalse;

	//	get the end node with the shortest path...
	TRef ShortestEndNode;
	float ShortestEndLength = 0.f;

	TRef BestDirectionEndNode;
	float BestDirectionDistance = 0.f;

	for ( u32 e=0;	e<m_EndNodes.GetSize();	e++ )
	{
		//	get it's path length
		float* pLength = m_EndNodeLength.Find( m_EndNodes[e] );
		float* pDistance = m_EndNodeDistance.Find( m_EndNodes[e] );
		if ( !pLength || !pDistance )
		{
			TLDebug_Break("missing distance for end node");
			continue;
		}

		//	get shortest node
		if ( !ShortestEndNode.IsValid() || *pLength < ShortestEndLength )
		{
			ShortestEndNode = m_EndNodes[e];
			ShortestEndLength = *pLength;
		}

		//	get best-direction node
		if ( !BestDirectionEndNode.IsValid() || *pDistance < BestDirectionDistance )
		{
			BestDirectionEndNode = m_EndNodes[e];
			BestDirectionDistance = *pDistance;
		}
	}

	//	error - missing any end node distance info
	if ( !ShortestEndNode.IsValid() || !BestDirectionEndNode.IsValid() )
		return SyncFalse;

	//	pick a node to follow
	TRef FollowNode;

	if ( m_PathMode == TLPath::TPathMode::Shortest || m_PathMode == TLPath::TPathMode::Loop )
	{
		FollowNode = ShortestEndNode;
	}
	else if ( m_PathMode == TLPath::TPathMode::Fastest )
	{
		FollowNode = BestDirectionEndNode;	//	use the end node closest to our final destination
	}

	//	continue along this path...
	TPtr<TLPath::TPathNode> pEndNode = m_pPathNetwork->GetNode( FollowNode );
	if ( !pEndNode )
	{
		TLDebug_Break("Missing end node");
		return SyncFalse;
	}

	//	get previous path so we don't go back on ourselves
	TPtr<TArray<TRef> >* pPreviousPath = m_EndNodePaths.Find( FollowNode );
	if ( !pPreviousPath )
	{
		TLDebug_Break("Missing end node path");
		RemoveEndNode( FollowNode );
		return SyncFalse;
	}
	TArray<TRef>& PreviousPath = *(*pPreviousPath).GetObject();

	//	if we dont follow any links, this path is a dead end
	Bool HasFollowedLinks = FALSE;
	TPtr<TLPath::TPathNode> pNodeTo = m_pPathNetwork->GetNode( m_NodeTo );

	//	loop through links to find valid links we can follow
	for ( u32 i=0;	i<pEndNode->GetLinks().GetSize();	i++ )
	{
		TPtr<TLPath::TPathNode>& pLinkNode = m_pPathNetwork->GetNode( pEndNode->GetLinkRef(i) );
		TRefRef LinkRef = pLinkNode->GetNodeRef();

		//	if we have visited this link before in this path, then skip this link
		if ( PreviousPath.Exists( LinkRef ) )
			continue;

		if ( m_CompletedNodes.Exists( LinkRef ) )
			continue;

		//	check to see if this link is one of our end nodes, if it is, 
		//	we ditch the one that's furthest away 
		float* pExistingLinkPathLength = m_EndNodeLength.Find( LinkRef );

		//	not been here before, create another end node
		float NewPathLength = *m_EndNodeLength.Find( pEndNode->GetNodeRef() );
		NewPathLength += ( pEndNode->GetPosition() - pLinkNode->GetPosition() ).Length();

		//	we're already at this link with another end node...
		if ( pExistingLinkPathLength )
		{
			//	if existing path is shorter then keep that and skip adding this link
			if ( (*pExistingLinkPathLength) < NewPathLength )
				continue;

			//	this new link is shorter than the other end node, so ditch the old one
			RemoveEndNode( LinkRef );
		}

#ifdef DEAD_END_CODE
		//	if this road leads to a dead end it's not going to form a loop
		if ( m_PathMode == TLPath::TPathMode::Loop )
		{
			if ( pLinkNode->IsOnDeadEnd() )
				continue;
		}
		else
		{
			//	if we're NOT in a loop, we want to see if we hit the target before we hit a dead end
			//	check to see if this link is a dead end
			SyncBool DeadEndResult = m_pPathNetwork->IsDeadEndBreakOnTarget( pEndNode, pLinkNode, m_NodeTo );
			if ( DeadEndResult == SyncWait )
			{

				//	if this is a direct link and we're doing a loop, don't fiddle with the other stuff, just ignore this link and 
				//	carry on searching the other links
				/*
				//	this link leads onto our target, get rid of all the other end nodes so we're forced to follow this path next time
				m_EndNodes.Empty();
				m_EndNodeLength.Empty();
				m_EndNodeDistance.Empty();
				m_EndNodePaths.Empty();
				*/
			}
			else if ( DeadEndResult == SyncTrue )
			{
				//	this link is a dead end (and doesnt reach our target), dont add it
				continue;
			}
		}
#endif

		Bool FoundDestination = FALSE;

		//	it's the node we're looking for! hurrah!
		if ( LinkRef == m_NodeTo )
		{
			FoundDestination = TRUE;

			//	if this is a loop, then we ignore the direct connection between From and To
			if ( m_PathMode == TLPath::TPathMode::Loop && FollowNode == m_NodeFrom )
			{
				FoundDestination = FALSE;

				//	gr: we dont want this to become an end node
				continue;
			}
		}

		//	detect following dead ends
		if ( m_DeadEnds.Exists( LinkRef ) )
		{
			TLDebug_Break("Following a dead end?");
			continue;
		}

		//	copy path
		TPtr<TArray<TRef> > pNewPath = new TArray<TRef>();
		pNewPath->Copy( PreviousPath );
		pNewPath->Add( LinkRef );

		//	add new end node
		m_EndNodes.Add( LinkRef );
		m_EndNodeLength.Add( LinkRef, NewPathLength );
		m_EndNodeDistance.Add( LinkRef, (pNodeTo->GetPosition() - pLinkNode->GetPosition()).Length() );
		m_EndNodePaths.Add( LinkRef, pNewPath );
	
		if ( FoundDestination )
		{
			m_pFinalPath = pNewPath;
			return SyncTrue;
		}

		//	this isnt a dead end as we've added a new end node
		HasFollowedLinks = TRUE;

		//	we've added a dead end node that leads to our target, so abort so that we're forced to follow ti next time
		//if ( DeadEndResult == SyncWait )
		//	break;
	}

	//	remove this end node now we've processed it, we would have either continued along links,
	//	or had no valid links and the end node will just disapear
	RemoveEndNode( FollowNode );
	m_CompletedNodes.AddUnique( FollowNode );

	//	seeing as we hit a dead end, force another update to make it seem like its doing something
	//	might cause stalls though
	if ( !HasFollowedLinks )
	{
		return Update();
	}
	/*
	//	this path has no valid links. dead end. remove it from the list of end nodes and goto next end node (on next update)
	if ( !HasFollowedLinks )
	{
		RemoveEndNode( FollowNode );
		return SyncWait;
	}
	*/

	return SyncWait;
}


//-----------------------------------------------
//	remove all end node data for this node
//-----------------------------------------------
void TLPath::TPathSpider_Path::RemoveEndNode(TRefRef EndNodeRef)
{
	//	remove end node
	m_EndNodes.Remove( EndNodeRef );
	m_EndNodeDistance.Remove( EndNodeRef );
	m_EndNodeLength.Remove( EndNodeRef );
	m_EndNodePaths.Remove( EndNodeRef );

}




//-----------------------------------------------
//	
//-----------------------------------------------
TLPath::TPathNetworkZones::TPathNetworkZones(TPtr<TLAsset::TPathNetwork>& pPathNetwork,TPtr<TLMaths::TQuadTreeZone>& pRootZone,Bool LeafsOnly) :
	m_pPathNetwork	( pPathNetwork ),
	m_pRootZone		( pRootZone ),
	m_LeafsOnly		( LeafsOnly )
{
	//	init zoning information...
	if ( !m_pPathNetwork )
	{
		TLDebug_Break("Path network expected");
		return;
	}
	
	if ( !m_pRootZone )
	{
		TLDebug_Break("Root zone expected");
		return;
	}
	


	const TArray<TLPath::TPathLink>& PathLinks = pPathNetwork->GetLinks();
	for ( u32 i=0;	i<PathLinks.GetSize();	i++ )
	{
		const TPathLink& PathLink = PathLinks[i];

		//	make an entry in the table for this link
		TPathLinkZones* pLinkZones = m_PathLinkZones.AddNew(PathLink);
		
		//	get all the zones it intersects
		m_pRootZone->GetIntersectingLeafZones( PathLink.GetLinkLine(), pLinkZones->m_Zones );

		//	now reverse that lookup for zone -> links
		for ( u32 z=0;	z<pLinkZones->m_Zones.GetSize();	z++ )
		{
			//	get the zone->links entry
			const TLMaths::TQuadTreeZone* pZone = pLinkZones->m_Zones[z];
			TZonePathLinks* pZoneLinks = m_ZonePathLinks.Find( pZone );
			if ( !pZoneLinks )
				pZoneLinks = m_ZonePathLinks.AddNew( pZone );

			//	add path link to that zone
			pZoneLinks->m_PathLinks.Add( PathLink );
		}
	}
}



//-----------------------------------------------
//	
//-----------------------------------------------
const TArray<const TLMaths::TQuadTreeZone*>* TLPath::TPathNetworkZones::GetPathLinkZones(const TLPath::TPathLink& PathLink)
{
	//	find entry
	TPathLinkZones* pPathLinkZones = m_PathLinkZones.Find( PathLink );

	//	return array within
	return pPathLinkZones ? &pPathLinkZones->m_Zones : NULL;
}

	
//-----------------------------------------------
//	
//-----------------------------------------------
const TArray<TLPath::TPathLink>* TLPath::TPathNetworkZones::GetZonePathLinks(const TLMaths::TQuadTreeZone* pZone)
{
	//	find entry
	TZonePathLinks* pZonePathLinks = m_ZonePathLinks.Find( pZone );

	//	return array within
	return pZonePathLinks ? &pZonePathLinks->m_PathLinks : NULL;
}



//-----------------------------------------------
//	
//-----------------------------------------------
Bool TLPath::TPathNetworkZones::GetRandomPathPositionInRandomZone(TLMaths::TTransform& NewTransform,TLPath::TPathLink& PathLink,const TArray<TLMaths::TQuadTreeZone*>& ZoneList)
{
	//	no zones provided
	if ( ZoneList.GetSize() == 0 )
		return FALSE;

	//	copy list and pick a zone at random
	TFixedArray<TLMaths::TQuadTreeZone*,100> TempZoneList;
	TempZoneList.Copy( ZoneList );

	//	find a random zone with some path links in it
	const TArray<TLPath::TPathLink>* pPathLinksInZone = NULL;
	const TLMaths::TQuadTreeZone* pPathLinkZone = NULL;
	while ( !pPathLinksInZone && TempZoneList.GetSize() )
	{
		s32 ZoneIndex = TempZoneList.GetRandIndex();
		pPathLinkZone = TempZoneList[ZoneIndex];
		pPathLinksInZone = GetZonePathLinks( pPathLinkZone );
		
		//	ignore entries with no path links in them
		if ( pPathLinksInZone && pPathLinksInZone->GetSize() == 0 )
			pPathLinksInZone = NULL;

		//	zone has no path links, remove and try again
		if ( !pPathLinksInZone )
		{
			TempZoneList.RemoveAt( ZoneIndex );
			continue;
		}
	}

	//	couldnt find a pathlink in an appropriate zone
	if ( !pPathLinksInZone )
		return FALSE;

	//	k now pick a random path from the list
	const TLPath::TPathLink& FoundPathLink = pPathLinksInZone->ElementRandConst();
	const TLMaths::TLine2D& PathLinkLine = FoundPathLink.GetLinkLine();

	//	now find a point along the path that's in the zone
	float AlongLine = TLMaths::Randf();
	float2 PointAlongLine;
	PathLinkLine.GetPointAlongLine( PointAlongLine, AlongLine );
	while ( !pPathLinkZone->GetShape().GetIntersection( PointAlongLine ) )
	{
		//	pick another
		AlongLine = TLMaths::Randf();
		PathLinkLine.GetPointAlongLine( PointAlongLine, AlongLine );
	}

	//	gr: error here... can put placed into a zone that's syncfalse.... i THINK it's because we're placed right on the edge of a zone, as it mostly goes wrong when going left/up
	//	most maybe find some way of ensuring the new transform is away from the edge of it's zone
	PathLink = FoundPathLink;

	//	reposition node along the path
	PathLink.GetTransformOnLink( NewTransform, AlongLine );

	return TRUE;
}

