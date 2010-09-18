/*------------------------------------------------------
	
	Binary data

-------------------------------------------------------*/
#pragma once
#include "TLTypes.h"
#include "TArray.h"
#include "TPtr.h"
#include "TString.h"
#include "TRef.h"


namespace TLBinary
{
	//	default unhandled type is just an invalid TRef which means no specific type
	template<typename TYPE>
	FORCEINLINE TRef	GetDataTypeRef()					{	return TRef();	}	

	//	get the size of the data a ref refers to, but only useful at compile time (so for templated functions) ... 
	//	... probably rarely used but in TLBinary::GetSizeOf() stops any potential code-errors
	template<u32 TYPEREF>
	FORCEINLINE u32		GetDataTypeSize()					{	return 0;	}
	u32					GetDataTypeSize(TRefRef TypeRef);
	
	//	get number of elements in this type ref
	template<u32 TYPEREF>
	FORCEINLINE u32		GetDataTypeElementCount()					{	return 0;	}
	u32					GetDataTypeElementCount(TRefRef TypeRef);
	
	//	same as above but ability to use a variable for the TYPE instead of explicitly using <XYZ>
	template<typename TYPE>
	FORCEINLINE TRef	GetDataTypeRef(const TYPE& Var)		{	return GetDataTypeRef<TYPE>();	}

	#define TLBinary_Private_DeclareDataTypeRef(ELEMENTTYPE,TYPE,TYPEREF,ELEMENTCOUNT)		\
		namespace TLBinary										\
		{														\
			template<> FORCEINLINE TRef	GetDataTypeRef<TYPE>()				{	return TYPEREF;	}	\
			template<> FORCEINLINE u32	GetDataTypeSize<TYPEREF>()			{	return sizeof(TYPE);	}	\
			template<> FORCEINLINE u32	GetDataTypeElementCount<TYPEREF>()	{	return ELEMENTCOUNT;	}	\
		}
	
	#define TLBinary_Private_DeclareDataTypeRefNoSize(ELEMENTTYPE,TYPE,TYPEREF)		\
		namespace TLBinary									\
		{													\
			template<> FORCEINLINE TRef	GetDataTypeRef<TYPE>()		{	return TYPEREF;	}	\
		}
	
	//	simple macro to associate type with ref by specialising the GetDataTypeRef template.
	//	if you don't specify this then it's likely your data will lose its type information when writing OUT to say XML
	#define TLBinary_DeclareDataTypeRef(TYPE)		\
		TLBinary_Private_DeclareDataTypeRef( TYPE, TYPE,		TLBinary_TypeRef( TYPE ), 1 );			\
	
	#define TLBinary_DeclareDataTypeWithRef(TYPE,TYPEREF)		\
		TLBinary_Private_DeclareDataTypeRef( TYPE, TYPE,		TYPEREF, 1 );			\

	//	another one to do TYPE and TYPE2,TYPE3,TYPE4
	#define TLBinary_DeclareDataTypeRefAll(TYPE)	\
		TLBinary_Private_DeclareDataTypeRef( TYPE, TYPE,		TLBinary_TypeRef( TYPE ), 1 );			\
		TLBinary_Private_DeclareDataTypeRef( TYPE, Type2<TYPE>,	TLBinary_TypeNRef( Type2, TYPE ), 2 );	\
		TLBinary_Private_DeclareDataTypeRef( TYPE, Type3<TYPE>,	TLBinary_TypeNRef( Type3, TYPE ), 3 );	\
		TLBinary_Private_DeclareDataTypeRef( TYPE, Type4<TYPE>,	TLBinary_TypeNRef( Type4, TYPE ), 4 );	

	#define TLBinary_DeclareDataTypeRefAllNoSize(TYPE)	\
		TLBinary_Private_DeclareDataTypeRefNoSize( TYPE, TYPE,			TLBinary_TypeRef( TYPE ) );			\
		TLBinary_Private_DeclareDataTypeRefNoSize( TYPE, Type2<TYPE>,	TLBinary_TypeNRef( Type2, TYPE ) );	\
		TLBinary_Private_DeclareDataTypeRefNoSize( TYPE, Type3<TYPE>,	TLBinary_TypeNRef( Type3, TYPE ) );	\
		TLBinary_Private_DeclareDataTypeRefNoSize( TYPE, Type4<TYPE>,	TLBinary_TypeNRef( Type4, TYPE ) );	
	
