#include "TSchemeEditor.h"
#include <TootleRender/TScreenManager.h>
#include <TootleGame/TWidgetButton.h>
#include <TootleGame/TWidgetManager.h>
#include <TootleInput/TUser.h>
#include <TootleScene/TScenegraph.h>
#include <TootleRender/TRendergraph.h>
#include <TootleRender/TRenderTarget.h>



#define ENABLE_ICON_WIDGETS
//#define DEBUG_NODE_INTERACTION		//	prints out drags, selection etc


TLGame::TSchemeEditor::TSchemeEditor() :
	m_CommonNodeData	( TRef() )
{
}

TLGame::TSchemeEditor::~TSchemeEditor()
{
	//	unselect all the nodes - need to do this to send out the end-edit message for node cleanups
	UnselectAllNodes();

	//	delete render target
	if ( m_EditorRenderTarget.IsValid() )
	{
		TLRender::g_pScreenManager->DeleteRenderTarget( m_EditorRenderTarget );
		m_EditorRenderTarget.SetInvalid();
	}

	//	delete our nodes
	TLRender::g_pRendergraph->RemoveNode( m_EditorRenderNodeRef );

	//	release ptrs
	m_pGraph = NULL;
}



//----------------------------------------------------------
//	
//----------------------------------------------------------
Bool TLGame::TSchemeEditor::Initialise(TRefRef EditorScheme,TRefRef GraphRef,TRefRef SchemeRootNode,TRefRef GameRenderTarget,TBinaryTree* pCommonNodeData)
{
	//	copy node params
	if ( pCommonNodeData )
		m_CommonNodeData.ReferenceDataTree( *pCommonNodeData );

	//	check params
	if ( !SchemeRootNode.IsValid() )
	{
		TLDebug_Break("Valid root node for the editor expected");
		return FALSE;
	}

	//	fetch graph 
	//	gr: no type checking atm, have to assume this is a graph...
	m_pGraph = TLCore::g_pCoreManager->GetManagerPtr<TLGraph::TGraphBase>( GraphRef );
	if ( !m_pGraph )
	{
		TLDebug_Break("Initialised scheme editor with graph ref that doesnt exist");
		return FALSE;
	}
	
	//	catch node creation from this graph
	this->SubscribeTo( m_pGraph );

	//	set vars
	m_SchemeRootNode = SchemeRootNode;
	m_GameRenderTarget = GameRenderTarget;

	//	pre-fetch render target
	m_pGameRenderTarget = TLRender::g_pScreenManager->GetRenderTarget( m_GameRenderTarget, m_pGameScreen );
	if ( !m_pGameRenderTarget )
	{
		TLDebug_Break("game's render target expected");
		return FALSE;
	}

	TLGraph::TGraphNodeBase* pRootNode = m_pGraph->FindNodeBase( m_SchemeRootNode );
	if ( !pRootNode )
	{
		TLDebug_Break("Missing root scheme node for editor - need to add async functionality?");
		return FALSE;
	}

	//	create node widgets so we can interact with the current scene
	CreateNodeWidgets( *pRootNode );

	//	create editor UI to add new nodes, stats etc
	if ( EditorScheme.IsValid() )
	{
		if ( !CreateEditorGui( EditorScheme ) )
			return FALSE;
	}

	//	init modes, first one added will be active one, this allows overloaded editors to dictate where to start
	AddStateModes();

	return TRUE;
}


//----------------------------------------------------------
//	
//----------------------------------------------------------
void TLGame::TSchemeEditor::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	input/widget input
	if(Message.GetMessageRef() == TRef_Static(A,c,t,i,o))
	{
		TRef ActionRef;
		Message.ResetReadPos();
		if ( !Message.Read(ActionRef) )
			return;
		
		TRef Ref;
		if ( Message.ImportData("Node", Ref) )
		{
			ProcessNodeMessage( Ref, ActionRef, Message );
			return;
		}

		TPtr<TBinaryTree>& pIconData = Message.GetChild("Icon");
		if ( pIconData )
		{
			ProcessIconMessage( pIconData, ActionRef, Message );
			return;
		}
		
		if ( ActionRef == m_NewSceneNodeDragAction && m_NewSceneNodeDragAction.IsValid() )
		{
			ProcessMouseMessage( ActionRef, Message, FALSE );
			return;
		}
		else if ( ActionRef == m_NewSceneNodeClickAction && m_NewSceneNodeClickAction.IsValid() )
		{
			ProcessMouseMessage( ActionRef, Message, TRUE );
			return;
		}

		//	catch mouse actions
		if ( TLGui::g_pWidgetManager->IsClickActionRef( ActionRef ) )
		{
			ProcessMouseMessage( ActionRef, Message, TRUE );
			return;
		}
		else if ( TLGui::g_pWidgetManager->IsMoveActionRef( ActionRef ) )
		{
			ProcessMouseMessage( ActionRef, Message, FALSE );
			return;
		}
		else
		{
			ProcessCommandMessage( ActionRef, Message );
			return;
		}
	}

	//	graph message
	if ( Message.GetSenderRef() == m_pGraph->GetGraphRef() )
	{
		OnGraphMessage( Message );
	}
}


