/*------------------------------------------------------
	
	A pod-interleaved array type, probably not POD restricted but
	just needs a binary ref for an element

	It's called a Struct instead of an Interleave, because it makes the code 
	much much neater to read and write

	StructArray
	{
		pod*	GetElement(member,i)
		pod*	GetElement(member,struct*)
		struct*	GetStruct(i)

		TDef
		{
			TMember[]
		}
		
		TStructs[]
		{
			Element
		}
	}

-------------------------------------------------------*/
#pragma once
#include "TKeyArray.h"
#include "TRef.h"
#include "TBinary.h"
#include "TLDebug.h"	//	include for the array check options (eg. DEBUG_CHECK_INDEXES)

#define TLSTRUCT_CHECK_ALIGNMENT	DEBUG_CHECK_INDEXES

namespace TLStruct
{
	//	Struct definition, one per StructArray
	class TDef;

	//	struct containing elements
	typedef TArray<u8>	TStructData;

	//	member definition which define the TStruct
	class TMember;
};





//----------------------------------------------------------------------------//
//	definition of a member in a TStruct, identified by a Ref
//----------------------------------------------------------------------------//
class TLStruct::TMember
{
public:
	void			SetInvalid()			{	m_Type.SetInvalid();	}
	bool			IsValid() const			{	return m_Type.IsValid();	}
	TRefRef			GetType() const			{	return m_Type;	}
	u32				GetTypeSize() const		{	return TLBinary::GetDataTypeSize( m_Type );	}
	u32				GetStructSize() const	{	return GetTypeSize() + m_Stride;	}	//	we can calculate the vertex size from just one element :)
	
public:
	TRef			m_Ref;		//	identifier of this member
	TRef			m_Type;		//	pod type (float,u8,u16,u32)
	u32				m_Offset;	//	byte offset from start of this TDef
	u32				m_Stride;	//	gap between last bit of data and first bit of data of the next case of this element in the struct. Maybe we can get rid of this, it's Struct-OurSize...
};




//----------------------------------------------------------------------------//
//	definition (list of members) and provides the access to data
//----------------------------------------------------------------------------//
class TLStruct::TDef
{
public:
	bool			IsValid() const						{	return m_Members.GetSize()!=0;	}
	void			SetInvalid()						{	m_Members.Empty();	}
/*
	bool			ImportData(TBinaryTree& Data);		//	load definition
	bool			ExportData(TBinaryTree& Data);		//	save definition
*/
	u32				GetStructSize() const				{	return (m_Members.GetSize()==0) ? 0 : m_Members.GetItemAt(0).GetStructSize();	}	//	get the size of one struct
	u32				GetStructCount(const TStructData& StructData) const					{	return GetStructCount( StructData, GetStructSize() );	}
	static u32		GetStructCount(const TStructData& StructData,u32 StructSize);

	//	member manipulation
	bool							HasMember(TRefRef MemberType) const		{	return m_Members.Exists(MemberType);	}
	const TLStruct::TMember*		GetMember(TRefRef MemberType) const		{	return m_Members.Find(MemberType);	}		//	note: lack of validity checking here!

//	const TLStruct::TMember&					GetMember(TRef MemberType) const		{	return *GetMemberPointer(MemberType);	}		//	note: lack of validity checking here!
	const TKeyArray<TRef,TLStruct::TMember>&	GetMembers() const						{	return m_Members;	}