	//	gr: accessor to get a define of a TRef for a type. You need to use these for case statements, plus it helps to pre-compile refs more
	#define TLBinary_TypeRef(Type)				TLBinary_TypeRef_##Type
	#define TLBinary_TypeNRef(TypeN,Type)		TLBinary_TypeRef_##TypeN##_##Type
	



	//	some special types we can't get from variables
	//	gr: "hex" versions are to detect hex-representation in XML.
	//		the "u8" data is written as integers
	#define TLBinary_TypeRef_Hex8				TRef_Static4(H,e,x,EIGHT)
	#define TLBinary_TypeRef_Hex16				TRef_Static(H,e,x,ONE,SIX)
	#define TLBinary_TypeRef_Hex32				TRef_Static(H,e,x,THREE,TWO)
	#define TLBinary_TypeRef_Hex64				TRef_Static(H,e,x,SIX,FOUR)
	#define TLBinary_TypeRef_String				TRef_Static(S,t,r,i,n)
	#define TLBinary_TypeRef_WideString			TRef_Static(W,S,t,r,i)

	FORCEINLINE TRef	GetDataTypeRef_Hex8()				{	return TLBinary_TypeRef(Hex8);	}
	FORCEINLINE TRef	GetDataTypeRef_Hex16()				{	return TLBinary_TypeRef(Hex16);	}
	FORCEINLINE TRef	GetDataTypeRef_Hex32()				{	return TLBinary_TypeRef(Hex32);	}
	FORCEINLINE TRef	GetDataTypeRef_Hex64()				{	return TLBinary_TypeRef(Hex64);	}
	FORCEINLINE TRef	GetDataTypeRef_String()				{	return TLBinary_TypeRef(String);	}
	FORCEINLINE TRef	GetDataTypeRef_WideString()			{	return TLBinary_TypeRef(WideString);	}
};



//---------------------------------------------------------
//	
//---------------------------------------------------------
class TBinary
{
public:
	FORCEINLINE TBinary() : m_ReadPos ( -1 )								{	}
	FORCEINLINE TBinary(const u8* pData,u32 DataLength) : m_ReadPos ( -1 )	{	WriteData( pData, DataLength );	}
	FORCEINLINE TBinary(const TArray<u8>& Data) : m_ReadPos ( -1 )			{	WriteArray( Data );	}
	FORCEINLINE TBinary(const TBinary& Data) : m_ReadPos ( -1 )				{	WriteData( Data );	}

	template<typename TYPE> DEPRECATED FORCEINLINE TYPE*		ReadNoCopy();						//	"read" the data for this next type, but return it as a pointer to the data. and move along the read pos too
	template<typename TYPE> FORCEINLINE TYPE*		Get() const;						//	return the data as this type. Does type checking and will return NULL if mis-match of types.
	template<typename TYPE> FORCEINLINE Bool		Read(TYPE& Var)						{	return ReadData( (u8*)&Var, sizeof(TYPE) );		}

	template<typename TYPE,u32 GROWBY,class SORTPOLICY> 
	FORCEINLINE Bool								Read(THeapArray<TYPE,GROWBY,SORTPOLICY>& Array)			{	return ReadArray( Array );	}
	template <typename TYPE,u32 SIZE,class SORTPOLICY>
	FORCEINLINE Bool								Read(TFixedArray<TYPE,SIZE,SORTPOLICY>& Array)			{	return ReadArray( Array );	}
	template<typename TYPE> 
	FORCEINLINE Bool								Read(TArray<TYPE>& Array)			{	return ReadArray( Array );	}

