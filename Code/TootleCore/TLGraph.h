
#pragma once

//#include "TPtrLinkedList.h"
//#include "TPtrTree.h"


#define TLGRAPH_OWN_CHILDREN


#include "TGraphBase.h"
#include "TLMessaging.h"
#include "TRelay.h"
#include "TManager.h"
#include <TootleCore/TEventChannel.h>
#include <TootleCore/TClassFactory.h>
#include <TootleAsset/TScheme.h>


namespace TLGraph
{
	template <class T>
	class TGraphNode;

	template <class T>
	class TGraph;


	//	node sorting
	template<typename NODETYPE>	
	FORCEINLINE TLArray::SortResult	TPtrRefSort(const TPtr<NODETYPE>& a,const TPtr<NODETYPE>& b,const void* pTestVal)
	{
		//	if pTestVal provided it's the ref
		TRefRef aRef = a->GetNodeRef();
		TRefRef bRef = pTestVal ? *(const TRef*)pTestVal : b->GetNodeRef();
			
		//	== turns into 0 (is greater) or 1(equals)
		return aRef < bRef ? TLArray::IsLess : (TLArray::SortResult)(aRef==bRef);	
	}

};

#define TGraph_DefaultGrowBy	300	//	gr: graphs get pretty big



//--------------------------------------------------------------------
//	TGraph class - templated graph management class
//--------------------------------------------------------------------
template <class T>
class TLGraph::TGraph : public TManager, public TLMessaging::TMessageQueue, public TLGraph::TGraphBase
{
	friend class TGraphNode<T>;
	
public:
	TGraph(TRef refManagerID) :
		TManager	( refManagerID ),
		m_NodeIndex	( &TLGraph::TPtrRefSort<T>, TGraph_DefaultGrowBy )
	{
	}
	
	virtual TRefRef				GetGraphRef() const						{	return TManager::GetManagerRef();	}

	//	Node access
	inline TPtr<T>&				GetRootNode()							{	return m_pRootNode; }
	inline TPtrArray<T>&		GetNodeList()							{	return m_NodeIndex;	}
	inline u32					GetNodeCount() const					{	return m_NodeIndex.GetSize();	}
	template<typename MATCHTYPE>
	TPtr<T>&					FindNodeMatch(const MATCHTYPE& Value);	//	find a TPtr in the graph that matches the specified value (will use == operator of node type to match)

	TPtr<T>&					FindNode(TRefRef NodeRef);				//	find a node
	TRef						GetFreeNodeRef(TRefRef BaseRef=TRef());	//	find an unused ref for a node - returns the ref
	TRefRef						GetFreeNodeRef(TRef& Ref);				//	find an unused ref for a node, modifies the ref provided
	virtual Bool				SendMessageToNode(TRefRef NodeRef,TPtr<TLMessaging::TMessage>& pMessage);	//	send message to node

	// Graph change requests
	virtual TRef				CreateNode(TRefRef NodeRef,TRefRef TypeRef,TRefRef ParentRef,TPtr<TLMessaging::TMessage> pInitMessage=NULL);		//	create node and add to the graph. returns ref of new node

	// [05/01/09] DB - Renamed to avoid ambiguity with ref type version of same routine
	TPtr<T>						DoCreateNode(TRefRef NodeRef,TRefRef TypeRef,TPtr<T> pParent=NULL,TPtr<TLMessaging::TMessage> pInitMessage=NULL);	//	create node and add to the graph. returns ref of new node

	Bool						AddNode(TPtr<T> pNode, TPtr<T> pParent=NULL);	//	returns the ref of the object. You can't always have the ref we constructed with :)
	FORCEINLINE Bool			AddNode(TPtr<T> pNode,TRefRef ParentRef)		{	return AddNode( pNode, FindNode( ParentRef ) );	}

	virtual Bool				RemoveNode(TRefRef NodeRef)				{	return RemoveNode( FindNode( NodeRef ) );	}
	FORCEINLINE Bool			RemoveNode(const T* pNode)				{	return RemoveNode( FindNode( pNode->GetNodeRef() ) );	}
	Bool						RemoveNode(TPtr<T> pNode);				//	Requests a node to be removed from the graph.  The node will be removed at a 'safe' time so is not immediate
	Bool						RemoveChildren(TPtr<T> pNode);			//	remove pNode's children (but not pNode)

	//	factory access
	Bool						AddFactory(TPtr< TClassFactory<T,FALSE> >& pFactory)
	{
		return (m_NodeFactories.Add(pFactory) != -1);			
	}

protected:
	virtual SyncBool			Initialise();
	virtual SyncBool			Update(float fTimeStep);
	virtual SyncBool			Shutdown();

	// Main update of the graph
	virtual void				UpdateGraph(float fTimeStep);
	void						UpdateGraphStructure();							// Adds/removes nodes that have been queued up for update

	template<typename MATCHTYPE>
	Bool						IsInGraph(const MATCHTYPE& Value)		{	return FindNodeMatch( Value ).IsValid();	}
	Bool						IsInGraph(TRefRef NodeRef)				{	return FindNode( NodeRef ).IsValid();	}
	Bool						IsInQueue(TPtr<T>& pNode);