//----------------------------------------------------------
//	create a node widget to allow us to drag around in-game nodes
//	recurses down the tree
//----------------------------------------------------------
void TLGame::TSchemeEditor::CreateNodeWidgets(TLGraph::TGraphNodeBase& Node)
{
	//	get the render node of the scene node
	TRefRef RenderNodeRef = Node.GetRenderNodeRef();
	if ( RenderNodeRef.IsValid() )
	{
		Bool CreateWidget = TRUE;

		//	make sure we don't already have a widget for this render node as that could cause problems
		for ( u32 w=0;	w<m_NodeWidgets.GetSize();	w++ )
		{
			TPtr<TLGui::TWidgetDrag>& pWidget = m_NodeWidgets[w];
			if ( pWidget->GetRenderNodeRef() == RenderNodeRef )
			{
				CreateWidget = FALSE;
				break;
			}
		}

		if ( CreateWidget )
		{
			//	export the node information so when we get the widget callback we know what node it's for in this graph
			TBinaryTree WidgetData( TRef_Invalid );	//	ref irrelavant
			WidgetData.ExportData("Node", Node.GetNodeRef() );

			TPtr<TLGui::TWidgetDrag> pWidget = new TLGui::TWidgetDrag( m_GameRenderTarget, RenderNodeRef, "global", "NDown", "NUp", "NDrag", &WidgetData );
			m_NodeWidgets.Add( pWidget );

			//	subscribe to widget to get the actions
			this->SubscribeTo( pWidget );
		}
	}

	//	subscribe to this node to get changes to the render node
	this->SubscribeTo( &Node );

	//	create widgets for children
	TArray<TLGraph::TGraphNodeBase*> ChildNodes;
	Node.GetChildrenBase( ChildNodes );
	for ( u32 c=0;	c<ChildNodes.GetSize();	c++ )
	{
		TLGraph::TGraphNodeBase* pChild = ChildNodes.ElementAt(c);
		CreateNodeWidgets( *pChild );
	}
}


//----------------------------------------------------------
//	create render target, widgets, icons etc
//----------------------------------------------------------
Bool TLGame::TSchemeEditor::CreateEditorGui(TRefRef EditorScheme)
{
	//	without a scheme, we don't create an interface
	if ( !EditorScheme.IsValid() )
		return FALSE;

	TLAsset::TScheme* pEditorScheme = TLAsset::GetAsset<TLAsset::TScheme>( EditorScheme );
	if ( !pEditorScheme )
	{
		TLDebug_Break("failed to load editor scheme");
		return FALSE;
	}

	//	create render target
	if ( !m_EditorRenderTarget.IsValid() )
	{
		TPtr<TLRender::TScreen>& pScreen = TLRender::g_pScreenManager->GetDefaultScreen();
		TPtr<TLRender::TRenderTarget> pRenderTarget = pScreen->CreateRenderTarget( EditorScheme.IsValid() ? EditorScheme : TRef("Editor") );
		if ( !pRenderTarget )
			return FALSE;
		m_EditorRenderTarget = pRenderTarget->GetRef();

		pRenderTarget->SetClearColour( TColour( 0.f, 0.f, 0.f, 0.0f ) );
		pRenderTarget->SetEnabled( TRUE );

		//	setup camera
		TPtr<TLRender::TCamera> pCamera = new TLRender::TOrthoCamera();
		pCamera->SetPosition( float3( 0.f, 0.f, -20.f ) );
		pRenderTarget->SetCamera( pCamera );

		//	create root render node
		if ( !m_EditorRenderNodeRef.IsValid() )
		{
			TLMessaging::TMessage Message( TLCore::InitialiseRef );
			Message.ExportData("RFClear", TLRender::TRenderNode::RenderFlags::EnableCull );
			m_EditorRenderNodeRef = TLRender::g_pRendergraph->CreateNode("EditorRoot", TRef(), TRef(), &Message );
			pRenderTarget->SetRootRenderNode( m_EditorRenderNodeRef );

			//	catch when the editor render node is created so we can create icons etc
			this->SubscribeTo( TLRender::g_pRendergraph );
		}

		//	subscribe to user's (mouse) actions
		TPtr<TLUser::TUser>& pUser = TLUser::g_pUserManager->GetUser("global");
		if ( pUser )
		{
			this->SubscribeTo( pUser );
		}
		else
		{
			TLDebug_Break("missing user to subscribe to actions");
		}
	}

	//	instance the scheme
	if ( !TLRender::g_pRendergraph->ImportScheme( pEditorScheme, m_EditorRenderNodeRef ) )
		return FALSE;

	//	read in data from the scheme
	u32 i;

	//	setup widgets
	TPtrArray<TBinaryTree> WidgetDatas;
	pEditorScheme->GetData().GetChildren("Widget",WidgetDatas);
	for ( i=0;	i<WidgetDatas.GetSize();	i++ )
		CreateEditorWidget( *WidgetDatas[i] );

	//	setup list of node types we can create
	TPtrArray<TBinaryTree> NewNodeDatas;			//	data's from the editor scheme dictating what nodes we can create
	if ( pEditorScheme->GetData().GetChildren("NewNode", NewNodeDatas ) )
	{
		m_pEditorIconMenu = new TLAsset::TMenu("Icons");
		for ( i=0;	i<NewNodeDatas.GetSize();	i++ )
		{
			TPtr<TBinaryTree>& pNewNodeData = NewNodeDatas[i];
			TLAsset::TMenu::TMenuItem* pMenuItem = m_pEditorIconMenu->AddMenuItem();
			if ( !pMenuItem )
			{
				TLDebug_Break("failed to add menu item");
				continue;
			}

			TRef TypeOrSchemeRef;
			if ( !pNewNodeData->ImportData("Type", TypeOrSchemeRef) && !pNewNodeData->ImportData("Scheme", TypeOrSchemeRef) )
			{
				TLDebug_Break("Icon data requires a SceneNode Type or a Scheme - otherwise we don't know what to create");
				continue;
			}

			//	setup menu item from data
			TTempString MenuItemString;
			if ( !pNewNodeData->ImportDataString("Name", MenuItemString ) )
				TypeOrSchemeRef.GetString( MenuItemString, TRUE );
			pMenuItem->SetString( MenuItemString );

			//	add the new node data to the menu item's data (named "newnode")
			pMenuItem->GetData().AddChild( pNewNodeData );
		}
	}

	//	what node should we put icons under?
	pEditorScheme->GetData().ImportData("IconRoot", m_EditorIconRootNodeRef );

	return TRUE;
}