	template<typename> FORCEINLINE Bool				Read(TString& String)				{	return ReadString( String );	}
	template<typename TYPE> FORCEINLINE Bool		ReadAndCut(TYPE& Var)				{	return ReadData( (u8*)&Var, sizeof(TYPE), TRUE );	}
	template<typename TYPE> FORCEINLINE Bool		ReadArray(TArray<TYPE>& Array);		//	reads out the size of the array from our data then the array elements
	FORCEINLINE Bool								ReadAll(TBinary& Data)				{	return Read( Data, GetSizeUnread() );	}	//	read the remaining data into this binary data
	Bool											Read(TBinary& Data,u32 Length);		//	read a chunk of data into this binary data
	Bool											ReadString(TString& String);		//	read data into a string - handles ascii and unicode
//	template<typename TYPE> FORCEINLINE Bool		ReadConvert(TYPE& Var);				//	read data, if the types mis match do a conversion (= operator)
	Bool											ReadData(u8* pData,u32 Length,Bool CutData=FALSE);	//	read data into address - CutData cuts the read data out of the array
	template<typename TYPE>
	Bool											ReadDataIntoArray(TArray<TYPE>& Array,u32 Elements)	{	u32 OldSize = Array.GetSize();	if ( !Array.SetSize( OldSize + Elements ) )	return false;	return ReadData( (u8*)&Array.ElementAt(OldSize), Elements * sizeof(TYPE) );	}

	template<typename TYPE> FORCEINLINE void	Write(const TYPE& Var)				{	SetDataTypeHint<TYPE>();	WriteData( (u8*)&Var, sizeof(TYPE) );		}
	template<typename TYPE> FORCEINLINE void	Write(const TArray<TYPE>& Array)	{	SetDataTypeHint<TYPE>();	WriteArray( Array );	}
	template<typename TYPE,u32 GROWBY,class SORTPOLICY> 
	FORCEINLINE void							Write(const THeapArray<TYPE,GROWBY,SORTPOLICY>& Array)			{	WriteArray( Array );	}
	template <typename TYPE,u32 SIZE,class SORTPOLICY>
	FORCEINLINE void							Write(const TFixedArray<TYPE,SIZE,SORTPOLICY>& Array)				{	WriteArray( Array );	}
//	template<typename TYPE> 
//	FORCEINLINE void							Write(const TArray<TYPE>& Array)	{	WriteArray( Array );	}
	
	template<typename> FORCEINLINE void			Write(const TString& String)		{	WriteString( String );	}
	//specialised below! template<> FORCEINLINE void	Write(const TBinary& Data)
	template<typename TYPE> FORCEINLINE void	Write(const TPtr<TYPE>& Pointer)	{	Debug_ReadWritePointerError();	}	//	cant read/write a pointer
	template<typename TYPE> FORCEINLINE void	Write(const TYPE*& Pointer)			{	Debug_ReadWritePointerError();	}	//	cant read/write a pointer
	template<typename TYPE> FORCEINLINE void	WriteToStart(const TYPE& Var)		{	SetDataTypeHint<TYPE>();	WriteDataToStart( (u8*)&Var, sizeof(TYPE) );	}
	template<typename TYPE> void				WriteArray(const TArray<TYPE>& Array);	//	write an array to the data. we write the element count into the data too to save doing it client side
	FORCEINLINE void							WriteString(const TString& String)	{	WriteArray( String.GetStringArray() );	SetDataTypeHint( TLBinary_TypeRef(WideString), TRUE );	}	//	gr: set the FORCED hint AFTER we write the string, the string uses the array functionality whihc writes an array size first. As long as strings are parsed with ReadString this isn't a problem
	FORCEINLINE void							WriteData(const u8* pData,u32 Length)	{	m_Data.Add( pData, Length );	}	//	add raw data to array
	FORCEINLINE void							WriteData(const TBinary& Data)		{	WriteData( Data.GetData(), Data.GetSize() );	}	//	add raw data to array
	template<typename TYPE>
	FORCEINLINE void							WriteData(const TArray<TYPE>& Data)	{	m_Data.Add( (const u8*)Data.GetData(), sizeof(TYPE) * Data.GetSize() );	}	//	write an array as if it were raw data
	Bool										WriteDataHexString(const TString& String,TRef TypeHint);	//	write data from hex string

	template<typename TYPE> FORCEINLINE void	AllocData(u32 Elements)				{	m_Data.AddAllocSize( Elements * sizeof(TYPE) );	}
	
