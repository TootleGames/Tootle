/*------------------------------------------------------
	
	Binary data in a tree format

-------------------------------------------------------*/
#pragma once
#include "TLTypes.h"
#include "TRef.h"
#include "TPtrArray.h"
#include "TBinary.h"



class TBinaryTree;




class TBinaryTree : public TBinary
{
public:
	TBinaryTree(TRefRef DataRef) : m_DataRef ( DataRef )				{	}
	TBinaryTree(TBinaryTree& OtherTree)									{	ReferenceDataTree( OtherTree, TRUE );	}

	FORCEINLINE TRefRef			GetDataRef() const						{	return m_DataRef;	}
	FORCEINLINE void			SetDataRef(TRefRef DataRef)				{	m_DataRef = DataRef;	}

	TPtr<TBinaryTree>&			GetChild(TRefRef DataRef)				{	return m_Children.FindPtr( DataRef );	}	//	return the first child we find
	TPtrArray<TBinaryTree>&		GetChildren()							{	return m_Children;	}		//	return all the children as an array
	const TPtrArray<TBinaryTree>&	GetChildren() const					{	return m_Children;	}		//	return all the children as an array
	FORCEINLINE u32				GetChildren(TRefRef DataRef,TPtrArray<TBinaryTree>& Children)	{	return m_Children.FindAll( Children, DataRef );	}	//	get all the sections with this ref into an array
	FORCEINLINE u32				GetChildCount() const					{	return GetChildren().GetSize();	}
	TPtr<TBinaryTree>&			AddChild(TRefRef ChildRef);				//	add new child
	TPtr<TBinaryTree>&			AddChild(TPtr<TBinaryTree>& pChild)			{	s32 Index = m_Children.Add( pChild );	return (Index == -1) ? TLPtr::GetNullPtr<TBinaryTree>() : m_Children.ElementAt(Index);	}	//	add child
	//gr: do not implement, causes too many ambiguities. In your client code convert to non const TPtr.......
	//TPtr<TBinaryTree>&			AddChild(const TPtr<TBinaryTree>& pChild)	{	TPtr<TBinaryTree> pNonConstChild = pChild;	return AddChild( pNonConstChild );	}
	Bool						RemoveChild(TRefRef ChildRef)			{	return m_Children.Remove( ChildRef );	}

	void						Empty(Bool Dealloc=FALSE)						{	TBinary::Empty(Dealloc);	m_Children.Empty(Dealloc);	}	//	delete tree
	void						Compact();										//	compact binary data and all our children
	TBinary&					GetData()										{	return (*this);	}
	const TBinary&				GetData() const									{	return (*this);	}
	Bool						CopyDataTree(const TBinaryTree& Data,Bool OverwriteDataRef=TRUE);			//	recursivly copy the tree from Data into this (allocs new data and copies the data)
	FORCEINLINE Bool			CopyDataTree(const TPtr<TBinaryTree>& pData,Bool OverwriteDataRef=TRUE)	{	const TBinaryTree* pBinaryTree = pData.GetObject();	return pBinaryTree ? CopyDataTree( *pBinaryTree, OverwriteDataRef ) : FALSE;	}
	Bool						ReferenceDataTree(const TBinaryTree& Data,Bool OverwriteDataRef=TRUE);			//	copy the tree by re-using the TPtr's to the data. The data is re-used and saves us allocating and copying data but without fear of deletion
	FORCEINLINE Bool			ReferenceDataTree(const TPtr<TBinaryTree>& pData,Bool OverwriteDataRef=TRUE)	{	const TBinaryTree* pBinaryTree = pData.GetObject();	return pBinaryTree ? ReferenceDataTree( *pBinaryTree, OverwriteDataRef ) : FALSE;	}
	Bool						AddUnreadChildren(TBinaryTree& Data);			//	add children from Data to this when unread
	Bool						IsDataTreeRead() const;							//	traverse tree to see if any data has been read (read pos is valid and not -1)

