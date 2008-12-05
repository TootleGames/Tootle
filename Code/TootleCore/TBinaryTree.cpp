#include "TBinaryTree.h"



TPtr<TBinaryTree> TBinaryTree::g_pNull;


//----------------------------------------------------------------
//	
//----------------------------------------------------------------
TBinaryTree::TBinaryTree(TRefRef DataRef) :
	m_DataRef	( DataRef )
{
	m_DataRef.GetString(m_Debug_DataRefString);
}


//----------------------------------------------------------------
//	add new child
//----------------------------------------------------------------
TPtr<TBinaryTree>& TBinaryTree::AddChild(TRefRef ChildRef)		
{
	//	add new ptr to the child array
	TPtr<TBinaryTree>* pNewChildPtr = m_Children.AddNew();
	if ( !pNewChildPtr )
		return g_pNull;

	//	put new tree in ptr
	TPtr<TBinaryTree>& pNewChild = *pNewChildPtr;
	pNewChild = new TBinaryTree( ChildRef );
	return pNewChild;
}


//----------------------------------------------------------------
//	recursivly copy the tree from Data into this (clone)
//----------------------------------------------------------------
Bool TBinaryTree::CopyTree(TPtr<TBinaryTree>& pData)
{
	//	copy this data
	SetDataRef( pData->GetDataRef() );
	GetData().Copy( pData->GetData() );

	//	remove existing children
	m_Children.Empty();

	//	copy children
	for ( u32 c=0;	c<pData->GetChildren().GetSize();	c++ )
	{
		TPtr<TBinaryTree>& pChild = pData->GetChildren().ElementAt(c);

		//	add a child
		TPtr<TBinaryTree>* ppNewChild = m_Children.AddNew();
		if ( !ppNewChild )
			return FALSE;
		TPtr<TBinaryTree>& pNewChild = *ppNewChild;

		//	copy child
		if ( !pNewChild->CopyTree( pChild ) )
			return FALSE;
	}

	return TRUE;
}


//----------------------------------------------------------------
//	debug_print the tree
//----------------------------------------------------------------
void TBinaryTree::Debug_PrintTree(u32 TreeLevel) const
{
	TTempString Indent;
	for ( u32 i=0;	i<TreeLevel;	i++ )
		Indent.Append('\t');

	//	print out name of branch
	TTempString String( Indent );
	GetDataRef().GetString( String );
	TLDebug_Print( String );

	//	print out data info
	String.Set( Indent );
	String.Appendf(" { %d bytes }", GetSize() );
	TLDebug_Print( String );

	String.Set( Indent );
	String.Appendf(" { %d bytes read }", GetReadPos() );
	TLDebug_Print( String );

	//	now do the rest of the tree
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		m_Children[c]->Debug_PrintTree( TreeLevel+1 );
	}
}


//----------------------------------------------------------------
//	compact binary data and all our children
//----------------------------------------------------------------
void TBinaryTree::Compact()
{
	TBinary::Compact();

	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		m_Children[c]->Compact();
	}
}