	TLStruct::TMember*						AddMember(TRefRef Member,TRefRef Type);		//	add member to definition
	TLStruct::TMember*						AddMember(const TLStruct::TMember& Member,bool AdjustStrides);	//	add member to definition

/*
	void									AddMember(TRef MemberType,const TLStruct::TMember& Member)					{	m_Members.Add( MemberType, Member );	}
	template<typename ELEMENTTYPE> void		AddMember(TRef MemberType,TStructData* pStructData=NULL,const TArray<ELEMENTTYPE>* pElementData=NULL);	//	add this element to the type and re-align all the vertex data to fit it in
	bool									RemoveMember(TRef MemberType,TStructData* pStructData=NULL);						//	remove element and remove entries from the data

	//	vertex manipulation
	bool									IsStructDataValid(const TStructData& StructData) const	{	return ( StructData.GetSize() % GetStructSize() == 0 );	}
	u32										GetVertexCount(const TStructData& StructData) const;
	s32										AddVertex(const TStructData& StructData);				//	add a vertex
	bool									RemoveVertex(u32 Index,const TStructData& StructData);	//	remove a vertex

	//	element manipulation
	template<typename TYPE> TYPE*			GetElement(TRef MemberType,TStructData& StructData,u32 Index)						{	return const_cast<TYPE*>( GetElement( MemberType, StructData, Index ) );	}
	template<typename TYPE> const TYPE*		GetElement(TRef MemberType,const TStructData& StructData,u32 Index) const;
	template<typename TYPE> TYPE*			GetNextElement(TRef MemberType,TYPE& Element,TStructData& StructData)				{	return const_cast<TYPE*>( GetNextElement( MemberType, StructData, Index ) );	}
	template<typename TYPE> const TYPE*		GetNextElement(TRef MemberType,const TYPE& Element,TStructData& StructData);
	template<typename TYPE> TYPE*			GetPrevElement(TRef MemberType,TYPE& Element,TStructData& StructData)				{	return const_cast<TYPE*>( GetNextElement( MemberType, StructData, Index ) );	}
	template<typename TYPE> const TYPE*		GetPrevElement(TRef MemberType,const TYPE& Element,TStructData& StructData);

	template<typename TYPE> bool			SetElement(TRef MemberType,const TArray<TYPE>& ElementData,TStructData& StructData);	//	copy the values of these elements into our vertex
	template<typename TYPE> bool			SetElement(TRef MemberType,u32 Index,const TYPE& ElementData,TStructData& StructData);	//	set this element's value. returns false if index is out of bounds or if type is wrong, or if this element doesn't exist

private:
	u8*									GetStructData(const TStructData& StructData,u32 VertexIndex)											{	return const_cast<u8*>( GetStructData( pStructData, Index ) );	}					//	hacky version but saves duplicating code
	u8*									GetElementData(const TLStruct::TMember& Member,const TStructData& StructData,u32 VertexIndex)		{	return const_cast<u8*>( GetElementData( Member, StructData, Index ) );	}	//	hacky version but saves duplicating code

	const u8*							GetStructData(const TStructData& StructData,u32 VertexIndex) const;									//	get the address for the start of a vertex
	const u8*							GetElementData(const TLStruct::TMember& Member,const TStructData& StructData,u32 VertexIndex) const	{	return GetStructData( StructData, Index ) + Member.m_Offset;	}
	s32									GetStructDataIndex(const TStructData& StructData,u32 VertexIndex) const;
	const TLStruct::TMember*			GetMemberPointer(TRef MemberType) const															{	return m_Members.Find(MemberType);	}
*/
private:
	TKeyArray<TRef,TLStruct::TMember>	m_Members;
};




//----------------------------------------------------------------------------//
//	actual array for you to use!
//	this will probably have to be templated for the data storage, and later, the TStruct type to aid type safety
//----------------------------------------------------------------------------//
class TStructArray
{
public:
	typedef void TStruct;		//	todo: template this so by default the GetStruct etc calls return a MyStruct* instead of void* and makes casts safe
	
public:
	template<typename TYPE>	
	TStruct*		Add(TRefRef Member,const TYPE& Value);		//	add another struct to the data and set this member's value. returns struct so we can quickly set the other members
	TStruct*		Add();										//	add another struct to the data
	