	virtual void				ProcessMessageFromQueue(TPtr<TLMessaging::TMessage>& pMessage);
	virtual void				OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);
	
	TPtr<T>&					FindPtr(const TGraphNode<T>* pNode) const;					//	find a TPtr in the graph that matches the node - workaround for non intrusive pointers

	//	base graph functions
	virtual TLGraph::TGraphNodeBase*	FindNodeBase(TRefRef NodeRef)		{	TPtr<T>& pNode = FindNode( NodeRef );	return pNode.GetObject<TLGraph::TGraphNodeBase>();	}
	virtual TLGraph::TGraphNodeBase*	GetRootNodeBase()					{	TPtr<T>& pNode = m_pRootNode;	return pNode.GetObject<TLGraph::TGraphNodeBase>();	}

	//	events
	virtual void				OnNodeAdded(TPtr<T>& pNode);				//	called after node has been added to graph and to parent
	virtual void				OnNodeRemoving(TPtr<T>& pNode)		{	}	//	called before node shutdown and before being removed from parent and graph
	virtual void				OnNodeRemoved(TPtr<T>& pNode);				//	called after node shutdown and after being removed from parent and graph

private:

	// Graph structure updates
	void						DoAddNode(TPtr<T>& pNode,TPtr<T>& pParent);		// Final stage add of the graph node
	void						DoRemoveNode(TPtr<T>& pNode,TPtr<T>& pParent);		// Final stage removal of the graph node

	//	gr: you don;t need another template declaration, by being inside a template class it is templated (it's TGraph<T>::TGraphRequest which is different to TGraph<X>::TGraphRequest)
	//template <class T>
	class TGraphUpdateRequest
	{
	public:
		explicit TGraphUpdateRequest(TPtr<T> pNode, TPtr<T> pParent, Bool bIsRemove) :
			m_pNode			( pNode ),
			m_pNodeParent	( pParent ),
			m_bRemoveNode	( bIsRemove )
		{
		}

		TPtr<T>&	GetNode()			{	return m_pNode;	}
		TPtr<T>&	GetNodeParent()		{	return m_pNodeParent;	}
		Bool		IsRemoveRequest()	{	return m_bRemoveNode;	}

		inline Bool	operator==(TRefRef NodeRef) const					{	return m_pNode == NodeRef;	}
		inline Bool	operator<(TRefRef NodeRef) const					{	return m_pNode < NodeRef;	}
		inline Bool	operator<(const TGraphUpdateRequest& Update) const	{	return m_pNode < Update.m_pNode;	}

	protected:
		TPtr<T>		m_pNode;					// Node to update
		TPtr<T>		m_pNodeParent;		// Node parent
		Bool		m_bRemoveNode;
	};

private:
	TPtr<T>								m_pRootNode;			// The root of the graph
	TPtrArray<TGraphUpdateRequest>		m_UpdateQueue;			// List of objects to add/remove from the graph
	TPtrArray<T>						m_NodeIndex;			//	list of all the nodes, used for fast node finding

	TPtrArray< TClassFactory<T,FALSE> >	m_NodeFactories;		//	array of graph node factories. if none, T is created
};



//--------------------------------------------------------------------
//	TGraphNode class	- essentially a linked list item and tree item that will provide links to all other items within the immediate heirarchy of the graph
//	gr: for speed when using TPtr's
//		use const TPtr& pPtr for public functions (child funcs)
//		use TPtr& pPtr for internal/protected functions (sibling funcs)
//--------------------------------------------------------------------
template <class T>
class TLGraph::TGraphNode : public TLMessaging::TRelay, public TLMessaging::TMessageQueue, public TLGraph::TGraphNodeBase
{
	friend class TGraph<T>;
public:
	TGraphNode(TRefRef NodeRef,TRefRef NodeTypeRef) : TGraphNodeBase ( NodeRef, NodeTypeRef )	{}
	virtual ~TGraphNode()
	{
		// Ensure this node isn't in the linked list any longer
		//Remove(this);
	}

	virtual void			Initialise(TPtr<TLMessaging::TMessage>& pMessage);	//	Initialise message - made into virtual func as it's so commonly used
	virtual void 			Update(float Timestep);					// Main node update called once per frame
	virtual void			Shutdown()							{}	// Shutdown routine	- called before being removed form the graph

	// Parent manipulation
	inline TPtr<T>&			GetParent()							{	return m_pParent;	}
	inline const TPtr<T>&	GetParent() const					{	return m_pParent;	}
	inline Bool				HasParent() const					{	return m_pParent.IsValid();	}

	// Child manipulation
#ifdef TLGRAPH_OWN_CHILDREN
	inline Bool				HasChildren() const					{	return (m_Children.GetSize() > 0);	}
	inline TPtrArray<T>&	GetChildren()						{	return m_Children;	}
	inline const TPtrArray<T>&	GetChildren() const				{	return m_Children;	}

#else
	inline Bool				HasChildren() const					{	return m_pChildFirst.IsValid();	}
	inline const TPtr<T>&	GetChildFirst() const				{	return m_pChildFirst;	}			//	cant call it ChildFirst because of windows macro
#endif
	template<typename MATCHTYPE>
	TPtr<T>&				FindChildMatch(const MATCHTYPE& Value);		//	find a TPtr in the graph that matches the specified value (will use == operator of node type to match)
	FORCEINLINE TPtr<T>&	FindChild(const TRef& NodeRef)				{	return FindChildMatch(NodeRef);	}


	inline Bool				operator==(const TPtr<TGraphNode<T> >& pNode) const	{	return this == pNode.GetObject();	}
	inline Bool				operator==(const TGraphNode<T>& Node) const			{	return this == (&Node);	}
	inline Bool				operator==(TRefRef NodeRef) const					{	return GetNodeRef() == NodeRef;	}
	inline Bool				operator<(TRefRef NodeRef) const					{	return GetNodeRef() == NodeRef;	}
	inline Bool				operator<(const TGraphNode<T>& Node) const			{	return GetNodeRef() == Node.GetNodeRef();	}

protected:
	virtual const TGraphNodeBase*	GetParentBase() const		{	return m_pParent.GetObject<TGraphNodeBase>();	}

