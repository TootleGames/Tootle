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
	TBinaryTree(TRefRef DataRef);

	TRefRef						GetDataRef() const						{	return m_DataRef;	}
	void						SetDataRef(TRefRef DataRef)				{	m_DataRef = DataRef;	m_DataRef.GetString(m_Debug_DataRefString);	}

	TPtr<TBinaryTree>&			GetChild(TRefRef DataRef)				{	return m_Children.FindPtr( DataRef );	}	//	return the first child we find
	TPtrArray<TBinaryTree>&		GetChildren()							{	return m_Children;	}		//	return all the children as an array
	u32							GetChildren(TRefRef DataRef,TPtrArray<TBinaryTree>& Children)	{	return m_Children.FindAll( Children, DataRef );	}	//	get all the sections with this ref into an array
	TPtr<TBinaryTree>&			AddChild(TRefRef ChildRef);				//	add new child
	TPtr<TBinaryTree>&			AddChild(TPtr<TBinaryTree>& pChild)		{	s32 Index = m_Children.Add( pChild );	return (Index == -1) ? TBinaryTree::g_pNull : m_Children.ElementAt(Index);	}	//	add child
	template <class T>
	TPtr<TBinaryTree>&			AddChildAndData(TRefRef ChildRef, T& Obj);	// Adds a child along with an initial piece of data 

	void						Empty(Bool Dealloc=FALSE)				{	TBinary::Empty(Dealloc);	m_Children.Empty(Dealloc);	}	//	delete tree
	void						Compact();								//	compact binary data and all our children
	TBinary&					GetData()								{	return (*this);	}
	Bool						CopyTree(TPtr<TBinaryTree>& Data);		//	recursivly copy the tree from Data into this (clone)

	template<class ARRAYTYPE> 
	Bool						ImportArrays(TRefRef ArrayRef,ARRAYTYPE& Array);
	template<class ARRAYTYPE> 
	void						ExportArray(TRefRef ArrayRef,const ARRAYTYPE& Array,Bool WriteIfEmpty=FALSE);
	template<typename TYPE> 
	Bool						ImportData(TRefRef DataRef,TYPE& Data);
	template<typename TYPE> 
	void						ExportData(TRefRef DataRef,const TYPE& Data);

	inline Bool					operator==(TRefRef DataRef)const		{	return GetDataRef() == DataRef;	}

	void						Debug_PrintTree(u32 TreeLevel=0) const;	//	debug_print the tree

protected:
	TRef						m_DataRef;								//	ref of data
	TPtrArray<TBinaryTree>		m_Children;								//	child binaries

private:
	static TPtr<TBinaryTree>	g_pNull;								//	NULL pointer so I can return a TPtr ref from various funcs
	TBufferString<6>			m_Debug_DataRefString;					//	for debugging the ref as a string
};







//---------------------------------------------------------
// Adds a child along with an initial piece of data 
//---------------------------------------------------------
template <class T>
TPtr<TBinaryTree>& TBinaryTree::AddChildAndData(TRefRef ChildRef, T& Obj)			
{
	TPtr<TBinaryTree>& pNewChild = AddChild( ChildRef );
	if ( pNewChild )
	{
		pNewChild->Write(Obj);
	}
	return pNewChild;
}




template<class ARRAYTYPE> 
Bool TBinaryTree::ImportArrays(TRefRef ArrayRef,ARRAYTYPE& Array)
{
	//	get all the data's children with this ref
	TPtrArray<TBinaryTree> DataArray;
	GetChildren( ArrayRef, DataArray);

	//	read the data from these child-data's into the specified array
	for ( u32 i=0;	i<DataArray.GetSize();	i++ )
	{
		TPtr<TBinaryTree>& pChild = DataArray[i];
		pChild->ResetReadPos();
		if ( !pChild->ReadArray( Array ) )
			return FALSE;
	}

	return TRUE;
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



template<typename TYPE> 
Bool TBinaryTree::ImportData(TRefRef DataRef,TYPE& Data)
{
	//	get the first child with this ref
	TPtr<TBinaryTree>& pData = GetChild( DataRef );
	if ( !pData )
		return FALSE;

	//	read out var
	pData->ResetReadPos();
	if ( !pData->Read( Data ) )
		return FALSE;

	return TRUE;
}


template<typename TYPE> 
void TBinaryTree::ExportData(TRefRef DataRef,const TYPE& Data)
{
	//	create a child
	TPtr<TBinaryTree>& pData = AddChild( DataRef );
	pData->Write( Data );
}