	template<class ARRAYTYPE> 
	Bool						ImportArrays(TRefRef ArrayRef,ARRAYTYPE& Array);	//	returns FALSE if failed, WAIT if nothing imported, TRUE if something imported
	template<class ARRAYTYPE> 
	void						ExportArray(TRefRef ArrayRef,const ARRAYTYPE& Array,Bool WriteIfEmpty=FALSE);
	template<typename TYPE> 
	Bool						ImportData(TRefRef DataRef,TYPE& Data,Bool ConvertData=FALSE);					//	returns FALSE if failed, WAIT if nothing imported, TRUE if something imported
	Bool						ImportDataString(TRefRef DataRef,TString& DataString);	//	gr: remove if I can get Read/Write() to work with a TString in TBinary
	template<typename TYPE> 
	void						ExportData(TRefRef DataRef,const TYPE& Data);
	void						ExportDataString(TRefRef DataRef,const TString& DataString);	//	gr: remove if I can get Read/Write() to work with a TString in TBinary

	FORCEINLINE Bool			operator==(TRefRef DataRef)const		{	return GetDataRef() == DataRef;	}

	void						Debug_PrintTree(u32 TreeLevel=0) const;					//	debug_print the tree
	void						Debug_FailedToRead(TRefRef ChildDataRef,TRefRef TypeRef);	//	print out what data we failed to read

protected:
	TRef						m_DataRef;								//	ref of data
	TPtrArray<TBinaryTree>		m_Children;								//	child binaries
};




//------------------------------------------------------
//	returns FALSE if failed, WAIT if nothing imported, TRUE if something imported
//	gr: changed back to Bool
//------------------------------------------------------
template<class ARRAYTYPE> 
Bool TBinaryTree::ImportArrays(TRefRef ArrayRef,ARRAYTYPE& Array)
{
	SyncBool Result = SyncWait;

	//	get all the data's children with this ref
	TPtrArray<TBinaryTree> DataArray;
	GetChildren( ArrayRef, DataArray);

	//	read the data from these child-data's into the specified array
	for ( u32 i=0;	i<DataArray.GetSize();	i++ )
	{
		TPtr<TBinaryTree>& pChild = DataArray[i];
		pChild->ResetReadPos();
		if ( !pChild->ReadArray( Array ) )
			return FALSE;//SyncFalse;

		//	data was read
		Result = SyncTrue;
	}

	return (Result==SyncTrue);
}


template<class ARRAYTYPE> 
void TBinaryTree::ExportArray(TRefRef ArrayRef,const ARRAYTYPE& Array,Bool WriteIfEmpty)
{
	//	dont write if array is empty
	if ( !WriteIfEmpty && Array.GetSize() == 0 )
		return;

	TPtr<TBinaryTree>& pData = AddChild( ArrayRef );
	pData->WriteArray( Array );
}



//------------------------------------------------------
//	returns FALSE if failed, WAIT if nothing imported, TRUE if something imported
//	gr: changed back to Bool
//------------------------------------------------------
template<typename TYPE> 
Bool TBinaryTree::ImportData(TRefRef DataRef,TYPE& Data,Bool ConvertData)
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
/*
	if ( ConvertData )
	{
		if ( !pData->ReadConvert( Data ) )
			return FALSE;
	}
	else
*/	{
		if ( !pData->Read( Data ) )
		{
			Debug_FailedToRead( DataRef, TLBinary::GetDataTypeRef<TYPE>() );
			return FALSE;
		}
	}

	return TRUE;
}


//------------------------------------------------------
//	
//------------------------------------------------------
template<typename TYPE> 
void TBinaryTree::ExportData(TRefRef DataRef,const TYPE& Data)
{
	//	create a child
	TPtr<TBinaryTree>& pData = AddChild( DataRef );
	pData->Write( Data );
}