	virtual void			UpdateAll(float Timestep);						//	update tree: update self, and children and siblings

	virtual void			OnAdded()							{}			// Added routine			- called once the node has been added to the graph

	// Sibling manipulation
#ifndef TLGRAPH_OWN_CHILDREN
	Bool					HasNext() const						{	return m_pNext.IsValid();	}
	inline TPtr<T>&			GetNext() 							{	return m_pNext;	}
	inline TPtr<T>&			GetPrevious() 						{	return m_pPrevious;	}
	inline const TPtr<T>&	GetNext() const						{	return m_pNext;	}
	inline const TPtr<T>&	GetPrevious() const					{	return m_pPrevious;	}
#endif

	template<typename MATCHTYPE>
	TPtr<T>&				FindNodeMatch(const MATCHTYPE& Value);		//	find a TPtr in the graph that matches the specified value (will use == operator of node type to match)
	TPtr<T>&				FindNode(const TRef& NodeRef)				{	return FindNodeMatch( NodeRef );	}
	template<typename MATCHTYPE>
	Bool					IsInGraph(const MATCHTYPE& Value)			{	return FindNodeMatch( Value ).IsValid();	}

	virtual void			ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);
	virtual void			ProcessMessageFromQueue(TPtr<TLMessaging::TMessage>& pMessage)	
	{	
		pMessage->ResetReadPos();
		ProcessMessage(pMessage);	
	}
	
	// Parent manipulation
	virtual void			SetParent(TPtr<T>& pNode);						//	gr: needs to make sure removes self from former parent?
	TPtr<T>&				FindPtr(const TGraphNode<T>* pNode) const;		//	find a child/sibling matching this node and return our TPtr to it - workaround for non intrusive smart pointers

	Bool					CheckIsThis(TPtr<T>& pThis);					//	check the node pointer is actually this, if not throws up a DebugBreak

	//	base functions
	virtual void			GetChildrenBase(TArray<TGraphNodeBase*>& ChildNodes);

private:
	virtual Bool			AddChild(TPtr<T>& pChild,TPtr<T>& pThis);		//	gr: add to END of child list
//	virtual Bool			InsertChild(TPtr<T>& pChild,TPtr<T>& pThis)	{	return SetChildFirst( pChild, pThis );	}	//	gr: add to START of child list
	Bool					RemoveChild(TPtr<T>& pItem);
	void					RemoveChildren();

	// Sibling manipulation
#ifdef TLGRAPH_OWN_CHILDREN

#else
	inline void				SetNext(const TPtr<T>& pNode)						{	m_pNext = pNode;	}	//	gr: need to correct the old m_pNext?
	inline void				SetPrevious(const TPtr<T>& pNode)					{	m_pPrevious = pNode;	}	//	gr: need to correct the old m_pPrevious?
	inline Bool				SetChildFirst(TPtr<T>& pNode,TPtr<T>& pThis);		//	assign node as first child
	Bool					AddSibling(TPtr<T>& pSibling,TPtr<T>& pThis);		//	gr: insert adds to END of siblings...
	Bool					RemoveSibling(TPtr<T>& pNode);
	void					RemoveSiblings();
#endif

private:
	TPtr<T>				m_pParent;			// Parent item

#ifdef TLGRAPH_OWN_CHILDREN
	TPtrArray<T>		m_Children;
#else
	TPtr<T>				m_pChildFirst;		// First child
	TPtr<T>				m_pPrevious;		//	prev sibling
	TPtr<T>				m_pNext;			//	Next sibling
#endif
};



//-------------------------------------------------------
// Main node update called once per frame
//-------------------------------------------------------
template <class T>
void TLGraph::TGraphNode<T>::Update(float Timestep)			
{
	// Process all queued messages first
	ProcessMessageQueue();
}


//-------------------------------------------------------
//	Initialise message - made into virtual func as it's so commonly used
//-------------------------------------------------------
template <class T>
void TLGraph::TGraphNode<T>::Initialise(TPtr<TLMessaging::TMessage>& pMessage)		
{
	//	gr: consider...
	//	GetNodeData().CopyTree( pMessage->GetData() );
}


//-------------------------------------------------------
//	gr: needs to make sure removes self from former parent?
//-------------------------------------------------------
template <class T>	
void TLGraph::TGraphNode<T>::SetParent(TPtr<T>& pNode)
{
	if ( pNode.GetObject() == this )
	{
		TLDebug_Break("Trying to set parent of this to this");
		return;
	}

	m_pParent = pNode;	
}	



//-------------------------------------------------------
//	find a child/sibling matching this node and return our TPtr to it - workaround for non intrusive smart pointers
//-------------------------------------------------------
template <class T>
TPtr<T>& TLGraph::TGraphNode<T>::FindPtr(const TGraphNode<T>* pNode) const
{
	if ( this == pNode )
	{
		TLDebug_Break("Parent should have already caught this pointer and returned a TPtr");
		return TLPtr::GetNullPtr<T>();
	}

#ifdef TLGRAPH_OWN_CHILDREN
	
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		TPtr<T>& pChild = m_Children[c];
		if ( pChild == pNode )
			return pChild;

		pChild->FindPtr( pNode );
	}

