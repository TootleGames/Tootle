/*------------------------------------------------------
	
	Templated graph and graphnode classes which handle node in a tree - 
	handles the update, queued tree changes, events for nodes

-------------------------------------------------------*/
#pragma once

//#include "TPtrLinkedList.h"
//#include "TPtrTree.h"


//#define DEBUG_PRINT_GRAPH_CHANGES	//	enable "removing node XXX" and "requesting remove node XX" etc prints


//	if enabled we check the parent heirachy on AddNode() - if a parent is being removed we dont ADD the node. 
//	Means more request-list checks - but means we WONT have potentially non-added nodes in the ADD request list
//	if disabled it's blindly added, and adds potentially non-added nodes to the add list. But much less list checks
//#define DONT_ADD_NODE_WHERE_PARENT_IS_BEING_REMOVED


#include "TGraphBase.h"
#include "TLMessaging.h"
#include "TRelay.h"
#include <TootleCore/TEventChannel.h>
#include <TootleCore/TClassFactory.h>
#include <TootleCore/TCoreManager.h>
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
class TLGraph::TGraph : public TLMessaging::TMessageQueue, public TLGraph::TGraphBase
{
	friend class TGraphNode<T>;
	
public:
	TGraph(TRefRef GraphRef) :
		TLGraph::TGraphBase	( GraphRef ),
		m_NodeIndex			( &TLGraph::TPtrRefSort<T>, TGraph_DefaultGrowBy )
	{
	}
	
	//	Node access
	FORCEINLINE TPtr<T>&		GetRootNode()							{	return m_pRootNode; }
	FORCEINLINE TPtrArray<T>&	GetNodeList()							{	return m_NodeIndex;	}
	FORCEINLINE u32				GetNodeCount() const					{	return m_NodeIndex.GetSize();	}
	template<typename MATCHTYPE>
	TPtr<T>&					FindNodeMatch(const MATCHTYPE& Value,Bool CheckRequestQueue=TRUE);	//	find a TPtr in the graph that matches the specified value (will use == operator of node type to match)
	TPtr<T>&					FindNode(TRefRef NodeRef,Bool CheckRequestQueue=TRUE);				//	find a node
	virtual TRef				GetFreeNodeRef(TRefRef BaseRef=TRef());	//	find an unused ref for a node - returns the ref
	TRefRef						GetFreeNodeRef(TRef& Ref);				//	find an unused ref for a node, modifies the ref provided

	// Messaging
	virtual Bool				SendMessageToNode(TRefRef NodeRef,TLMessaging::TMessage& Message);	//	send message to node

	// Graph change requests
	virtual TRef				CreateNode(TRefRef NodeRef,TRefRef TypeRef,TRefRef ParentRef,TLMessaging::TMessage* pInitMessage=NULL,Bool StrictNodeRef=FALSE);		//	create node and add to the graph. returns ref of new node
	FORCEINLINE Bool			MoveNode(TRefRef NodeRef,TRefRef NewParentRef)		{	return MoveNode( FindNode( NodeRef ), NewParentRef.IsValid() ? FindNode( NewParentRef ) : GetRootNode() );	}
	virtual Bool				RemoveNode(TRefRef NodeRef)							{	return RemoveNode( FindNode( NodeRef ) );	}
	FORCEINLINE Bool			RemoveNode(TRef& NodeRef);							//	simple remove node wrapper which invalidates the node ref as well
	virtual Bool				RemoveChildren(TRefRef NodeRef)						{	TPtr<T>& pNode = FindNode( NodeRef )	;	return pNode ? RemoveChildren( *pNode ) : FALSE;	}			//	remove pNode's children (but not pNode)
	Bool						RemoveChildren(T& Node);							//	remove pNode's children (but not pNode)

	//	factory access
	FORCEINLINE Bool			AddFactory(TPtr< TClassFactory<T,FALSE> >& pFactory);	//	add factory to graph

protected:
	virtual SyncBool			Initialise();
	virtual SyncBool			Update(float fTimeStep);
	virtual SyncBool			Shutdown();

	virtual void				UpdateGraph(float fTimeStep);					// Main update of the graph
	void						UpdateGraphStructure();							// Adds/removes nodes that have been queued up for update

public:
	///////////////////////////////////////////////////////////////////////////////////
	// These should be private.  A couple of places need updating before we can 
	// make this change.
	///////////////////////////////////////////////////////////////////////////////////
	TPtr<T>						DoCreateNode(TRef NodeRef,TRefRef TypeRef,TPtr<T> pParent=NULL,TLMessaging::TMessage* pInitMessage=NULL, Bool StrictNodeRef=FALSE);	//	create node and add to the graph. returns ref of new node

	Bool						AddNode(TPtr<T>& pNode,TPtr<T>& pParent);		//	returns the ref of the object. You can't always have the ref we constructed with :)
	FORCEINLINE Bool			AddNode(TPtr<T>& pNode)							{	return AddNode( pNode, m_pRootNode );	}
	FORCEINLINE Bool			AddNode(TPtr<T>& pNode,TRefRef ParentRef)		{	return AddNode( pNode, FindNode( ParentRef ) );	}

//	FORCEINLINE Bool			RemoveNode(const T* pNode)				{	return RemoveNode( FindNode( pNode->GetNodeRef() ) );	}
	Bool						RemoveNode(TPtr<T> pNode);				//	Requests a node to be removed from the graph.  The node will be removed at a 'safe' time so is not immediate

	Bool						MoveNode(TPtr<T>& pNode,TPtr<T>& pParent);		//	give this node a new parent via a request

	///////////////////////////////////////////////////////////////////////////////////
protected:
	template<typename MATCHTYPE>
	FORCEINLINE Bool			IsInGraph(const MATCHTYPE& Value,Bool CheckRequestQueue=TRUE)		{	return FindNodeMatch( Value, CheckRequestQueue ).IsValid();	}
	virtual Bool				IsInGraph(TRefRef NodeRef,Bool CheckRequestQueue=TRUE)				{	return FindNode( NodeRef, CheckRequestQueue ).IsValid();	}
	FORCEINLINE Bool			IsInGraph(T* pNode,Bool CheckRequestQueue=TRUE)						{	return pNode ? FindNode( pNode->GetNodeRef(), CheckRequestQueue ).IsValid() : FALSE;	}
	FORCEINLINE Bool			IsInGraph(TPtr<T>& pNode,Bool CheckRequestQueue=TRUE)				{	return IsInGraph( pNode.GetObjectPointer(), CheckRequestQueue );	}
	FORCEINLINE Bool			IsInRequestQueue(TPtr<T>& pNode) const		{	return IsInRequestQueue( pNode->GetNodeRef() );	}
	FORCEINLINE Bool			IsInRequestQueue(TRefRef NodeRef) const		{	return m_RequestQueue.Exists( NodeRef );	}
	FORCEINLINE Bool			IsInAddQueue(TPtr<T>& pNode) const			{	return IsInAddQueue( pNode->GetNodeRef() );	}
	Bool						IsInAddQueue(TRefRef NodeRef) const;		//	is in the request queue and marked to Add
	FORCEINLINE Bool			IsInRemoveQueue(TPtr<T>& pNode) const		{	return IsInRemoveQueue( pNode->GetNodeRef() );	}
	Bool						IsInRemoveQueue(TRefRef NodeRef) const;		//	is in the request queue and marked to remove

