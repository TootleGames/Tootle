/*
 *  TTree.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 28/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include "Tree.h"
#include "Window.h"
#include "modeltest.h"
#include <TootleFileSys/TLFile.h>

//	shows debug info in the data() strings
//#define DEBUG_COLUMN_DATA


Qt::Tree::Tree(TRefRef ControlRef,TPtr<TLGui::TTreeItem>& pRootItem,const TArray<TRef>& Columns) :
	TLGui::TTree			( ControlRef, pRootItem ),
	m_DataModel				( Columns, *this )
{
}


bool Qt::Tree::Initialise(TLGui::TWindow& Parent)
{
	QTreeView::setParent( static_cast<Qt::TWindow*>( &Parent ) );
	QTreeView::setModel( &m_DataModel );
	
	//	initialise size of the columns
	QTreeView::setColumnWidth( 0, 150 );

	//	allow multi selection with shift, ctrl etc
	QTreeView::setSelectionMode( QAbstractItemView::ExtendedSelection );

	//	catch events
	bool success = true;
	
	//	gr: for some reason, 'this' doesnt work, but the widget pointer does....
	success &= connect( &GetWidget(), SIGNAL(pressed(const QModelIndex&)), SLOT(OnItemClicked(const QModelIndex&)));
//	success &= connect( &GetWidget(), SIGNAL(clicked(const QModelIndex&)), SLOT(OnItemClicked(const QModelIndex&)));
//	success &= connect( &GetWidget(), SIGNAL(entered(const QModelIndex&)), SLOT(OnItemClicked(const QModelIndex&)));
//	success &= connect( &GetWidget(), SIGNAL(activated(const QModelIndex&)), SLOT(OnItemClicked(const QModelIndex&)));
	if ( !success )
		TLDebug_Break("failed to connect events");

	//	in debug, make a model-test, like a unit test, it'll verify the integrety of the model and assert if anything hasn't been implemented correctly
#if defined(_DEBUG)
	/*ModelTest* pTest = */new ModelTest( &m_DataModel, static_cast<Qt::TWindow*>( &Parent ) );
#endif
	
	return TLGui::TTree::Initialise( Parent );
}


//--------------------------------------------------------
//	catch selection chnages
//--------------------------------------------------------
void Qt::Tree::selectionChanged ( const QItemSelection & selected, const QItemSelection & deselected )
{
	//	call inherited stuff
	QTreeView::selectionChanged( selected, deselected );
/*
	//	this is double deleting (or something, corrupted heap) on pc when the list is destructed
	QModelIndexList DeselectedIndexes = deselected.indexes();

	//	process de-selections
	for ( u32 i=0;	i<DeselectedIndexes.count();	i++ )
	{
		QModelIndex Index = deselected.indexes().value(i);
		TLGui::TTreeItem* pItem = m_DataModel.GetTreeItem( Index );
		if ( pItem )
		{
			pItem->OnSelectedChanged(false);
		}
	}

	for ( u32 i=0;	i<selected.count();	i++ )
	{
		QModelIndex Index = selected.indexes().value(i);
		TLGui::TTreeItem* pItem = m_DataModel.GetTreeItem( Index );
		if ( pItem )
		{
			pItem->OnSelectedChanged(true);
		}
	}
*/
}

//--------------------------------------------------------
//	handler for the pressed signal which occurs when a mouse button is clicked on an item
//--------------------------------------------------------
void Qt::Tree::OnItemClicked(const QModelIndex& index)
{
	TLGui::TTreeItem* pItem = m_DataModel.GetTreeItem(index);
	if ( !pItem )
	{
		TLDebug_Break("Item expected");
		return;
	}
	
	//	get the mouse button
	Qt::MouseButtons Buttons = QApplication::mouseButtons();
	
	//	if right-clicked then show a popup menu on this item
	if ( Buttons & Qt::RightButton )
	{
		//	let the derived types instigate a menu
		pItem->OnRightClick();
	}
}


//--------------------------------------------------------
//	Add node to model
//--------------------------------------------------------
Bool Qt::Tree::OnItemAdding(TPtr<TLGui::TTreeItem>& pItem,TLGui::TTreeItem* pParent)
{
	//	add to model
	Debug_VerifyTree();
	bool Result = m_DataModel.OnItemAdding( *pItem, pParent );
	Debug_VerifyTree();
	
	return Result;
}

