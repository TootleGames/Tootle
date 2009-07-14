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
#include <TootleGame/TWidgetDrag.h>


namespace TLGame
{
	class TSchemeEditor;
}



//----------------------------------------------
//	customisable scheme editor, currently to
//	manipulate just scene nodes, but designing it to manipulate all node types
//----------------------------------------------
class TLGame::TSchemeEditor : public TLMessaging::TSubscriber, public TLMessaging::TPublisher
{
public:
	TSchemeEditor();
	~TSchemeEditor();

	Bool						Initialise(TRefRef EditorScheme,TRefRef GraphRef,TRefRef SchemeRootNode,TRefRef GameRenderTarget);

protected:
	virtual void				ProcessMessage(TLMessaging::TMessage& Message);		//	

	Bool						CreateEditorGui(TRefRef EditorScheme);				//	create render target, widgets, icons etc
	void						CreateEditorWidget(TBinaryTree& WidgetData);		//	create a widget from scheme XML

	void						CreateNodeWidgets(TLGraph::TGraphNodeBase& Node);		//	create a node widget to allow us to drag around in-game nodes. recurses down the tree
	void						OnNodeSelected(TRefRef NodeRef);						//	node has been selected
	void						OnNodeDrag(TRefRef NodeRef,const float3& DragAmount);	//	node has been dragged
	void						OnNodeUnselected(TRefRef NodeRef);						//	node has been selected

	void						UnselectAllNodes();			//	unselect all nodes

private:
	TRef							m_EditorRenderTarget;	//	render target of our editor
	TRef							m_EditorRenderNodeRef;	//	root node for our editors render target
	TPtrArray<TLGui::TWidget>		m_EditorWidgets;		//	widgets to drag from editor to game, and general UI widgets
	TPtrArray<TBinaryTree>			m_NewNodeData;			//	data's from the editor scheme dictating what nodes we can create

	TPtr<TLGraph::TGraphBase>		m_pGraph;				//	which graph are we modifying
	TRef							m_SchemeRootNode;		//	root node to modify - we manipulate all the nodes below this
	TRef							m_GameRenderTarget;		//	render target that the user is viewing the game through
	TPtrArray<TLGui::TWidgetDrag>	m_NodeWidgets;			//	widgets to manipulate existing nodes

	TArray<TRef>					m_SelectedNodes;		//	nodes selected
};


