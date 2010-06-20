/*------------------------------------------------------

	Scheme editor - this is the base editor that handles
	having a menu (but menu/interface is controlled by assets
	specified by a game) and dragging and dropping new objects
	into the game and manipulating existing objects
	
	The editor sits in it's own render target, but can manipulate
	ortho and projection game render targets

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TSubscriber.h>
#include <TootleCore/TGraphBase.h>
#include <TootleCore/TStateMachine.h>
#include <TootleGame/TWidgetDrag.h>
#include <TootleGame/TMenu.h>
#include <TootleRender/TRenderTarget.h>


namespace TLGame
{
	class TSchemeEditor;
}



//----------------------------------------------
//	customisable scheme editor, currently to
//	manipulate just scene nodes, but designing it to manipulate all node types
//----------------------------------------------
class TLGame::TSchemeEditor : public TLMessaging::TPublisherSubscriber, public TLMessaging::TMessageQueue, public TStateMachine
{
protected:
	class Mode_Base;	
	class Mode_Idle;			//	non-specific mode
	class Mode_Node;			//	mode where in-game objects can be dragged around
	class Mode_Icon;			//	mode where icons are visible

public:
	TSchemeEditor();
	virtual ~TSchemeEditor();

	Bool						Initialise(TRefRef EditorScheme,TRefRef GraphRef,TRefRef SchemeRootNode,TRefRef GameRenderTarget,TBinaryTree* pCommonNodeData);
	virtual void				Update(float Timestep);
	
	FORCEINLINE TRefRef						GetEditorRenderTargetRef() const	{	return m_EditorRenderTarget;	}
	FORCEINLINE TRefRef						GetEditorRenderNodeRef() const		{	return m_EditorRenderNodeRef;	}
	FORCEINLINE TLRender::TRenderTarget*	GetGameRenderTarget() const			{	return m_pGameRenderTarget;	}
	FORCEINLINE TRef						GetGameRenderTargetRef() const		{	TLRender::TRenderTarget* pRenderTarget = GetGameRenderTarget();	return pRenderTarget ? pRenderTarget->GetRef() : TRef();	}
	FORCEINLINE TRef						GetGameRenderNodeRef() const		{	TLRender::TRenderTarget* pRenderTarget = GetGameRenderTarget();	return pRenderTarget ? pRenderTarget->GetRootRenderNodeRef() : TRef();	}

protected:
	
	virtual void				ProcessMessageFromQueue(TLMessaging::TMessage& Message)			{	ProcessMessage( Message );	}

	virtual TRefRef				GetSubscriberRef() const							{	static TRef Ref("ScEditor");	return Ref;	}
	virtual void				ProcessMessage(TLMessaging::TMessage& Message);		//	
	virtual void				AddStateModes();									//	add state machine modes here. overload to add custom modes
	void						ChangeMode(TRefRef NewMode)							{	m_NextMode = NewMode;	}	//	on next update change mode
	Bool						GetGameWorldPosFromScreenPos(float3& WorldPos,const int2& ScreenPos,float ViewDepth=0.f);	//	get world pos in-game from cursor

	void						EnableNodeWidgets(Bool Enable);							//	enable/disable node widgets
	virtual void				ProcessNodeMessage(TRefRef NodeRef,TRefRef ActionRef,TLMessaging::TMessage& Message);		//	handle a [widget]message from a game node
	FORCEINLINE Bool			HasNewNodesSelected() const							{	return m_NewSceneNodeDragAction.IsValid() && m_SelectedNodes.GetSize() > 0;	}
	void						DropNewNodes()										{	if ( HasNewNodesSelected() )	DropNewNode( m_SelectedNodes );	}
	Bool						SelectNode(TRefRef NodeRef);						//	select a node - returns true if NEWLY selected. if it was alreayd selected, false is returned
	TRef						SelectSingleNode();									//	make sure only one TRIGGER node is selected. returns the now selected node (if any)
	void						UnselectNode(TRef NodeRef);							//	unselect a node
	void						UnselectNode(TArray<TRef>& NodeRefs);				//	unselect a list of nodes
	void						UnselectAllNodes();									//	unselect all nodes
	FORCEINLINE Bool			HasSelectedNodes() const							{	return m_SelectedNodes.GetSize() > 0;	}
	virtual void				OnNodeSelected(TRefRef NodeRef);					//	called when a node is selected
	virtual void				OnNodeUnselected(TRefRef NodeRef);					//	called when a node is unselected
	virtual void				OnNewNodeDropped(TRefRef NodeRef)					{	}	//	called when a new node has been dropped
	virtual void				OnNodeDeleted(TArray<TRef>& NodeRefs)				{	}	//	scene nodes have been removed from the graph and removed from our system
	virtual void				OnNodeClickReleased(TRefRef NodeRef, Bool WasDragged);		//	overloadable behaviour when mouse is released from a node

	virtual void				ProcessIconMessage(TPtr<TBinaryTree>& pIconData,TRefRef ActionRef,TLMessaging::TMessage& Message);		//	handle a [widget]message from a editor icon
	virtual TRef				InstanceIconNode(TPtr<TBinaryTree>& pIconData, TLMessaging::TMessage& InitMessage);
	void						EnableIconWidgets(Bool Enable);												//	enable/disable node widgets
	virtual void				CreateEditorIcons(TRefRef ParentRenderNode);								//	create icons for the editor - overload this to render your own icons
	void						OnCreatedIconRenderNode(TRefRef IconRenderNodeRef,TBinaryTree& IconData);	//	internal UI setup when an icon render node is created
	TPtr<TLAsset::TMenu>&		GetEditorIconMenu()															{	return m_pEditorIconMenu;	}

	virtual void				ProcessMouseMessage(TRefRef ActionRef,TLMessaging::TMessage& Message,Bool IsClickAction);		//	handle mouse messages 

	virtual Bool				ProcessCommandMessage(TRefRef CommandRef,TLMessaging::TMessage& Message);	//	handle other messages (assume are commands)
	virtual void				ClearScheme();				//	remove all nodes

	void						Debug_GetSelectedNodeRefStrings(TString& String);	//	get all the selected node refs as strings

private:
	Bool						CreateEditorGui(TRefRef EditorScheme);				//	create render target, widgets, icons etc
	void						CreateEditorWidget(TBinaryTree& WidgetData);		//	create a widget from scheme XML

	void						CreateNodeWidgets(TLGraph::TGraphNodeBase& Node);		//	create nodes widget to allow us to drag around in-game nodes. recurses down the tree, decrementing RecurseLevels until it's zero, at which point it doesn't create nodes for the children
	void						RemoveNodeWidget(TRefRef NodeRef);						//	remove widget for this node
	void						OnNodeDrag(TRefRef NodeRef,const float3& DragAmount);	//	node has been dragged
	void						DropNewNode(TRefRef NodeRef);							//	drop a node into the game
	void						DropNewNode(TArray<TRef>& NodeArray);					//	drop a bunch of nodes (probbaly selected nodes)

	void						DeleteNode(TRefRef NodeRef);							//	delete a node and it's widgets

	void						OnGraphMessage(TLMessaging::TMessage& Message);			//	handle graph change from our graph

protected:
	TRef							m_NextMode;				//	if set then on next update we will change mode

	TRef							m_EditorRenderTarget;	//	render target of our editor
	TRef							m_EditorRenderNodeRef;	//	root node for our editors render target
	TRef							m_EditorIconRootNodeRef;	//	node to put icons under
	THeapArray<TRef>				m_EditorWidgets;		//	general UI widgets
	THeapArray<TRef>				m_EditorIconWidgets;	//	widgets to drag from editor to game
	TPtr<TLAsset::TMenu>			m_pEditorIconMenu;		//	icons in menu form with NewNode data attached

	TPtr<TLGraph::TGraphBase>		m_pGraph;				//	which graph are we modifying
	TBinaryTree						m_CommonNodeData;		//	data attached to the init of all nodes we create
	TRef							m_SchemeRootNode;		//	root node to modify - we manipulate all the nodes below this
	TRef							m_GameRenderTarget;		//	render target that the user is viewing the game through
	TPtr<TLRender::TScreen>			m_pGameScreen;			//	cache ptr to game's screen which contains render target
	TPtr<TLRender::TRenderTarget>	m_pGameRenderTarget;	//	cache ptr to game's render target
	TPtrArray<TLGui::TWidgetDrag>	m_NodeWidgets;			//	widgets to manipulate existing nodes
	TRef							m_NewSceneNodeDragAction;	//	action for the mouse to look out for when we are dragging a new scene node
	TRef							m_NewSceneNodeClickAction;	//	action for the mouse to look out for when we are dragging a new scene node

	THeapArray<TRef>				m_SelectedNodes;		//	nodes selected
};





class TLGame::TSchemeEditor::Mode_Base : public TStateMode
{
protected:
	TLGame::TSchemeEditor&		GetEditor()		{	return *GetStateMachine<TLGame::TSchemeEditor>();	}
};


class TLGame::TSchemeEditor::Mode_Idle : public TLGame::TSchemeEditor::Mode_Base
{
protected:
	virtual Bool			OnBegin(TRefRef PreviousMode);
};


class TLGame::TSchemeEditor::Mode_Node : public TLGame::TSchemeEditor::Mode_Base
{
protected:
	virtual Bool			OnBegin(TRefRef PreviousMode);
	virtual void			OnEnd(TRefRef NextMode);
};


class TLGame::TSchemeEditor::Mode_Icon : public TLGame::TSchemeEditor::Mode_Base
{
protected:
	virtual Bool			OnBegin(TRefRef PreviousMode);	
};