//----------------------------------------------------------
//	create a widget from scheme XML
//----------------------------------------------------------
void TLGame::TSchemeEditor::CreateEditorWidget(TBinaryTree& WidgetData)
{
	//	todo: need a "widget is valid" function
	TPtr<TLGui::TWidget> pWidget = new TLGui::TWidgetButton( m_EditorRenderTarget, WidgetData );
	
	this->SubscribeTo( pWidget );
	m_EditorWidgets.Add( pWidget );
}


//----------------------------------------------------------
//	select a node - returns true if NEWLY selected. if it was alreayd selected, false is returned
//----------------------------------------------------------
Bool TLGame::TSchemeEditor::SelectNode(TRefRef SceneNode)
{
	//	dont do anything if already selected
	if ( m_SelectedNodes.Exists( SceneNode ) )
		return FALSE;
	
	#ifdef DEBUG_NODE_INTERACTION
		TTempString Debug_String("Selecting node ");
		SceneNode.GetString( Debug_String );
		TLDebug_Print( Debug_String );
	#endif

	//	add to list
	m_SelectedNodes.Add( SceneNode );
	
	//	notify
	TLMessaging::TMessage EditMessage("EdtStart");
	m_pGraph->SendMessageToNode( SceneNode, EditMessage );

	//	go into node mode if we're not already in it
	if ( GetCurrentModeRef() != "Node" && !IsChangingMode() )
		SetMode("Node");

	OnNodeSelected( SceneNode );

	return TRUE;
}


//----------------------------------------------------------
//	make sure only one TRIGGER node is selected
//----------------------------------------------------------
TRef TLGame::TSchemeEditor::SelectSingleNode()
{
	TRef KeepSelected;

	for ( s32 i=m_SelectedNodes.GetLastIndex();	i>=0;	i-- )
	{
		if ( !KeepSelected.IsValid() )
		{
			KeepSelected = m_SelectedNodes[i];
		}
		else
		{
			UnselectNode( m_SelectedNodes[i] );
		}
	}

	return KeepSelected;
}


//----------------------------------------------------------
//	node has been selected
//----------------------------------------------------------
void TLGame::TSchemeEditor::UnselectNode(TRef SceneNode)
{
	#ifdef DEBUG_NODE_INTERACTION
		TTempString Debug_String("Unselecting node ");
		SceneNode.GetString( Debug_String );
		TLDebug_Print( Debug_String );
	#endif

	if ( !m_SelectedNodes.Remove( SceneNode ) )
		return;
	
	TLMessaging::TMessage EditMessage("EdtEnd");
	m_pGraph->SendMessageToNode( SceneNode, EditMessage );

	//	if no more selected nodes then unset the new-node drag actions
	if ( m_SelectedNodes.GetSize() == 0 )
	{
		//	nothing selected, if in node mode, switch to idle mode
		//	gr: enabling this in rare cases can cause two mdoe switches in one frame which causes issues when 
		//	instancing schemes in graphs
	//	if ( GetCurrentModeRef() == "Node" )
	//		SetMode("Idle");
	}

	//	notify of unselection
	OnNodeUnselected( SceneNode );
}


//----------------------------------------------------------
//	called when a node is unselected
//----------------------------------------------------------
void TLGame::TSchemeEditor::OnNodeUnselected(TRefRef NodeRef)
{
}

//----------------------------------------------------------
//	called when a node is selected
//----------------------------------------------------------
void TLGame::TSchemeEditor::OnNodeSelected(TRefRef NodeRef)
{
}


//----------------------------------------------------------
//	node has been dragged
//----------------------------------------------------------
void TLGame::TSchemeEditor::OnNodeDrag(TRefRef SceneNode,const float3& DragAmount)
{
	TLMessaging::TMessage TransformMessage("DoTransform");
	TransformMessage.ExportData("Translate", DragAmount );

#ifdef DEBUG_NODE_INTERACTION
	TTempString Debug_String("Dragging node ");
	SceneNode.GetString( Debug_String );
	Debug_String.Appendf("; %2.2f, %2.2f, %2.2f", DragAmount.x, DragAmount.y, DragAmount.z );
	TLDebug_Print( Debug_String );
#endif
	
	m_pGraph->SendMessageToNode( SceneNode, TransformMessage );
}

	
//----------------------------------------------------------
//	unselect all nodes
//----------------------------------------------------------
void TLGame::TSchemeEditor::UnselectNode(TArray<TRef>& NodeRefs)
{
	for ( s32 i=NodeRefs.GetLastIndex();	i>=0;	i-- )
	{
		UnselectNode( NodeRefs[i] );
	}
}