	virtual void				ProcessMessageFromQueue(TLMessaging::TMessage& Message);
	virtual void				OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);
	
	FORCEINLINE TPtr<T>&		FindPtr(const TGraphNode<T>* pNode) const	{	return FindNode( pNode->GetNodeRef() );	}

	//	base graph functions
	virtual TLGraph::TGraphNodeBase*	FindNodeBase(TRefRef NodeRef)		{	TPtr<T>& pNode = FindNode( NodeRef );	return pNode.GetObjectPointer();	}
	virtual TLGraph::TGraphNodeBase*	GetRootNodeBase()					{	TPtr<T>& pNode = m_pRootNode;	return pNode.GetObjectPointer();	}

	//	events
	virtual void				OnNodeAdded(TPtr<T>& pNode,Bool SendAddedMessage);	//	called after node has been added to graph and to parent - publishes an added message
	virtual void				OnNodeMoved(TPtr<T>& pNode,TPtr<T>& pOldParentNode);	//	called after node has been moved to a new parent in the scenegraph - publishes a move message
	virtual void				OnNodeRemoving(TPtr<T>& pNode)		{	}	//	called before node shutdown and before being removed from parent and graph
	virtual void				OnNodeRemoved(TRefRef NodeRef);				//	called after node shutdown and after being removed from parent and graph

private:

	// Graph structure updates
	void						DoAddNode(TPtr<T>& pNode,TPtr<T>& pParent);		// Final stage add of the graph node
	void						DoRemoveNode(TPtr<T>& pNode,TPtr<T>& pParent);	// Final stage removal of the graph node
	void						DoMoveNode(TPtr<T>& pNode,TPtr<T>& pNewParent);	// Final stage move of a graph node

	//	gr: you don;t need another template declaration, by being inside a template class it is templated (it's TGraph<T>::TGraphRequest which is different to TGraph<X>::TGraphRequest)
	//template <class T>
	class TGraphUpdateRequest
	{
	public:
		explicit TGraphUpdateRequest(TPtr<T>& pNode,TPtr<T>& pParent, TRefRef Command) :
			m_pNode			( pNode ),
			m_pNodeParent	( pParent ),
			m_Command		( Command )
		{
		}

		FORCEINLINE TPtr<T>&	GetNode()					{	return m_pNode;	}
		FORCEINLINE TPtr<T>&	GetNodeParent()				{	return m_pNodeParent;	}
		FORCEINLINE TRefRef		GetCommand() const			{	return m_Command;	}
		
		FORCEINLINE Bool		IsAddRequest() const		{	return m_Command == STRef3(A,d,d);	}
		FORCEINLINE Bool		IsRemoveRequest() const		{	return m_Command == STRef(R,e,m,o,v);	}
		FORCEINLINE Bool		IsMoveRequest() const		{	return m_Command == STRef4(M,o,v,e);	}

		FORCEINLINE Bool		operator==(TRefRef NodeRef) const		{	return m_pNode == NodeRef;	}

	protected:
		TPtr<T>		m_pNode;			//	Node to update
		TPtr<T>		m_pNodeParent;		//	Node parent
		TRef		m_Command;			//	like an enum - "Add" "Remove" "Move"
	};

private:
	TPtr<T>								m_pRootNode;			//	The root of the graph
	TPtrArray<TGraphUpdateRequest>		m_RequestQueue;			//	List of objects to add/remove from the graph
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
class TLGraph::TGraphNode : public TLMessaging::TEventChannelInterface, public TLMessaging::TMessageQueue, public TLGraph::TGraphNodeBase
{
	friend class TGraph<T>;
public:
	TGraphNode(TRefRef NodeRef,TRefRef NodeTypeRef) : TGraphNodeBase ( NodeRef, NodeTypeRef )	{}
	virtual ~TGraphNode()
	{
		//	gr: check has gone through shutdown?
		// Ensure this node isn't in the linked list any longer
		//Remove(this);
	}

	// Parent manipulation
	FORCEINLINE TPtr<T>&			GetParent()							{	return m_pParent;	}
	FORCEINLINE const TPtr<T>&		GetParent() const					{	return m_pParent;	}
	FORCEINLINE Bool				HasParent() const					{	return m_pParent.IsValid();	}
	virtual const TGraphNodeBase*	GetParentBase() const				{	return m_pParent;	}	//	gr: automaticcly casts down

	FORCEINLINE TRef				GetParentRef()						{	return (m_pParent.IsValid() ? m_pParent->GetNodeRef() : TRef());	}


	// Child manipulation
	FORCEINLINE Bool				HasChildren() const					{	return (m_Children.GetSize() > 0);	}
	FORCEINLINE TPtrArray<T>&		GetChildren()						{	return m_Children;	}
	FORCEINLINE const TPtrArray<T>&	GetChildren() const					{	return m_Children;	}
	virtual void					GetChildrenBase(TArray<TGraphNodeBase*>& ChildNodes);
	template<typename MATCHTYPE>
	TPtr<T>&						FindChildMatch(const MATCHTYPE& Value);				//	find a TPtr in the graph that matches the specified value (will use == operator of node type to match)
	FORCEINLINE TPtr<T>&			FindChild(const TRef& NodeRef)						{	return FindChildMatch(NodeRef);	}

	FORCEINLINE Bool				operator==(const TPtr<TGraphNode<T> >& pNode) const	{	return this == pNode.GetObjectPointer();	}
	FORCEINLINE Bool				operator==(const TGraphNode<T>& Node) const			{	return this == (&Node);	}
	FORCEINLINE Bool				operator==(TRefRef NodeRef) const					{	return GetNodeRef() == NodeRef;	}

protected:
	virtual void 					Update(float Timestep);						// Main node update called once per frame
	virtual void					UpdateAll(float Timestep);					//	update tree: update self, and children and siblings
	virtual void					Shutdown();									// Shutdown routine	- called before being removed form the graph. Base code sends out a shutdown message to our subscribers
	virtual void					GetShutdownMessageData(TLMessaging::TMessage& ShutdownMessage)	{	}	//	add additional data to the shutdown message

