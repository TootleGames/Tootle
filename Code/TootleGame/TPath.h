/*------------------------------------------------------

	Runtime path stuff. Mostly using the network path assets

	TLPath contains linear paths (TPaths)
	Spidering (route finding)

-------------------------------------------------------*/
#pragma once

#include <TootleAsset/TPath.h>
#include <TootleCore/TKeyArray.h>
#include <TootleMaths/TQuadTree.h>


namespace TLPath
{
	class TPath;				//	linear path (route) of a path network
	class TPathSpider;			//	base route finder
	class TPathSpider_Path;		//	path finder
	class TPathNetworkZones;	//	class that links a quadtree to a path network for fast zone<->path lookup

	namespace TPathMode
	{
		enum Type
		{
			Fastest = 0,	//	most effecient and fastest at runtime
			Shortest,		//	shortest distance/least expensive route
			Loop,			//	find looping paths
		};
	};
};





//--------------------------------------------------
//	path in a network - could be an asset? no need at the moment
//--------------------------------------------------
class TLPath::TPath
{
public:
	TPath(TRefRef PathNetworkRef);

	FORCEINLINE TRefRef				GetPathNetworkRef() const	{	return m_PathNetworkRef;	}

	FORCEINLINE TArray<TRef>&		GetNodes()					{	return m_Nodes;	}
	FORCEINLINE const TArray<TRef>&	GetNodes() const			{	return m_Nodes;	}

	SyncBool						FindPath(TRefRef StartNode,TRefRef EndNode,TLPath::TPathMode::Type PathMode, Bool Blocking);	//	create a find path spider
	SyncBool						FindPathRandom(TRefRef StartNode,u32 NodesInRoute, Bool Blocking);	//	just follow N random nodes to make a path
	SyncBool						UpdateFindPath();			//	continue updating find path spider

protected:
	TRef					m_PathNetworkRef;	//	ref of path network asset
	THeapArray<TRef>		m_Nodes;			//	list of nodes along the network to make up the path
	TPtr<TPathSpider_Path>	m_pPathSpider;		//	path spider
};





//---------------------------------------------------------
//	base class to async spider the road map
//	both for testing the integrity of the network and 
//	as a visual way of my route finder
//---------------------------------------------------------
class TLPath::TPathSpider
{
public:
	TPathSpider(TPtr<TLAsset::TPathNetwork>& pPathNetwork,TRef StartingNodeRef=TRef());

	virtual SyncBool			Update();		//	do next spidering

protected:
	void						DoSpiderNode(TRefRef RoadNodeRef,s32& MaxLinksToProcess);

protected:
	TRef						m_StartingNodeRef;	
	TPtr<TLAsset::TPathNetwork>	m_pPathNetwork;
	THeapArray<TRef,200,TSortPolicySorted<TRef> >	m_EndNodes;			//	nodes to be processed next
	THeapArray<TRef,200,TSortPolicySorted<TRef> >	m_CompletedNodes;	//	nodes that have been visited
};



class TLPath::TPathSpider_Path : public TLPath::TPathSpider
{
public:
	TPathSpider_Path(TPtr<TLAsset::TPathNetwork>& pPathNetwork,TRefRef PathFromNode,TRefRef PathToNode,TPathMode::Type PathMode);

	virtual SyncBool		Update();		//	do next spidering
	TPtr<THeapArray<TRef> >&	GetFinalPath()	{	return m_pFinalPath;	}

protected:
	void					RemoveEndNode(TRefRef EndNodeRef);		//	remove all end node data for this node

protected:
	TRef								m_NodeFrom;
	TRef								m_NodeTo;
	TPathMode::Type						m_PathMode;
	TPtr<THeapArray<TRef> >					m_pFinalPath;

	TKeyArray<TRef,float>				m_EndNodeLength;		//	current travel distance for each end node
	TKeyArray<TRef,float>				m_EndNodeDistance;		//	for each end node, this is the distance to the ToNode
	TKeyArray<TRef,TPtr<THeapArray<TRef> > >	m_EndNodePaths;		//	for each end node we keep a record of the path
	THeapArray<TRef>						m_DeadEnds;			//	dead ends we've found
};



//------------------------------------------------------------
//	class that links a quadtree to a path network for fast zone<->path lookup
//------------------------------------------------------------
class TLPath::TPathNetworkZones
{
public:	//	gr: public so we can declare them as data types
	class TPathLinkZones
	{
	public:
		TFixedArray<const TLMaths::TQuadTreeZone*,10>	m_Zones;
	};

	class TZonePathLinks
	{
	public:
		TFixedArray<TLPath::TPathLink,10>	m_PathLinks;
	};

public:
	TPathNetworkZones(TPtr<TLAsset::TPathNetwork>& pPathNetwork,TPtr<TLMaths::TQuadTreeZone>& pRootZone,Bool LeafsOnly);

	const TArray<const TLMaths::TQuadTreeZone*>*	GetPathLinkZones(const TLPath::TPathLink& PathLink);
	const TArray<TLPath::TPathLink>*				GetZonePathLinks(const TLMaths::TQuadTreeZone* pZone);

	Bool											GetRandomPathPositionInRandomZone(TLMaths::TTransform& NewTransform,TLPath::TPathLink& PathLink,const TArray<TLMaths::TQuadTreeZone*>& ZoneList);

protected:
	TPtr<TLAsset::TPathNetwork>			m_pPathNetwork;
	TPtr<TLMaths::TQuadTreeZone>		m_pRootZone;
	Bool								m_LeafsOnly;
	TKeyArray<TPathLink,TPathLinkZones>					m_PathLinkZones;		//	lookup pathlink -> zone list
	TKeyArray<const TLMaths::TQuadTreeZone*,TZonePathLinks>	m_ZonePathLinks;		//	lookup zone -> pathlink list
};


TLCore_DeclareIsDataType( TLPath::TPathNetworkZones::TPathLinkZones );
TLCore_DeclareIsDataType( TLPath::TPathNetworkZones::TZonePathLinks );