	FORCEINLINE Bool				SetSize(u32 NewSize)				{	return m_Data.SetSize( NewSize );	}
	FORCEINLINE u32					GetSize() const						{	return m_Data.GetSize();	}
	FORCEINLINE Bool				IsUnread() const					{	return (m_ReadPos == -1);	}
	FORCEINLINE void				SetUnread()							{	m_ReadPos = -1;	}
	FORCEINLINE void				ResetReadPos()						{	m_ReadPos = 0;	}
	FORCEINLINE s32					GetReadPos() const					{	return m_ReadPos;	}
	FORCEINLINE u32					GetSizeUnread() const;
	FORCEINLINE u8*					GetData(u32 Offset=0)				{	return &m_Data[Offset];	}
	FORCEINLINE const u8*			GetData(u32 Offset=0) const			{	return &m_Data[Offset];	}
	FORCEINLINE TArray<u8>&			GetDataArray()						{	return m_Data;	}
	FORCEINLINE const TArray<u8>&	GetDataArray() const				{	return m_Data;	}
	void							GetDataHexString(TString& String,bool FromWideString,bool ToWideString) const;	//	get all the data in the form of a hex string (does not include size, type hint or read info)
	FORCEINLINE TRefRef				GetDataTypeHint() const				{	return m_DataTypeHint;	}
	FORCEINLINE void				SetDataTypeHint(TRefRef DataTypeHint,Bool Force=FALSE)	{	m_DataTypeHint = DataTypeHint.GetData() * ( DataTypeHint == m_DataTypeHint || GetSize() == 0 || Force );	}	//	if we're mixing data types, then set the hint to "unknown". 
	template<typename TYPE> void	SetDataTypeHint()					{	SetDataTypeHint( TLBinary::GetDataTypeRef<TYPE>() );	}

	FORCEINLINE void				Empty(Bool Dealloc=FALSE)			{	m_Data.Empty(Dealloc);	m_ReadPos = -1;	}
	FORCEINLINE void				Compact()							{	m_Data.Compact();	}

	FORCEINLINE void				Copy(const TBinary& BinaryData)		{	GetDataArray().Copy( BinaryData.GetDataArray() );	m_DataTypeHint = BinaryData.GetDataTypeHint();	}
	u32								GetChecksum() const;				//	get the checksum for the data
	SyncBool						Compress();							//	compress this data - data size should shrink
	SyncBool						Decompress();						//	decompress this data. data size should increase

	//	gr: this is much faster, but very unsafe, we can look at using it for various bits of functionality, 
	//		better to use ReadNoCopy() at runtime for the same efficiency AND safety
	template<typename TYPE> 
	FORCEINLINE const TYPE&			Debug_GetDataAs() const				{	return *((const TYPE*)m_Data.GetData());	}

protected:
	FORCEINLINE Bool				CheckDataAvailible(u32 DataSize) const;				//	see if this amount of data is readable
	FORCEINLINE void				MoveReadPos(u32 MoveAmount)							{	m_ReadPos += MoveAmount;	}	//	move read pos along
	FORCEINLINE void				WriteDataToStart(const u8* pData,u32 Length)	{	m_Data.InsertAt( 0, pData, Length );	}	//	add data to array

private:
	void							Debug_ReadWritePointerError();		//	throw a break if we try to read or write a pointer

protected:
	s32								m_ReadPos;			//	current read position
	THeapArray<u8>					m_Data;				//	all the file binary data
	TRef							m_DataTypeHint;		//	this tells us what kind of data is stored. this is NOT required, but merely a hint for XML output; so if the data is declared as a float[s] then it'll be turned into a readable float in XML
};


/*
//--------------------------------------------------------------------
//	read data, if the types mis match do a conversion (= operator)
//--------------------------------------------------------------------
template<typename TYPE> 
FORCEINLINE Bool TBinary::ReadConvert(TYPE& Var)
{
	//	we don't know our type, we just have to read
	if ( !GetDataTypeHint().IsValid() )
		return Read( Var );

	//	get the type of data Var is
	TRef VarType = TLBinary::GetDataTypeRef<TYPE>();

	//	is the same type anyway
	if ( VarType == GetDataTypeHint() )
		return Read( Var );

	//	do conversion

	//	gr: this was a genius idea, but need to get an instance of our HintType to copy into, or
	//	at least cast to with ReadNoCopy... but cannot do it without some kinda ref->variable/alloc system
	TLDebug_Break("not implemented yet");
	return Read( Var );
}
*/