	virtual void					ProcessMessage(TLMessaging::TMessage& Message);
	virtual void					ProcessMessageFromQueue(TLMessaging::TMessage& Message);

	virtual void					OnAdded()									{}	// Added routine - called once the node has been added to the graph
	virtual void					OnMoved(const TPtr<T>& pOldParentNode)		{}	// Moved routine - called after the node has been moved (so this->GetParent is the new parent)
	virtual void					OnChildMovedFrom(TPtr<T>& pOldChild)		{}	//	called to a parent when one of it's direct children has moved to somewhere else (pOldChild->GetParent is it's new parent)
	virtual void					OnChildMovedTo(TPtr<T>& pNewChild,const TPtr<T>& pOldParentNode)	{}	//	called to the new parent of a child after it's been moved. note, this AND OnChildAdded is called, so only use this for very specific cases
	virtual void					OnChildAdded(TPtr<T>& pNewChild)			{}	//	called to the new parent of a child after it's been added to the graph

	// Sibling manipulation
	template<typename MATCHTYPE>
	FORCEINLINE TPtr<T>&			FindNodeMatch(const MATCHTYPE& Value);		//	find a TPtr in the graph that matches the specified value (will use == operator of node type to match)
	FORCEINLINE TPtr<T>&			FindNode(const TRef& NodeRef)				{	return FindNodeMatch( NodeRef );	}
	template<typename MATCHTYPE>
	FORCEINLINE Bool				IsInGraph(const MATCHTYPE& Value)			{	return FindNodeMatch( Value ).IsValid();	}

	// Parent manipulation
	virtual void					SetParent(TPtr<T>& pNode);					//	gr: needs to make sure removes self from former parent?
	TPtr<T>&						FindPtr(const TGraphNode<T>* pNode) const;	//	find a child/sibling matching this node and return our TPtr to it - workaround for non intrusive smart pointers

	Bool							CheckIsThis(TPtr<T>& pThis);				//	check the node pointer is actually this, if not throws up a DebugBreak

private:
	Bool							AddChild(TPtr<T>& pChild,TPtr<T>& pThis);	//	gr: add to END of child list
	FORCEINLINE Bool				RemoveChild(TPtr<T>& pNode)					{	return m_Children.Remove( pNode );	}
	FORCEINLINE void				RemoveChildren()							{	m_Children.Empty();	}	//	TPtrArray deallocs and cleans up for us

	//	accessor to this derived type so we can call functions on the objects (ie. on TRenderNode) without the need for a virtual, 
	//	compiler can inline and optimise this much more easily. USE WHENEVER POSSIBLE! (see IsEnabled for example)
	T&								This()										{	return *static_cast<T*>( this );	}		
	const T&						This() const								{	return *static_cast<const T*>( this );	}

	void							ProcessSubscribeOrPublishTo(TRefRef SubOrPub,TBinaryTree& PubSubData);	//	handle publishto/subscribeto message data 

private:
	TPtr<T>							m_pParent;			// Parent item
	TPtrArray<T>					m_Children;
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
// Shutdown routine	- called before being removed form the graph. Base code sends out a shutdown message to our subscribers
//-------------------------------------------------------
template <class T>
void TLGraph::TGraphNode<T>::Shutdown()
{
	if ( HasSubscribers() )
	{
		//	this is "OnShutdown" to differentiate from the "Shutdown" COMMAND for the core (and maybe future "Shutdown" commands we might want to send to nodes)
		TLMessaging::TMessage ShutdownMessage( "OnShutdown", GetNodeRef() );
		ShutdownMessage.ExportData("Type", GetNodeTypeRef() );

		//	add node-type-specific shutdown data
		GetShutdownMessageData( ShutdownMessage );

		PublishMessage( ShutdownMessage );
	}
}

//-------------------------------------------------------
//	gr: needs to make sure removes self from former parent?
//-------------------------------------------------------
template <class T>	
void TLGraph::TGraphNode<T>::SetParent(TPtr<T>& pNode)
{
	if ( pNode.GetObjectPointer() == this )
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

	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		TPtr<T>& pChild = m_Children[c];
		if ( pChild == pNode )
			return pChild;

		pChild->FindPtr( pNode );
	}

	//	no match
	return TLPtr::GetNullPtr<T>();
}



//---------------------------------------------------------
//	find a TPtr in the graph that matches the specified value (will use == operator of node type to match)
//---------------------------------------------------------
template <class T>
template<typename MATCHTYPE>
TPtr<T>& TLGraph::TGraphNode<T>::FindNodeMatch(const MATCHTYPE& Value)		
{	
	if ( *this == Value )
	{
		TLDebug_Break("Parent should have already caught this pointer and returned a TPtr");
		return TLPtr::GetNullPtr<T>();
	}

	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		TPtr<T>& pChild = m_Children[c];
		if ( pChild == Value )
			return pChild;
	
		TPtr<T>& pPtr = pChild->FindNodeMatch( Value );
		if ( pPtr )
			return pPtr;
	}


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
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		TPtr<T>& pChild = m_Children[c];
		if ( pChild == Value )
			return pChild;

		TPtr<T>& pResult = pChild->FindChildMatch( Value );
		if ( pResult )
			return pResult;
	}

	//	no match
	return TLPtr::GetNullPtr<T>();
}


//-------------------------------------------------------
//
//-------------------------------------------------------
template <class T>
void TLGraph::TGraphNode<T>::UpdateAll(float Timestep)
{
	//	call the IsEnabled() function on T. Saves using virtuals :)
	if ( !This().IsEnabled() )
	{
		//	still process messages in the queue though - eg. enable messages
		ProcessMessageQueue();
		return;
	}

	// Update this
	Update( Timestep );

	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		T& Child = *(m_Children[c]);
		Child.UpdateAll( Timestep );
	}
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
	{
		TLDebug_Break("child expected - should have alreayd been checked");
		return FALSE;
	}

	pChild->SetParent( pThis );
	m_Children.Add( pChild );
	
	return TRUE;
}


//-------------------------------------------------------
//	
//-------------------------------------------------------
template <class T>
Bool TLGraph::TGraphNode<T>::CheckIsThis(TPtr<T>& pThis)
{
	if ( pThis.GetObjectPointer() == this )
		return TRUE;

	//	gr: do NOT return the Break() result, always return FALSE
	TLDebug_Break("Pointer expected to be this");

	return FALSE;
}