	u32				GetSize() const								{	return m_Definition.GetStructCount( m_Data );	}	//	get number of structs
	TStruct*		GetStruct(u32 Index)						{	return &m_Data[ GetStructDataIndex( Index ) ];	}
	const TStruct*	GetStruct(u32 Index) const					{	return &m_Data[ GetStructDataIndex( Index ) ];	}
	template<typename TYPE>	TYPE*		GetStruct(u32 Index)		{	return reinterpret_cast<TYPE*>( GetStruct( Index ) );	}
	template<typename TYPE>	const TYPE*	GetStruct(u32 Index) const	{	return reinterpret_cast<const TYPE*>( GetStruct( Index ) );	}
	
	template<typename TYPE>	TYPE&			GetElement(TRefRef Member,u32 Index)			{	return GetElement<TYPE>( Member, GetStruct( Index ) );	}
	template<typename TYPE>	const TYPE&		GetElement(TRefRef Member,u32 Index) const		{	return GetElement<TYPE>( Member, GetStruct( Index ) );	}
	template<typename TYPE>	TYPE&			GetElement(TRefRef Member,TStruct* pStruct);
	template<typename TYPE>	const TYPE&		GetElement(TRefRef Member,const TStruct* pStruct) const;
	
	template<typename TYPE>	void			SetAll(TRefRef Member,const TYPE& Value);
	
	const TLStruct::TDef&	GetDefinition() const							{	return m_Definition;	}

	bool			Debug_VerifyIsStruct(const TStruct*& pStruct) const		{	return true;	}	//	todo
	
public:	//	definition access
	//	add an element to this definition; very very handy for predefined vertex types;
	template<class VERTEXTYPE,typename MEMBERTYPE>
	bool			AddMember(TRefRef Member,MEMBERTYPE VERTEXTYPE::* pVertexMember);
	template<typename TYPE>	
	bool			AddMember(TRefRef Member)					{	return AddMember( Member, TLBinary::GetDataTypeRef<TYPE>() );	}
	bool			AddMember(TRefRef Member,TRefRef Type);		//	add member
	
	
private:
	void			ReAlignData(const TLStruct::TMember& NewMember);	//	realign data when a member has been added
	TStruct*		GetStructAtDataIndex(u32 DataIndex)					{	return const_cast<TStruct*>( const_cast<const TStructArray*>(this)->GetStructAtDataIndex( DataIndex ) );	}
	const TStruct*	GetStructAtDataIndex(u32 DataIndex) const;
	u32				GetStructDataIndex(const TStruct* pStruct) const;
	u32				GetStructDataIndex(u32 StructIndex) const;
	void*			GetElement(const TLStruct::TMember* pMember,TStruct* pStruct,TRefRef ExpectedType)	{	return const_cast<void*>( const_cast<const TStructArray*>(this)->GetElement( pMember, pStruct, ExpectedType ) );	}
	const void*		GetElement(const TLStruct::TMember* pMember,const TStruct* pStruct,TRefRef ExpectedType) const;

private:
	TLStruct::TDef			m_Definition;
	THeapArray<u8>			m_Data;
	//	TLStruct::TStructData	m_Data;
};



//----------------------------------------------------------------------------//
//
//----------------------------------------------------------------------------//
template<typename TYPE>	
FORCEINLINE TYPE& TStructArray::GetElement(TRefRef Member,TStruct* pStruct)
{
	void* pElementData = GetElement( m_Definition.GetMember(Member), pStruct, TLBinary::GetDataTypeRef<TYPE>() );
	if ( !pElementData )
	{
		static TYPE Error_Dummy;
		return Error_Dummy;
	}
	
	return *static_cast<TYPE*>( pElementData );
}

template<typename TYPE>	
FORCEINLINE const TYPE& TStructArray::GetElement(TRefRef Member,const TStruct* pStruct) const
{
	void* pElementData = GetElement( m_Definition.GetMember(Member), pStruct, TLBinary::GetDataTypeRef<TYPE>() );
	if ( !pElementData )
	{
		static TYPE Error_Dummy;
		return Error_Dummy;
	}
	
	return *static_cast<const TYPE*>( pElementData );
}