//--------------------------------------------------------
//	Add node to model
//--------------------------------------------------------
Bool Qt::Tree::OnItemAdded(TPtr<TLGui::TTreeItem>& pItem)
{
	//	add to model
	Debug_VerifyTree();
	bool Result = m_DataModel.OnItemAdded( *pItem );
	Debug_VerifyTree();
	
	if ( !Result )
		return false;
	
	//	by default, expand new items
	QModelIndex ItemModelIndex = m_DataModel.GetModelIndex( *pItem );
	setExpanded( ItemModelIndex, true );
	
	return true;
}


//-----------------------------------------------------------
//	move node around in control
//-----------------------------------------------------------
Bool Qt::Tree::OnItemMoved(TPtr<TLGui::TTreeItem>& pOldParent,TPtr<TLGui::TTreeItem>& pItem)
{
	Debug_VerifyTree();
	bool Result = m_DataModel.OnItemMoved( *pOldParent, *pItem );
	Debug_VerifyTree();
	return Result;
}


//-----------------------------------------------------------
//	move node around in control
//-----------------------------------------------------------
Bool Qt::Tree::OnItemChanged(TLGui::TTreeItem& Item,TRefRef Data)
{
	Debug_VerifyTree();
	bool Result = m_DataModel.OnItemChanged( Item, Data );
	Debug_VerifyTree();
	return Result;
}


//-----------------------------------------------------------
//	move node around in control
//-----------------------------------------------------------
Bool Qt::Tree::OnItemMoving(TPtr<TLGui::TTreeItem>& pOldParent,TPtr<TLGui::TTreeItem>& pItem)
{
	Debug_VerifyTree();
	bool Result = m_DataModel.OnItemMoving( *pOldParent, *pItem );
	Debug_VerifyTree();
	return Result;
}


//---------------------------------------------------
//	remove node from control
//---------------------------------------------------
Bool Qt::Tree::OnItemRemoving(TLGui::TTreeItem& Item)
{
	Debug_VerifyTree();
	bool Result = m_DataModel.OnItemRemoving( Item );
	Debug_VerifyTree();
	return Result;
}


//---------------------------------------------------
//	remove node from control
//---------------------------------------------------
Bool Qt::Tree::OnItemRemoved(TLGui::TTreeItem& Item)
{
	Debug_VerifyTree();
	bool Result = m_DataModel.OnItemRemoved( Item );
	Debug_VerifyTree();
	return Result;
}

//-------------------------------------------------------
//	bind this action - must be done by widget based object
//-------------------------------------------------------
void Qt::Tree::BindAction(QAction& Action)
{
	if ( !connect( &Action, SIGNAL(triggered()), this, SLOT(OnAction())) )
	{
		TLDebug_Break("Failed to setup menu handler");
	}
}

QMenu* Qt::Tree::AllocMenu(const TLAsset::TMenu& Menu)
{
	Qt::TWindow& Window = static_cast<Qt::TWindow&>( GetParentWindow() );
	return Window.AllocMenu( Menu );
}


Qt::TreeDataModel::TreeDataModel(const TArray<TRef>& Columns,Qt::Tree& Owner) :
	m_pOwner		( &Owner ),
	m_Columns		( Columns ),
	m_RootItemAdded	( false )
{
	if ( !m_pOwner )
	{
		TLDebug_Break("Owner expected - crash will occur!");
	}
}

TLGui::TTreeItem* Qt::TreeDataModel::GetTreeItem(const QModelIndex& ModelIndex)
{
	void* pModelItemData = ModelIndex.internalPointer();
	TLGui::TTreeItem* pItem = static_cast<TLGui::TTreeItem*>( pModelItemData );
	GetOwner().Debug_VerifyTree( pItem );
	return pItem;
}

const TLGui::TTreeItem* Qt::TreeDataModel::GetTreeItem(const QModelIndex& ModelIndex) const
{
	void* pModelItemData = ModelIndex.internalPointer();
	TLGui::TTreeItem* pItem = static_cast<TLGui::TTreeItem*>( pModelItemData );
	GetOwner().Debug_VerifyTree( pItem );
	return pItem;
}