//-------------------------------------------------------
//	
//-------------------------------------------------------
template<class T>
void TLGraph::TGraphNode<T>::ProcessMessage(TLMessaging::TMessage& Message)
{
	TRefRef MessageRef = Message.GetMessageRef();

	switch ( MessageRef.GetData() )
	{
		//	Initialise - 
		case TRef_Static(I,n,i,t,i):	//	TLCore::InitialiseRef:
		{
			//	first read properties (eg. Values to base creation of stuff from)
			SetProperty(Message);

			//	then Initialise (ie. using those values set in SetProperty)
			Initialise(Message);

			//	then store any data that hasn't been read in the node data (eg. bits of game specific data not handled by generic nodes, but used later)
			GetNodeData().AddUnreadChildren( Message, TRUE );
			return;
		}

		//	gr: do we use this? why would something shutdown a node via a message and not remove it?
		case TRef_Static(S,h,u,t,d):	//	TLCore::ShutdownRef:
		{
			Shutdown();
			return;
		}

		// Reflection for setting property data on a node via the messaging system
		case TRef_Static(S,e,t,P,r):	//	TLCore::SetPropertyRef:
		{
			SetProperty(Message);

			//	gr: now store unread data in the node - this is NOT in the base code as we want to only add data that has NOT been read
			//		ie. game specific data, so this order is explicit to save CPU work
			GetNodeData().AddUnreadChildren( Message, TRUE );
			return;
		}

		// Reflection for getting property data from a node via the messaging system
		case TRef_Static(G,e,t,P,r):	//	TLCore::GetPropertyRef:
		{
			TRef Manager;
			Message.ImportData(TLCore::ManagerRef, Manager);

			// Valid manager?  If so we can send a response
			if(Manager.IsValid())
			{
				TLMessaging::TMessage Response(TLCore::PropertyRef, GetNodeRef());

				// generate a response with the property information requested
				GetProperty(Message, Response);

				// Now send the response message 
				TRefRef Sender = Message.GetSenderRef();

				TLCore::g_pCoreManager->SendMessageTo(Sender, Manager, Response);
			}
			return;
		}

		//	publish or subscribe to a node/manager
		case TRef_Static(P,u,b,T,o):
		case TRef_Static(S,u,b,T,o):
		{
			//	subscribe to managers and nodes via the data inside
			TPtrArray<TBinaryTree>& MessageChildren = Message.GetChildren();
			for ( u32 i=0;	i<MessageChildren.GetSize();	i++ )
			{
				TBinaryTree& Data = *MessageChildren[i];
				ProcessSubscribeOrPublishTo( MessageRef, Data ); 
			}
			return;
		}

		default:
		#ifdef _DEBUG
		{
			//	gr: this is a bit expensive. I'm sending small messages around now when car control changes. 
			//	this gets to the physics node, which doesnt do anything with it - which is fine - but this then prints out and game slows to a crawl
			/*
			TTempString DebugString("Unhandled message ");
			MessageRef.GetString( DebugString );
			DebugString.Append(", node: ");
			GetNodeRef().GetString( DebugString );
			DebugString.Append(" (");
			GetNodeTypeRef().GetString( DebugString );
			DebugString.Append(")");
			TLDebug_Print( DebugString );
			*/
		}
		#endif
		break;
	}
}


//---------------------------------------------------------
//	handle publishto/subscribeto message data 
//---------------------------------------------------------
template<class T>
void TLGraph::TGraphNode<T>::ProcessSubscribeOrPublishTo(TRefRef SubOrPub,TBinaryTree& PubSubData)
{
	//	gr: the way ive done the code means this kinda only works for publisher/subscribers
	//		we can simplify it a bit if we merge the publisher and subscriber classes
	TLMessaging::TPublisher* pOtherPublisher = NULL;
	TLMessaging::TSubscriber* pOtherSubscriber = NULL;

	//	subscribe to a "manager"
	if ( PubSubData.GetDataRef() == TRef_Static(M,a,n,a,g) )
	{
		TRef ManagerRef;
		PubSubData.ResetReadPos();
		if ( !PubSubData.Read( ManagerRef ) )
			return;
		
		//	get manager
		TLCore::TManager* pManager = TLCore::g_pCoreManager->GetManager( ManagerRef );
		pOtherPublisher = pManager;
		pOtherSubscriber = pManager;
	}
	else if ( PubSubData.GetDataRef() == TRef_Static4(N,o,d,e) )
	{
		TRef NodeRef;
		PubSubData.ResetReadPos();
		if ( !PubSubData.Read( NodeRef ) )
			return;

		//	work out where to find this node
		//	todo use the graph this is in when we add a ptr back to the graph
		//TLGraph::TGraphBase* pGraph = this;
		TLGraph::TGraphBase* pGraph = NULL;
		TRef GraphRef;
		if ( PubSubData.ImportData( TRef_Static(G,r,a,p,h), GraphRef ) )
			pGraph = TLCore::g_pCoreManager->GetManager<TLGraph::TGraphBase>( GraphRef );

		//	missing graph?
		if ( !pGraph )
		{
			TLDebug_Break("Failed to find graph to subscribe/publish to node of");
			return;
		}

		//	get node
		TLGraph::TGraphNodeBase* pNode = pGraph->FindNodeBase( NodeRef );
		pOtherPublisher = pNode;
		pOtherSubscriber = pNode;
	}

	//	failed to get publisher/subscriber
	if ( !pOtherPublisher )
		return;

	//	subscribe/publish to this object
	if ( SubOrPub == TRef_Static(S,u,b,T,o) )
	{
		this->SubscribeTo( pOtherPublisher );
	}
	else if ( SubOrPub == TRef_Static(P,u,b,T,o) )
	{
		pOtherSubscriber->SubscribeTo( this );
	}
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
		ChildNodes.Add( Children[c].GetObjectPointer() );
	}
}

//---------------------------------------------------------
//	
//---------------------------------------------------------
template<class T>
void TLGraph::TGraphNode<T>::ProcessMessageFromQueue(TLMessaging::TMessage& Message)	
{	
	Message.ResetReadPos();
	ProcessMessage(Message);	
}