#else

	//	if pNode is one of our members return it before more recursive searching
	if ( m_pChildFirst.GetObject() == pNode )
		return m_pChildFirst;

	if ( m_pNext.GetObject() == pNode )
		return m_pNext;

	TPtr<T> pPtr;

	//	search children
	if ( m_pChildFirst )
	{
		pPtr = m_pChildFirst->FindPtr( pNode );
		if ( pPtr )
			return pPtr;
	}

	//	search siblings
	if ( m_pNext )
	{
		pPtr = m_pNext->FindPtr( pNode );
		if ( pPtr )
			return pPtr;
	}

#endif

	//	no match
	return TLPtr::GetNullPtr<T>();
}



//---------------------------------------------------------
//	find a TPtr in the graph that matches the specified value (will use == operator of node type to match)
//---------------------------------------------------------
template <class T>
template<typename MATCHTYPE>
TPtr<T>&	TLGraph::TGraphNode<T>::FindNodeMatch(const MATCHTYPE& Value)		
{	
	if ( *this == Value )
	{
		TLDebug_Break("Parent should have already caught this pointer and returned a TPtr");
		return TLPtr::GetNullPtr<T>();
	}

#ifdef TLGRAPH_OWN_CHILDREN

	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		TPtr<T>& pChild = m_Children[c];
		if ( pChild == Value )
			return pChild;
	
		TPtr<T>& pPtr = pChild->FindNodeMatch( Value );
		if ( pPtr )
			return pPtr;
	}

#else

	//	if pNode is one of our members return it before more recursive searching
	if ( m_pChildFirst == Value )
		return m_pChildFirst;

	if ( m_pNext == Value )
		return m_pNext;

	//	search children
	if ( m_pChildFirst )
	{
		TPtr<T>& pPtr = m_pChildFirst->FindNodeMatch( Value );
		if ( pPtr )
			return pPtr;
	}

	//	search siblings
	if ( m_pNext )
	{
		TPtr<T>& pPtr = m_pNext->FindNodeMatch( Value );
		if ( pPtr )
			return pPtr;
	}

#endif

	//	no match
	return TLPtr::GetNullPtr<T>();
}



//---------------------------------------------------------
//	same as FindNode but only searches children
//	gr: function is a little messy now, but only to allow return of references
//		should be quite a bit faster, even if it's a bit unreadable
//---------------------------------------------------------
template <class T>
template<typename MATCHTYPE>
TPtr<T>&	TLGraph::TGraphNode<T>::FindChildMatch(const MATCHTYPE& Value)		
{
#ifdef TLGRAPH_OWN_CHILDREN

	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		TPtr<T>& pChild = m_Children[c];
		if ( pChild == Value )
			return pChild;

		TPtr<T>& pResult = pChild->FindChildMatch( Value );
		if ( pResult )
			return pResult;
	}

#else

	if ( !m_pChildFirst )
		return TLPtr::GetNullPtr<T>();

	//	if pNode is one of our members return it before more recursive searching
	if ( m_pChildFirst == Value )
		return m_pChildFirst;

	//	check first child's children
	TPtr<T>& pFirstChildFind = m_pChildFirst->FindChild( Value );
	if ( pFirstChildFind )
		return pFirstChildFind;

	//	go through siblings
	TPtr<T> pChild = m_pChildFirst;
	while ( pChild->GetNext() )
	{
		if ( pChild->GetNext() == Value )
			return pChild->GetNext();

		//	check children of next
		TPtr<T>& pChildNextFind = pChild->GetNext()->FindChild( Value );
		if ( pChildNextFind )
			return pChildNextFind;

		//	goto next
		pChild = pChild->GetNext();
	}

#endif

	//	no match
	return TLPtr::GetNullPtr<T>();
}


//-------------------------------------------------------
//
//-------------------------------------------------------
template <class T>
void TLGraph::TGraphNode<T>::UpdateAll(float Timestep)
{
	// Update this
	Update( Timestep );

#ifdef TLGRAPH_OWN_CHILDREN

	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		TPtr<T>& pChild = m_Children[c];
		pChild->UpdateAll( Timestep );
	}

#else

	// Update children
	if(m_pChildFirst.IsValid())
		m_pChildFirst->UpdateAll( Timestep );

	// Update siblings
	if(m_pNext.IsValid())
		m_pNext->UpdateAll( Timestep );

#endif
}



//-------------------------------------------------------
//	gr: add to END of child list
//-------------------------------------------------------
template <class T>
Bool TLGraph::TGraphNode<T>::AddChild(TPtr<T>& pChild,TPtr<T>& pThis)
{
	if ( !CheckIsThis(pThis) )
		return FALSE;

	if ( !pChild )
		return FALSE;

#ifdef TLGRAPH_OWN_CHILDREN

	pChild->SetParent( pThis );
	m_Children.Add( pChild );
	
#else

	if ( m_pChildFirst.IsValid() )
	{
		m_pChildFirst->AddSibling( pChild, m_pChildFirst );
	}
	else
	{
		// Assign as the first child
		SetChildFirst( pChild, pThis );
	}

#endif

	return TRUE;
}


//-------------------------------------------------------
//	gr: add to START of child list
//-------------------------------------------------------
#ifndef TLGRAPH_OWN_CHILDREN
template <class T>
Bool TLGraph::TGraphNode<T>::SetChildFirst(TPtr<T>& pChild,TPtr<T>& pThis)
{
	if ( !CheckIsThis(pThis) )
		return FALSE;

	if ( !pChild )
		return FALSE;

	if ( pChild == m_pChildFirst )
		return FALSE;

	TPtr<T> pOldFirstChild = m_pChildFirst;
	m_pChildFirst = pChild;

	m_pChildFirst->SetPrevious( NULL );
	m_pChildFirst->SetNext( pOldFirstChild );
		
	if ( pOldFirstChild )
		pOldFirstChild->SetPrevious( m_pChildFirst );

	m_pChildFirst->SetParent( pThis );

	return TRUE;
}
#endif


