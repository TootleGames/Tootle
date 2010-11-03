/*------------------------------------------------------


-------------------------------------------------------*/
#include "TStructArray.h"
#include "TLUnitTest.h"


TEST(TLStructArray_PlainTest)
{
	TStructArray a;

	u8 u = 128;
	u8 u2 = 129;
	float f = 1234.56f;
	float f2 = 99.99f;

	//	should be empty
	CHECK( a.GetSize() == 0 );

	//	simple u8 and float test
	CHECK( a.AddMember<u8>("u8") );
	CHECK( a.GetSize() == 0 );

	TStructArray::TStruct* pStruct0 = a.Add("u8",u);
	CHECK( pStruct0 != NULL );
	CHECK( a.GetSize() == 1 );
	CHECK( a.GetElement<u8>("u8",0u) == u );
	CHECK( a.GetElement<u8>("u8",pStruct0) == u );

	
	//	add another member
	CHECK( a.AddMember<float>("float") );	
	pStruct0 = NULL;	//	now invalid!
	CHECK( a.GetSize() == 1 );
	CHECK( a.GetElement<u8>("u8",0u) == u );

	//	overwrite the float space for our first member
	a.GetElement<float>("float",0u) = f;
	CHECK( a.GetElement<float>("float",0u) == f );
	CHECK( a.GetSize() == 1 );

	//	add another float
	TStructArray::TStruct* pStruct1 = a.Add("float",f2);
	CHECK( a.GetSize() == 2 );
	CHECK( a.GetElement<u8>("u8",1) == TLMemory::Debug_AllocPattern );
	CHECK( pStruct1 );
	CHECK( a.GetElement<float>("float",1) == a.GetElement<float>("float",pStruct1) );
	CHECK( a.GetElement<u8>("u8",1) == a.GetElement<u8>("u8",pStruct1) );
	a.GetElement<u8>("u8",1) = u2;
	CHECK( a.GetElement<u8>("u8",1) == u2 );

}


class TestStruct
{
public:
	TestStruct()						{}
	TestStruct(u16 a,float b,TRef c) :
		m_16	( a ),
		m_f		( b ),
		m_Ref	( c )
	{
	}		
	
public:
	u16		m_16;
	float	m_f;
	TRef	m_Ref;
};