//----------------------------------------------------------
//	unselect all nodes
//----------------------------------------------------------
void TLGame::TSchemeEditor::UnselectAllNodes()
{
	UnselectNode( m_SelectedNodes );
	m_NewSceneNodeDragAction.SetInvalid();
	m_NewSceneNodeClickAction.SetInvalid();
}


//----------------------------------------------------------
//	remove all nodes
//----------------------------------------------------------
void TLGame::TSchemeEditor::ClearScheme()
{
	UnselectAllNodes();

	m_pGraph->RemoveChildren( m_SchemeRootNode );
}



//----------------------------------------------------------
//	handle a [widget]message from a game node
//----------------------------------------------------------
void TLGame::TSchemeEditor::ProcessNodeMessage(TRefRef NodeRef,TRefRef ActionRef,TLMessaging::TMessage& Message)
{
	if ( ActionRef == "NDrag" )
	{
		//	if this node hasn't been selected before, select this node and unselect everything else
		if ( SelectNode( NodeRef ) )
		{
			for ( s32 i=m_SelectedNodes.GetSize();	i>=0;	i-- )
				if ( m_SelectedNodes[i] != NodeRef )
					UnselectNode( m_SelectedNodes[i] );
		}

		//	get world change
		float3 WorldChange;
		if ( Message.ImportData("Move3",WorldChange ) )
		{
			OnNodeDrag( NodeRef, WorldChange );
		}
	}
	else if ( ActionRef == "NDown" )
	{
		//	gr: selection/unselection is done when mouse is released to determine if we need to unselect or select depending on whether we dragged or not
		//	but, if we're in icon mode we have touched a node rather than the menu, so switch to node-editor mode
		if ( GetCurrentModeRef() == "Icon" )
		{
			//	gr: switch to idle mode so the node menu doesn't pop up, we'll switch to node mode when we actually select a node
			SetMode("Idle");
			//SetMode("Node");
		}
	}
	else if ( ActionRef == "NUp" )
	{
		//	gr: only unselect the node when mouse is release if we did some dragging. If we have simply touched the node withot moving it
		//		then we can assume the user explicitly tried to select it
		Bool WasDragged = FALSE;
		Message.ImportData("DidDrag", WasDragged );
		if ( WasDragged )
		{
			//UnselectNode( NodeRef );
			//SetMode("Idle");
		}
		else if ( GetCurrentModeRef() == "Node" || GetCurrentModeRef() == "Idle" )
		{
			//	 wasn't dragged, and isn't selected, so select it (assume is a tap to select)
			if ( SelectNode( NodeRef ) )
			{
				for ( s32 i=m_SelectedNodes.GetSize();	i>=0;	i-- )
					if ( m_SelectedNodes[i] != NodeRef )
						UnselectNode( m_SelectedNodes[i] );
			}
			else
			{
				//	was already selected, so unselect
				UnselectNode( NodeRef );
				SetMode("Idle");
			}
		}
	}
}

	
//----------------------------------------------------------
//	get world pos in-game from cursor
//----------------------------------------------------------
Bool TLGame::TSchemeEditor::GetGameWorldPosFromScreenPos(float3& WorldPos,const int2& ScreenPos,float ViewDepth)
{
	if ( !m_pGameScreen || !m_pGameRenderTarget )
		return FALSE;

	return m_pGameScreen->GetWorldPosFromScreenPos( *m_pGameRenderTarget, WorldPos, ViewDepth, ScreenPos );
}