//-------------------------------------------------------
//
//-------------------------------------------------------
template <class T>
Bool TLGraph::TGraphNode<T>::RemoveChild(TPtr<T>& pItem)
{
#ifdef TLGRAPH_OWN_CHILDREN

	s32 ChildIndex = m_Children.FindIndex( pItem );
	if ( ChildIndex == -1 )
		return FALSE;
	
	m_Children.RemoveAt( ChildIndex );

	return TRUE;

#else

	// Recurse through the childs siblings
	if ( m_pChildFirst.IsValid() )
	{
		return m_pChildFirst->RemoveSibling(pItem);
	}

	// No children
	return FALSE;

#endif
}


//-------------------------------------------------------
//
//-------------------------------------------------------
#ifndef TLGRAPH_OWN_CHILDREN
template <class T>
Bool TLGraph::TGraphNode<T>::AddSibling(TPtr<T>& pSibling,TPtr<T>& pThis)
{
	if ( !CheckIsThis(pThis) )
		return FALSE;

	// Recurse throught he linked list until we find the end
	if ( m_pNext.IsValid() )
	{
		return m_pNext->AddSibling( pSibling, m_pNext );
	}
	else
	{
		// Assign the sibling to the list
		SetNext( pSibling );
		pSibling->SetPrevious( pThis );
		pSibling->SetParent( GetParent() );

		return TRUE;
	}
}
#endif


//-------------------------------------------------------
//
//-------------------------------------------------------
#ifndef TLGRAPH_OWN_CHILDREN
template <class T>
Bool TLGraph::TGraphNode<T>::RemoveSibling(TPtr<T>& pNode)
{
	if ( this == pNode.GetObject() )
	{
		// Remove the children
		RemoveChildren();

		// Swap the previous and next pointers on the previous and next nodes
		TPtr<T> pTemp = m_pNext;

		if(m_pPrevious.IsValid())
			m_pPrevious->SetNext(pTemp);
		
		if(m_pNext.IsValid())
			m_pNext->SetPrevious(m_pPrevious);

		// Unassign all ponters
		m_pPrevious = NULL;
		m_pNext = NULL;

		m_pParent = NULL;
		m_pChildFirst = NULL;

		return TRUE;
	}
	else
	{
		//Bool bRemoved = FALSE;

		if(m_pChildFirst.IsValid())
		{
			if(m_pChildFirst->RemoveSibling(pNode))
				return TRUE;
		}

		if(m_pNext.IsValid())
			return m_pNext->RemoveSibling(pNode);

		// Not removed
		return FALSE;
	}
}
#endif


//-------------------------------------------------------
//
//-------------------------------------------------------
template <class T>
void TLGraph::TGraphNode<T>::RemoveChildren()
{
#ifdef TLGRAPH_OWN_CHILDREN

	m_Children.Empty();	//	TPtrArray class NULL's children for us so everything gets released okay

#else

	if(m_pChildFirst.IsValid())
	{
		// Recursively remove the children
		m_pChildFirst->RemoveChildren();

		// remove the main child
		RemoveSibling(m_pChildFirst);
		m_pChildFirst = NULL;
	}

#endif
}

//-------------------------------------------------------
//
//-------------------------------------------------------
#ifndef TLGRAPH_OWN_CHILDREN
template <class T>
void TLGraph::TGraphNode<T>::RemoveSiblings()
{
	if(m_pNext.IsValid())
	{
		// Recursively remove the siblings
		m_pNext->RemoveSiblings();

		// remove the main sibling
		Remove(m_pNext);
		m_pNext = NULL;
	}
}
#endif


//-------------------------------------------------------
//	
//-------------------------------------------------------
template <class T>
Bool TLGraph::TGraphNode<T>::CheckIsThis(TPtr<T>& pThis)
{
	if ( pThis.GetObject() == this )
		return TRUE;

	//	gr: do NOT return the Break() result, always return FALSE
	TLDebug_Break("Pointer expected to be this");

	return FALSE;
}

//-------------------------------------------------------
//	
//-------------------------------------------------------
template<class T>
void TLGraph::TGraphNode<T>::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	TRefRef MessageRef = pMessage->GetMessageRef();

	if(MessageRef == TLCore::InitialiseRef)
	{
		Initialise(pMessage);
	}
	else if(MessageRef == TLCore::ShutdownRef)
	{
		Shutdown();
	}
	else 
	{
		TTempString DebugString("Unhandled message ");
		MessageRef.GetString( DebugString );
		DebugString.Append(", node: ");
		GetNodeRef().GetString( DebugString );
		DebugString.Append(" (");
		GetNodeTypeRef().GetString( DebugString );
		DebugString.Append(")");
		TLDebug_Print( DebugString );
	}
		

	// Super class process message
	TLMessaging::TRelay::ProcessMessage(pMessage);
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
template<class T>
void TLGraph::TGraphNode<T>::GetChildrenBase(TArray<TGraphNodeBase*>& ChildNodes)
{
	TPtrArray<T>& Children = GetChildren();

	for ( u32 c=0;	c<Children.GetSize();	c++ )
	{
		ChildNodes.Add( Children[c].GetObject<TGraphNodeBase>() );
	}
}




template <class T>
SyncBool TLGraph::TGraph<T>::Initialise()
{
	// Create the root node
	m_pRootNode = new T( "Root", TRef() );

	return SyncTrue;
}