/*
//----------------------------------------------------------------------------//
//	get pointer to an element in this vertex definition
//----------------------------------------------------------------------------//
template<typename TYPE>
const TYPE* TLStruct::TDef::GetElementData(TRef Element,const u8* pStructData,u32 Index) const
{
	//	get the info
	const TLStruct::TMember* pElementInfo = GetElementInfoPointer( Element );
	if ( !pElementInfo )
		return NULL;

	//	check type
	TRef TypeRef = TLBinary::GetDataTypeRef<TYPE>();
	if ( TypeRef != pElementInfo->GetType() )
	{
		TDebugString Debug_String;
		Debug_String << "Vertex Element type mis-match. Requested element as " << TypeRef << " but vertex element's definiton is " << pElementInfo->GetType();
		TLDebug_Break( Debug_String );
		return NULL;
	}

	//	get the element data and cast it to return
	u8* pStructData = GetElementData( *pElementInfo, pStructData, Index );
	return (TYPE*)pStructData;
}
*/

/*
//----------------------------------------------------------------------------//
//	copy the values of these elements into our vertex
//----------------------------------------------------------------------------//
template<typename TYPE>
bool TLStruct::TDef::SetElement(TRef MemberType,TStructData& StructData,const TArray<TYPE>& ElementData)
{
	//	get the vertex data
	TYPE* pVertexElementData = GetElementData<TYPE>( Element, pStructData, 0 );
	if ( !pVertexElementData )
	{
		//	element doesn't exist, or is of wrong type
		TLDebug_Break("Error getting element data");
		return false;
	}

	//	check count
	u32 StructSize = GetStructSize();
	u32 VertexCount = GetVertexCount(StructData,StructSize);

	//	mis-match of vertex count
	if ( VertexCount != ElementData.GetSize() )
	{
		TDebugString Debug_String;
		Debug_String << "Mis-matched element data count (" << ElementData.GetSize() << ") and vertex count (" << VertexCount << ")";
		TLDebug_Break( Debug_String );
		return false;
	}

	//	iterate through elements and copy
	const TYPE* pElementData = ElementData.GetData();
	for ( u32 i=0;	i<VertexCount;	i++ )
	{
		//	copy element into vertex
		*pVertexElementData = *pElementData;

		//	next
		pElementData++;
		pVertexElementData = GetNextElement( MemberType, *pVertexElementData, StructData );
	}

	return true;
}
*/


/*
//----------------------------------------------------------------------------//
//	set this element's value. returns false if index is out of bounds or if type is wrong, or if this element doesn't exist
//----------------------------------------------------------------------------//
template<typename TYPE> 
bool TLStruct::TDef::SetElement(TRef Element,u32 Index,const TYPE& ElementData,TStructData& StructData)
{
	//	get the element data in the vertex data
	TYPE* pData = GetElementData<TYPE>( Element, StructData, Index );
	if ( !pData )
		return false;

	//	set it
	*pData = ElementData;
	return true;
}

 */