//----------------------------------------------------------
//	handle a [widget]message from a editor icon
//----------------------------------------------------------
void TLGame::TSchemeEditor::ProcessIconMessage(TPtr<TBinaryTree>& pIconData,TRefRef ActionRef,TLMessaging::TMessage& Message)
{
	if ( ActionRef == "IcoDrag" && m_SelectedNodes.GetSize()==0 && GetCurrentModeRef() == "icon" )
	{
		//	create a new node at the mouse position - convert screen pos to game render target world pos
		int2 ScreenPos;
		if ( !Message.ImportData("Pos2", ScreenPos) )
			return;
		
		//	error, mouse is off screen or something
		float3 WorldPos;
		if ( !GetGameWorldPosFromScreenPos( WorldPos, ScreenPos ) )
			return;

		//	make up init data for the node (or root node in scheme case)
		TLMessaging::TMessage InitMessage(TLCore::InitialiseRef);
		InitMessage.ExportData("Translate", WorldPos );
		
		//	add common data
		InitMessage.ReferenceDataTree( m_CommonNodeData );
	
		//	add data specified in the editor scheme (xml)
		TPtr<TBinaryTree>& pInitData = pIconData->GetChild("Init");
		if ( pInitData )
			InitMessage.ReferenceDataTree( pInitData );

		//	create node either from type or importing scheme under it
		TRef Type;
		TRef NewSceneNode;
		if ( pIconData->ImportData("Type", Type) )
		{
			//	set this new node as the "new scene node" that we're dropping into the game.
			//	the editor's mouse controls take over now
			TRef BaseNodeRef = "EdNode";
			//TRef BaseNodeRef = Type;
			NewSceneNode = m_pGraph->CreateNode( BaseNodeRef, Type, m_SchemeRootNode, &InitMessage );
	
			TTempString Debug_String("Created new scene node ");
			NewSceneNode.GetString( Debug_String );
			Debug_String.Append(" from node TYPE ");
			Type.GetString( Debug_String );
			TLDebug_Print( Debug_String );
		}
		else if ( pIconData->ImportData("Scheme", Type) )
		{
			TRefRef SchemeRef = Type;
			TLAsset::TScheme* pScheme = TLAsset::GetAsset<TLAsset::TScheme>(SchemeRef);
			if ( !pScheme )
			{
				TTempString Debug_String("Failed to load scheme ");
				SchemeRef.GetString( Debug_String );
				Debug_String.Append(" for new icon-node");
				TLDebug_Break( Debug_String );
				return;
			}

			//	make a base node - still trying to decide if this is the best method
			TRef BaseNodeRef = "EdNode";
			//TRef BaseNodeRef = Type;
			NewSceneNode = m_pGraph->CreateNode( BaseNodeRef, "object", m_SchemeRootNode, &InitMessage );

			TTempString Debug_String("Created new scene node ");
			NewSceneNode.GetString( Debug_String );
			Debug_String.Append(" from SCEHEME ");
			SchemeRef.GetString( Debug_String );
			TLDebug_Print( Debug_String );

			//	import the scheme under neath it
			//	we do NOT use strict refs as the scheme is to be re-instanced...
			//	maybe move this option INTO the scheme XML itself?
			TLScene::g_pScenegraph->ImportScheme( *pScheme, NewSceneNode, FALSE, &m_CommonNodeData );
		}
		else 
		{
			TLDebug_Break("type or scheme expected");
			return;
		}		

		//	should have set this new scene node
		if ( !NewSceneNode.IsValid() )
		{
			TLDebug_Break("Failed to create new scene node");
			return;
		}

		//	unselect current nodes and select new nodes only
		UnselectAllNodes();

		//	set as selected - this also sends the EdtStart message to disable physics etc
		//	getting in the way of the drag
		//	gr: now just stored in selected node list
		SelectNode( NewSceneNode );

		//	store off the action to listen for, for the mouse
		if ( !Message.ImportData("InpAction", m_NewSceneNodeDragAction ) )
		{
			TLDebug_Break("Don't know what to look out for when dragging new scene node, handle me gracefully!");
			//UnselectNode( m_NewSceneNode );
			return;
		}

		if ( !TLGui::g_pWidgetManager->IsMoveActionRef( m_NewSceneNodeDragAction ) )
		{
			TTempString Debug_String("NewScene node drag action ref ");
			m_NewSceneNodeDragAction.GetString( Debug_String );
			Debug_String.Append(" is not a widget manager move action");
			TLDebug_Break( Debug_String );
		}

		//	get the equivelent mouse action
		m_NewSceneNodeClickAction = TLGui::g_pWidgetManager->GetClickActionFromMoveAction( m_NewSceneNodeDragAction );
		if ( !m_NewSceneNodeClickAction.IsValid() )
		{
			TLDebug_Break("Missing click action ref");
		}
	}
	
}


//----------------------------------------------------------
//	create icons for the editor - overload this to render your own icons
//----------------------------------------------------------
void TLGame::TSchemeEditor::CreateEditorIcons(TRefRef ParentRenderNode)
{
	//	no icon data
	if ( !m_pEditorIconMenu )
		return;
	TPtrArray<TLAsset::TMenu::TMenuItem>& IconMenuItems = m_pEditorIconMenu->GetMenuItems();

	float3 IconPosition( 0.f, 0.f, 5.f );
	float3 IconScale( 3.f, 3.f, 1.f );
	float3 IconPositionStep( 0.f, IconScale.y * 0.7f, 0.f );

	for ( u32 i=0;	i<IconMenuItems.GetSize();	i++ )
	{
		TLAsset::TMenu::TMenuItem& IconMenuItem = *IconMenuItems[i];
		TPtr<TBinaryTree>& pIconData = IconMenuItem.GetData().GetChild("NewNode");
		if ( !pIconData )
		{
			TLDebug_Break("Icon menu item is missing the \"NewNode\" data from the editor's scheme");
			continue;
		}

		//	create icon render node
		TLMessaging::TMessage InitMessage(TLCore::InitialiseRef);

		InitMessage.ExportData("scale", IconScale );

		TRef IconRef;
		TRef TypeRef;

		float3 FinalPositionStep = IconPositionStep;

		if ( pIconData->ImportData("IconRef", IconRef ) )
		{
			// Create a graphic
			InitMessage.ExportData("MeshRef", TRef("d_cube"));
			InitMessage.ExportData("TextureRef", IconRef);

			// For the sprite icons we need to move them to match the icons box.
			// This is because the iconroot will actually be at 0,0 and therefore the icons are 
			// relative to that.  For the text they are positioned using the 'boxdatum' which
			// effectively shifts them to the box pos in the graphic as their root pos.
			// There's no easy way to do this form here so going to do a bit of trial and error
			// for now. Eventually the icons will be used instead of the text so we can find 
			// a singular usable way of positioning the icons in the box correctly then
			float3 FinalPosition = float3(5.5f, 8.0f, 0.0f) + IconPosition;
			InitMessage.ExportData("translate", FinalPosition );

			// Get the size of the icon from the XML for the icon step so we move 
			// the next icon correctly. This is NOT in pixels.
			// NOTE: We could get the size in pixels from the editor xml and then  
			// scale that to determine the correct step for the perspective?  
			float2 IconSize(0,3.5f);
			pIconData->ImportData("Size", IconSize );
			FinalPositionStep.xy() = IconSize;
		}
		else
		{
			// Create a text item
			InitMessage.ExportData("translate", IconPosition );

			InitMessage.ExportDataString("string", IconMenuItem.GetString() );
			InitMessage.ExportData("FontRef", TRef("fdebug") );
			InitMessage.ExportData("boxdatum", TRef("Icons") );
			InitMessage.ExportData("boxnode", TRef("em_obj") );

			TypeRef = TRef("TxText");
		}

		TRef IconRenderNodeRef = TLRender::g_pRendergraph->CreateNode("Icon", TypeRef, ParentRenderNode, &InitMessage );

		//	do internal stuff
		OnCreatedIconRenderNode( IconRenderNodeRef, *pIconData );

		IconPosition += FinalPositionStep;
	}
}