template<class T>
void TLGraph::TGraph<T>::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{
	if(refPublisherID == "CORE")
	{
		// Subscribe to the update messages
		if(refChannelID == TLCore::UpdateRef)
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
	}
	
	// Super event channel routine
	TManager::OnEventChannelAdded(refPublisherID, refChannelID);
}


template <class T>
SyncBool TLGraph::TGraph<T>::Update(float fTimeStep)
{
	//	check that the Null ptr is still null
	//	if this is caught it means that somewhere, someone has had a NULL (!IsValid())
	//	Ptr returned from the graph, then assigned it to something anyway (so missing a Pointer check)
#ifdef _DEBUG
	TLPtr::GetNullPtr<T>();
#endif

	//	update graph structure, upadte each node etc
	UpdateGraph( fTimeStep );

	return SyncTrue;
}


template <class T>
SyncBool TLGraph::TGraph<T>::Shutdown()
{
#ifndef TLGRAPH_OWN_CHILDREN
	m_pRootNode->RemoveSibling(m_pRootNode);
#endif

	m_pRootNode = NULL;

	return SyncTrue;
}



//-------------------------------------------------------
//	find node in the graph by ref
//-------------------------------------------------------
template <class T>
TPtr<T>& TLGraph::TGraph<T>::FindNode(const TRef& NodeRef)			
{	
	if ( !NodeRef.IsValid() )
	{
		return TLPtr::GetNullPtr<T>();
	}
	
	//	search the graph list
	TPtr<T>& pIndexNode = m_NodeIndex.FindPtr( NodeRef );
	if ( pIndexNode )
		return pIndexNode;

	//	look in the update queue for new nodes
	TPtr<TGraphUpdateRequest>& pMatchingUpdate = m_UpdateQueue.FindPtr( NodeRef );
	if ( pMatchingUpdate )
		return pMatchingUpdate->GetNode();

	return TLPtr::GetNullPtr<T>();
}


//---------------------------------------------------
//	find an unused ref for a node - returns the ref
//---------------------------------------------------
template <class T>
TRef TLGraph::TGraph<T>::GetFreeNodeRef(TRefRef BaseRef)
{
	TRef TempRef = BaseRef;
	GetFreeNodeRef( TempRef );
	return TempRef;
}


//---------------------------------------------------
//	find an unused ref for a node, modifies the ref provided
//---------------------------------------------------
template <class T>
TRefRef TLGraph::TGraph<T>::GetFreeNodeRef(TRef& Ref)
{
#ifdef _DEBUG
	TLPtr::GetNullPtr<T>();
#endif

	//	keep searching through the graph for this ref whilst a node is found
	while ( FindNode( Ref ).IsValid() )
	{
		//	try next ref
		Ref.Increment();
	}

	//	if it's in the queue, it also needs changing
	while ( m_UpdateQueue.Exists( Ref ) )
	{
		//	try next ref
		Ref.Increment();
	}

	return Ref;
}

	
//---------------------------------------------------------
//	create node and add to the graph. returns ref of new node
//---------------------------------------------------------
template <class T>
TRef TLGraph::TGraph<T>::CreateNode(TRefRef NodeRef,TRefRef TypeRef,TRefRef ParentRef,TPtr<TLMessaging::TMessage> pInitMessage)
{
	TPtr<T> pParent = FindNode( ParentRef );
	TPtr<T> pNode = DoCreateNode( NodeRef, TypeRef, pParent, pInitMessage );

	if ( !pNode )
		return TRef();

	return pNode->GetNodeRef();
}


//---------------------------------------------------------
//	create node and add to the graph
//---------------------------------------------------------
template <class T>
TPtr<T> TLGraph::TGraph<T>::DoCreateNode(TRefRef NodeRef,TRefRef TypeRef,TPtr<T> pParent,TPtr<TLMessaging::TMessage> pInitMessage)
{
	TPtr<T> pNewNode;

	//	Go through the factory list and try to create the specified node type
	for( u32 f=0;	f<m_NodeFactories.GetSize();	f++)
	{
		//	have factory try to create new node
		m_NodeFactories[f]->CreateInstance( pNewNode, NodeRef, TypeRef );

		// If a node was created then return that node
		if ( pNewNode )
			break;
	}

	//	wasn't created by factory, create default node
	if ( !pNewNode )
	{
		pNewNode = new T( NodeRef, TypeRef );
	}

	//	add to graph
	if ( !AddNode( pNewNode, pParent ) )
	{
		pNewNode = NULL;
		return NULL;
	}

	//	make up init message if none provided
	if ( !pInitMessage )
	{
		pInitMessage = new TLMessaging::TMessage( TLCore::InitialiseRef );
	}
	else if ( pInitMessage->GetMessageRef() != TLCore::InitialiseRef )
	{
		TTempString DebugString("Init message for node creation has wrong MessageRef (");
		pInitMessage->GetMessageRef().GetString( DebugString );
		DebugString.Append("), should be TLCore::InitialiseRef. Retry to correct the ref");
		if ( TLDebug_Break( DebugString ) )
			pInitMessage->SetMessageRef( TLCore::InitialiseRef );
	}

	//	send init message
	pNewNode->QueueMessage( pInitMessage );

	//	return ref of new node
	return pNewNode;
}


