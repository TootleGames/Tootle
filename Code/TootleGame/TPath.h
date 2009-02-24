/*------------------------------------------------------

	Runtime path stuff. Mostly using the network path assets

	TLPath contains linear paths (TPaths)
	Spidering (route finding)

-------------------------------------------------------*/
#pragma once

#include <TootleAsset/TPath.h>
#include <TootleCore/TKeyArray.h>

namespace TLPath
{
	class TPath;			//	linear path (route) of a path network
	class TPathSpider;		//	base route finder
	class TPathSpider_Path;	//	path finder


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
	SyncBool						UpdateFindPath();			//	continue updating find path spider

protected:
	TRef					m_PathNetworkRef;	//	ref of path network asset
	TArray<TRef>			m_Nodes;			//	list of nodes along the network to make up the path
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
	TArray<TRef>				m_EndNodes;			//	nodes to be processed next
	TArray<TRef>				m_CompletedNodes;	//	nodes that have been visited
};



class TLPath::TPathSpider_Path : public TLPath::TPathSpider
{
public:
	TPathSpider_Path(TPtr<TLAsset::TPathNetwork>& pPathNetwork,TRefRef PathFromNode,TRefRef PathToNode,TPathMode::Type PathMode);

	virtual SyncBool		Update();		//	do next spidering
	TPtr<TArray<TRef> >&	GetFinalPath()	{	return m_pFinalPath;	}

protected:
	void					RemoveEndNode(TRefRef EndNodeRef);		//	remove all end node data for this node

protected:
	TRef								m_NodeFrom;
	TRef								m_NodeTo;
	TPathMode::Type						m_PathMode;
	TPtr<TArray<TRef> >					m_pFinalPath;

	TKeyArray<TRef,float>				m_EndNodeLength;		//	current travel distance for each end node
	TKeyArray<TRef,float>				m_EndNodeDistance;		//	for each end node, this is the distance to the ToNode
	TKeyArray<TRef,TPtr<TArray<TRef> > >	m_EndNodePaths;		//	for each end node we keep a record of the path
	TArray<TRef>						m_DeadEnds;			//	dead ends we've found
};