//----------------------------------------------------------------------
//	internal UI setup when an icon render node is created
//----------------------------------------------------------------------
void TLGame::TSchemeEditor::OnCreatedIconRenderNode(TRefRef IconRenderNodeRef,TBinaryTree& IconData)
{
#ifdef ENABLE_ICON_WIDGETS
	//	create draggable widget on this icon
	TBinaryTree WidgetData("Widget");
	WidgetData.ExportData("Node", IconRenderNodeRef );
	WidgetData.ExportData("ActDown", TRef("IcoDown") );
	WidgetData.ExportData("ActDrag", TRef("IcoDrag") );
	WidgetData.ExportData("VertDrag", FALSE );

	//	custom data
	TPtr<TBinaryTree>& pWidgetIconData = WidgetData.AddChild("Icon");
	if ( pWidgetIconData )
	{
		//	mark all the icon data as unread so it will be added to the widget
		IconData.SetTreeUnread();

		//	add all the data specified in the XML including "init" data for the node when it's created
		//	this will include the "Type" or "Scheme" specification
		pWidgetIconData->ReferenceDataTree( IconData );
	}

	//	create widget
	TPtr<TLGui::TWidgetDrag> pWidget = new TLGui::TWidgetDrag( m_EditorRenderNodeRef, WidgetData );
	m_EditorIconWidgets.Add( pWidget );
	this->SubscribeTo( pWidget );
#endif
}


//----------------------------------------------------------
//	editor render node is ready to be used
//----------------------------------------------------------
void TLGame::TSchemeEditor::EnableIconWidgets(Bool EnableIcons)
{
	//	enable/disable icon widgets
	for ( u32 i=0;	i<m_EditorIconWidgets.GetSize();	i++ )
	{
		m_EditorIconWidgets[i]->SetEnabled( EnableIcons );
	}
}

//----------------------------------------------------------
//	handle mouse messages 
//----------------------------------------------------------
void TLGame::TSchemeEditor::ProcessMouseMessage(TRefRef ActionRef,TLMessaging::TMessage& Message,Bool IsClickAction)
{
	//	dragging new scene node around
	if ( ActionRef == m_NewSceneNodeDragAction && m_SelectedNodes.GetSize() )
	{
		#ifdef DEBUG_NODE_INTERACTION
			TTempString Debug_String("Dragging selected nodes ");
			Debug_GetSelectedNodeRefStrings( Debug_String );
			TLDebug_Print( Debug_String );
		#endif

		//	get the screen cursor pos
		int2 ScreenPos;
		if ( !Message.ImportData("Cursor", ScreenPos ) )
		{
			TLDebug_Break("Mouse move message missing cursor pos");
			return;
		}

		//	convert to world pos in game [render target]

		//	get game's render target
		float3 WorldPos;
		if ( !GetGameWorldPosFromScreenPos( WorldPos, ScreenPos ) )
		{
			TLDebug_Print("mouse outside window, dropping nodes");
			DropNewNode( m_SelectedNodes );
			return;
		}

		//	move the node to the cursor's world pos
		TLMessaging::TMessage SetMessage(TLCore::SetPropertyRef);
		SetMessage.ExportData("translate", WorldPos);

		for ( u32 i=0;	i<m_SelectedNodes.GetSize();	i++ )
			m_pGraph->SendMessageToNode( m_SelectedNodes[i], SetMessage );
		return;
	}

	//	drop new scene node into game (let go)
	if ( ActionRef == m_NewSceneNodeClickAction && HasNewNodesSelected() )
	{
		#ifdef DEBUG_NODE_INTERACTION
			TTempString Debug_String("Mouse click action (assume up) - releasing [new] selected nodes ");
			Debug_GetSelectedNodeRefStrings( Debug_String );
			TLDebug_Print( Debug_String );
		#endif

		//	drop new nodes into world, create widgets and unselect them
		DropNewNode( m_SelectedNodes );
		return;
	}



}