//------------------------------------------------------------------
//	
//------------------------------------------------------------------
QModelIndex Qt::TreeDataModel::GetModelIndex(const TLGui::TTreeItem& TreeItem,int Column)
{
	const TLGui::TTreeItem* pParent = TreeItem.GetParent();
	
	//	we don't return indexes for columns > 0
	if ( Column >= GetColumns().GetSize() || Column < 0 )
		return QModelIndex();
	
	//	get this item's row index (from the parent)
	s32 ChildIndex = 0;
	if ( pParent )
		ChildIndex = pParent->GetChildren().FindIndex( TreeItem );
	
	if ( ChildIndex < 0 )
	{
		TLDebug_Break("Invalid child index");
		ChildIndex = 0;
	}
	
	const TLGui::TTreeItem* pChild = &TreeItem;
	return createIndex( ChildIndex, Column, (void*)pChild );
}

//------------------------------------------------------------------
//	add item to model
//------------------------------------------------------------------
bool Qt::TreeDataModel::OnItemAdding(TLGui::TTreeItem& Item,TLGui::TTreeItem* pParent)
{
	//	item should not have a parent yet (certainly not the one passed in, or our row counts are going to be wrong before hand)
	if ( pParent && Item.GetParent() == pParent )
	{
		TLDebug_Break("Inserting item which is already assigned to the specified parent");
		return false;
	}
	
	//	get the parent's model index
	QModelIndex ParentModelIndex;
	u32 Row = 0;
	if ( pParent )
	{
		ParentModelIndex = GetModelIndex( *pParent, 0 );
		
		//	row here will be invalid as we haven't been added yet.
		//	presumably we'll be the next one. If not (eg. in a insert-sorted array) then we'll have to pass this in as a param
		Row = pParent->GetChildren().GetSize();
		//Row = pParent->GetChildren().FindIndex( Item );
	}

	beginInsertRows( ParentModelIndex, Row, Row );
	
	return true;
}

//------------------------------------------------------------------
//	add item to model
//------------------------------------------------------------------
bool Qt::TreeDataModel::OnItemAdded(TLGui::TTreeItem& Item)
{
	//	get the parent's model index
	const TLGui::TTreeItem* pParent = Item.GetParent();
	QModelIndex ParentModelIndex;
	u32 Row = 0;
	if ( pParent )
	{
		ParentModelIndex = GetModelIndex( *pParent, 0 );
		Row = pParent->GetChildren().FindIndex( Item );
	}
	
	//	when root item gets added we need to take note
	if ( !pParent )
		m_RootItemAdded = true;
	
	endInsertRows();
	return true;
}

//------------------------------------------------------------------
//
//------------------------------------------------------------------
bool Qt::TreeDataModel::OnItemRemoving(TLGui::TTreeItem& Item)
{
	//	get the parent's model index
	const TLGui::TTreeItem* pParent = Item.GetParent();
	QModelIndex ParentModelIndex;
	u32 Row = 0;
	if ( pParent )
	{
		ParentModelIndex = GetModelIndex( *pParent, 0 );
		Row = pParent->GetChildren().FindIndex( Item );
	}

	beginRemoveRows( ParentModelIndex, Row, Row );
	return true;
}


//------------------------------------------------------------------
//
//------------------------------------------------------------------
bool Qt::TreeDataModel::OnItemRemoved(TLGui::TTreeItem& Item)
{
	//	get the parent's model index
	const TLGui::TTreeItem* pParent = Item.GetParent();
	QModelIndex ParentModelIndex;
	u32 Row = 0;
	if ( pParent )
	{
		ParentModelIndex = GetModelIndex( *pParent, 0 );
		Row = pParent->GetChildren().FindIndex( Item );
	}
	
	endRemoveRows();
	return true;
}


//------------------------------------------------------------------
//	
//------------------------------------------------------------------
bool Qt::TreeDataModel::OnItemMoving(TLGui::TTreeItem& OldParent,TLGui::TTreeItem& Item)
{
	TLDebug_Break("unhandled");
	return true;
}