template <class T>
SyncBool TLGraph::TGraph<T>::Initialise()
{
	if(!m_pRootNode)
	{
		// Create the root node
		m_pRootNode = new T( "Root", TRef() );
		
		//	call notifications for root node - 
		//	gr: note - root node has no Init message...
		OnNodeAdded( m_pRootNode, TRUE );
		m_pRootNode->OnAdded();
	}

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


//-------------------------------------------------------
//	dealloc root node
//-------------------------------------------------------
template <class T>
SyncBool TLGraph::TGraph<T>::Shutdown()
{
	m_pRootNode = NULL;

	return SyncTrue;
}



//-------------------------------------------------------
//	find node in the graph by ref
//-------------------------------------------------------
template <class T>
TPtr<T>& TLGraph::TGraph<T>::FindNode(TRefRef NodeRef,Bool CheckRequestQueue)			
{	
	//	invalid ref is not gonna be in the graph
	if ( !NodeRef.IsValid() )
		return TLPtr::GetNullPtr<T>();
	
	//	search the graph list
	TPtr<T>& pIndexNode = m_NodeIndex.FindPtr( NodeRef );
	if ( pIndexNode )
		return pIndexNode;

	//	look in the update queue for new nodes (will be doing redundant checks for the nodes being removed, but never mind
	if ( CheckRequestQueue )
	{
		TPtr<TGraphUpdateRequest>& pMatchingUpdate = m_RequestQueue.FindPtr( NodeRef );
		if ( pMatchingUpdate )
			return pMatchingUpdate->GetNode();
	}

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
		//	gr: call the GetNullPtr func to detect Non-NULL null TPtr's
		TLPtr::GetNullPtr<T>();
#endif

	//	gr: this can be sped up by finding the existing index in the node index array
	//	and then keep incrementing and check the NEXT index in the array - 
	//	the array is sorted so the next one along is either the Increment() or it's 
	//	something else. We can keep incrementing the index and ref until we reach the 
	//	end of the array or something similar. We don't need to do a whole binary chop
	//	for a match each time

	//	keep searching through the graph for this ref whilst a node is found
	while ( FindNode( Ref ).IsValid() )
	{
#ifdef _DEBUG
		//	gr: call the GetNullPtr func to detect Non-NULL null TPtr's
		TLPtr::GetNullPtr<T>();
#endif

		//	try next ref
		Ref.Increment();
	}

	//	gr: this is slower, and NOT sorted, so cannot be sped up... but the array
	//		should be much smaller...
	//	if it's in the queue, it also needs changing
	while ( m_RequestQueue.Exists( Ref ) )
	{
#ifdef _DEBUG
		//	gr: call the GetNullPtr func to detect Non-NULL null TPtr's
		TLPtr::GetNullPtr<T>();
#endif

		//	try next ref
		Ref.Increment();
	}

	return Ref;
}

	
//---------------------------------------------------------
//	create node and add to the graph. returns ref of new node
//---------------------------------------------------------
template <class T>
TRef TLGraph::TGraph<T>::CreateNode(TRefRef NodeRef,TRefRef TypeRef,TRefRef ParentRef,TLMessaging::TMessage* pInitMessage,Bool StrictNodeRef)
{
	TPtr<T>& pParent = FindNode( ParentRef );
	TPtr<T> pNode = DoCreateNode( NodeRef, TypeRef, pParent ? pParent : m_pRootNode, pInitMessage, StrictNodeRef );

	if ( !pNode )
		return TRef();

	return pNode->GetNodeRef();
}


//---------------------------------------------------------
//	create node and add to the graph
//---------------------------------------------------------
template <class T>
TPtr<T> TLGraph::TGraph<T>::DoCreateNode(TRef NodeRef,TRefRef TypeRef,TPtr<T> pParent,TLMessaging::TMessage* pInitMessage,Bool StrictNodeRef)
{
	//	counter is only 1? did you try and remove a c-pointer? this?
	//	must be at least 2 because the param is NOT a reference
	if ( pParent && pParent.GetRefCount() == 1 )
	{
		if ( !TLDebug_Break("DoCreateNode's parent with a new TPtr... called with a c-pointer and not a real TPtr or TRef?") )
			return NULL;
	}


	TPtr<T> pNewNode;

	//	if we don't have strict node refs, make sure this node ref isn't already in use
	if ( !StrictNodeRef )
	{
		TRef OldRef = NodeRef;

		//	not strict, so allow an invalid node ref, and generate a valid one
		if ( !NodeRef.IsValid() )
			NodeRef.Set("Node");

		NodeRef = GetFreeNodeRef( NodeRef );
	
		if ( OldRef != NodeRef )
		{
	#ifdef _DEBUG
			//	gr: keep string generation still so we can break point and get these refs readable
			TTempString ChangedRefString("Node ");
			OldRef.GetString( ChangedRefString );
			ChangedRefString.Append(" changed ref to ");
			NodeRef.GetString( ChangedRefString );
			//	gr: removed as it's printing out a bit much atm
			//TLDebug_Print( ChangedRefString );
	#endif
		}
	}

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
	//	substitute parent for root if no parent specified
	if ( !AddNode( pNewNode, pParent ? pParent : m_pRootNode ) )
	{
		pNewNode = NULL;
		return NULL;
	}

	//	make up init message if none provided - this is to trigger the Initialise(). even if we have no data...
	//	maybe we shouldn't? just have to remember not all nodes will get an Initialise() message in that case...
	if ( !pInitMessage )
	{
		TLMessaging::TMessage Message( TLCore::InitialiseRef );

		//	send init message
		pNewNode->QueueMessage( Message );
	}
	else
	{
		if ( pInitMessage->GetMessageRef() != TLCore::InitialiseRef )
		{
			TTempString DebugString("Init message for node creation has wrong MessageRef (");
			pInitMessage->GetMessageRef().GetString( DebugString );
			DebugString.Append("), should be TLCore::InitialiseRef. Retry to correct the ref");
			
			
			if ( TLDebug_Break( DebugString ) )
				pInitMessage->SetMessageRef( TLCore::InitialiseRef );
		}

		//	send init message
		pNewNode->QueueMessage( *pInitMessage );

	}

	//	return ref of new node
	return pNewNode;
}


//---------------------------------------------------------
//	Requests a node to be added to the graph.  The node will be added at a 'safe' time so is not immediate
//---------------------------------------------------------
template <class T>
Bool TLGraph::TGraph<T>::AddNode(TPtr<T>& pNode,TPtr<T>& pParent)	
{
	//	gr: code has changed; if Parent should be specified. 
	//	There is a version of AddNode() that does not take a param and instead passes the root node as the parent
	if ( !pParent )
	{
		TLDebug_Break("Parent expected");

		//	gr: call again but using the root node
		if ( m_pRootNode )
			return AddNode( pNode, m_pRootNode );
		else
			return FALSE;
	}

	if ( !pNode )
	{
		TLDebug_Break("Node expected");
		return FALSE;
	}

	if ( !pNode->GetNodeRef().IsValid() )
	{
		TTempString Debug_String("Tried to add node without Ref to graph ");
		GetGraphRef().GetString( Debug_String );
		TLDebug_Break( Debug_String );
		return FALSE;
	}

	//	gr: AddNode no longer changes no refs. CreateNode does that now
	if ( FindNode( pNode->GetNodeRef() ) )
	{
		TTempString Debug_String("Node with ref ");
		pNode->GetNodeRef().GetString( Debug_String );
		Debug_String.Append(" already exists in graph ");
		GetGraphRef().GetString( Debug_String );
		TLDebug_Break( Debug_String );
		return FALSE;
	}

	//	gr: FindNode searches the request queue so this is redundant 
	//	- kept in for debug just in case this changes or isn't right
#ifdef _DEBUG
	// Check to see if the node is already in the queue
	if ( IsInRequestQueue( pNode ) )
	{
		TLDebug_Break("Matching node ref unexpectedly found in request queue, should have been found in FindNode()");
		return FALSE;
	}
#endif

#ifdef DONT_ADD_NODE_WHERE_PARENT_IS_BEING_REMOVED
	Bool CheckParentIsInGraph = TRUE;

	//	check UP the tree to see if a parent is going to be removed (not just parent, but parent's parent etc)
	T* pCurrentParent = pParent;
	while ( pCurrentParent )
	{
		//	check parent's validity
		TPtr<TGraphUpdateRequest>& pRequest = m_RequestQueue.FindPtr( pCurrentParent->GetNodeRef() );
		if ( pRequest )
		{
			//	check to see if the parent is in the to-remove queue? abort add if it is
			if ( pRequest->IsRemoveRequest() )
			{
	#ifdef _DEBUG
				TTempString Debug_String("Trying to add node ");
				pNode->GetNodeRef().GetString( Debug_String );
				Debug_String.Append(" to parent node ");
				pParent->GetNodeRef().GetString( Debug_String );
				Debug_String.Append(" which is in the remove queue. Aborting add.");
				TLDebug_Warning( Debug_String );
	#endif
				return FALSE;
			}
			else
			{
				//	parent is about to be added, so treat as okay
				CheckParentIsInGraph = FALSE;
				break;
			}
		}

		//	try next parent
		pCurrentParent = pCurrentParent->GetParent();
	}

	//	there is no add/remove request for a parent, so check it's in the graph
	if ( CheckParentIsInGraph && !IsInGraph( pParent, FALSE ) )
	{
#ifdef _DEBUG
		TTempString Debug_String("Trying to add node ");
		pNode->GetNodeRef().GetString( Debug_String );
		Debug_String.Append(" to parent node ");
		pParent->GetNodeRef().GetString( Debug_String );
		Debug_String.Append(" which is not in the graph. Aborting add.");
		TLDebug_Break( Debug_String );
#endif
		return FALSE;
	}
#endif

	//	add the node to the requet queue
	m_RequestQueue.AddNewPtr( new TGraphUpdateRequest(pNode, pParent, STRef3(A,d,d) ) );

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
		TLDebug_Print("Attempting to remove NULL node");
		return FALSE;
	}

	// Check to make sure we aren;t trying to remove the root node - only done at shutdown
	if(pNode == m_pRootNode)
	{
		TLDebug_Break("Attempting to remove root node");
		return FALSE;
	}

	//	counter is only 1? did you try and remove a c-pointer? this?
	//	must be at least 2 because the param is NOT a reference
	if ( pNode.GetRefCount() == 1 )
	{
		if ( !TLDebug_Break("RemoveNode with a new TPtr... called with a c-pointer and not a real TPtr or TRef?") )
			return FALSE;
	}

	// Check to see if the node is already in the queue
	TPtr<TGraphUpdateRequest>& pRequest = m_RequestQueue.FindPtr( pNode->GetNodeRef() );
	if ( pRequest )
	{
#ifdef _DEBUG
		TTempString Debug_String("Attempting to remove node ");
		pNode->GetNodeRef().GetString( Debug_String );

		if ( pRequest->IsAddRequest() )
		{
			Debug_String.Append(" which is in the graph ADD queue");
			TLDebug_Warning( Debug_String );
		}
		else
		{
			Debug_String.Append(" which is already in the graph REMOVE queue");
			TLDebug_Warning( Debug_String );
		}
#endif

		return FALSE;
	}

	//	if the parent is being removed, save some time/space etc by not adding to the queue
	TPtr<T>& pParent = pNode->GetParent();
	TPtr<TGraphUpdateRequest>& pParentRequest = m_RequestQueue.FindPtr( pParent->GetNodeRef() );

	if ( pParentRequest )
	{
		if ( pParentRequest->IsRemoveRequest() )
		{
			TLDebug_Print("Not adding node to remove queue as parent is already queued for removal.");
			return TRUE;
		}
		else
		{
			//	parent is in the ADD queue - work out how to handle this when we come to it..
			//	just remove from parent and shutdown? or do remove routines too?
			TLDebug_Break("Removing node from graph, but parent is being added. Need to handle this properly - NODE NOT REMOVED");
			return FALSE;
		}
	}
	else if ( !IsInGraph( pParent, FALSE ) )
	{
#ifdef _DEBUG
		TTempString Debug_String("Attempting to remove node ");
		pNode->GetNodeRef().GetString( Debug_String );
		Debug_String.Append(" from graph ");
		GetGraphRef().GetString( Debug_String );
		Debug_String.Append(" but parent is not in graph (maybe already removed)" );
		TLDebug_Warning(Debug_String);
#endif
		
		//	gr: added shutdown routine for node so it's still cleaned up, shouldnt be any harm doing this
		pNode->Shutdown();
		return TRUE;
	}

#if defined(_DEBUG) && defined(DEBUG_PRINT_GRAPH_CHANGES)
	TTempString Debug_String("Requesting remove node ");
	pNode->GetNodeRef().GetString( Debug_String );
	TLDebug_Print(Debug_String);
#endif
	
	// No - Add the node to the queue
	m_RequestQueue.AddNewPtr( new TGraphUpdateRequest(pNode, pParent, STRef(R,e,m,o,v) ) );

	return TRUE;
}