TEST(TLStructArray_StructTest)
{
	TStructArray a;
	TestStruct aTestStruct;

	//	should be empty
	CHECK( a.GetSize() == 0 );
	CHECK( a.GetDefinition().GetStructSize() == 0 );
/*
	//	add members
	a.AddMember( "m_u16", &TestStruct::m_16 );
	CHECK( a.GetDefinition().GetStructSize() == sizeof(TestStruct) );
	CHECK( a.GetDefinition().GetMember("m_u16")->m_Offset == 0 );
	CHECK( a.GetDefinition().GetMember("m_u16")->GetTypeSize() == sizeof(aTestStruct.m_16) );
	
	a.AddMember( "m_f", &TestStruct::m_f );
	CHECK( a.GetDefinition().GetStructSize() == sizeof(TestStruct) );
	CHECK( a.GetDefinition().GetMember("m_f")->m_Offset == sizeof(aTestStruct.m_16) );
	CHECK( a.GetDefinition().GetMember("m_f")->GetTypeSize() == sizeof(aTestStruct.m_f) );

	a.AddMember( "m_Ref", &TestStruct::m_Ref );
	CHECK( a.GetDefinition().GetStructSize() == sizeof(TestStruct) );
	CHECK( a.GetDefinition().GetMember("m_Ref")->m_Offset == sizeof(aTestStruct.m_16) + sizeof(aTestStruct.m_f) );
	CHECK( a.GetDefinition().GetMember("m_Ref")->GetTypeSize() == sizeof(aTestStruct.m_Ref) );
*/
	//	should be empty
	CHECK( a.GetSize() == 0 );

	//	make up some test data to add in
	/*
	THeapArray<TestStruct> Examples;
	Examples.Add( TestStruct( 0, 0, "0" ) );
	Examples.Add( TestStruct( 1, 1.f, "1" ) );
	Examples.Add( TestStruct( 99, 99.f, "99" ) );

	a.Add( Exmaples );
*/
}
/*
//----------------------------------------------------------------------------//
//	load definition
//----------------------------------------------------------------------------//
bool TLStruct::TDef::ImportData(TBinaryTree& Data)
{
	TPtrArray<TBinaryTree>& DataChildren = Data.GetChildren();
	for ( u32 i=0;	i<DataChildren.GetSize();	i++ )
	{
		TBinaryTree& MemberData = *DataChildren[i];
		if ( ElementData.GetDataRef() != TRef_Static(M,e,m,b,e) )
			continue;

		//	read the element type
		ElementData.ResetReadPos();
		TRef ElementTypeRef;
		if ( !ElementData.Read(ElementTypeRef) )
		{
			TLDebug_Break("Failed to read element from vertex definiton data");
			return false;
		}

		//	convert to real element type
		TLStruct::Type ElementType = TLStruct::GetType( ElementTypeRef );
		if ( ElementType == TLStruct::INVALID )
		{
			TDebugString Debug_String;
			Debug_String << "Element type " << ElementTypeRef << " is not a valid element type";
			TLDebug_Break( Debug_String );
			return false;
		}

		//	pull out the data for the element
		TLStruct::TMember ElementInfo;
		bool Success = true;
		Success &= ElementData.ImportData( TRef_Static4(T,y,p,e), ElementInfo.m_Type );
		Success &= ElementData.ImportData( TRef_Static(E,l,e,m,e), ElementInfo.m_Elements );
		Success &= ElementData.ImportData( TRef_Static(O,f,f,s,e), ElementInfo.m_Offset );
		Success &= ElementData.ImportData( TRef_Static(S,t,r,i,d), ElementInfo.m_Stride );
		if ( !Success )
		{
			TDebugString Debug_String;
			Debug_String << "failed to import vertex element data " << ElementTypeRef;
			TLDebug_Break( Debug_String );
			return false;
		}

		//	add to vertex
		AddElement( ElementType, ElementInfo );
	}

	return true;
}



//----------------------------------------------------------------------------//
//	save definition
//----------------------------------------------------------------------------//
bool TLStruct::TDef::ExportData(TBinaryTree& Data)
{
	//	go through each defined element and save it
	const TKeyArray<TLStruct::Type,TLStruct::TMember>& Elements = GetElements();

	for ( u32 e=0;	e<Elements.GetSize();	e++ )
	{
		//	add data entry for this element
		const TLStruct::Type& Element = Elements.GetKeyAt(e);
		TRef ElementRef = TLStruct::GetRef( Element );
		TBinaryTree& ElementData = *Data.ExportData( TRef_Static(E,l,e,m,e), ElementRef );
	
		//	get the data
		const TLStruct::TMember& ElementData = Elements.GetItemAt( e );
		
		//	export the data in the element info
		ElementData.ExportData( TRef_Static4(T,y,p,e), ElementData.m_Type );
		ElementData.ExportData( TRef_Static(E,l,e,m,e), ElementData.m_Elements );
		ElementData.ExportData( TRef_Static(O,f,f,s,e), ElementData.m_Offset );
		ElementData.ExportData( TRef_Static(S,t,r,i,d), ElementData.m_Stride );
	}

	return true;
}

//----------------------------------------------------------------------------//
//	convert this vertex data into another format
//----------------------------------------------------------------------------//
bool TLStruct::TDef::ConvertTo(const TVertexDef& OtherDef,const TArray<u8>& Data)
{
	TLDebug_Break("todo");
	return false;
}


//----------------------------------------------------------------------------//
//	count number of vertexes
//----------------------------------------------------------------------------//
u32 TLStruct::TDef::GetVertexCount(const TArray<u8>& VertexData) const
{
	u32 DataSize = VertexData.GetSize();
	u32 VertexSize = GetVertexSize();
	if ( DataSize % VertexSize != 0 )
	{
		TLDebug_Break("Vertex data mis-alignment");
		return 0;
	}

	return (VertexSize==0) ? 0 : DataSize / VertexSize;
}

//----------------------------------------------------------------------------//
//	get the size of one vertex
//----------------------------------------------------------------------------//
u32 TLStruct::TDef::GetVertexSize() const
{
	//	we only need one info to calculate the vertex size :)
	if ( m_Elements.GetSize() == 0 )
		return 0;
	return m_Elements[0].GetVertexSize();
#if 0
	u32 Size = 0;
	for ( u32 i=0;	i<m_Elements.GetSize();	i++ )
	{
		const TLStruct::TMember& ElementInfo = m_Elements.GetItemAt(i);
		Size += ElementInfo.GetSize();
	}
	return Size;
#endif
}

//----------------------------------------------------------------------------//
//	get the address for the start of a vertex
//----------------------------------------------------------------------------//
const u8* TLStruct::TDef::GetElementData(const TLStruct::TMember& ElementInfo,u32 Index,const u8* pVertexData) const
{
	//	get vertex size
	//	don't calc vertex size if we don't need it
	u32 VertexSize = (Index == 0) ? 0 : GetVertexSize();
	u32 VertexOffset = Index * VertexSize;

	//	return data offset
	return pVertexData[VertexOffset];
}


//----------------------------------------------------------------------------//
//	add a vertex
//----------------------------------------------------------------------------//
s32 TLStruct::TDef::AddVertex(const TArray<u8>& VertexData)
{
	//	add data onto the end of the array
	u32 VertexSize = GetVertexSize();
	u32 OldSize = VertexData.GetSize();
	if ( !VertexData.SetSize( OldSize + VertexSize ) )
		return -1;

	return OldSize;
}


//----------------------------------------------------------------------------//
//	remove a vertex
//----------------------------------------------------------------------------//
bool TLStruct::TDef::RemoveVertex(u32 VertexIndex,const TArray<u8>& VertexData)
{
	//	get the data index for this vertex
	u32 VertexSize = GetVertexSize();
	s32 DataIndex = GetVertexDataIndex( VertexData, VertexIndex );
	if ( DataIndex < 0 )
		return false;
	return VertexData.RemoveAt( (u32)DataIndex, VertexSize );
}

*/