//------------------------------------------------------------------
//	
//------------------------------------------------------------------
bool Qt::TreeDataModel::OnItemChanged(TLGui::TTreeItem& Item,TRefRef Data)
{
	int FirstColumn,LastColumn;

	//	all data!
	if ( !Data.IsValid() )
	{
		FirstColumn = 0;
		LastColumn = GetColumns().GetLastIndex();
	}
	else
	{
		//	specific data
		FirstColumn = GetColumns().FindIndex( Data );
		if ( FirstColumn == -1 )
			return false;
		LastColumn = FirstColumn;
	}


	//	notify change of the data
	QModelIndex FirstIndex = GetModelIndex( Item, FirstColumn );
	QModelIndex LastIndex = GetModelIndex( Item, LastColumn );
	
	emit dataChanged( FirstIndex, LastIndex );
	return true;
}

//------------------------------------------------------------------
//	
//------------------------------------------------------------------
bool Qt::TreeDataModel::OnItemMoved(TLGui::TTreeItem& OldParent,TLGui::TTreeItem& Item)
{
	TLDebug_Break("unhandled");
	return true;
}


QModelIndex Qt::TreeDataModel::index(int row, int column,const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent) || column >= GetColumns().GetSize() )
        return QModelIndex();

	//	invisible parent = get root item
    if (!parent.isValid() )
	{
		if ( row != 0 )
			return QModelIndex();

		//	get the root item
		const TLGui::TTreeItem* pChildItem = GetRootItem();
		if ( !pChildItem || !m_RootItemAdded )
		{
			if ( m_RootItemAdded )
				TLDebug_Break("Expected the root item to exist");
			return QModelIndex();
		}
		return createIndex( row, column, (void*)pChildItem );
	}

	//	return nth child
	const TLGui::TTreeItem* pParentItem = GetTreeItem( parent );
	if ( !pParentItem )
	{
		TLDebug_Break("Parent expected");
		return QModelIndex();
	}
	
	const TPtrArray<TLGui::TTreeItem>& Children = pParentItem->GetChildren();
	const TLGui::TTreeItem* pChildItem = Children[row];
	return createIndex( row, column, (void*)pChildItem );	
}
		

QModelIndex Qt::TreeDataModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();
	
	//	we don't return indexes for columns > 0
	if ( child.column() >= GetColumns().GetSize() || child.column() < 0 )
		return QModelIndex();
	
	//	the PARENT item, of column 1, is still parent with column zero. the parent of a column, is actually the parent of the row
	u32 ParentColumn = 0;	//child.column();
	
	const TLGui::TTreeItem* pChild = GetTreeItem( child );
	const TLGui::TTreeItem* pParent = pChild ? pChild->GetParent() : NULL;
	
	//	parent of the root item is the invisible index
	if ( !pParent )
		return QModelIndex();

	//	if the parent is the root item then the parent is the invalid one, with a row of 0
	const TLGui::TTreeItem* pRootItem = GetRootItem();
	if ( !pRootItem )
	{
		TLDebug_Break("Root item expected");
		return QModelIndex();
	}
	if ( pParent == pRootItem )
		return m_RootItemAdded ? createIndex( 0, ParentColumn, (void*)pParent ) : QModelIndex();

	//	get an item for the parent, which includes parent's index
	const TLGui::TTreeItem* pParentParent = pParent->GetParent();
	if ( !pParentParent )
	{
		TLDebug_Break("Parent of parent expected");
		return QModelIndex();
	}
	u32 ParentIndex = pParentParent->GetChildren().FindIndex( pParent );
	return createIndex( ParentIndex, ParentColumn, (void*)pParent );
}

int Qt::TreeDataModel::rowCount(const QModelIndex &parent) const
{
	//	the row count for a 2nd column... is always zero... not sure how this works with the model system 
	//	but this stops it thinking that column2 parent can have a different tree
	if ( parent.column() > 0 )
		return 0;
	
	//	if invalid (invisible root) then return 1 (has root item) or 0 (tree is empty)
	if ( !parent.isValid() )
	{
		const TLGui::TTreeItem* pRootItem = GetRootItem();
		return (pRootItem && m_RootItemAdded) ? 1 : 0;
	}

	//	get the item
	const TLGui::TTreeItem* pParent = GetTreeItem( parent );
	if ( !pParent )
	{
		TLDebug_Break("item expected");
		return 0;
	}
	
	//	return child count
	const TPtrArray<TLGui::TTreeItem>& Children = pParent->GetChildren();
	return Children.GetSize();
}