//--------------------------------------------------------------------
//	function specialisations
//--------------------------------------------------------------------
template<> FORCEINLINE Bool TBinary::Read(TBinary& Data)				
{
	return ReadArray( Data.GetDataArray() );	
}

template<> FORCEINLINE void TBinary::Write(const TBinary& Data)
{	
	WriteArray( Data.GetDataArray() );	
}

/*
template<>
template<typename TYPE,u32 GROWBY,class SORTPOLICY> 
FORCEINLINE Bool TBinary::Read(THeapArray<TYPE,GROWBY,SORTPOLICY>& Array)			
{
	return ReadArray( Array );	
}

template<>
template<typename TYPE,u32 SIZE,class SORTPOLICY>
FORCEINLINE Bool TBinary::Read(TFixedArray<TYPE,SIZE,SORTPOLICY>& Array)
{
	return ReadArray( Array );	
}

template<>
template<typename TYPE>
FORCEINLINE Bool TBinary::Read(TArray<TYPE>& Array)
{
	return ReadArray( Array );	
}
*/

//--------------------------------------------------------------------
//	reads out the size of the array from our data then the array elements
//--------------------------------------------------------------------
template<typename TYPE>
Bool TBinary::ReadArray(TArray<TYPE>& Array)
{
	//	read out array size
	u32 ArraySize = 0;
	if ( !Read( ArraySize ) )
		return FALSE;

	//	resize array and read each element
	if ( !Array.SetSize( ArraySize ) )
		return FALSE;

	//	gr: speed up, for data types we can just read the data in as one big chunk
	/*	todo: test before commit
	if ( Array.IsElementDataType() )
	{
		if ( !ReadData( (u8*)Array.GetData(), Array.GetDataSize() )
			return FALSE;
	}
	else	*/
	{
		//	read each element (must be done to support arrays of arrays
		for ( u32 i=0;	i<Array.GetSize();	i++ )
		{
			if ( !Read( Array[i] ) )
			{
				//	failed on this element, resize array to how much we did read
				Array.SetSize(i);
				return FALSE;
			}
		}
	}

	return TRUE;
}


//--------------------------------------------------------------------
//	write an array to the data. we write the element count into the data too
//	to save doing it client side
//--------------------------------------------------------------------
template<typename TYPE>
void TBinary::WriteArray(const TArray<TYPE>& Array)
{
	//	write array size
	u32 ArraySize = Array.GetSize();	
	Write( ArraySize );	
	
	//	gr: to fix the writing of an array of an array we have to write the elements individually... if it's not a data type
	if ( Array.IsElementDataType() )
	{
		WriteData( (u8*)Array.GetData(), Array.GetDataSize() );	
	}
	else
	{
		//	write each element individually
		for ( u32 i=0;	i<Array.GetSize();	i++ )
		{
			Write( Array[i] );
		}
	}
}


//--------------------------------------------------------------------
//	return the data as this type. Does type checking and will return NULL if mis-match of types. If the data type is unknown then we will blindly cast and return something
//--------------------------------------------------------------------
template<typename TYPE> 
FORCEINLINE TYPE* TBinary::Get() const
{
	if ( !CheckDataAvailible( sizeof(TYPE) ) )	
		return NULL;	

	//	compare data type hints
	TRef DataType = GetDataTypeHint();
	if ( DataType.IsValid() )
	{
		TRef TypeHint = TLBinary::GetDataTypeRef<TYPE>();
		if ( DataType != TypeHint )
			return NULL;
	}
	else
	{
		//	hint not specified, so we assume it's okay to cast. These should be rare anyway
	}

	TYPE* pData = (TYPE*)GetData( m_ReadPos );
	return pData;	
}


//--------------------------------------------------------------------
//	"read" the data for this next type, but return it as a pointer to the data
//	and move along the read pos too
//--------------------------------------------------------------------
template<typename TYPE> 
FORCEINLINE TYPE* TBinary::ReadNoCopy()						
{
	if ( !CheckDataAvailible( sizeof(TYPE) ) )	
		return NULL;	
	
	TYPE* pData = (TYPE*)GetData( m_ReadPos );	
	MoveReadPos( sizeof(TYPE) );	
	return pData;	
}

