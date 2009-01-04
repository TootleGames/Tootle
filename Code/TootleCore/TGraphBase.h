/*------------------------------------------------------
	
	Base graph class to do generic nodey/graph stuff without 
	needing to know the graph instance/type

	all returns and operations use TRef's so we don't need to
	know the type

-------------------------------------------------------*/
#pragma once

#include "TRef.h"
#include "TLMessaging.h"

namespace TLAsset
{
	class TScheme;
	class TSchemeNode;
}

namespace TLGraph
{
	class TGraphBase;
};


//--------------------------------------------------------------------
//	base graph class
//--------------------------------------------------------------------
class TLGraph::TGraphBase
{
public:
	TGraphBase()				{}
	
	virtual Bool				SendMessageToNode(TRefRef NodeRef,TPtr<TLMessaging::TMessage>& pMessage)=0;	//	send message to node

	virtual TRef				CreateNode(TRefRef NodeRef,TRefRef TypeRef,TRefRef ParentRef,TPtr<TLMessaging::TMessage> pInitMessage)=0;	//	create node and add to the graph. returns ref of new node
	void						RemoveNodes(const TArray<TRef>& NodeRefs);							//	remove an array of nodes by their ref
	virtual Bool				RemoveNode(TRefRef NodeRef)=0;										//	remove an array of nodes by their ref

	Bool						ImportScheme(const TPtr<TLAsset::TScheme>& pScheme,TRefRef ParentNodeRef=TRef());		//	import scheme into this graph
	Bool						ImportSchemeNode(const TPtr<TLAsset::TSchemeNode>& pSchemeNode,TRefRef ParentRef,TArray<TRef>& ImportedNodes);				//	import scheme node (tree) into this graph

protected:
};


