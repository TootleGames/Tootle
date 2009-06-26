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
		//	gr: shouldn't get a case where we have a type, but no data...
		if ( GetSize() == 0 )
		{
			TTempString Debug_String("Data has a type (");
			GetDataTypeHint().GetString( Debug_String );
			Debug_String.Append(") but has zero size - TBinary is corrupt");
			TLDebug_Break( Debug_String );
		}
		else
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


//------------------------------------------------------
//	print out what data we failed to read
//------------------------------------------------------
void TBinaryTree::Debug_FailedToRead(TRefRef ChildDataRef,TRefRef TypeRef)
{
#ifdef _DEBUG
	TTempString Debug_String("Failed to read TBinaryTree ");
	GetDataRef().GetString( Debug_String );
	Debug_String.Append(", child ref: ");
	ChildDataRef.GetString( Debug_String );
	Debug_String.Append(". Trying to read data type ");
	TypeRef.GetString( Debug_String );
	TLDebug_Break( Debug_String );
#endif
}


//------------------------------------------------------
//	traverse tree to see if any data has been read (read pos is valid and not -1)
//------------------------------------------------------
Bool TBinaryTree::IsDataTreeRead() const
{
	//	our data has been read 
	if ( GetReadPos() >= 0 )
		return TRUE;

	//	check children
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		if ( m_Children[c]->IsDataTreeRead() )
			return TRUE;
	}

	//	no data has been read
	return FALSE;
}


//------------------------------------------------------
//	add children from Data to this when unread
//------------------------------------------------------
Bool TBinaryTree::AddUnreadChildren(TBinaryTree& Data)
{
	Bool Changed = FALSE;

	//	loop through each child to see whether to add it or not
	for ( u32 i=0;	i<Data.GetChildren().GetSize();	i++ )
	{
		TPtr<TBinaryTree> pChild = Data.GetChildren().ElementAt(i);
		
		//	if read pos isnt reset we can assume we didn't read the data in
		//	gr: now if any children HAVE been read we don't store this data
		if ( pChild->IsDataTreeRead() )
			continue;

		//	gr: does this need to clone? (in assets)
		//	save this child
		AddChild( pChild );

	
	#ifdef _DEBUG
		TTempString Debug_String("Storing unread child data (");
		pChild->GetDataRef().GetString( Debug_String );
		Debug_String.Append(") from data (");
		Data.GetDataRef().GetString( Debug_String );
		Debug_String.Append(") into this data (");
		this->GetDataRef().GetString( Debug_String );
		Debug_String.Append("): ");
		pChild->GetDataRef().GetString( Debug_String );
		if ( pChild->GetDataTypeHint().IsValid() )
		{
			Debug_String.Append("(type: ");
			pChild->GetDataTypeHint().GetString( Debug_String );
			Debug_String.Append(")");
		}
		TLDebug_Print( Debug_String );
	#endif

		Changed = TRUE;
	}

	return Changed;
}