//----------------------------------------------------------------------------//
//	get number of structs
//----------------------------------------------------------------------------//
u32 TLStruct::TDef::GetStructCount(const TStructData& StructData,u32 StructSize)
{
	//	don't mod zero!
	if ( StructSize == 0 )
		return 0;

	u32 DataSize = StructData.GetSize();

#if defined(TLSTRUCT_CHECK_ALIGNMENT)
	if ( DataSize % StructSize != 0 )
	{
		TLDebug_Break("data size/struct mis-alignment");
		return 0;
	}
#endif

	return DataSize / StructSize;
}


//----------------------------------------------------------------------------//
//	realign data when a member has been added
//----------------------------------------------------------------------------//
void TStructArray::ReAlignData(const TLStruct::TMember& NewMember)
{
	u32 MemberSize = NewMember.GetTypeSize();
	if ( MemberSize > m_Definition.GetStructSize() )
	{
		TLDebug_Break("Error: New struct size is too small as new member is bigger than struct size. Means old struct size would be negative");
		return;
	}

	//	re-align data to put in a gap for the new element
	u32 OldStructSize = m_Definition.GetStructSize() - MemberSize;
	u32 StructCount = m_Definition.GetStructCount( m_Data, OldStructSize );

	//	nothing to change (presumably an empty array)
	if ( !StructCount )
		return;

	//	for speed, pre-allocate the extra data
	u32 AdditionalNewData = StructCount * MemberSize;
	if ( !m_Data.AddAllocSize( AdditionalNewData ) )
	{
		TDebugString Debug_String;
		Debug_String << "Failed to allocate the extra " << AdditionalNewData << " bytes required to add member to struct data";
		TLDebug_Break( Debug_String );
		return;
	}
	
	//	little buffer to make the inserting easier
	TFixedArray<u8,32> InsertBuffer( MemberSize, TLMemory::Debug_AllocPattern );

	//	re-align data to fit in gap by pushing in the space for our new element for each vertex
	//	we go backwards so it's easy to calculate the offset
	s32 StructDataIndex = OldStructSize * (StructCount-1);	//	index = data start of a struct
	for ( ;	StructDataIndex>=0;	StructDataIndex-=OldStructSize )
	{
		//m_Data.ShiftArray( StructDataIndex, MemberSize );
		m_Data.InsertAt( StructDataIndex + NewMember.m_Offset, InsertBuffer );
	}
}

//----------------------------------------------------------------------------//
//	add member
//----------------------------------------------------------------------------//
bool TStructArray::AddMember(TRefRef Member,TRefRef Type)
{
	//	add new member to definition
	TLStruct::TMember* pNewMember = m_Definition.AddMember( Member, Type );
	if ( !pNewMember )
		return false;

	//	realign our data
	ReAlignData( *pNewMember );
	return true;
}



