/*------------------------------------------------------
	
	Templated graph and graphnode classes which handle node in a tree - 
	handles the update, queued tree changes, events for nodes

-------------------------------------------------------*/
#pragma once


//#define DEBUG_PRINT_GRAPH_CHANGES	//	enable "removing node XXX" and "requesting remove node XX" etc prints


//	if enabled we check the parent heirachy on AddNode() - if a parent is being removed we dont ADD the node. 
//	Means more request-list checks - but means we WONT have potentially non-added nodes in the ADD request list
//	if disabled it's blindly added, and adds potentially non-added nodes to the add list. But much less list checks
//#define DONT_ADD_NODE_WHERE_PARENT_IS_BEING_REMOVED


//	to speed up the generation of the graph, I've turned this on. Instead of adds being queued up, this creates
//	them and puts them into the graph immediately. Remove's and moves are still put in a queue
#define GRAPH_IMMEDIATE_ADD

#include "TGraphBase.h"
#include "TLMessaging.h"
#include "TRelay.h"
#include "TEventChannel.h"
#include "TClassFactory.h"
#include "TCoreManager.h"
#include "TLCore.h"

#include <TootleAsset/TScheme.h>


namespace TLGraph
{
	template <class T>
	class TGraphNode;

	template <class T>
	class TGraph;
};

#define TGraph_NodeIndexGrowBy	500	//	gr: graphs get pretty big
#define TGraph_NodeQueueGrowBy	500	//	gr: request queue gets big too



template<class TYPE>
class TNodeFactory : public TBaseClassFactory<TYPE>
{
public:
	TYPE*	CreateInstance(TRefRef InstanceRef,TRefRef TypeRef)
	{
		return CreateObject( InstanceRef, TypeRef );
	}
	
protected:
	virtual TYPE*	CreateObject(TRefRef InstanceRef,TRefRef TypeRef)=0;

private:
	virtual TYPE*	CreateObjectBase(TRefRef InstanceRef,TRefRef TypeRef)	{	return CreateObject( InstanceRef, TypeRef );	}
};



//--------------------------------------------------------------------
//	put this with your graph somewhere so that the graph has fast-resized arrays please!
//--------------------------------------------------------------------
#define TLGraph_DeclareGraph(T)	\
	namespace TLCore				\
	{								\
		template<> FORCEINLINE Bool IsDataType<T*>()	{	return TRUE;	}	\
		template<> FORCEINLINE Bool IsDataType<TLGraph::TGraph<T>::TGraphUpdateRequest>()	{	return TRUE;	}	\
	}



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
		m_pRootNode			( NULL )
	{
	}
	
	//	Node access
	FORCEINLINE T&				GetRootNode()							{	return *m_pRootNode; }	//	we assume the root node is always valid
	FORCEINLINE const T&		GetRootNode() const						{	return *m_pRootNode; }	//	we assume the root node is always valid
	FORCEINLINE TArray<T*>&		GetNodeList()							{	return m_NodeIndex;	}
	FORCEINLINE u32				GetNodeCount() const					{	return m_NodeIndex.GetSize();	}

	T*							FindNode(TRefRef NodeRef,Bool CheckRequestQueue=TRUE);				//	find a node
	const T*					FindNode(TRefRef NodeRef,Bool CheckRequestQueue=TRUE) const;		//	find a node

	template<class OBJ>
	OBJ*						FindNode(TRefRef NodeRef,Bool CheckRequestQueue=TRUE);				//	find a node

	virtual TRef				GetFreeNodeRef(TRefRef BaseRef=TRef());	//	find an unused ref for a node - returns the ref
	TRef						GetFreeNodeRef(TRef& Ref);				//	find an unused ref for a node, modifies the ref provided

	// Messaging
	virtual Bool				SendMessageToNode(TRefRef NodeRef,TLMessaging::TMessage& Message);	//	send message to node

	// Graph change requests
	virtual TRef				CreateNode(TRefRef NodeRef,TRefRef TypeRef,TRefRef ParentRef,TLMessaging::TMessage* pInitMessage=NULL,Bool StrictNodeRef=FALSE);		//	create node and add to the graph. returns ref of new node
	FORCEINLINE Bool			MoveNode(TRefRef NodeRef,TRefRef NewParentRef)		{	return MoveNode( *FindNode( NodeRef ), (NewParentRef.IsValid() ? *FindNode( NewParentRef ) : GetRootNode()) );	}
	
	virtual Bool				RemoveNode(TRefRef NodeRef)							{	T* pNode = FindNode( NodeRef );		return pNode ? RemoveNode(*pNode) : false;	}
	FORCEINLINE Bool			RemoveNode(TRef& NodeRef);							//	simple remove node wrapper which invalidates the node ref as well
	virtual Bool				RemoveChildren(TRefRef NodeRef)						{	T* pNode = FindNode( NodeRef )	;	return pNode ? RemoveChildren( *pNode ) : false;	}			//	remove pNode's children (but not pNode)
	Bool						RemoveChildren(T& Node);							//	remove pNode's children (but not pNode)

	//	factory access
	FORCEINLINE Bool			AddFactory(TPtr< TNodeFactory<T> >& pFactory)		{	return m_NodeFactories.Add( pFactory ) != -1;	}

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
	T*							DoCreateNode(TRef NodeRef,TRefRef TypeRef,T& Parent,TLMessaging::TMessage* pInitMessage=NULL, Bool StrictNodeRef=FALSE);	//	create node and add to the graph. returns ref of new node

	Bool						AddNode(T& Node,T &Parent);				//	returns the ref of the object. You can't always have the ref we constructed with :)
	Bool						RemoveNode(T& Node);					//	Requests a node to be removed from the graph.  The node will be removed at a 'safe' time so is not immediate
	Bool						MoveNode(T& Node,T& Parent);			//	give this node a new parent via a request

	///////////////////////////////////////////////////////////////////////////////////
