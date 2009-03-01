#include "TGraphBase.h"
#include <TootleAsset/TScheme.h>


//--------------------------------------------------------------------	
//	import scheme into this graph
//--------------------------------------------------------------------	
Bool TLGraph::TGraphBase::ImportScheme(const TPtr<TLAsset::TScheme>& pScheme,TRefRef ParentNodeRef,Bool StrictNodeRefs)
{
	if ( !pScheme )
	{
		TLDebug_Break("Scheme expected");
		return FALSE;
	}

	if ( pScheme->GetAssetType() != "Scheme" )
	{
		TTempString Debug_String("Trying to import scheme asset ");
		pScheme->GetAssetRef().GetString( Debug_String );
		Debug_String.Append(" but is wrong asset type: ");
		pScheme->GetAssetType().GetString( Debug_String );
		TLDebug_Break( Debug_String );
		return FALSE;
	}

	const TLAsset::TScheme& Scheme = *pScheme;

	//	keep track of all the node's we've imported so we can remove them again if it fails
	TArray<TRef> ImportedNodes;
	const TPtrArray<TLAsset::TSchemeNode>& SchemeNodes = Scheme.GetNodes();

	for ( u32 n=0;	n<SchemeNodes.GetSize();	n++ )
	{
		TArray<TRef> NodeImportedNodes;
		if ( !ImportSchemeNode( *SchemeNodes[n], ParentNodeRef, NodeImportedNodes, StrictNodeRefs ) )
		{
			//	remove nodes we added 
			RemoveNodes( NodeImportedNodes );
			continue;
		}

		//	added this tree okay, add to the overall list of nodes added
		ImportedNodes.Add( NodeImportedNodes );
	}

#ifdef _DEBUG
	TTempString DebugString("Scheme ");
	Scheme.GetAssetRef().GetString( DebugString );
	DebugString.Appendf(" %d nodes into graph", ImportedNodes.GetSize() );
	TLDebug_Print( DebugString );
#endif

	//	if no nodes added, fail
	if ( !ImportedNodes.GetSize() )
		return FALSE;

	return TRUE;
}


//--------------------------------------------------------------------	
//	import scheme node (tree) into this graph
//--------------------------------------------------------------------	
Bool TLGraph::TGraphBase::ImportSchemeNode(const TLAsset::TSchemeNode& SchemeNode,TRefRef ParentRef,TArray<TRef>& ImportedNodes,Bool StrictNodeRefs)
{
	//	create an init message with all the data in the SchemeNode
	TPtr<TLMessaging::TMessage>	pInitMessage = new TLMessaging::TMessage( TLCore::InitialiseRef );
	pInitMessage->ReferenceDataTree( SchemeNode.GetData(), FALSE );

	//	create node
	TRef NewNodeRef = CreateNode( SchemeNode.GetNodeRef(), SchemeNode.GetTypeRef(), ParentRef, pInitMessage, StrictNodeRefs );

	//	failed to create node
	if ( !NewNodeRef.IsValid() )
		return FALSE;

	//	created, add to list
	ImportedNodes.Add( NewNodeRef );

	//	import child nodes
	const TPtrArray<TLAsset::TSchemeNode>& ChildSchemeNodes = SchemeNode.GetChildren();
	for ( u32 n=0;	n<ChildSchemeNodes.GetSize();	n++ )
	{
		const TPtr<TLAsset::TSchemeNode>& pChildSchemeNode = ChildSchemeNodes[n];

		if ( !ImportSchemeNode( *pChildSchemeNode, NewNodeRef, ImportedNodes, StrictNodeRefs ) )
			return FALSE;
	}

	return TRUE;
}


//--------------------------------------------------------------------	
//	export a node into a scheme node
//--------------------------------------------------------------------	
TPtr<TLAsset::TSchemeNode> TLGraph::TGraphBase::ExportSchemeNode(TGraphNodeBase* pNode)
{
	if ( !pNode )
	{
		TLDebug_Break("Node expected");
		return NULL;
	}

	//	create scheme node
	TPtr<TLAsset::TSchemeNode> pSchemeNode = new TLAsset::TSchemeNode( pNode->GetNodeRef(), GetGraphRef(), pNode->GetNodeTypeRef() );

	//	export data from node to scheme node
	const TBinaryTree& NodeData = pNode->GetNodeData( TRUE );
	pSchemeNode->GetData().ReferenceDataTree( NodeData );

	//	export children into this node
	TArray<TGraphNodeBase*> RootChildren;
	pNode->GetChildrenBase( RootChildren );
	for ( u32 c=0;	c<RootChildren.GetSize();	c++ )
	{
		TPtr<TLAsset::TSchemeNode> pChildSchemeNode = ExportSchemeNode( RootChildren[c] );
		pSchemeNode->AddChild( pChildSchemeNode );
	}

	return pSchemeNode;
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


//--------------------------------------------------------------------	
//	export node tree to a scheme
//--------------------------------------------------------------------	
TPtr<TLAsset::TScheme> TLGraph::TGraphBase::ExportScheme(TRef SchemeAssetRef,TRef SchemeRootNode,Bool IncludeSchemeRootNode)
{
	TLGraph::TGraphNodeBase* pSchemeRootNode = NULL;

	//	get the root node for the scheme
	if ( SchemeRootNode.IsValid() )
	{
		pSchemeRootNode = FindNodeBase(SchemeRootNode);
	}
	else
	{
		//	none specified, so use graph root node
		pSchemeRootNode = GetRootNodeBase();
	}

	//	no node
	if ( !pSchemeRootNode )
	{
		TLDebug_Break("Failed to find scheme root node to export");
		return FALSE;
	}

	//	gr: currently we're not putting this scheme asset into the asset list, so no need to
	//		check that its free...
	//SchemeAssetRef = TLAsset::GetFreeAssetRef( SchemeAssetRef );

	//	create asset
	TPtr<TLAsset::TScheme> pScheme = new TLAsset::TScheme(SchemeAssetRef);

	//	if we're including the root node then export from that one recursively
	if ( IncludeSchemeRootNode )
	{
		TPtr<TLAsset::TSchemeNode> pSchemeNode = ExportSchemeNode( pSchemeRootNode );
		pScheme->AddNode( pSchemeNode );
	}
	else
	{
		//	only adding children of the root node
		TArray<TGraphNodeBase*> RootChildren;
		pSchemeRootNode->GetChildrenBase( RootChildren );
		for ( u32 c=0;	c<RootChildren.GetSize();	c++ )
		{
			TPtr<TLAsset::TSchemeNode> pSchemeNode = ExportSchemeNode( RootChildren[c] );
			pScheme->AddNode( pSchemeNode );
		}
	}

	return pScheme;
}




TLGraph::TGraphNodeBase::TGraphNodeBase(TRefRef NodeRef,TRefRef NodeTypeRef) :
	m_NodeRef		( NodeRef ),
	m_NodeTypeRef	( NodeTypeRef ),
	m_NodeData		( "Data" )
{
	//	get debug strings
	m_NodeRef.GetString( m_Debug_NodeRefString );	
	m_NodeTypeRef.GetString( m_Debug_NodeTypeRefString );
}