//----------------------------------------------------------------------------//
//	add member to definition
//----------------------------------------------------------------------------//
TLStruct::TMember* TLStruct::TDef::AddMember(const TLStruct::TMember& Member,bool AdjustStrides)
{
	//	check a member with this ref doesn't already exist
	if ( HasMember( Member.m_Ref ) )
	{
		TDebugString Debug_String;
		Debug_String << "StructArray definition already has a member called " << Member.m_Ref;
		TLDebug_Break( Debug_String );
		return NULL;
	}

	//	fetch the type size, this will validate it
	u32 TypeSize = TLBinary::GetDataTypeSize(Member.m_Type);
	if ( TypeSize == 0 )
	{
		TDebugString Debug_String;
		Debug_String << "Failed to get POD type size for " << Member.m_Type;
		TLDebug_Break( Debug_String );
		return NULL;
	}

	//	we don't want to adjust strides when based on a struct... maybe can calculate this?
	//	or just recalculate strides...
	if ( AdjustStrides )
	{
		//	update the stride on all the other elements
		for ( u32 e=0;	e<m_Members.GetSize();	e++ )
		{
			TLStruct::TMember& OtherMember = m_Members.GetItemAt(e);
			OtherMember.m_Stride += TypeSize;
		}
	}

	//	now add in this new info for completeness
	TLStruct::TMember* pNewMember = m_Members.Add( Member.m_Ref, Member );
	
	//	something went wrong!
	if ( !pNewMember )
	{
		TLDebug_Break("Failed to add new member, struct is corrupt!");
		return NULL;
	}

	return pNewMember;
}

//----------------------------------------------------------------------------//
//	add member to definition
//----------------------------------------------------------------------------//
TLStruct::TMember* TLStruct::TDef::AddMember(TRefRef Member,TRefRef Type)
{
	//	setup the new member
	TLStruct::TMember NewMember;
	NewMember.m_Ref = Member;
	NewMember.m_Offset = GetStructSize();		//	goes on the end
	NewMember.m_Type = Type;
	NewMember.m_Stride = GetStructSize();		//	the stride is the rest of the struct (maybe we can get rid of this)

	return AddMember( NewMember, true );
}


//----------------------------------------------------------------------------//
//	add another struct to the data
//----------------------------------------------------------------------------//
TStructArray::TStruct* TStructArray::Add()
{
	//	if the struct is empty, we have nothing to add!
	u32 StructSize = m_Definition.GetStructSize();
	if ( StructSize == 0 )
		return NULL;

	s32 DataIndex = m_Data.Add( TLMemory::Debug_AllocPattern, StructSize );
	if ( DataIndex == -1 )
		return NULL;
	return GetStructAtDataIndex( DataIndex );
}



const TStructArray::TStruct* TStructArray::GetStructAtDataIndex(u32 DataIndex) const
{
	//	check for alignment to make sure this data index IS the start of a struct
	u32 StructSize = m_Definition.GetStructSize();
	if ( StructSize == 0 )
		return NULL;
	
#if defined(DEBUG_CHECK_INDEXES)
	if ( DataIndex % StructSize != 0 )
	{
		TLDebug_Break("DataIndex provided for struct is mis-aligned");
		return NULL;
	}
#endif
	
	return &m_Data[ DataIndex ];	
}

const void* TStructArray::GetElement(const TLStruct::TMember* pMember,const TStructArray::TStruct* pStruct,TRefRef ExpectedType) const
{
	//	assumes pStruct is not null (only a pointer because it's a void*) and is in the array
	if ( !pStruct || !Debug_VerifyIsStruct( pStruct ) )
	{
		TLDebug_Break("Use of invalid struct");
		return NULL;
	}
	
	//	check member is valid
	if ( !pMember )
	{
		TLDebug_Break("Member expected");
		return NULL;
	}
	
#if defined(_DEBUG)
	if ( ExpectedType.IsValid() && pMember->GetType() != ExpectedType )
	{
		TLDebug_Break("Member type mis-match");
		return NULL;
	}
#endif
	
	//	return the data
	const u8* pData = reinterpret_cast<const u8*>( pStruct );
	return &pData[ pMember->m_Offset ];
}



u32 TStructArray::GetStructDataIndex(const TStruct* pStruct) const
{
	if ( !pStruct )
		TLDebug_Break("Struct Expected");

	//	negatively out of range
	if ( pStruct < m_Data.GetData() )
	{
		TLDebug_Break("Struct is not a struct of this data");
		return 0;
	}
		
	u32 DataIndex = m_Data.GetData() - (u8*)pStruct;
#if defined(DEBUG_CHECK_INDEXES)
	m_Data.ElementAtConst( DataIndex );
#endif
	return DataIndex;
}


u32 TStructArray::GetStructDataIndex(u32 StructIndex) const
{
	StructIndex *= m_Definition.GetStructSize();
#if defined(DEBUG_CHECK_INDEXES)
	m_Data.ElementAtConst( StructIndex );
#endif
	return StructIndex;
}