/*
//----------------------------------------------------------------------------//
//	add this element to the type and re-align all the vertex data to fit it in
//----------------------------------------------------------------------------//
template<typename ELEMENTTYPE>
bool TLStruct::TDef::AddMember(TRef MemberType,TStructData* pStructData,const TArray<ELEMENTTYPE>* pElementData)
{
	u32 OldStructSize = GetStructSize();

	//	calc new vertex info
	TLStruct::TMember NewMember;
	NewMember.m_Offset = OldStructSize;		//	goes on the end
	NewMember.SetType<ELEMENTTYPE>();
	NewMember.m_Stride = OldStructSize;
	if ( NewMember.GetSize() != sizeof(ELEMENTTYPE) )
	{
		TLDebug_Break("error calculating size of element");
		return false;
	}

	//	re-align data and put in our new data
	if ( pStructData )
	{
		u32 VertexCount = GetVertexCount();

		//	if no element data has been specified, make up some
		THeapArray<ELEMENTTYPE> TempElementData;
		if ( !pElementData )
		{
			pElementData = &TempElementData;
			TempElementData.SetSize( VertexCount );
			ELEMENTTYPE TempElementValue;
			TempElementData.SetAll( TempElementValue );
		}

		//	for speed, pre-allocate the extra data
		u32 AdditionalNewData = VertexCount * sizeof(ELEMENTTYPE);
		if ( !StructData.AddAlloc( AdditionalNewData ) )
		{
			TDebugString Debug_String;
			Debug_String << "Failed to allocate the extra " << AdditionalNewData << " bytes required to add element to vertex data";
			TLDebug_Break( Debug_String );
			return false;
		}

		//	re-align data to fit in gap by pushing in the space for our new element for each vertex
		//	we go backwards so it's easy to calculate the offset
		s32 StructDataIndex = OldStructSize * (VertexCount-1) + NewMember.m_Offset;	//	index = data start of a vertex + element offset
		ELEMENTTYPE* pElementData = pElementData ? &pElementData->GetAt(VertexCount-1) : NULL;
		for ( ;	StructDataIndex>=0;	StructDataIndex-=OldStructSize,pElementData-- )
		{
			//	plain insert, but we might as well insert our new data!
			StructData.Insert( StructDataIndex, (u8*)pElementData, sizeof(ELEMENTTYPE) );
		}
	}

	//	update the stride on all the other elements
	for ( u32 e=0;	e<m_Elements.GetSize();	e++ )
	{
		TLStruct::TMember& OtherElement = m_Elements.GetItemAt(e);
		OtherElement.m_Stride += NewMember.GetSize();
	}

	//	now add in this new info for completeness
	m_Elements.Add( Element, NewMember );
	
	return true;
}
*/


/*
//----------------------------------------------------------------------------//
//	remove element and remove entries from the data
//----------------------------------------------------------------------------//
template<typename ELEMENTTYPE>
bool TLStruct::TDef::RemoveMember(TRef MemberType,TStructData* pStructData)
{
	TLStruct::TMember* pOldMember = m_Members.Find( MemberType );
	if ( !pOldMember )
		return false;

	//	get the element size we're about to cut out
	u32 OldMemberSize = pOldMember->GetTypeSize();
	u32 OldMemberOffset = pOldMember->GetOffset();

	//	remove data from vertexes
	if ( pStructData )
	{
		u32 VertexCount = GetVertexCount( StructData );

		//	we go backwards so it's easy to calculate the offset
		s32 StructDataIndex = OldStructSize * (VertexCount-1) + OldMemberOffset;	//	index = data start of a vertex + element offset
		for ( ;	StructDataIndex>=0;	StructDataIndex-=OldStructSize )
		{
			//	plain insert, but we might as well insert our new data!
			StructData.RemoveAt( StructDataIndex, OldMemberSize );
		}
	}

	//	remove member from the vertex definition
	m_Members.Remove( MemberType );

	//	update the strides and offsets on the other elements
	//	update the stride on all the other elements
	for ( u32 e=0;	e<m_Members.GetSize();	e++ )
	{
		TLStruct::TMember& OtherMember = m_Elements.GetItemAt(e);
		OtherMember.m_Stride -= OldMemberSize;

		//	if other element is after the one we just removed, then move it's offset
		if ( OtherElement.m_Offset > OldMemberOffset )
			OtherElement.m_Offset -= OldMemberSize;
	}

	return true;
}
*/