//---------------------------------------------------------
//	Requests a node to be added to the graph.  The node will be added at a 'safe' time so is not immediate
//---------------------------------------------------------
template <class T>
Bool TLGraph::TGraph<T>::AddNode(TPtr<T> pNode, TPtr<T> pParent)	
{
	// Check to see if the node is already in the queue
	if(IsInQueue(pNode))
	{
		//TODO: Check to see if the node is to be added or removed?
		return FALSE;
	}

	//	not allowed invalid nodes, so make it at least valid
	if ( !pNode->GetNodeRef().IsValid() )
	{
		TRef NewRef = pNode->GetNodeRef();
		NewRef.Increment();
		pNode->SetNodeRef( NewRef );
	}

	//	if node ref is already in use, change it
	TRefRef NodeRef = pNode->GetNodeRef();
	TRef NewRef = GetFreeNodeRef( pNode->GetNodeRef() );

	if ( NewRef != NodeRef )
	{
		TRefRef OldRef = NodeRef;
		
		TTempString ChangedRefString("Node ");
		OldRef.GetString( ChangedRefString );
		ChangedRefString.Append(" changed ref to ");
		NewRef.GetString( ChangedRefString );
		TLDebug_Print( ChangedRefString );

		pNode->SetNodeRef( NewRef );
	}

	// Valid parent node??  Check to ensure the parent is either in the queue or in the scenegraph
	// If not valid then attach to the root node by default (which shoudl *ALWAYS* be valid
	if(pParent.IsValid())
	{
		// In the queue? OK to proceed if so
		if(!IsInQueue(pParent))
		{
			// In the graph heirarchy?? Ok to proceed if so
			if(!IsInGraph(pParent))
				return FALSE;
		}
		else
		{
				//TODO: Check to see if the parent is to be added or removed?  Need to ensure we attach to a valid node
		}
	}
	else
	{
		//	gr: need to handle having no root?
		pParent = m_pRootNode;
	}

	// No - add the node to the queue
	TPtr<TGraphUpdateRequest> pUpdateRequest = new TGraphUpdateRequest(pNode, pParent, FALSE);

	m_UpdateQueue.Add(pUpdateRequest);

	return TRUE;
}

//-----------------------------------------------------
//	Requests a node to be removed from the graph.  The node will be removed at a 'safe' time so is not immediate
//-----------------------------------------------------
template <class T>
Bool TLGraph::TGraph<T>::RemoveNode(TPtr<T> pNode)
{
	if ( !pNode )
	{
		TTempString Debug_String("Attempting to remove node ");
		pNode->GetNodeRef().GetString( Debug_String );
		Debug_String.Append(" that doesn't exist in graph...");
		TLDebug_Print( Debug_String );
		return FALSE;
	}

	// Check to make sure we aren;t trying to remove the root node - only done at shutdown
	if(pNode == m_pRootNode)
	{
		TLDebug_Break("Attempting to remove root node");
		return FALSE;
	}

	// Check to see if the node is already in the queue
	if(IsInQueue(pNode))
	{
		//TODO: Check to see if the node is to be added or removed?
		return FALSE;
	}

	// Valid parent node??  Check to ensure the parent is either in the queue or in the scenegraph
	// If not valid then attach to the root node by default (which shoudl *ALWAYS* be valid
	TPtr<T>& pParent = pNode->GetParent();
	if(pParent.IsValid())
	{
		// In the queue? OK to proceed if so
		if(!IsInQueue(pParent))
		{
			// In the graph heirarchy?? Ok to proceed if so
			if(!IsInGraph(pParent))
				return FALSE;
		}
		else
		{
			//TODO: Check to see if the parent is to be added or removed? Need to ensure it will be added to a valid node
		}
	}
	else
	{
		pParent = m_pRootNode;
	}


	// No - Add the node to the queue
	TPtr<TGraphUpdateRequest> pUpdateRequest = new TGraphUpdateRequest(pNode, pParent, TRUE);

	m_UpdateQueue.Add(pUpdateRequest);

	return TRUE;
}




//-----------------------------------------------------
//	remove pNode's children (but not pNode)
//-----------------------------------------------------
template <class T>
Bool TLGraph::TGraph<T>::RemoveChildren(TPtr<T> pNode)
{
	if ( !pNode )
		return FALSE;

	Bool AnyRemoved = FALSE;

	//	remove each child from the graph
	TPtrArray<T>& Children = pNode->GetChildren();
	for ( u32 c=0;	c<Children.GetSize();	c++ )
	{
		AnyRemoved |= RemoveNode( Children[c] );
	}

	return AnyRemoved;
}


/*
	Checks to see if a node is in the list of nodes to be updated
*/
template <class T>
Bool TLGraph::TGraph<T>::IsInQueue(TPtr<T>& pNode)
{
	for(u32 uIndex = 0; uIndex < m_UpdateQueue.GetSize(); uIndex++)
	{
		TPtr<TGraphUpdateRequest>& pUpdateRequest = m_UpdateQueue.ElementAt(uIndex);

		if(pUpdateRequest->GetNode() == pNode)
			return TRUE;
	}

	// Not in the queue
	return FALSE;
}


template <class T>
void TLGraph::TGraph<T>::UpdateGraph(float fTimeStep)
{
	// Process all queued messages first
	ProcessMessageQueue();

	// Update the graph nodes
	m_pRootNode->UpdateAll(fTimeStep);

	// Update the graph structure with any changes
	TGraph<T>::UpdateGraphStructure();
}

/*
	Adds/removes nodes that have been queued up for update
*/
template <class T>
void TLGraph::TGraph<T>::UpdateGraphStructure()
{
	// Go through the queue and make the changes required
	for(u32 uIndex = 0; uIndex < m_UpdateQueue.GetSize(); uIndex++)
	{
		TPtr<TGraphUpdateRequest> pRequest = m_UpdateQueue.ElementAt(uIndex);

		// Add/Remvoe the node
		if ( pRequest->IsRemoveRequest() )
		{
			DoRemoveNode(pRequest->GetNode(), pRequest->GetNodeParent() );
		}
		else
		{
			DoAddNode(pRequest->GetNode(), pRequest->GetNodeParent() );
		}
	}

	// Empty queue
	m_UpdateQueue.Empty();
}