int Qt::TreeDataModel::columnCount(const QModelIndex &parent) const
{
	//	all items have the same column count
	return GetColumns().GetSize();
}


bool Qt::TreeDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if ( role != Qt::DisplayRole && role != Qt::EditRole )
		return false;
	
	//	get the item
	TLGui::TTreeItem* pItem = const_cast<TLGui::TTreeItem*>( GetTreeItem( index ) );
	if ( !pItem )
		return false;
	
	u32 ColumnIndex = index.column();
	TRef ColumnDataRef = GetColumns().ElementAtConst( ColumnIndex );

	//	can we modify this data?
	if ( !pItem->IsDataMutable( ColumnDataRef ) )
		return false;

	bool DataChanged = false;

	//	get the original data to see what to convert to
	TBinary* pData = pItem->GetData( ColumnDataRef );
	if ( pData )
	{
		TBinary NewData;
		if ( !Qt::GetData( NewData, value, pData->GetDataTypeHint() ) )
			return false;

		//	apply the new data
		DataChanged = pItem->SetData( NewData, ColumnDataRef );
	}
	else
	{
		//	no binary data with this item, so lets just try and set it via string
		TTempString NewString;
		NewString << value.toString();
		DataChanged = pItem->SetData( NewString, ColumnDataRef );
	}
	
	//	no change 
	if ( !DataChanged )
		return false;

	//	notify change, then return
	emit dataChanged( index, index );
	return true;
}

QVariant Qt::TreeDataModel::data(const QModelIndex &index, int role) const
{
	if ( role != Qt::DisplayRole && role != Qt::EditRole )
        return QVariant();
	
    if (!index.isValid())
        return QVariant();
	
	u32 ColumnIndex = index.column();
	TRef ColumnDataRef = GetColumns().ElementAtConst( ColumnIndex );
	
	//	get the item
	TLGui::TTreeItem* pItem = const_cast<TLGui::TTreeItem*>( GetTreeItem( index ) );
	TTempString String;

	if ( !pItem )
	{
		TLDebug_Break("Item expected");
		String = "Item expected";
	}
	else
	{
		//	if the item provides data, use that
		TBinary* pData = pItem->GetData( ColumnDataRef );
		if ( pData )
		{
			pData->ResetReadPos();
			if ( !TLFile::ExportBinaryData( String, *pData ) )
			{
				String << "Error with " << ColumnDataRef;
			}
		}
		else
		{
			//	not writable, so just fetch string for this column
			pItem->GetData( String, ColumnDataRef );
		}
	}
	
	//	show column number
#if defined(DEBUG_COLUMN_DATA)
	String << " [col " << ColumnDataRef << "(" << ColumnIndex << ")]";
#endif 
	
	return QVariant( Qt::GetString(String) );
}


Qt::ItemFlags Qt::TreeDataModel::flags(const QModelIndex &index) const
{
	//	get the tree item
	TLGui::TTreeItem* pItem = const_cast<TLGui::TTreeItem*>( GetTreeItem(index) );
	if ( !pItem )
		return Qt::NoItemFlags;

	Qt::ItemFlags Flags = Qt::NoItemFlags;
	
	//	get the binary data for this item
	u32 ColumnIndex = index.column();
	TRef ColumnDataRef = GetColumns().ElementAtConst( ColumnIndex );

	//	if we have one, it's modifiable
	if ( pItem->IsDataMutable( ColumnDataRef ) )
		Flags |= Qt::ItemIsEditable;
	
	//	everything bar the root is draggable (for re-parenting)
	if ( !pItem->IsRootItem() )
		Flags |= Qt::ItemIsDragEnabled;
	
	//	everything can have something dropped (for parenting)
	Flags |= Qt::ItemIsDropEnabled;

	return Flags | QAbstractItemModel::flags( index );
}


QVariant Qt::TreeDataModel::headerData(int section, Qt::Orientation orientation,int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		TRef ColumnDataRef = GetColumns().ElementAtConst( section );
		TTempString String;
		String << ColumnDataRef;
		return QVariant( Qt::GetString(String) );
	}

    return QVariant();
}
