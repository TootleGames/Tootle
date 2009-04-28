#include "TBinaryTree.h"




//----------------------------------------------------------------
//	add new child
//----------------------------------------------------------------
TPtr<TBinaryTree>& TBinaryTree::AddChild(TRefRef ChildRef)		
{
	//	add new ptr to the child array
	TPtr<TBinaryTree>* pNewChildPtr = m_Children.AddNew();
	if ( !pNewChildPtr )
		return TLPtr::GetNullPtr<TBinaryTree>();

	//	put new tree in ptr
	TPtr<TBinaryTree>& pNewChild = *pNewChildPtr;
	pNewChild = new TBinaryTree( ChildRef );
	return pNewChild;
}


//----------------------------------------------------------------
//	recursivly copy the tree from Data into this (clone)
//----------------------------------------------------------------
Bool TBinaryTree::CopyDataTree(const TBinaryTree& Data,Bool OverwriteDataRef)
{
	//	copy ref
	if ( OverwriteDataRef )
		SetDataRef( Data.GetDataRef() );

	//	copy this data
	GetData().Copy( Data.GetData() );

	//	remove existing children
	m_Children.Empty();

	//	copy children
	for ( u32 c=0;	c<Data.GetChildren().GetSize();	c++ )
	{
		const TPtr<TBinaryTree>& pDataChild = Data.GetChildren().ElementAtConst(c);

		//	create a new binary tree to copy into
		TPtr<TBinaryTree> pNewChild = new TBinaryTree( TRef() );
		if ( !pNewChild->CopyDataTree( pDataChild, TRUE ) )
			return FALSE;

		//	add to children
		m_Children.Add( pNewChild );
	}

	return TRUE;
}


//----------------------------------------------------------------
//	copy the tree by re-using the TPtr's to the data. The data is re-used and saves us allocating and copying data but without fear of deletion
//----------------------------------------------------------------
Bool TBinaryTree::ReferenceDataTree(const TBinaryTree& Data,Bool OverwriteDataRef)
{
	//	copy ref
	if ( OverwriteDataRef )
		SetDataRef( Data.GetDataRef() );

	//	still have to copy the root data
	GetData().Copy( Data.GetData() );

	//	remove existing children
	m_Children.Empty();

	//	pre-alloc array in case it's big
	u32 DataChildCount = Data.GetChildren().GetSize();
	m_Children.AddAllocSize( DataChildCount );

	//	reference the children of Data. We don't need to do this recursivly as obviously the children are the same, and still pointing at THEIR children :)
	for ( u32 c=0;	c<DataChildCount;	c++ )
	{
		const TPtr<TBinaryTree>& pDataChild = Data.GetChildren().ElementAtConst(c);

		//	add to children
		m_Children.Add( pDataChild );
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

	String.Set( Indent );
	String.Appendf("¬ %d bytes", GetSize() );
	TLDebug_Print( String );

	String.Set( Indent );
	String.Appendf("¬ %d bytes read", GetReadPos() );
	TLDebug_Print( String );

	//	print out data info
	if ( GetDataTypeHint().IsValid() )
	{
		String.Set( Indent );
		String.Append("¬ type: ");
		GetDataTypeHint().GetString( String );

		//	print out the actual data for some types
		switch ( GetDataTypeHint().GetData() )
		{
			case TLBinary_TypeRef(u8):				String.Appendf(" [%d]", Debug_GetDataAs<u8>() );		break;
			case TLBinary_TypeRef(s8):				String.Appendf(" [%d]", Debug_GetDataAs<s8>() );		break;
			case TLBinary_TypeRef(u16):				String.Appendf(" [%d]", Debug_GetDataAs<u16>() );		break;
			case TLBinary_TypeRef(s16):				String.Appendf(" [%d]", Debug_GetDataAs<s16>() );		break;
			case TLBinary_TypeRef(u32):				String.Appendf(" [%d]", Debug_GetDataAs<u32>() );		break;

			case TLBinary_TypeRef(s32):				String.Appendf(" [%d]", Debug_GetDataAs<s32>() );		break;
			case TLBinary_TypeNRef(Type2,s32):		String.Appendf(" [%d,%d]", Debug_GetDataAs<Type2<s32> >().x, Debug_GetDataAs<Type2<s32> >().y );		break;
			case TLBinary_TypeNRef(Type3,s32):		String.Appendf(" [%d,%d,%d]", Debug_GetDataAs<Type3<s32> >().x, Debug_GetDataAs<Type3<s32> >().y, Debug_GetDataAs<Type3<s32> >().z );		break;
			case TLBinary_TypeNRef(Type4,s32):		String.Appendf(" [%d,%d,%d,%d]", Debug_GetDataAs<Type4<s32> >().x, Debug_GetDataAs<Type4<s32> >().y, Debug_GetDataAs<Type4<s32> >().z, Debug_GetDataAs<Type4<s32> >().w );		break;
			
			case TLBinary_TypeRef(float):			String.Appendf(" [%.4f]", Debug_GetDataAs<float>() );		break;
			case TLBinary_TypeRef(float2):			String.Appendf(" [%.4f,%.4f]", Debug_GetDataAs<float2>().x, Debug_GetDataAs<float2>().y );	break;
			case TLBinary_TypeRef(float3):			String.Appendf(" [%.4f,%.4f,%.4f]", Debug_GetDataAs<float3>().x, Debug_GetDataAs<float3>().y, Debug_GetDataAs<float3>().z );	break;
			case TLBinary_TypeRef(float4):			String.Appendf(" [%.4f,%.4f,%.4f,%.4f]", Debug_GetDataAs<float4>().x, Debug_GetDataAs<float4>().y, Debug_GetDataAs<float4>().z, Debug_GetDataAs<float4>().w );	break;
			case TLBinary_TypeRef(TRef):	
			{
				TRefRef Data = Debug_GetDataAs<TRef>();
				String.Append(" [");
				Data.GetString( String );
				String.Append("]");
			}
			break;
		};

		TLDebug_Print( String );
	}

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




//------------------------------------------------------
//	returns FALSE if failed, WAIT if nothing imported, TRUE if something imported
//	gr: changed back to Bool
//------------------------------------------------------
Bool TBinaryTree::ImportDataString(TRefRef DataRef,TString& DataString)
{
	//	get the first child with this ref
	TPtr<TBinaryTree>& pData = GetChild( DataRef );
	if ( !pData )
	{
		//	no matching child
		return FALSE;//SyncWait;
	}

	//	read out var
	pData->ResetReadPos();
	if ( !pData->ReadString( DataString ) )
		return FALSE;

	return TRUE;
}


//------------------------------------------------------
//	
//------------------------------------------------------
void TBinaryTree::ExportDataString(TRefRef DataRef,const TString& DataString)
{
	//	create a child
	TPtr<TBinaryTree>& pData = AddChild( DataRef );
	pData->WriteString( DataString );
}