//-----------------------------------------------------
//	simple remove node wrapper which invalidates the node ref as well
//-----------------------------------------------------
template <class T>
Bool TLGraph::TGraph<T>::RemoveNode(TRef& NodeRef)				
{	
	if ( !NodeRef.IsValid() )	
		return FALSE;	
	
	//	get node and remove it
	TPtr<T>& pNode = FindNode( NodeRef );
	Bool Result = RemoveNode( pNode );
	
	//	node is now invalid - so invalidate ref
	NodeRef.SetInvalid();

	return Result;	
}


//-----------------------------------------------------
//	remove pNode's children (but not pNode)
//-----------------------------------------------------
template <class T>
Bool TLGraph::TGraph<T>::RemoveChildren(T& Node)
{
	/*
	//	counter is only 1? did you try and remove a c-pointer? this?
	//	must be at least 2 because the param is NOT a reference
	if ( pNode.GetRefCount() == 1 )
	{
		if ( !TLDebug_Break("RemoveChildren called with a new TPtr... called with a c-pointer and not a real TPtr or TRef?") )
			return NULL;
	}
*/
	Bool AnyRemoved = FALSE;

	//	remove each child from the graph
	TPtrArray<T>& Children = Node.GetChildren();
	for ( u32 c=0;	c<Children.GetSize();	c++ )
	{
		AnyRemoved |= RemoveNode( Children[c] );
	}

	return AnyRemoved;
}



