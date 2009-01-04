#include "TGraphBase.h"
#include <TootleAsset/TScheme.h>


//--------------------------------------------------------------------	
//	import scheme into this graph
//--------------------------------------------------------------------	
Bool TLGraph::TGraphBase::ImportScheme(const TPtr<TLAsset::TScheme>& pScheme,TRefRef ParentNodeRef)
{
	if ( !pScheme )
	{
		TLDebug_Break("Scheme expected");
		return FALSE;
	}

	//	keep track of all the node's we've imported so we can remove them again if it fails
	TArray<TRef> ImportedNodes;
	const TPtrArray<TLAsset::TSchemeNode>& SchemeNodes = pScheme->GetNodes();

	for ( u32 n=0;	n<SchemeNodes.GetSize();	n++ )
	{
		TArray<TRef> NodeImportedNodes;
		if ( !ImportSchemeNode( SchemeNodes[n], ParentNodeRef, NodeImportedNodes ) )
		{
			//	remove nodes we added 
			RemoveNodes( NodeImportedNodes );
			continue;
		}

		//	added this tree okay, add to the overall list of nodes added
		ImportedNodes.Add( NodeImportedNodes );
	}

	TTempString DebugString("Scheme ");
	pScheme->GetAssetRef().GetString( DebugString );
	DebugString.Appendf(" %d nodes into graph", ImportedNodes.GetSize() );
	TLDebug_Print( DebugString );

	//	if no nodes added, fail
	if ( !ImportedNodes.GetSize() )
		return FALSE;

	return TRUE;
}


//--------------------------------------------------------------------	
//	import scheme node (tree) into this graph
//--------------------------------------------------------------------	
Bool TLGraph::TGraphBase::ImportSchemeNode(const TPtr<TLAsset::TSchemeNode>& pSchemeNode,TRefRef ParentRef,TArray<TRef>& ImportedNodes)
{
	//	create an init message with all the data in the SchemeNode
	TPtr<TLMessaging::TMessage>	pInitMessage = new TLMessaging::TMessage( TLCore::InitialiseRef );
	pInitMessage->CopyDataTree( pSchemeNode->GetData(), FALSE );

	//	create node
	TRef NewNodeRef = CreateNode( pSchemeNode->GetNodeRef(), pSchemeNode->GetTypeRef(), ParentRef, pInitMessage );

	//	failed to create node
	if ( !NewNodeRef.IsValid() )
		return FALSE;

	//	created, add to list
	ImportedNodes.Add( NewNodeRef );

	//	import child nodes
	const TPtrArray<TLAsset::TSchemeNode>& ChildSchemeNodes = pSchemeNode->GetChildren();
	for ( u32 n=0;	n<ChildSchemeNodes.GetSize();	n++ )
	{
		if ( !ImportSchemeNode( ChildSchemeNodes[n], NewNodeRef, ImportedNodes ) )
			return FALSE;
	}

	return TRUE;
}


//--------------------------------------------------------------------	
//	remove an array of nodes by their ref
//--------------------------------------------------------------------	
void TLGraph::TGraphBase::RemoveNodes(const TArray<TRef>& NodeRefs)
{
	for ( u32 n=0;	n<NodeRefs.GetSize();	n++ )
	{
		RemoveNode( NodeRefs.ElementAtConst(n) );
	}
}
