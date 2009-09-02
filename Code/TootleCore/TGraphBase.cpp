#include "TGraphBase.h"
#include <TootleAsset/TScheme.h>


#ifdef _DEBUG
//#define DEBUG_SCHEME_IMPORT_EXPORT
#endif


//--------------------------------------------------------------------	
//	import scheme into this graph
//--------------------------------------------------------------------	
Bool TLGraph::TGraphBase::ImportScheme(const TLAsset::TScheme& Scheme,TRefRef ParentNodeRef,Bool StrictNodeRefs,TBinaryTree* pCommonInitData)
{
	//	keep track of all the node's we've imported so we can remove them again if it fails
	TArray<TRef> ImportedNodes;
	const TPtrArray<TLAsset::TSchemeNode>& SchemeNodes = Scheme.GetNodes();

	for ( u32 n=0;	n<SchemeNodes.GetSize();	n++ )
	{
		TArray<TRef> NodeImportedNodes;
		if ( !ImportSchemeNode( *SchemeNodes[n], ParentNodeRef, NodeImportedNodes, StrictNodeRefs, pCommonInitData ) )
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
	//	gr; could be empty of nodes...
//	if ( !ImportedNodes.GetSize() )
//		return FALSE;

	return TRUE;
}


//--------------------------------------------------------------------	
//	import scheme node (tree) into this graph
//--------------------------------------------------------------------	
Bool TLGraph::TGraphBase::ImportSchemeNode(TLAsset::TSchemeNode& SchemeNode,TRefRef ParentRef,TArray<TRef>& ImportedNodes,Bool StrictNodeRefs,TBinaryTree* pCommonInitData)
{
#ifdef DEBUG_SCHEME_IMPORT_EXPORT
	TTempString Debug_String("Importing scheme node: ");
	SchemeNode.GetNodeRef().GetString( Debug_String );
	Debug_String.Append("(");
	SchemeNode.GetTypeRef().GetString( Debug_String );
	Debug_String.Append(")");
	TLDebug_Print( Debug_String );
#endif

	SchemeNode.GetData().SetTreeUnread();

	//	create an init message with all the data in the SchemeNode
	TLMessaging::TMessage Message( TLCore::InitialiseRef );
	Message.ReferenceDataTree( SchemeNode.GetData() );

	//	add common data too
	if ( pCommonInitData )
	{
		pCommonInitData->SetTreeUnread();
		Message.ReferenceDataTree( *pCommonInitData );
	}

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

		if ( !ImportSchemeNode( *pChildSchemeNode, NewNodeRef, ImportedNodes, StrictNodeRefs, pCommonInitData ) )
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

#ifdef DEBUG_SCHEME_IMPORT_EXPORT
	TTempString Debug_String("Exporting scheme node: ");
	pNode->GetNodeRef().GetString( Debug_String );
	Debug_String.Append("(");
	pNode->GetNodeTypeRef().GetString( Debug_String );
	Debug_String.Append(")");
	TLDebug_Print( Debug_String );
#endif

	//	create scheme node
	TPtr<TLAsset::TSchemeNode> pSchemeNode = new TLAsset::TSchemeNode( pNode->GetNodeRef(), GetGraphRef(), pNode->GetNodeTypeRef() );

	// Test to see if the node is flagged to be ignored when storing to a scheme
	if(!pNode->GetNodeData().HasChild("IgnoreData"))
	{
		//	export data from node to scheme node
		pNode->UpdateNodeData();

		const TBinaryTree& NodeData = pNode->GetNodeData();
		pSchemeNode->GetData().ReferenceDataTree( NodeData );

		//	export children into this node
		TArray<TGraphNodeBase*> RootChildren;
		pNode->GetChildrenBase( RootChildren );
		for ( u32 c=0;	c<RootChildren.GetSize();	c++ )
		{
			TPtr<TLAsset::TSchemeNode> pChildSchemeNode = ExportSchemeNode( RootChildren[c] );
			pSchemeNode->AddChild( pChildSchemeNode );
		}
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
Bool TLGraph::TGraphBase::ExportScheme(TLAsset::TScheme& Scheme,TRef SchemeRootNode,Bool IncludeSchemeRootNode)
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

	//	if we're including the root node then export from that one recursively
	if ( IncludeSchemeRootNode )
	{
		TPtr<TLAsset::TSchemeNode> pSchemeNode = ExportSchemeNode( pSchemeRootNode );
		Scheme.AddNode( pSchemeNode );
	}
	else
	{
		//	only adding children of the root node
		TArray<TGraphNodeBase*> RootChildren;
		pSchemeRootNode->GetChildrenBase( RootChildren );
		for ( u32 c=0;	c<RootChildren.GetSize();	c++ )
		{
			TPtr<TLAsset::TSchemeNode> pSchemeNode = ExportSchemeNode( RootChildren[c] );
			Scheme.AddNode( pSchemeNode );
		}
	}

	return TRUE;
}

	
//--------------------------------------------------------------------	
//	re-import scheme into this graph. Nodes will be re-sent an Initialise message. 
//	Add missing and delete new (non-scheme) nodes via params
//--------------------------------------------------------------------	
Bool TLGraph::TGraphBase::ReimportScheme(const TLAsset::TScheme& Scheme,TRefRef ParentNodeRef,Bool StrictNodeRefs,Bool AddMissingNodes,Bool RemoveUnknownNodes,TBinaryTree* pCommonInitData)
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
		if ( !ReimportSchemeNode( *SchemeNodes[n], ParentNodeRef, StrictNodeRefs, AddMissingNodes, RemoveUnknownNodes, pCommonInitData ) )
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
Bool TLGraph::TGraphBase::ReimportSchemeNode(TLAsset::TSchemeNode& SchemeNode,TRefRef ParentRef,Bool StrictNodeRefs,Bool AddMissingNodes,Bool RemoveUnknownNodes,TBinaryTree* pCommonInitData)
{
	//	create an init message with all the data in the SchemeNode
	SchemeNode.GetData().SetTreeUnread();

	TLMessaging::TMessage Message( TLCore::InitialiseRef );
	Message.ReferenceDataTree( SchemeNode.GetData() );

	//	add common data too
	if ( pCommonInitData )
	{
		pCommonInitData->SetTreeUnread();
		Message.ReferenceDataTree( *pCommonInitData );
	}

	//	restore node if missing
	if ( !IsInGraph( SchemeNode.GetNodeRef() ) )
	{
		//	don't restore node
		if ( !AddMissingNodes )
			return TRUE;
	
		//	re-create node
		TFixedArray<TRef,100> ImportedNodes;
		return ImportSchemeNode( SchemeNode, ParentRef, ImportedNodes, TRUE, pCommonInitData );
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

		if ( !ReimportSchemeNode( *pChildSchemeNode, SchemeNode.GetNodeRef(), StrictNodeRefs, AddMissingNodes, RemoveUnknownNodes, pCommonInitData ) )
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


//------------------------------------------------------------
//	get array of children's refs
//------------------------------------------------------------
void TLGraph::TGraphNodeBase::GetChildren(TArray<TRef>& ChildNodeRefs,Bool Recursive)
{
	//	get children
	TArray<TGraphNodeBase*> ChildNodes;
	GetChildrenBase( ChildNodes );

	for ( u32 c=0;	c<ChildNodes.GetSize();	c++ )
	{
		TGraphNodeBase* pChild = ChildNodes[c];

		//	add this child's ref...
		ChildNodeRefs.Add( pChild->GetNodeRef() );	

		//	now add childrens children
		if ( Recursive )
			pChild->GetChildrenTree( ChildNodeRefs );
	}
}