//-------------------------------------------------------------
//	give this node a new parent via a request
//-------------------------------------------------------------
template <class T>
Bool TLGraph::TGraph<T>::MoveNode(TPtr<T>& pNode,TPtr<T>& pParent)
{
	if ( !pNode || !pParent )
	{
		TLDebug_Break("Node or parent expected");
		return FALSE;
	}

	//	check theyre not the same node
	if ( pNode->GetNodeRef() == pParent->GetNodeRef() )
	{
		TLDebug_Break("Attempting to move a node to be under itself");
		return FALSE;
	}

	//	this is already the node's parent
	if ( pNode->GetParent() == pParent )
		return FALSE;

	//	create new request
	m_RequestQueue.AddNewPtr( new TGraphUpdateRequest( pNode, pParent, STRef4(M,o,v,e) ) );

	return TRUE;
}


//-------------------------------------------------------------
//	find a request for a node and see if it's an ADD request
//-------------------------------------------------------------
template <class T>
Bool TLGraph::TGraph<T>::IsInAddQueue(TRefRef NodeRef) const
{
	//	get request
	TPtr<TGraphUpdateRequest>& pUpdateRequest = m_RequestQueue.FindPtr( NodeRef );
	if ( !pUpdateRequest )
		return FALSE;

	return pUpdateRequest->IsAddRequest();
}


//-------------------------------------------------------------
//	find a request for a node and see if it's a REMOVE request
//-------------------------------------------------------------
template <class T>
Bool TLGraph::TGraph<T>::IsInRemoveQueue(TRefRef NodeRef) const
{
	//	get request
	TPtr<TGraphUpdateRequest>& pUpdateRequest = m_RequestQueue.FindPtr( NodeRef );
	if ( !pUpdateRequest )
		return FALSE;

	return pUpdateRequest->IsRemoveRequest();
}


//----------------------------------------------------------------
//	
//----------------------------------------------------------------
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

//----------------------------------------------------------------
//	Adds/removes nodes that have been queued up for update
//----------------------------------------------------------------
template <class T>
void TLGraph::TGraph<T>::UpdateGraphStructure()
{
	// Go through the queue and make the changes required
	for(u32 uIndex = 0; uIndex < m_RequestQueue.GetSize(); uIndex++)
	{
		TGraphUpdateRequest& Request = *m_RequestQueue.ElementAt(uIndex);
		TPtr<T>& pRequestNode = Request.GetNode();
		if ( !pRequestNode )
		{
			TLDebug_Break("Node expected");
			continue;
		}

		switch ( Request.GetCommand().GetData() )
		{
			case STRef(R,e,m,o,v):
				DoRemoveNode( pRequestNode, Request.GetNodeParent() );
				break;

			case STRef3(A,d,d):
				DoAddNode( pRequestNode, Request.GetNodeParent() );
				break;

			case STRef4(M,o,v,e):
				DoMoveNode( pRequestNode, Request.GetNodeParent() );
				break;

			default:
				TLDebug_Break("Unknown request queue command");
				break;
		}
	}

	// Empty queue
	m_RequestQueue.Empty();
}

//----------------------------------------------------------------
//	Final stage add of the graph node
//----------------------------------------------------------------
template <class T>
void TLGraph::TGraph<T>::DoAddNode(TPtr<T>& pNode, TPtr<T>& pParent)
{
	// Final check to ensure the parent node is in the graph
	if(!IsInGraph(pParent))
	{
		#if defined(_DEBUG)
		{
			#ifdef DONT_ADD_NODE_WHERE_PARENT_IS_BEING_REMOVED
				//	Should not get here...
				TLDebug_Break("Parent isnt in graph(any more?)");
			#else
				//	Should not get here...
				TTempString Debug_String("Parent isn't in graph(any more?) upon add of ");
				pNode->GetNodeRef().GetString( Debug_String );
				TLDebug_Warning( Debug_String );
			#endif
		}
		#endif
		return;
	}

	pParent->AddChild(pNode,pParent);

	//	gr: moved this to BEFORE the node notification. OnNodeAdded now processes the message queue for this node
	//		this is so that the init message gets processed BEFORE this node can do it's first update 
	//		previously the node got added to the graph, and it's parent, and had an update before it got it's init message 
	//		- we want the init message much earlier; specificcly for me the parent node was incorporating a new node's shape in 
	//		it's bounds despite being set not to include that child... because the child hadn't been setup yet.
	//	do graph's OnAdded event
	OnNodeAdded( pNode, TRUE );

	// Tell the node it has been added to the graph
	pNode->OnAdded();

	//	tell the parent it has a new child
	pParent->OnChildAdded( pNode );
}


//----------------------------------------------------------------
//	Final stage removal of the graph node
//----------------------------------------------------------------
template <class T>
void TLGraph::TGraph<T>::DoRemoveNode(TPtr<T>& pNode,TPtr<T>& pParent)
{
	//	remove children from tree - need to do this to shutdown all the nodes otherwise things won't get cleaned up and we'll leak memory
	//	do this FIRST so that the tree gets cleaned up from the deepest leaf first
	TPtrArray<T>& NodeChildren = pNode->GetChildren();

	//	gr: go in reverse as the child nodes are removed from pNode in DoRemoveNode
	for ( s32 c=NodeChildren.GetLastIndex();	c>=0;	c-- )
	{
		DoRemoveNode( NodeChildren[c], pNode );
	}

	//	save off node ref to avoid any possible NULL accesses
	TRef NodeRef = pNode->GetNodeRef();

#if defined(_DEBUG) && defined(DEBUG_PRINT_GRAPH_CHANGES)
	TTempString Debug_String("Removing node ");
	NodeRef.GetString( Debug_String );
	Debug_String.Append(" from graph ");
	GetGraphRef().GetString( Debug_String );
	TLDebug_Print(Debug_String);
#endif
	
	//	Last chance to clear anything up for graph specific code
	OnNodeRemoving( pNode );

	//	shutdown node
	pNode->Shutdown();

	//	remove from parent - gr: there is now always a parent, if none specified then the parent would have been set to the root
	//	note: if the node is MISSING from the NodeIndex this line will probably delete the node
	pParent->RemoveChild(pNode);

	//	remove from index
	s32 NodeIndexIndex = m_NodeIndex.FindIndex( NodeRef );
	if ( NodeIndexIndex == -1 )
	{
		TLDebug_Break("Error removing node from graph... not found in node index");
	}
	else
	{
		//	this SHOULD be the very final TPtr removal for this graph. assume from here on TPtr (or at least it's counter) is now deleted
		m_NodeIndex.RemoveAt( NodeIndexIndex );
	}

	//	notify post-removal
	OnNodeRemoved( NodeRef );
}