/*
//----------------------------------------------------------------------------//
//	get next element in the vertex array. returns NULL if no more. Assumes the Element is valid and already in the array
//----------------------------------------------------------------------------//
template<typename TYPE> 
const TYPE* TLStruct::TDef::GetNextElement(TRef MemberType,const TYPE& Element,TStructData& StructData)
{
	//	get the step information
	u32 StructSize = GetStructSize();

	//	take the current address and move
	const u8* pNextElement = &Element;
	pNextElement += StructSize;

	//	check if this is out of range, in which case we've gone past the end of the array
	if ( pNextElement >= &StructData.ElementLastConst() )
		return NULL;

	return (const TYPE*)pNextElement;
}


//----------------------------------------------------------------------------//
//	get prev element in the vertex array. returns NULL if no more
//----------------------------------------------------------------------------//
template<typename TYPE> 
const TYPE* TLStruct::TDef::GetPrevElement(TRef MemberType,const TYPE& Element,TStructData& StructData)
{
	//	get the step information
	u32 StructSize = GetStructSize();

	//	take the current address and move
	const u8* pNextElement = &Element;
	pNextElement -= StructSize;

	//	check if this is out of range, in which case we've gone past the end of the array
	if ( pNextElement < StructData.GetData() )
		return NULL;

	return (const TYPE*)pNextElement;

}
*/


//----------------------------------------------------------------------------//
//	
//----------------------------------------------------------------------------//
template<typename TYPE>
FORCEINLINE void TStructArray::SetAll(TRefRef Member,const TYPE& Value)
{
	//	initialise data
	for ( u32 i=0;	i<GetSize();	i++ )
	{
		TYPE& Data = GetElement<TYPE>( Member, i );
		Data = Value;
	}
}


//----------------------------------------------------------------------------//
//	add an element to this definition; very very handy for predefined vertex types;
//	eg. class MyVertexType { float3 m_Pos; }
//	AddElement( POSITION, &MyVertexType::m_Pos );
//----------------------------------------------------------------------------//
template<class VERTEXTYPE,typename MEMBERTYPE>
bool TStructArray::AddMember(TRefRef Member,MEMBERTYPE VERTEXTYPE::* pVertexMember)
{
	//u32 OldStructSize = m_Definition.GetStructSize();

	//	make up new member info
	TLStruct::TMember NewMember;
	NewMember.m_Ref = Member;

	//	calc offset 
	VERTEXTYPE Vertex;
	MEMBERTYPE* pTestVertexMember = &(Vertex.*pVertexMember);
	//	gr: it's possible this won't work with multiple inheritance, but I think it will because of the use of templates and not-crap c-casts
	NewMember.m_Offset = ((u8*)pTestVertexMember) - ((u8*)&Vertex);
	
	//	calc type
	NewMember.m_Type = TLBinary::GetDataTypeRef<MEMBERTYPE>();
	
	//	calc stride
	u32 VertexTypeSize = sizeof(VERTEXTYPE);
	u32 MemberTypeSize = sizeof(MEMBERTYPE);
	
	//	err, not possible, but check anyway
	if ( MemberTypeSize > VertexTypeSize )
	{
		TLDebug_Break("Vertex member is bigger than size of the vertex?");
		return false;
	}
	
	NewMember.m_Stride = VertexTypeSize - MemberTypeSize;

	//	attempt add
	TLStruct::TMember* pNewMember = m_Definition.AddMember( NewMember, false );
	if ( !pNewMember )
		return false;

	//	realign data
	ReAlignData( *pNewMember );
	return true;
}



//----------------------------------------------------------------------------//
//	add another struct to the data and set this member's value. returns struct so we can quickly set the other members
//----------------------------------------------------------------------------//
template<typename TYPE>	
TStructArray::TStruct* TStructArray::Add(TRefRef Member,const TYPE& Value)
{
	//	add new struct entry
	TStruct* pStruct = Add();
	if ( !pStruct )
		return NULL;

	//	set that member's value
	GetElement<TYPE>( Member, pStruct ) = Value;
	return pStruct;
}