//----------------------------------------------------------
//	handle other messages (assume are commands)
//----------------------------------------------------------
Bool TLGame::TSchemeEditor::ProcessCommandMessage(TRefRef CommandRef,TLMessaging::TMessage& Message)
{
	//	gui command for editor from widget
	switch ( CommandRef.GetData() )
	{
		case TRef_Static4(Q,u,i,t):
		{
			//	todo: do some shutdown
			//	send close message
			TLMessaging::TMessage OutMessage( CommandRef, "Editor" );
			PublishMessage( OutMessage );
			return TRUE;
		}
		break;

		case TRef_Static4(S,a,v,e):
		{
			//	send save message - currently handled externally...
			TLMessaging::TMessage OutMessage( CommandRef, "Editor" );
			PublishMessage( OutMessage );

			//	gr: cant do this as this message/stack may be coming from something we delete when we change mode
			//		add this back in when the set mode is stalled
			//SetMode("Idle");
			return TRUE;
		}
		break;

		case TRef_Static(C,l,e,a,r):
			ClearScheme();

			//	gr: cant do this as this message/stack may be coming from something we delete when we change mode
			//		add this back in when the set mode is stalled
			//SetMode("Idle");
			return TRUE;
			break;

		//	delete selected node[s]
		case TRef_Static(D,e,l,e,t):
			{
				TArray<TRef> SelectedNodes;
				SelectedNodes.Copy( m_SelectedNodes );
				for ( u32 i=0;	i<SelectedNodes.GetSize();	i++ )
					DeleteNode( SelectedNodes[i] );
			}
			//	all nodes will have been deleted and unselected so go back to idle mode
			//	gr: cant do this as this message/stack may be coming from something we delete when we change mode
			//		add this back in when the set mode is stalled
			//if ( !m_SelectedNodes.GetSize() )
			//	SetMode("Idle");
			return TRUE;
			break;

		default:
		{
#ifdef _DEBUG
			TTempString Debug_String("Unknown editor command ");
			CommandRef.GetString( Debug_String );
			TLDebug_Print( Debug_String );
#endif	
		}
		break;
	}

	return FALSE;
}


//----------------------------------------------------------
//	add state machine modes here. overload to add custom modes
//----------------------------------------------------------
void TLGame::TSchemeEditor::AddStateModes()
{
	AddMode<Mode_Idle>("Idle");
	AddMode<Mode_Node>("Node");
	AddMode<Mode_Icon>("Icon");
}


//----------------------------------------------------------
//	enable/disable node widgets
//----------------------------------------------------------
void TLGame::TSchemeEditor::EnableNodeWidgets(Bool Enable)
{
	for ( u32 i=0;	i<m_NodeWidgets.GetSize();	i++ )
	{
		TPtr<TLGui::TWidgetDrag>& pWidget = m_NodeWidgets[i];
		pWidget->SetEnabled( Enable );
	}
}


//----------------------------------------------------------
//	get all the selected node refs as strings
//----------------------------------------------------------
void TLGame::TSchemeEditor::Debug_GetSelectedNodeRefStrings(TString& String)
{
	u32 SelectedNodeCount = m_SelectedNodes.GetSize();
	if ( SelectedNodeCount == 0 )
	{
		String.Append("(none)");
		return;
	}

	for ( u32 i=0;	i<SelectedNodeCount;	i++ )
	{
		m_SelectedNodes[i].GetString( String );
		if ( i != m_SelectedNodes.GetLastIndex() )
			String.Append(",");
	}
	
}


//----------------------------------------------------------
//	handle graph change from our graph
//----------------------------------------------------------
void TLGame::TSchemeEditor::OnGraphMessage(TLMessaging::TMessage& Message)
{
	//	new [scene] node, eg. child of an instanced scheme node
	if ( Message.GetMessageRef() == "NodeAdded" )
	{
		TRef SceneNode;
		Message.ResetReadPos();
		if( !Message.Read( SceneNode ) )
			return;

		//	not dragging a new node.. but a new node has turned up
		if ( !m_NewSceneNodeDragAction.IsValid() /*|| !m_NewSceneNodeClickAction.IsValid() */)
		{
			TLDebug_Print("New node has appeared in graph, but we're not dragging in a new node...");
			DropNewNode( SceneNode );
		}
		else
		{
			//	new node, let's select it and it'll be dragged around like everything else
			//	but... i dont know where to position it...
			SelectNode( SceneNode );
		}
		return;
	}
}


//----------------------------------------------------------
//	drop a node into the game
//----------------------------------------------------------
void TLGame::TSchemeEditor::DropNewNode(TRefRef NodeRef)
{
	#ifdef DEBUG_NODE_INTERACTION
		TTempString Debug_String("Dropping new node: ");
		NodeRef.GetString( Debug_String );
		TLDebug_Print( Debug_String );
	#endif

	//	fetch node and create widget
	TLGraph::TGraphNodeBase* pNode = m_pGraph->FindNodeBase( NodeRef );
	if ( pNode )
	{
		CreateNodeWidgets( *pNode );
	}
	else
	{
		TLDebug_Break("Node expected");
	}

	//	unselect node
	UnselectNode( NodeRef );

	//	if no more selected nodes then unset the new-node drag actions
	if ( m_SelectedNodes.GetSize() == 0 )
	{
		m_NewSceneNodeDragAction.SetInvalid();
		m_NewSceneNodeClickAction.SetInvalid();
	}

	//	do notifications of drop
	OnNewNodeDropped( NodeRef );
}