protected:
	virtual Bool				IsInGraph(TRefRef NodeRef,Bool CheckRequestQueue=TRUE)				{	return FindNode( NodeRef, CheckRequestQueue ) != NULL;	}
	FORCEINLINE Bool			IsInGraph(const T& Node,Bool CheckRequestQueue=TRUE)				{	return FindNode( Node.GetNodeRef(), CheckRequestQueue ) != NULL;	}
	FORCEINLINE Bool			IsInRequestQueue(const T& Node) const		{	return IsInRequestQueue( Node.GetNodeRef() );	}
	FORCEINLINE Bool			IsInRequestQueue(TRefRef NodeRef) const		{	return m_RequestQueue.Exists( NodeRef );	}
	FORCEINLINE Bool			IsInAddQueue(const T& Node) const			{	return IsInAddQueue( Node.GetNodeRef() );	}
	Bool						IsInAddQueue(TRefRef NodeRef) const;		//	is in the request queue and marked to Add
	FORCEINLINE Bool			IsInRemoveQueue(const T& Node) const		{	return IsInRemoveQueue( Node.GetNodeRef() );	}
	Bool						IsInRemoveQueue(TRefRef NodeRef) const;		//	is in the request queue and marked to remove

	virtual void				ProcessMessageFromQueue(TLMessaging::TMessage& Message);
	virtual void				OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);
	
	//	base graph functions
	virtual TLGraph::TGraphNodeBase*	FindNodeBase(TRefRef NodeRef)		{	return FindNode( NodeRef );	}
	virtual TLGraph::TGraphNodeBase*	GetRootNodeBase()					{	return m_pRootNode;	}

	//	events
	virtual void				OnNodeAdded(T& Node,Bool SendAddedMessage);	//	called after node has been added to graph and to parent - publishes an added message
	virtual void				OnNodeMoved(T& Node,T& OldParentNode);		//	called after node has been moved to a new parent in the scenegraph - publishes a move message
	virtual void				OnNodeRemoving(T& Node)				{	}	//	called before node shutdown and before being removed from parent and graph
	virtual void				OnNodeRemoved(TRefRef NodeRef);				//	called after node shutdown and after being removed from parent and graph

private:

	// Graph structure updates
	void						DoAddNode(T& Node,T& Parent);		// Final stage add of the graph node
	void						DoRemoveNode(T& Node);				// Final stage removal of the graph node
	void						DoMoveNode(T& Node,T& NewParent);	// Final stage move of a graph node

public:	//	gr: exposed atm for the IsDataType func
	class TGraphUpdateRequest
	{
	public:
		TGraphUpdateRequest(T& Node,T& Parent, TRefRef Command) :
			m_pNode			( &Node ),
			m_pNodeParent	( &Parent ),
			m_Command		( Command )
		{
		}

		FORCEINLINE T&				GetNode()					{	return *m_pNode;	}
		FORCEINLINE const T&		GetNode() const				{	return *m_pNode;	}
		FORCEINLINE T&				GetNodeParent()				{	return *m_pNodeParent;	}
		FORCEINLINE TRefRef			GetCommand() const			{	return m_Command;	}
		
		FORCEINLINE Bool			IsAddRequest() const		{	return m_Command == STRef3(A,d,d);	}
		FORCEINLINE Bool			IsRemoveRequest() const		{	return m_Command == STRef(R,e,m,o,v);	}
		FORCEINLINE Bool			IsMoveRequest() const		{	return m_Command == STRef4(M,o,v,e);	}

		FORCEINLINE Bool			operator==(TRefRef NodeRef) const		{	return GetNode() == NodeRef;	}

	protected:
		T*			m_pNode;			//	Node to update
		T*			m_pNodeParent;		//	Node parent
		TRef		m_Command;			//	like an enum - "Add" "Remove" "Move"
	};

