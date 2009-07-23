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


namespace TLGame
{
	class TSchemeEditor;
}



//----------------------------------------------
//	customisable scheme editor, currently to
//	manipulate just scene nodes, but designing it to manipulate all node types
//----------------------------------------------
class TLGame::TSchemeEditor : public TLMessaging::TSubscriber, public TLMessaging::TPublisher, public TStateMachine
{
protected:
	class Mode_Base;	
	class Mode_NodeEditor;		//	mode where icons are visible and in-game objects can be dragged around

public:
	TSchemeEditor();
	virtual ~TSchemeEditor();

	Bool						Initialise(TRefRef EditorScheme,TRefRef GraphRef,TRefRef SchemeRootNode,TRefRef GameRenderTarget,TBinaryTree* pCommonNodeData);

protected:
	virtual void				ProcessMessage(TLMessaging::TMessage& Message);		//	
	virtual void				AddStateModes();									//	add state machine modes here. overload to add custom modes

	Bool						CreateEditorGui(TRefRef EditorScheme);				//	create render target, widgets, icons etc
	void						CreateEditorWidget(TBinaryTree& WidgetData);		//	create a widget from scheme XML
	void						OnEditorRenderNodeAdded();							//	editor render node is ready to be used

	void						CreateNodeWidgets(TLGraph::TGraphNodeBase& Node);		//	create nodes widget to allow us to drag around in-game nodes. recurses down the tree
	virtual void				ProcessNodeMessage(TRefRef NodeRef,TRefRef ActionRef,TLMessaging::TMessage& Message);		//	handle a [widget]message from a game node
	void						OnNodeSelected(TRefRef NodeRef);						//	node has been selected
	void						OnNodeDrag(TRefRef NodeRef,const float3& DragAmount);	//	node has been dragged
	void						OnNodeUnselected(TRefRef NodeRef);						//	node has been selected
	void						EnableNodeWidgets(Bool Enable);							//	enable/disable node widgets
	Bool						GetGameWorldPosFromScreenPos(float3& WorldPos,const int2& ScreenPos,float ViewDepth=0.f);	//	get world pos in-game from cursor

	virtual void				ProcessIconMessage(TPtr<TBinaryTree>& pIconData,TRefRef ActionRef,TLMessaging::TMessage& Message);		//	handle a [widget]message from a editor icon
	void						CreateEditorIcons();									//	create icons for the editor

	virtual void				ProcessMouseMessage(TRefRef ActionRef,TLMessaging::TMessage& Message,Bool IsClickAction);		//	handle mouse messages 
	void						UnselectAllNodes();			//	unselect all nodes
	virtual void				ClearScheme();				//	remove all nodes

	virtual void				ProcessCommandMessage(TRefRef CommandRef,TLMessaging::TMessage& Message);	//	handle other messages (assume are commands)

protected:
	TRef							m_EditorRenderTarget;	//	render target of our editor
	TRef							m_EditorRenderNodeRef;	//	root node for our editors render target
	TRef							m_EditorIconRootNodeRef;	//	node to put icons under
	TPtrArray<TLGui::TWidget>		m_EditorWidgets;		//	widgets to drag from editor to game, and general UI widgets
	TPtrArray<TBinaryTree>			m_NewNodeData;			//	data's from the editor scheme dictating what nodes we can create

	TPtr<TLGraph::TGraphBase>		m_pGraph;				//	which graph are we modifying
	TBinaryTree						m_CommonNodeData;		//	data attached to the init of all nodes we create
	TRef							m_SchemeRootNode;		//	root node to modify - we manipulate all the nodes below this
	TRef							m_GameRenderTarget;		//	render target that the user is viewing the game through
	TPtr<TLRender::TScreen>			m_pGameScreen;			//	cache ptr to game's screen which contains render target
	TPtr<TLRender::TRenderTarget>	m_pGameRenderTarget;	//	cache ptr to game's render target
	TPtrArray<TLGui::TWidgetDrag>	m_NodeWidgets;			//	widgets to manipulate existing nodes
	TRef							m_NewSceneNode;			//	ref of the new in-game node we've created from an icon and currently dragging into the game
	TRef							m_NewSceneNodeDragAction;	//	action for the mouse to look out for when we are dragging a new scene node
	TRef							m_NewSceneNodeClickAction;	//	action for the mouse to look out for when we are dragging a new scene node

	TArray<TRef>					m_SelectedNodes;		//	nodes selected
};





class TLGame::TSchemeEditor::Mode_Base : public TStateMode
{
protected:
	TLGame::TSchemeEditor&		GetEditor()		{	return *GetStateMachine<TLGame::TSchemeEditor>();	}
};


class TLGame::TSchemeEditor::Mode_NodeEditor : public TLGame::TSchemeEditor::Mode_Base
{
protected:
	virtual Bool			OnBegin(TRefRef PreviousMode)		{	GetEditor().EnableNodeWidgets(TRUE);	return TRUE;	}
	virtual void			OnEnd(TRefRef NextMode)				{	GetEditor().EnableNodeWidgets(FALSE);	}
};

