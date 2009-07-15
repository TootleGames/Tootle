#include "TGraphBase.h"
#include <TootleAsset/TScheme.h>


//--------------------------------------------------------------------	
//	import scheme into this graph
//--------------------------------------------------------------------	
Bool TLGraph::TGraphBase::ImportScheme(const TLAsset::TScheme& Scheme,TRefRef ParentNodeRef,Bool StrictNodeRefs,TLMessaging::TMessage* pCommonInitMessage)
{
	//	just do a quick type check...
	if ( Scheme.GetAssetType() != "Scheme" )
	{
		TTempString Debug_String("Trying to import scheme asset ");
		Scheme.GetAssetRef().GetString( Debug_String );
		Debug_String.Append(" but is wrong asset type: ");
		Scheme.GetAssetType().GetString( Debug_String );
		TLDebug_Break( Debug_String );
		return FALSE;
	}

	//	keep track of all the node's we've imported so we can remove them again if it fails
	TArray<TRef> ImportedNodes;
	const TPtrArray<TLAsset::TSchemeNode>& SchemeNodes = Scheme.GetNodes();

	for ( u32 n=0;	n<SchemeNodes.GetSize();	n++ )
	{
		TArray<TRef> NodeImportedNodes;
		if ( !ImportSchemeNode( *SchemeNodes[n], ParentNodeRef, NodeImportedNodes, StrictNodeRefs, pCommonInitMessage ) )
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
Bool TLGraph::TGraphBase::ImportSchemeNode(const TLAsset::TSchemeNode& SchemeNode,TRefRef ParentRef,TArray<TRef>& ImportedNodes,Bool StrictNodeRefs,TLMessaging::TMessage* pCommonInitMessage)
{
	//	create an init message with all the data in the SchemeNode
	TLMessaging::TMessage Message( TLCore::InitialiseRef );
	Message.ReferenceDataTree( SchemeNode.GetData(), FALSE );

	//	add common data too
	if ( pCommonInitMessage )
		Message.ReferenceDataTree( *pCommonInitMessage, FALSE );

	//	create node
	TRef NewNodeRef = CreateNode( SchemeNode.GetNodeRef(), SchemeNode.GetTypeRef(), ParentRef, &Message, StrictNodeRefs );

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

		if ( !ImportSchemeNode( *pChildSchemeNode, NewNodeRef, ImportedNodes, StrictNodeRefs, pCommonInitMessage ) )
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

	
//--------------------------------------------------------------------	
//	re-import scheme into this graph. Nodes will be re-sent an Initialise message. 
//	Add missing and delete new (non-scheme) nodes via params
//--------------------------------------------------------------------	
Bool TLGraph::TGraphBase::ReimportScheme(const TLAsset::TScheme& Scheme,TRefRef ParentNodeRef,Bool StrictNodeRefs,Bool AddMissingNodes,Bool RemoveUnknownNodes,TLMessaging::TMessage* pCommonInitMessage)
{
	if ( !StrictNodeRefs )
	{
		TLDebug_Break("Cannot use the ReimportScheme system if we didn't use strict node ref's the first time. Seeing as you passed in false, can only assume you didn't use strict refs orignally...");
		return FALSE;
	}

	//	just do a quick type check...
	if ( Scheme.GetAssetType() != "Scheme" )
	{
		TTempString Debug_String("Trying to import scheme asset ");
		Scheme.GetAssetRef().GetString( Debug_String );
		Debug_String.Append(" but is wrong asset type: ");
		Scheme.GetAssetType().GetString( Debug_String );
		TLDebug_Break( Debug_String );
		return FALSE;
	}

	//	loop through all the scheme nodes
	const TPtrArray<TLAsset::TSchemeNode>& SchemeNodes = Scheme.GetNodes();
	for ( u32 n=0;	n<SchemeNodes.GetSize();	n++ )
	{
		TArray<TRef> NodeImportedNodes;
		if ( !ReimportSchemeNode( *SchemeNodes[n], ParentNodeRef, StrictNodeRefs, AddMissingNodes, RemoveUnknownNodes, pCommonInitMessage ) )
		{
			//	remove nodes we added 
			RemoveNodes( NodeImportedNodes );
			continue;
		}
	}

	return TRUE;
}


//--------------------------------------------------------------------	
//	re-init and restore node tree
//--------------------------------------------------------------------	
Bool TLGraph::TGraphBase::ReimportSchemeNode(const TLAsset::TSchemeNode& SchemeNode,TRefRef ParentRef,Bool StrictNodeRefs,Bool AddMissingNodes,Bool RemoveUnknownNodes,TLMessaging::TMessage* pCommonInitMessage)
{
	//	create an init message with all the data in the SchemeNode
	TLMessaging::TMessage Message( TLCore::InitialiseRef );
	Message.ReferenceDataTree( SchemeNode.GetData(), FALSE );

	//	add common data too
	if ( pCommonInitMessage )
		Message.ReferenceDataTree( *pCommonInitMessage, FALSE );

	//	restore node if missing
	if ( !IsInGraph( SchemeNode.GetNodeRef() ) )
	{
		//	don't restore node
		if ( !AddMissingNodes )
			return TRUE;
	
		//	re-create node
		TFixedArray<TRef,100> ImportedNodes;
		return ImportSchemeNode( SchemeNode, ParentRef, ImportedNodes, TRUE, pCommonInitMessage );
	}

	//	node exists, re-init
	SendMessageToNode( SchemeNode.GetNodeRef(), Message );

	//	check for children of this node that are NOT in our scheme
	if ( RemoveUnknownNodes )
	{
		TLDebug_Break("todo: gr: needs a bit of thought of how to do this without access to children...");
	}

	//	re-import child scheme nodes
	const TPtrArray<TLAsset::TSchemeNode>& ChildSchemeNodes = SchemeNode.GetChildren();
	for ( u32 n=0;	n<ChildSchemeNodes.GetSize();	n++ )
	{
		const TPtr<TLAsset::TSchemeNode>& pChildSchemeNode = ChildSchemeNodes[n];

		if ( !ReimportSchemeNode( *pChildSchemeNode, SchemeNode.GetNodeRef(), StrictNodeRefs, AddMissingNodes, RemoveUnknownNodes, pCommonInitMessage ) )
			return FALSE;
	}

	return TRUE;
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



//------------------------------------------------------------
//	 base initialise for a node
//------------------------------------------------------------
void TLGraph::TGraphNodeBase::Initialise(TLMessaging::TMessage& Message)
{
}


//------------------------------------------------------------
//	base SetProperty function 
//------------------------------------------------------------
void TLGraph::TGraphNodeBase::SetProperty(TLMessaging::TMessage& Message)
{
}

//------------------------------------------------------------
//	GetProperty message handler
//------------------------------------------------------------
void TLGraph::TGraphNodeBase::GetProperty(TLMessaging::TMessage& Message, TLMessaging::TMessage& Response)
{
}