private:
	T*											m_pRootNode;			//	The root of the graph
	TPointerArray<TGraphUpdateRequest,true,TSortPolicyNone<TGraphUpdateRequest*>,TGraph_NodeQueueGrowBy>		m_RequestQueue;			//	List of objects to add/remove from the graph
	THeapArray<TRef,TGraph_NodeQueueGrowBy,TSortPolicySorted<TRef> >	m_AddingNodes;			//	list of node refs in the add-queue. Kept seperately for fast searching as we cannot sort the request queue
	TPointerSortedArray<T,true,TRef,TGraph_NodeIndexGrowBy>				m_NodeIndex;			//	list of all the nodes, used for fast node finding

	TPtrArray< TNodeFactory<T> >				m_NodeFactories;		//	array of graph node factories. if none, T is created
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
	TGraphNode(TRefRef NodeRef,TRefRef NodeTypeRef) : 
		TGraphNodeBase ( NodeRef, NodeTypeRef ), 
		m_pParent (NULL)	
	{
	}
	virtual ~TGraphNode()
	{
		//	gr: check has gone through shutdown?
		// Ensure this node isn't in the linked list any longer
		//Remove(this);
	}

	// Parent manipulation
	FORCEINLINE T*					GetParent()							{	return m_pParent;	}
	FORCEINLINE const T*			GetParent() const					{	return m_pParent;	}
	FORCEINLINE Bool				HasParent() const					{	return m_pParent != NULL;	}
	virtual const TGraphNodeBase*	GetParentBase() const				{	return m_pParent;	}	//	gr: automaticcly casts down

	FORCEINLINE TRef				GetParentRef()						{	return (m_pParent ? m_pParent->GetNodeRef() : TRef());	}


	// Child manipulation
	FORCEINLINE Bool				HasChildren() const					{	return (m_Children.GetSize() > 0);	}
	FORCEINLINE TPointerArray<T>&			GetChildren()						{	return m_Children;	}
	FORCEINLINE const TPointerArray<T>&		GetChildren() const					{	return m_Children;	}
	virtual void					GetChildrenBase(TArray<TGraphNodeBase*>& ChildNodes);
	template<typename MATCHTYPE>
	T*								FindChildMatch(const MATCHTYPE& Value);				//	find a TPtr in the graph that matches the specified value (will use == operator of node type to match)
	FORCEINLINE T*					FindChild(const TRef& NodeRef)						{	return FindChildMatch(NodeRef);	}
	TRef							FindChildOfType(TRefRef NodeTypeRef);

	FORCEINLINE Bool				operator==(const T* pNode) const			{	return this == pNode;	}
	FORCEINLINE Bool				operator<(TRefRef NodeRef) const			{	return GetNodeRef() == NodeRef;	}
	FORCEINLINE Bool				operator==(TRefRef NodeRef) const			{	return GetNodeRef() == NodeRef;	}
	FORCEINLINE Bool				operator<(const T& That) const				{	return GetNodeRef() < That.GetNodeRef();	}
	FORCEINLINE Bool				operator==(const T& That) const				{	return GetNodeRef() == That.GetNodeRef();	}

protected:
	virtual void 					Update(float Timestep);						// Main node update called once per frame
	virtual void					UpdateAll(float Timestep);					//	update tree: update self, and children and siblings
	virtual void					Shutdown();									// Shutdown routine	- called before being removed form the graph. Base code sends out a shutdown message to our subscribers
	virtual void					GetShutdownMessageData(TLMessaging::TMessage& ShutdownMessage)	{	}	//	add additional data to the shutdown message

	virtual void					ProcessMessage(TLMessaging::TMessage& Message);
	virtual void					ProcessMessageFromQueue(TLMessaging::TMessage& Message);

	virtual void					OnAdded()									{}	// Added routine - called once the node has been added to the graph
	virtual void					OnMoved(const T& OldParentNode)				{}	// Moved routine - called after the node has been moved (so this->GetParent is the new parent)
	virtual void					OnChildMovedFrom(T& OldChild)				{}	//	called to a parent when one of it's direct children has moved to somewhere else (pOldChild->GetParent is it's new parent)
	virtual void					OnChildMovedTo(T& NewChild,const T& OldParentNode)	{}	//	called to the new parent of a child after it's been moved. note, this AND OnChildAdded is called, so only use this for very specific cases
	virtual void					OnChildAdded(T& NewChild)			{}	//	called to the new parent of a child after it's been added to the graph

	// Parent manipulation
	virtual void					SetParent(T& Node);							//	gr: needs to make sure removes self from former parent?