/*
	Final stage add of the graph node
*/
template <class T>
void TLGraph::TGraph<T>::DoAddNode(TPtr<T>& pNode, TPtr<T>& pParent)
{
	// Final check to ensure the parent node is in the graph
	if(!IsInGraph(pParent))
	{
		// Should not get here...
		TLDebug_Break("Parent isnt in graph(any more?)");
		return;
	}

	pParent->AddChild(pNode,pParent);

	// Tell the node it has been added to the graph
	pNode->OnAdded();

	//	do graph's OnAdded event
	OnNodeAdded( pNode );


}

/*
	Final stage removal of the graph node
*/
template <class T>
void TLGraph::TGraph<T>::DoRemoveNode(TPtr<T>& pNode,TPtr<T>& pParent)
{
	// Last chance to clear anything up
	OnNodeRemoving( pNode );
	pNode->Shutdown();

	Bool bSuccess = FALSE;

	if(pParent.IsValid())
	{
		bSuccess = pParent->RemoveChild(pNode);
	}
	else
	{
		if(m_pRootNode.IsValid())
			bSuccess = m_pRootNode->RemoveChild(pNode);
	}


	if(bSuccess)
	{
		OnNodeRemoved( pNode );
	}
	else
	{
		TTempString Debug_String("Failed to remove node ");
		pNode->GetNodeRef().GetString( Debug_String );
		TLDebug_Print( Debug_String );
	}
}


template <class T>
void TLGraph::TGraph<T>::ProcessMessageFromQueue(TPtr<TLMessaging::TMessage>& pMessage)
{
	TArray<TRef> TargetList;

	Bool bNoMoreTargets = FALSE;

	// Get all target ID's
	do
	{
		TPtr<TBinaryTree> pData = pMessage->GetChild("TARGETID");

		if(pData.IsValid())
		{
			TRef refTargetID;

			if(pData->Read(refTargetID))
				TargetList.Add(refTargetID);
		}
		else
			bNoMoreTargets = TRUE;

	} while(!bNoMoreTargets);

	if(TargetList.GetSize() > 0)
	{
		// Go through and find the target to send the message to
		for(u32 uIndex = 0; uIndex < TargetList.GetSize(); uIndex++)
		{
			TRef refTargetID = TargetList.ElementAt(uIndex);

			TPtr<T> pNode = FindNode(refTargetID);

			if(pNode.IsValid())
			{
				pNode->QueueMessage(pMessage);
			}
			else
			{
				// Not found.  Could bring up an error message here or just ignore...
			}
		}
	}
}



//---------------------------------------------------------
//	find a TPtr in the graph that matches the node - workaround for non intrusive pointers
//---------------------------------------------------------
template <class T>
TPtr<T>& TLGraph::TGraph<T>::FindPtr(const TGraphNode<T>* pNode) const		
{	
	if ( pNode == m_pRootNode.GetObject() )
		return m_pRootNode;

	if ( !m_pRootNode )
	{		
		return TLPtr::GetNullPtr<T>();
	}

	return m_pRootNode->FindPtr(pNode);	
}


//---------------------------------------------------------
//	find a TPtr in the graph that matches the specified value (will use == operator of node type to match)
//---------------------------------------------------------
template<class T>
template<typename MATCHTYPE>
TPtr<T>& TLGraph::TGraph<T>::FindNodeMatch(const MATCHTYPE& Value)		
{	
	if ( !m_pRootNode )
	{
		return TLPtr::GetNullPtr<T>();
	}

	if ( m_pRootNode == Value )
		return m_pRootNode;

	return m_pRootNode->FindNodeMatch( Value );	
}


//---------------------------------------------------------
//	called after node shutdown and after being removed from parent and graph
//---------------------------------------------------------
template<class T>
void TLGraph::TGraph<T>::OnNodeRemoved(TPtr<T>& pNode)
{
	//	remove from index
	s32 NodeIndexIndex = m_NodeIndex.FindIndex( pNode->GetNodeRef() );
	if ( NodeIndexIndex == -1 )
	{
		TLDebug_Break("Error removing node from graph... not found in node index");
	}
	else
	{
		m_NodeIndex.RemoveAt( NodeIndexIndex );
	}

	// Send the removed node notificaiton
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("GRAPHCHANGE");

	if(pMessage)
	{
		PublishMessage(pMessage);
	}
}

//---------------------------------------------------------
//	called after node has been added to the graph and parent
//---------------------------------------------------------
template<class T>
void TLGraph::TGraph<T>::OnNodeAdded(TPtr<T>& pNode)
{
	//	add to index
	m_NodeIndex.Add( pNode );

	// Send the added node notificaiton
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("GRAPHCHANGE");

	if(pMessage)
	{
		PublishMessage(pMessage);
	}
}


//---------------------------------------------------------
//	send message to node
//---------------------------------------------------------
template<class T>
Bool TLGraph::TGraph<T>::SendMessageToNode(TRefRef NodeRef,TPtr<TLMessaging::TMessage>& pMessage)
{
	//	get node to send message to
	TPtr<T>& pNode = FindNode( NodeRef );
	if ( !pNode )
		return FALSE;

	//	queue message
	if ( !pNode->QueueMessage( pMessage ) )
		return FALSE;

	return TRUE;
}