//----------------------------------------------------------
//	drop a bunch of nodes (probbaly selected nodes)
//----------------------------------------------------------
void TLGame::TSchemeEditor::DropNewNode(TArray<TRef>& NodeArray)
{
	//	gr: do this in reverse as the array is likely to be the selected node list, and the array gets changed in DropNewNode
	for ( s32 i=NodeArray.GetLastIndex();	i>=0;	i-- )
	{
		DropNewNode( NodeArray[i] );
	}
}

//----------------------------------------------------------
//	delete a node and it's widgets
//----------------------------------------------------------
void TLGame::TSchemeEditor::DeleteNode(TRefRef NodeRef)
{
	TRef RootNodeRef = NodeRef;

	//	get root node list
	TLGraph::TGraphNodeBase* pSchemeRootNode = m_pGraph->FindNodeBase( m_SchemeRootNode );
	if ( !pSchemeRootNode )
	{
		TLDebug_Break("Scehem root node expected");
		return;
	}
	TArray<TRef> RootNodeRefs;
	pSchemeRootNode->GetChildren( RootNodeRefs );

	//	loop through list of ROOT nodes associated with the editor - if we're trying to delete a sub-node then we need to delete the parent node
	//	this is required for scheme node usage
	s32 RootNodeIndex = -1;
	while ( RootNodeIndex == -1 && RootNodeRef.IsValid() )
	{
		RootNodeIndex = RootNodeRefs.FindIndex( RootNodeRef );

		//	not a root node, try the parent
		if ( RootNodeIndex == -1 )
		{
			//	get the parent's ref node
			const TLGraph::TGraphNodeBase* pNode = m_pGraph->FindNodeBase( RootNodeRef );
			const TLGraph::TGraphNodeBase* pNodeParent = pNode ? pNode->GetParentBase() : NULL;
			if ( pNodeParent )
				RootNodeRef = pNodeParent->GetNodeRef();
			else
				RootNodeRef.SetInvalid();
		}
	}

	//	not found
	TLGraph::TGraphNodeBase* pNode = m_pGraph->FindNodeBase( RootNodeRef );
	if ( !RootNodeRef.IsValid() || !pNode )
	{
		TTempString Debug_String("Failed to delete node ");
		NodeRef.GetString( Debug_String );
		Debug_String.Append(" as it (and it's parents) are not in our node list");
		TLDebug_Break( Debug_String );
		return;
	}

	//	get a list of all the child node refs
	TArray<TRef> ChildNodeRefs;
	pNode->GetChildrenTree( ChildNodeRefs );

	//	remove root node from graph (will remove children)
	m_pGraph->RemoveNode( RootNodeRef );

	//	include root node in the removed-node list now
	ChildNodeRefs.Add(RootNodeRef);

	//	find and remove widgets associated with these nodes
	for ( u32 c=0;	c<ChildNodeRefs.GetSize();	c++ )
		RemoveNodeWidget( ChildNodeRefs[c] );

	//	unselect nodes
	UnselectNode( ChildNodeRefs );

	//	overloaded notification
	OnNodeDeleted( ChildNodeRefs );
}


//----------------------------------------------------------
//	remove widget for this node
//----------------------------------------------------------
void TLGame::TSchemeEditor::RemoveNodeWidget(TRefRef NodeRef)
{
	//	find widget associated with node
	for ( s32 i=m_NodeWidgets.GetLastIndex();	i>=0;	i-- )
	{
		TRef WidgetNodeRef;
		if ( !m_NodeWidgets[i]->GetWidgetData().ImportData("Node", WidgetNodeRef ) )
			continue;

		//	wrong widget
		if ( WidgetNodeRef != NodeRef )
			continue;

		//	remove this widget
		m_NodeWidgets.RemoveAt(i);
	}
}


Bool TLGame::TSchemeEditor::Mode_Idle::OnBegin(TRefRef PreviousMode)		
{
	GetEditor().EnableNodeWidgets(TRUE);	
	GetEditor().EnableIconWidgets(FALSE);	
	return TRUE;	
}



Bool TLGame::TSchemeEditor::Mode_Node::OnBegin(TRefRef PreviousMode)
{
	GetEditor().EnableNodeWidgets(TRUE);	
	GetEditor().EnableIconWidgets(FALSE);	
	return TRUE;	
}

void TLGame::TSchemeEditor::Mode_Node::OnEnd(TRefRef NextMode)
{
	//	when we go out of node mode, unselect any nodes we had selected
	//	gr: unless we're going to link mode
	if ( NextMode != "Link" )
		GetEditor().UnselectAllNodes();
}


Bool TLGame::TSchemeEditor::Mode_Icon::OnBegin(TRefRef PreviousMode)
{	
	//	gr: allow user to select & drag node widgets still. when they do this we'll be set back to Node mode
	GetEditor().EnableNodeWidgets(TRUE);	

	GetEditor().EnableIconWidgets(TRUE);	
	GetEditor().UnselectAllNodes();	
	return TRUE;	
}