private:
	Bool							AddChild(T& Child);							//	gr: add to END of child list
	FORCEINLINE Bool				RemoveChild(T& Node)						{	return m_Children.Remove( &Node );	}
	FORCEINLINE void				RemoveChildren()							{	m_Children.DeleteElements();	}

	//	accessor to this derived type so we can call functions on the objects (ie. on TRenderNode) without the need for a virtual, 
	//	compiler can inline and optimise this much more easily. USE WHENEVER POSSIBLE! (see IsEnabled for example)
	T&								This()										{	return *static_cast<T*>( this );	}		
	const T&						This() const								{	return *static_cast<const T*>( this );	}

	void							ProcessSubscribeOrPublishTo(TRefRef SubOrPub,TBinaryTree& PubSubData);	//	handle publishto/subscribeto message data 

private:
	T*								m_pParent;			// Parent item
	TPointerArray<T>				m_Children;
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
void TLGraph::TGraphNode<T>::SetParent(T& Node)
{
	if ( &Node == this )
	{
		TLDebug_Break("Trying to set parent of this to this");
		return;
	}

	m_pParent = &Node;	
}	


//---------------------------------------------------------
// Searches for a child node of the type TypeRef.
// Returns a TRef of a child of the specified type if found, otherwise returns an invalid TRef
//---------------------------------------------------------
template <class T>
TRef	TLGraph::TGraphNode<T>::FindChildOfType(TRefRef TypeRef)		
{
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		T& Child = *m_Children[c];
		if ( Child.GetNodeTypeRef() == TypeRef )
			return Child.GetNodeRef();
		
		TRef ChildRef = Child.FindChildOfType( TypeRef );
		if ( ChildRef.IsValid() )
			return ChildRef;
	}
	
	//	no match
	return TRef();
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
Bool TLGraph::TGraphNode<T>::AddChild(T& Child)
{
	Child.SetParent( This() );
	m_Children.Add( &Child );
	
	return TRUE;
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
			TDebugString Debug_String;
			Debug_String << "Unhandled message " << MessageRef << " on node " << GetNodeAndTypeRef();
			TLDebug_Print( Debug_String );
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
	TArray<T*>& Children = GetChildren();

	for ( u32 c=0;	c<Children.GetSize();	c++ )
	{
		ChildNodes.Add( Children[c] );
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
		OnNodeAdded( *m_pRootNode, TRUE );
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
	// Remove all update requests
	m_RequestQueue.SetAll(NULL);
	m_RequestQueue.Empty(true);
	m_AddingNodes.Empty(true);

	// Remove the root node from the graph.  This will cause the entire graph to be shutdown and removed.
	if ( m_pRootNode )
	{
		DoRemoveNode(*m_pRootNode);

		// The root node will be deleted during the DoRemoveNode call
		// so we can simply set the root node pointer to null
		m_pRootNode = NULL;
	}
	
	return SyncTrue;
}



//-------------------------------------------------------
//	find node in the graph by ref
//-------------------------------------------------------
template <class T>
T* TLGraph::TGraph<T>::FindNode(TRefRef NodeRef,Bool CheckRequestQueue)			
{	
	//	invalid ref is not gonna be in the graph
	if ( !NodeRef.IsValid() )
		return NULL;
	
	//	search the graph list
	T* pIndexNode = m_NodeIndex.FindPtr( NodeRef );
	if ( pIndexNode )
		return pIndexNode;
	
	//	look in the update queue for new nodes (will be doing redundant checks for the nodes being removed, but never mind
	//	gr: if this becomes slow, we could check the adding queue first for a match, then do this slow check
	if ( CheckRequestQueue )
	{
		TGraphUpdateRequest* pMatchingUpdate = m_RequestQueue.FindPtr( NodeRef );
		if ( pMatchingUpdate )
			return &pMatchingUpdate->GetNode();
	}
	
	return NULL;
}

//-------------------------------------------------------
//	find node in the graph by ref
//-------------------------------------------------------
template <class T>
const T* TLGraph::TGraph<T>::FindNode(TRefRef NodeRef,Bool CheckRequestQueue) const
{	
	//	invalid ref is not gonna be in the graph
	if ( !NodeRef.IsValid() )
		return NULL;
	
	//	search the graph list
	const T* pIndexNode = m_NodeIndex.FindPtr( NodeRef );
	if ( pIndexNode )
		return pIndexNode;
	
	//	look in the update queue for new nodes (will be doing redundant checks for the nodes being removed, but never mind
	//	gr: if this becomes slow, we could check the adding queue first for a match, then do this slow check
	if ( CheckRequestQueue )
	{
		const TGraphUpdateRequest* pMatchingUpdate = m_RequestQueue.FindPtr( NodeRef );
		if ( pMatchingUpdate )
			return pMatchingUpdate->GetNode();
	}
	
	return NULL;
}

template <class T>
template <class OBJ>
OBJ* TLGraph::TGraph<T>::FindNode(TRefRef NodeRef,Bool CheckRequestQueue)
{	
	T* pObj = FindNode(NodeRef, CheckRequestQueue);

	if(pObj)
		return reinterpret_cast<OBJ*>(pObj);

	return NULL;
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
//	find a free node ref from this array. 
//	returns true if we have a free ref, wait if we have found an unused one, but changed the original
//	false is returned if we've looped back to the original ref
//---------------------------------------------------
template<typename ARRAYTYPE>
inline SyncBool FindFreeRef(ARRAYTYPE& Array,TRef& Ref,TRefRef OriginalRef)
{
	//	sort the indexes
	Array.Sort();

	//	find a node with this ref
	s32 Index = Array.FindIndex( Ref );

	//	no node with this ref
	if ( Index == -1 )
		return SyncTrue;
	
	while ( true )
	{
		//	increment the ref and see if it matches the next one
		Ref.Increment();
		Index++;

		//	if we've gone back to the original ref then we've run out of refs
		if ( Ref == OriginalRef )
			return SyncFalse;

		//	won't match another
		if ( (u32)Index >= Array.GetSize() )
		{
			//	gr: unless the ref has wrapped around in which case we need to start the search again
			if ( Ref < OriginalRef )
			{
				Index = Array.FindIndex(Ref);
				if ( Index == -1 )
					return SyncWait;
				continue;
			}
			return SyncWait;
		}

		//	see if the next one is free
		if ( !(Array[Index] == Ref) )
			return SyncWait;
	}
}


//---------------------------------------------------
//	find an unused ref for a node, modifies the ref provided
//---------------------------------------------------
template <class T>
TRef TLGraph::TGraph<T>::GetFreeNodeRef(TRef& Ref)
{
	//	save off the original ref so we know if we've looped through all 16 million refs...
	TRef OriginalRef = Ref;
	while ( true )
	{
		bool Changed = false;

		//	find a free ref in the node indexes
		SyncBool FindResult = FindFreeRef( m_NodeIndex, Ref, OriginalRef );
		Changed |= (FindResult == SyncWait);
		if ( FindResult == SyncFalse )
			return TRef_Invalid;

		//	find a free ref in the adding nodes
		FindResult = FindFreeRef( m_AddingNodes, Ref, OriginalRef );
		Changed |= (FindResult == SyncWait);
		if ( FindResult == SyncFalse )
			return TRef_Invalid;
	
		//	if the ref hasn't been changed by either array test then the ref is free
		if ( !Changed )
			break;
	}
	
	return Ref;
}

	
//---------------------------------------------------------
//	create node and add to the graph. returns ref of new node
//---------------------------------------------------------
template <class T>
TRef TLGraph::TGraph<T>::CreateNode(TRefRef NodeRef,TRefRef TypeRef,TRefRef ParentRef,TLMessaging::TMessage* pInitMessage,Bool StrictNodeRef)
{
	T* pParent = FindNode( ParentRef );
	TRef TempRef = NodeRef;
	T* pNode = DoCreateNode( TempRef, TypeRef, pParent ? *pParent : *m_pRootNode, pInitMessage, StrictNodeRef );

	if ( !pNode )
		return TRef();

	return pNode->GetNodeRef();
}


//---------------------------------------------------------
//	create node and add to the graph
//---------------------------------------------------------
template <class T>
T* TLGraph::TGraph<T>::DoCreateNode(TRef NodeRef,TRefRef TypeRef,T& Parent,TLMessaging::TMessage* pInitMessage,Bool StrictNodeRef)
{
	T* pNewNode = NULL;

	//	if we don't have strict node refs, make sure this node ref isn't already in use
	if ( !StrictNodeRef )
	{
		TRef OldRef = NodeRef;

		//	not strict, so allow an invalid node ref, and generate a valid one
		if ( !NodeRef.IsValid() )
		{
			if ( m_NodeIndex.GetSize() > 0 )
			{
				NodeRef = m_NodeIndex.ElementLastConst()->GetNodeRef();
				NodeRef.Increment();
			}
			else
			{
				NodeRef.Set( TRef_Static4(N,o,d,e) );
			}
		}
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
		pNewNode = m_NodeFactories[f]->CreateInstance( NodeRef, TypeRef );

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
	if ( !AddNode( *pNewNode, Parent ) )
	{
		TLMemory::Delete( pNewNode );
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

	//	process the init message immediately
	#if defined(GRAPH_IMMEDIATE_ADD)
		pNewNode->ProcessMessageQueue();
	#endif

	//	return ref of new node
	return pNewNode;
}


//---------------------------------------------------------
//	Requests a node to be added to the graph.  The node will be added at a 'safe' time so is not immediate
//---------------------------------------------------------
template <class T>
Bool TLGraph::TGraph<T>::AddNode(T& Node,T& Parent)	
{
	if ( !Node.GetNodeRef().IsValid() )
	{
		TTempString Debug_String("Tried to add node without Ref to graph ");
		GetGraphRef().GetString( Debug_String );
		TLDebug_Break( Debug_String );
		return FALSE;
	}

	//	gr: AddNode no longer changes no refs. CreateNode does that now
	if ( FindNode( Node.GetNodeRef() ) )
	{
		TDebugString Debug_String;
		Debug_String << "Node with ref " << Node.GetNodeRef() << " already exists in graph " << GetGraphRef();
		TLDebug_Break( Debug_String );
		return FALSE;
	}

#ifdef DONT_ADD_NODE_WHERE_PARENT_IS_BEING_REMOVED
	Bool CheckParentIsInGraph = TRUE;

	//	check UP the tree to see if a parent is going to be removed (not just parent, but parent's parent etc)
	T* pCurrentParent = &Parent;
	while ( pCurrentParent )
	{
		//	check parent's validity
		TGraphUpdateRequest* pRequest = m_RequestQueue.FindPtr( pCurrentParent->GetNodeRef() );
		if ( pRequest )
		{
			TGraphUpdateRequest& Request = *pRequest;
			//	check to see if the parent is in the to-remove queue? abort add if it is
			if ( Request.IsRemoveRequest() )
			{
	#ifdef _DEBUG
				TDebugString Debug_String;
				Debug_String << "Trying to add node " << Node.GetNodeRef() << " to parent node " << pParent->GetNodeRef() << " which is in the remove queue. Aborting add.";
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
	if ( CheckParentIsInGraph && !IsInGraph( Parent, FALSE ) )
	{
#ifdef _DEBUG
		TDebugString Debug_String;
		Debug_String << "Trying to add node " << Node.GetNodeRef() << " to parent node " << Parent.GetNodeRef() << " which is not in the graph. Aborting add.";
		TLDebug_Break( Debug_String );
#endif
		return FALSE;
	}
#endif

#if defined(GRAPH_IMMEDIATE_ADD)
	//	add straight into the graph
	DoAddNode( Node, Parent );
#else
	//	add the node to the requet queue
	m_RequestQueue.AddNewPtr( new TGraphUpdateRequest( Node, Parent, STRef3(A,d,d) ) );
	m_AddingNodes.Add( Node.GetNodeRef() );
#endif

	return TRUE;
}

//-----------------------------------------------------
//	Requests a node to be removed from the graph.  The node will be removed at a 'safe' time so is not immediate
//-----------------------------------------------------
template <class T>
Bool TLGraph::TGraph<T>::RemoveNode(T& Node)
{
	// Check to make sure we aren;t trying to remove the root node - only done at shutdown
	if( &Node == m_pRootNode)
	{
		TLDebug_Break("Attempting to remove root node");
		return FALSE;
	}

	// Check to see if the node is already in the queue
	TGraphUpdateRequest* pRequest = m_RequestQueue.FindPtr( Node.GetNodeRef() );
	if ( pRequest )
	{
#ifdef _DEBUG
		TDebugString Debug_String;
		Debug_String << "Attempting to remove node " << Node.GetNodeRef();
		if ( pRequest->IsAddRequest() )
		{
			Debug_String << " which is in the graph ADD queue";
			TLDebug_Warning( Debug_String );
		}
		else
		{
			Debug_String << " which is already in the graph REMOVE queue";
			TLDebug_Warning( Debug_String );
		}
#endif

		return FALSE;
	}

	//	if the parent is being removed, save some time/space etc by not adding to the queue
	T* pParent = Node.GetParent();
	TGraphUpdateRequest* pParentRequest = pParent ? m_RequestQueue.FindPtr( pParent->GetNodeRef() ) : NULL;

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
	else if ( !IsInGraph( *pParent, FALSE ) )
	{
#ifdef _DEBUG
		TDebugString Debug_String;
		Debug_String << "Attempting to remove node " << Node.GetNodeRef() << " from graph " << GetGraphRef() << " but parent is not in graph (maybe already removed)";
		TLDebug_Warning(Debug_String);
#endif
		
		//	gr: added shutdown routine for node so it's still cleaned up, shouldnt be any harm doing this
		Node.Shutdown();
		return TRUE;
	}

#if defined(_DEBUG) && defined(DEBUG_PRINT_GRAPH_CHANGES)
	TDebugString Debug_String;
	Debug_String << "Requesting remove node " << Node.GetNodeRef();
	TLDebug_Print(Debug_String);
#endif
	
	// No - Add the node to the queue
	m_RequestQueue.Add( new TGraphUpdateRequest( Node, *pParent, STRef(R,e,m,o,v) ) );

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
	T* pNode = FindNode( NodeRef );
	if ( !pNode )
	{
		NodeRef.SetInvalid();
		return false;
	}
	
	Bool Result = RemoveNode( *pNode );
	
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
	TPointerArray<T>& Children = Node.GetChildren();
	for ( u32 c=0;	c<Children.GetSize();	c++ )
	{
		AnyRemoved |= RemoveNode( *Children[c] );
	}

	return AnyRemoved;
}



//-------------------------------------------------------------
//	give this node a new parent via a request
//-------------------------------------------------------------
template <class T>
Bool TLGraph::TGraph<T>::MoveNode(T& Node,T& Parent)
{
	//	check theyre not the same node
	if ( &Node == &Parent )
	{
		TLDebug_Break("Attempting to move a node to be under itself");
		return FALSE;
	}

	//	this is already the node's parent
	if ( Node.GetParent() == &Parent )
		return FALSE;

	//	create new request
	m_RequestQueue.Add( new TGraphUpdateRequest( Node, Parent, STRef4(M,o,v,e) ) );

	return TRUE;
}


//-------------------------------------------------------------
//	find a request for a node and see if it's an ADD request
//-------------------------------------------------------------
template <class T>
Bool TLGraph::TGraph<T>::IsInAddQueue(TRefRef NodeRef) const
{
	return m_AddingNodes.Exists( NodeRef );
}


//-------------------------------------------------------------
//	find a request for a node and see if it's a REMOVE request
//-------------------------------------------------------------
template <class T>
Bool TLGraph::TGraph<T>::IsInRemoveQueue(TRefRef NodeRef) const
{
	//	get request
	TGraphUpdateRequest* pUpdateRequest = m_RequestQueue.Find( NodeRef );
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
		T& RequestNode = Request.GetNode();

		switch ( Request.GetCommand().GetData() )
		{
			case STRef(R,e,m,o,v):
				DoRemoveNode( RequestNode );
				break;

			case STRef3(A,d,d):
				DoAddNode( RequestNode, Request.GetNodeParent() );
				break;

			case STRef4(M,o,v,e):
				DoMoveNode( RequestNode, Request.GetNodeParent() );
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
void TLGraph::TGraph<T>::DoAddNode(T& Node,T& Parent)
{
	// Final check to ensure the parent node is in the graph
	if(!IsInGraph(Parent))
	{
		#if defined(_DEBUG)
		{
			#ifdef DONT_ADD_NODE_WHERE_PARENT_IS_BEING_REMOVED
				//	Should not get here...
				TLDebug_Break("Parent isnt in graph(any more?)");
			#else
				//	Should not get here...
				TDebugString Debug_String("Parent isn't in graph(any more?) upon add of ");
				Debug_String << Node.GetNodeRef();
				TLDebug_Warning( Debug_String );
			#endif
		}
		#endif
		return;
	}

	//	remove from the adding list
	m_AddingNodes.Remove( Node.GetNodeRef() );

	//	add to new parent's child list
	Parent.AddChild(Node);

	//	gr: moved this to BEFORE the node notification. OnNodeAdded now processes the message queue for this node
	//		this is so that the init message gets processed BEFORE this node can do it's first update 
	//		previously the node got added to the graph, and it's parent, and had an update before it got it's init message 
	//		- we want the init message much earlier; specificcly for me the parent node was incorporating a new node's shape in 
	//		it's bounds despite being set not to include that child... because the child hadn't been setup yet.
	//	do graph's OnAdded event
	OnNodeAdded( Node, true );

	// Tell the node it has been added to the graph
	Node.OnAdded();

	//	tell the parent it has a new child
	Parent.OnChildAdded( Node );
}


//----------------------------------------------------------------
//	Final stage removal of the graph node
//----------------------------------------------------------------
template <class T>
void TLGraph::TGraph<T>::DoRemoveNode(T& Node)
{
	//	remove children from tree - need to do this to shutdown all the nodes otherwise things won't get cleaned up and we'll leak memory
	//	do this FIRST so that the tree gets cleaned up from the deepest leaf first
	TPointerArray<T>& NodeChildren = Node.GetChildren();

	//	gr: go in reverse as the child nodes are removed from pNode in DoRemoveNode
	for ( s32 c=NodeChildren.GetLastIndex();	c>=0;	c-- )
	{
		DoRemoveNode( *NodeChildren[c] );
	}

	//	save off node ref to avoid any possible NULL accesses
	TRef NodeRef = Node.GetNodeRef();

#if defined(_DEBUG) && defined(DEBUG_PRINT_GRAPH_CHANGES)
	TDebugString Debug_String;
	Debug_String << "Removing node " << NodeRef << " from graph " << GetGraphRef();
	TLDebug_Print(Debug_String);
#endif
	
	//	Last chance to clear anything up for graph specific code
	OnNodeRemoving( Node );

	//	shutdown node
	Node.Shutdown();

	//	remove from parent
	T* pParent = Node.GetParent();
	if ( pParent )
		pParent->RemoveChild(Node);

	//	remove from index
	s32 NodeIndexIndex = m_NodeIndex.FindIndex( NodeRef );
	if ( NodeIndexIndex == -1 )
	{
		TLDebug_Break("Error removing node from graph... not found in node index");
	}
	else
	{
		//	gr: this used to be the last place the TPtr was released (in theory)
		m_NodeIndex.RemoveAt( NodeIndexIndex );
	}

	//	decide where the node should be deleted now... managed by parent, or graph?
	T* pNode = &Node;
	TLMemory::Delete( pNode );

	//	notify post-removal
	OnNodeRemoved( NodeRef );
}


//----------------------------------------------------------------
//	actual move of a graph node
//----------------------------------------------------------------
template <class T>
void TLGraph::TGraph<T>::DoMoveNode(T& Node,T& NewParent)
{
	// Final check to ensure the parent node is in the graph
	if(!IsInGraph(NewParent))
	{
		#if defined(_DEBUG)
		{
			#ifdef DONT_ADD_NODE_WHERE_PARENT_IS_BEING_REMOVED
				//	Should not get here...
				TLDebug_Break("Parent isnt in graph(any more?)");
			#else
				//	Should not get here...
				TDebugString Debug_String;
				Debug_String << "Parent isn't in graph(any more?) upon move of " << Node.GetNodeRef();
				TLDebug_Warning( Debug_String );
			#endif
		}
		#endif
		return;
	}

	//	remove from old parent
	T& OldParent = *Node.GetParent();
	OldParent.RemoveChild( Node );
	
	//	set new parent and add to new parent
	Node.SetParent( NewParent );
	NewParent.AddChild( Node );

	//	graph publish a moved message
	OnNodeMoved( Node, OldParent );

	//	Tell the node it has been moved in the graph
	Node.OnMoved( OldParent );

	//	tell the parents of a change
	OldParent.OnChildMovedFrom( Node );
	NewParent.OnChildMovedTo( Node, OldParent );
	NewParent.OnChildAdded( Node );
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
		TPtr<TBinaryTree>& pData = Message.GetChild( TRef_Static(T,A,R,G,E) );
		if ( !pData )
			break;

		TRef refTargetID;
		if ( pData->Read(refTargetID) )
			TargetList.Add(refTargetID);
	}

	if(TargetList.GetSize() > 0)
	{
		// Go through and find the target to send the message to
		for( u32 uIndex=0;	uIndex<TargetList.GetSize();	uIndex++)
		{
			TRefRef refTargetID = TargetList.ElementAt(uIndex);

			T* pNode = FindNode(refTargetID);
			if ( pNode )
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
	else
	{
		// No target nodes to send the message to so process the 
		// message by the graph instead
		Message.ResetReadPos();
		ProcessMessage(Message);
	}
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
	Message.ExportData("Node", NodeRef );

	PublishMessage(Message);
}

//---------------------------------------------------------
//	called after node has been added to the graph and parent
//---------------------------------------------------------
template<class T>
void TLGraph::TGraph<T>::OnNodeAdded(T& Node,Bool SendAddedMessage)
{
	//	add to index
	m_NodeIndex.Add( &Node );

	//	gr: process initial message queue (assume this only has the Init message in it)
	Node.ProcessMessageQueue();

	if ( SendAddedMessage )
	{
		// Send the added node notificaiton
		TLMessaging::TMessage Message( TRef_Static(N,o,d,e,A), GetGraphRef());	//	"nodeAdded"
		Message.Write( Node.GetNodeRef() );
		Message.ExportData( TRef_Static4(N,o,d,e), Node.GetNodeRef() );
		
		//	if no parent data, then node has no parent
		T* pParent = Node.GetParent();
		if ( pParent )
			Message.ExportData( TRef_Static(P,a,r,e,n), pParent->GetNodeRef() );

		PublishMessage(Message);
	}
}


//---------------------------------------------------------
//	called after node has been moved in the graph
//---------------------------------------------------------
template<class T>
void TLGraph::TGraph<T>::OnNodeMoved(T& Node,T& OldParentNode)
{
	//	make up a moved node notificaiton
	TLMessaging::TMessage Message( TRef_Static(N,o,d,e,M), GetGraphRef() );
	Message.Write( Node.GetNodeRef() );
	Message.ExportData( TRef_Static4(N,o,d,e), Node.GetNodeRef() );
	
	Message.ExportData("OldParent", OldParentNode.GetNodeRef() );
	Message.ExportData("NewParent", Node.GetParent()->GetNodeRef() );

	PublishMessage(Message);
}

//---------------------------------------------------------
//	send message to node
//---------------------------------------------------------
template<class T>
Bool TLGraph::TGraph<T>::SendMessageToNode(TRefRef NodeRef,TLMessaging::TMessage& Message)
{
	//	get node to send message to
	T* pNode = FindNode( NodeRef );
	if ( !pNode )
	{
#ifdef _DEBUG
		TDebugString Debug_String;
		Debug_String << "Sent message to a node (" << NodeRef << ") that doesn't exist... add to a lost/queue for later queue?";
#endif
		return FALSE;
	}

	//	queue message
	if ( !pNode->QueueMessage( Message ) )
		return FALSE;

	return TRUE;
}