//--------------------------------------------------------------------
//	
//--------------------------------------------------------------------
FORCEINLINE u32 TBinary::GetSizeUnread() const			
{	
	if ( m_ReadPos == -1 )
	{
		TLDebug_Break("Invalid Readpos");	
		return 0;
	}
	
	return GetSize() - m_ReadPos;
}


//-----------------------------------------------------------
//	see if this amount of data is readable
//-----------------------------------------------------------
FORCEINLINE Bool TBinary::CheckDataAvailible(u32 DataSize) const
{
	//	read position hasn't been initialised
	if ( m_ReadPos < 0 )
	{
		TLDebug_Break("Binary read position hasn't been reset");
		return FALSE;
	}

	//	have we got this amount of data left?
	if ( GetSizeUnread() < DataSize )
	{
#ifdef _DEBUG
		TString Debug_String("Trying to read more data(%d bytes) than we have remaining(%d bytes).", DataSize, GetSizeUnread() );
		if ( GetDataTypeHint().IsValid() )
		{
			Debug_String.Append(" Data type: ");
			GetDataTypeHint().GetString( Debug_String );
		}

		TLDebug_Break( Debug_String );
#endif
		return FALSE;
	}

	return TRUE;
}



#define TLBinary_TypeRef_u8				TRef_Static2(u,EIGHT)
#define TLBinary_TypeRef_Type2_u8		TRef_Static4(u,EIGHT,UNDERSCORE,TWO)
#define TLBinary_TypeRef_Type3_u8		TRef_Static4(u,EIGHT,UNDERSCORE,THREE)
#define TLBinary_TypeRef_Type4_u8		TRef_Static4(u,EIGHT,UNDERSCORE,FOUR)
#define TLBinary_TypeRef_u16			TRef_Static3(u,ONE,SIX)
#define TLBinary_TypeRef_Type2_u16		TRef_Static5(u,ONE,SIX,UNDERSCORE,TWO)
#define TLBinary_TypeRef_Type3_u16		TRef_Static5(u,ONE,SIX,UNDERSCORE,THREE)
#define TLBinary_TypeRef_Type4_u16		TRef_Static5(u,ONE,SIX,UNDERSCORE,FOUR)
#define TLBinary_TypeRef_u32			TRef_Static3(u,THREE,TWO)
#define TLBinary_TypeRef_Type2_u32		TRef_Static5(u,THREE,TWO,UNDERSCORE,TWO)
#define TLBinary_TypeRef_Type3_u32		TRef_Static5(u,THREE,TWO,UNDERSCORE,THREE)
#define TLBinary_TypeRef_Type4_u32		TRef_Static5(u,THREE,TWO,UNDERSCORE,FOUR)
#define TLBinary_TypeRef_u64			TRef_Static3(u,SIX,FOUR)
#define TLBinary_TypeRef_Type2_u64		TRef_Static5(u,SIX,FOUR,UNDERSCORE,TWO)
#define TLBinary_TypeRef_Type3_u64		TRef_Static5(u,SIX,FOUR,UNDERSCORE,THREE)
#define TLBinary_TypeRef_Type4_u64		TRef_Static5(u,SIX,FOUR,UNDERSCORE,FOUR)
#define TLBinary_TypeRef_s8				TRef_Static2(s,EIGHT)
#define TLBinary_TypeRef_Type2_s8		TRef_Static4(s,EIGHT,UNDERSCORE,TWO)
#define TLBinary_TypeRef_Type3_s8		TRef_Static4(s,EIGHT,UNDERSCORE,THREE)
#define TLBinary_TypeRef_Type4_s8		TRef_Static4(s,EIGHT,UNDERSCORE,FOUR)
#define TLBinary_TypeRef_char			TLBinary_TypeRef_s8
#define TLBinary_TypeRef_Type2_char		TLBinary_TypeRef_Type2_s8
#define TLBinary_TypeRef_Type3_char		TLBinary_TypeRef_Type3_s8
#define TLBinary_TypeRef_Type4_char		TLBinary_TypeRef_Type4_s8
#define TLBinary_TypeRef_s16			TRef_Static3(s,ONE,SIX)
#define TLBinary_TypeRef_Type2_s16		TRef_Static5(s,ONE,SIX,UNDERSCORE,TWO)
#define TLBinary_TypeRef_Type3_s16		TRef_Static5(s,ONE,SIX,UNDERSCORE,THREE)
#define TLBinary_TypeRef_Type4_s16		TRef_Static5(s,ONE,SIX,UNDERSCORE,FOUR)
#define TLBinary_TypeRef_s32			TRef_Static3(s,THREE,TWO)
#define TLBinary_TypeRef_Type2_s32		TRef_Static5(s,THREE,TWO,UNDERSCORE,TWO)
#define TLBinary_TypeRef_Type3_s32		TRef_Static5(s,THREE,TWO,UNDERSCORE,THREE)
#define TLBinary_TypeRef_Type4_s32		TRef_Static5(s,THREE,TWO,UNDERSCORE,FOUR)
#define TLBinary_TypeRef_s64			TRef_Static3(s,SIX,FOUR)
#define TLBinary_TypeRef_Type2_s64		TRef_Static5(s,SIX,FOUR,UNDERSCORE,TWO)
#define TLBinary_TypeRef_Type3_s64		TRef_Static5(s,SIX,FOUR,UNDERSCORE,THREE)
#define TLBinary_TypeRef_Type4_s64		TRef_Static5(s,SIX,FOUR,UNDERSCORE,FOUR)
#define TLBinary_TypeRef_float			TRef_Static3(f,l,t)
#define TLBinary_TypeRef_Type2_float	TRef_Static4(f,l,t,TWO)
#define TLBinary_TypeRef_Type3_float	TRef_Static4(f,l,t,THREE)
#define TLBinary_TypeRef_Type4_float	TRef_Static4(f,l,t,FOUR)
#define TLBinary_TypeRef_float2			TLBinary_TypeNRef(Type2,float)
#define TLBinary_TypeRef_float3			TLBinary_TypeNRef(Type3,float)
#define TLBinary_TypeRef_float4			TLBinary_TypeNRef(Type4,float)
#define TLBinary_TypeRef_Bool			TRef_Static4(B,o,o,l)
#define TLBinary_TypeRef_Type2_Bool		TRef_Static(B,o,o,l,TWO)
#define TLBinary_TypeRef_Type3_Bool		TRef_Static(B,o,o,l,THREE)
#define TLBinary_TypeRef_Type4_Bool		TRef_Static(B,o,o,l,FOUR)
#define TLBinary_TypeRef_TRef			TRef_Static4(T,R,e,f)
#define TLBinary_TypeRef_Type2_TRef		TRef_Static(T,R,e,f,TWO)
#define TLBinary_TypeRef_Type3_TRef		TRef_Static(T,R,e,f,THREE)
#define TLBinary_TypeRef_Type4_TRef		TRef_Static(T,R,e,f,FOUR)
#define TLBinary_TypeRef_TTypedRef			TRef_Static(T,y,R,e,f)
#define TLBinary_TypeRef_Type2_TTypedRef	TRef_Static(T,y,R,f,TWO)
#define TLBinary_TypeRef_Type3_TTypedRef	TRef_Static(T,y,R,f,THREE)
#define TLBinary_TypeRef_Type4_TTypedRef	TRef_Static(T,y,R,f,FOUR)


TLBinary_DeclareDataTypeRefAll( u8 );
TLBinary_DeclareDataTypeRefAll( u16 );
TLBinary_DeclareDataTypeRefAll( u32 );
TLBinary_DeclareDataTypeRefAll( u64 );
TLBinary_DeclareDataTypeRefAll( s8 );
TLBinary_DeclareDataTypeRefAllNoSize( char );	//	gr: pc compiler thinks this is a different type to s8...
TLBinary_DeclareDataTypeRefAll( s16 );
TLBinary_DeclareDataTypeRefAll( s32 );
TLBinary_DeclareDataTypeRefAll( s64 );
TLBinary_DeclareDataTypeRefAll( float );
TLBinary_DeclareDataTypeRefAll( Bool );


TLBinary_DeclareDataTypeRefAll( TRef );

TLBinary_DeclareDataTypeRefAll( TTypedRef );