//----------------------------------------------------------------
//	actual move of a graph node
//----------------------------------------------------------------
template <class T>
void TLGraph::TGraph<T>::DoMoveNode(TPtr<T>& pNode, TPtr<T>& pNewParent)
{
	// Final check to ensure the parent node is in the graph
	if(!IsInGraph(pNewParent))
	{
		#if defined(_DEBUG)
		{
			#ifdef DONT_ADD_NODE_WHERE_PARENT_IS_BEING_REMOVED
				//	Should not get here...
				TLDebug_Break("Parent isnt in graph(any more?)");
			#else
				//	Should not get here...
				TTempString Debug_String("Parent isn't in graph(any more?) upon move of ");
				pNode->GetNodeRef().GetString( Debug_String );
				TLDebug_Warning( Debug_String );
			#endif
		}
		#endif
		return;
	}

	//	remove from old parent
	TPtr<T> pOldParent = pNode->GetParent();
	pOldParent->RemoveChild( pNode );
	
	//	set new parent and add to new parent
	pNode->SetParent( pNewParent );
	pNewParent->AddChild( pNode, pNewParent );

	//	graph publish a moved message
	OnNodeMoved( pNode, pOldParent );

	//	Tell the node it has been moved in the graph
	pNode->OnMoved( pOldParent );

	//	tell the parents of a change
	pOldParent->OnChildMovedFrom( pNode );
	pNewParent->OnChildMovedTo( pNode, pOldParent );
	pNewParent->OnChildAdded( pNode );
}




template <class T>
void TLGraph::TGraph<T>::ProcessMessageFromQueue(TLMessaging::TMessage& Message)
{
	//	gr: speed up, this list is rarely going to be big, so use a non-allocated array on the stack
	//	increase fixed size if we really ever have a big list
	TFixedArray<TRef,10> TargetList;

	// Get all target ID's
	while ( TRUE )
	{
		TPtr<TBinaryTree>& pData = Message.GetChild("TARGETID");
		if ( !pData )
			break;

		TRef refTargetID;
		if ( pData->Read(refTargetID) )
			TargetList.Add(refTargetID);
	}

	// Go through and find the target to send the message to
	for( u32 uIndex=0;	uIndex<TargetList.GetSize();	uIndex++)
	{
		TRefRef refTargetID = TargetList.ElementAt(uIndex);

		TPtr<T>& pNode = FindNode(refTargetID);
		if (  pNode.IsValid() )
		{
			pNode->QueueMessage(Message);
		}
		else
		{
			// Not found.  Could bring up an error message here or just ignore...
			TLDebug_Break("Graph Message for node that doesn't exist... what to do?");
		}
	}
}


//---------------------------------------------------------
//	find a TPtr in the graph that matches the specified value (will use == operator of node type to match)
//---------------------------------------------------------
template<class T>
template<typename MATCHTYPE>
TPtr<T>& TLGraph::TGraph<T>::FindNodeMatch(const MATCHTYPE& Value,Bool CheckRequestQueue)		
{	
	//	search the graph list
	TPtr<T>& pIndexNode = m_NodeIndex.FindPtr( Value );
	if ( pIndexNode )
		return pIndexNode;

	//	look in the update queue for new nodes (will be doing redundant checks for the nodes being removed, but never mind
	if ( CheckRequestQueue )
	{
		TPtr<TGraphUpdateRequest>& pMatchingUpdate = m_RequestQueue.FindPtr( Value );
		if ( pMatchingUpdate )
			return pMatchingUpdate->GetNode();
	}

	return TLPtr::GetNullPtr<T>();
}


//---------------------------------------------------------
//	called after node shutdown and after being removed from parent and graph
//---------------------------------------------------------
template<class T>
void TLGraph::TGraph<T>::OnNodeRemoved(TRefRef NodeRef)
{
	// Send the removed node notificaiton
	TLMessaging::TMessage Message("NodeRemoved", GetGraphRef());
	Message.Write(NodeRef);

	PublishMessage(Message);
}

//---------------------------------------------------------
//	called after node has been added to the graph and parent
//---------------------------------------------------------
template<class T>
void TLGraph::TGraph<T>::OnNodeAdded(TPtr<T>& pNode,Bool SendAddedMessage)
{
	//	add to index
	m_NodeIndex.Add( pNode );

	//	gr: process initial message queue (assume this only has the Init message in it)
	pNode->ProcessMessageQueue();

	if ( SendAddedMessage )
	{
		// Send the added node notificaiton
		TLMessaging::TMessage Message("NodeAdded", GetGraphRef());
		Message.Write(pNode->GetNodeRef());

		PublishMessage(Message);
	}
}


//---------------------------------------------------------
//	called after node has been moved in the graph
//---------------------------------------------------------
template<class T>
void TLGraph::TGraph<T>::OnNodeMoved(TPtr<T>& pNode,TPtr<T>& pOldParentNode)
{
	//	make up a moved node notificaiton
	TLMessaging::TMessage Message("NodeMoved", GetGraphRef());
	Message.Write( pNode->GetNodeRef() );
	Message.ExportData("OldParent", pOldParentNode->GetNodeRef() );
	Message.ExportData("NewParent", pNode->GetParent()->GetNodeRef() );

	PublishMessage(Message);
}

//---------------------------------------------------------
//	send message to node
//---------------------------------------------------------
template<class T>
Bool TLGraph::TGraph<T>::SendMessageToNode(TRefRef NodeRef,TLMessaging::TMessage& Message)
{
	//	get node to send message to
	TPtr<T>& pNode = FindNode( NodeRef );
	if ( !pNode )
	{
#ifdef _DEBUG
		TTempString Debug_String("Sent message to a node (");
		NodeRef.GetString( Debug_String );
		Debug_String.Append(") that doesn't exist... add to a lost/queue for later queue?");
#endif
		return FALSE;
	}

	//	queue message
	if ( !pNode->QueueMessage( Message ) )
		return FALSE;

	return TRUE;
}

//---------------------------------------------------------
//	add factory to graph
//---------------------------------------------------------
template<class T>
Bool TLGraph::TGraph<T>::AddFactory(TPtr<TClassFactory<T,FALSE> >& pFactory)
{
	return (m_NodeFactories.Add(pFactory) != -1);			
}
