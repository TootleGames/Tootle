/*------------------------------------------------------
	
	Binary data

-------------------------------------------------------*/
#pragma once
#include "TLTypes.h"
#include "TArray.h"
#include "TPtr.h"
#include "TString.h"


namespace TLBinary
{
	template<typename TYPE>
	FORCEINLINE TRefRef	GetDataTypeRef()					{	static TRef g_TypeRef;	return g_TypeRef;	}	//	default unhandled type is just an invalid TRef which means no specific type

	template<typename TYPE>
	FORCEINLINE TRefRef	GetDataTypeRef(const TYPE& Var)		{	return GetDataTypeRef<TYPE>();	}

	//	simple macro to associate type with ref
	#define TLBinary_DeclareDataTypeRef(TYPE,TYPEREF)	\
	namespace TLBinary									\
	{													\
		template<> FORCEINLINE TRefRef	GetDataTypeRef<TYPE>()	{	static TRef g_TypeRef(TYPEREF);	return g_TypeRef;	}	\
	}
	
	//	some special types we can't get from variables
	FORCEINLINE TRef	GetDataTypeRef_Hex8()				{	return TRef("Hex8");	}
	FORCEINLINE TRef	GetDataTypeRef_Hex16()				{	return TRef("Hex16");	}
	FORCEINLINE TRef	GetDataTypeRef_Hex32()				{	return TRef("Hex32");	}
	FORCEINLINE TRef	GetDataTypeRef_Hex64()				{	return TRef("Hex64");	}
	FORCEINLINE TRef	GetDataTypeRef_String()				{	return TRef("String");	}
	FORCEINLINE TRef	GetDataTypeRef_WideString()			{	return TRef("WString");	}
};


//---------------------------------------------------------
//	
//---------------------------------------------------------
class TBinary
{
public:
	TBinary();
	TBinary(const u8* pData,u32 DataLength);
	TBinary(const TArray<u8>& Data);

	template<typename TYPE> FORCEINLINE TYPE*		ReadNoCopy();						//	"read" the data for this next type, but return it as a pointer to the data. and move along the read pos too
	template<typename TYPE> FORCEINLINE Bool		Read(TYPE& Var)						{	return ReadData( (u8*)&Var, sizeof(TYPE) );		}
	template<typename TYPE> FORCEINLINE Bool		Read(TArray<TYPE>& Array)			{	return ReadArray( Array );	}
	template<typename TYPE> FORCEINLINE Bool		Read(TString& String)				{	return ReadArray( String.GetStringArray() );	}
	template<typename TYPE> FORCEINLINE Bool		ReadAndCut(TYPE& Var)				{	return ReadData( (u8*)&Var, sizeof(TYPE), TRUE );	}
	template<typename TYPE> FORCEINLINE Bool		ReadArray(TArray<TYPE>& Array);		//	reads out the size of the array from our data then the array elements
	FORCEINLINE Bool								ReadAll(TBinary& Data)				{	return Read( Data, GetSizeUnread() );	}	//	read the remaining data into this binary data
	Bool											Read(TBinary& Data,u32 Length);		//	read a chunk of data into this binary data

	template<typename TYPE> FORCEINLINE void	Write(const TYPE& Var)				{	WriteData( (u8*)&Var, sizeof(TYPE) );		}
	template<typename TYPE> FORCEINLINE void	Write(const TArray<TYPE>& Array)	{	WriteArray( Array );	}
	template<typename TYPE> FORCEINLINE void	Write(const TString& String)		{	WriteArray( String.GetStringArray() );	}
	template<typename TYPE> FORCEINLINE void	Write(const TPtr<TYPE>& Pointer)	{	Debug_ReadWritePointerError();	}	//	cant read/write a pointer
	template<typename TYPE> FORCEINLINE void	Write(const TYPE*& Pointer)			{	Debug_ReadWritePointerError();	}	//	cant read/write a pointer
	template<typename TYPE> FORCEINLINE void	WriteToStart(const TYPE& Var)		{	WriteDataToStart( (u8*)&Var, sizeof(TYPE) );	}
	template<typename TYPE> void				WriteArray(const TArray<TYPE>& Array);	//	write an array to the data. we write the element count into the data too to save doing it client side

	FORCEINLINE Bool				SetSize(u32 NewSize)				{	return m_Data.SetSize( NewSize );	}
	FORCEINLINE u32					GetSize() const						{	return m_Data.GetSize();	}
	FORCEINLINE void				ResetReadPos()						{	m_ReadPos = 0;	}
	FORCEINLINE s32					GetReadPos() const					{	return m_ReadPos;	}
	u32								GetSizeUnread() const;
	FORCEINLINE u8*					GetData(u32 Offset=0)				{	return &m_Data[Offset];	}
	FORCEINLINE const u8*			GetData(u32 Offset=0) const			{	return &m_Data[Offset];	}
	FORCEINLINE TArray<u8>&			GetDataArray()						{	return m_Data;	}
	FORCEINLINE const TArray<u8>&	GetDataArray() const				{	return m_Data;	}
	FORCEINLINE TRefRef				GetDataTypeHint() const				{	return m_DataTypeHint;	}
	FORCEINLINE void				SetDataTypeHint(TRefRef DataTypeHint)	{	m_DataTypeHint = ( m_DataTypeHint.IsValid() && DataTypeHint != m_DataTypeHint ) ? TRef() : DataTypeHint;	}	//	if we're mixing data types, then set the hint to "unknown"

	FORCEINLINE void				Empty(Bool Dealloc=FALSE)			{	m_Data.Empty(Dealloc);	m_ReadPos = -1;	}
	FORCEINLINE void				Compact()							{	m_Data.Compact();	}

	FORCEINLINE void				Copy(const TBinary& BinaryData)		{	GetDataArray().Copy( BinaryData.GetDataArray() );	m_DataTypeHint = BinaryData.GetDataTypeHint();	}
	u32								GetChecksum() const;				//	get the checksum for the data
	SyncBool						Compress();							//	compress this data - data size should shrink
	SyncBool						Decompress();						//	decompress this data. data size should increase

protected:
	Bool							CheckDataAvailible(u32 DataSize) const;				//	see if this amount of data is readable
	FORCEINLINE void				MoveReadPos(u32 MoveAmount)							{	m_ReadPos += MoveAmount;	}	//	move read pos along
	Bool							ReadData(u8* pData,u32 Length,Bool CutData=FALSE);	//	read data into address - CutData cuts the read data out of the array
	FORCEINLINE void				WriteData(const u8* pData,u32 Length)			{	m_Data.Add( pData, Length );	}	//	add data to array
	FORCEINLINE void				WriteDataToStart(const u8* pData,u32 Length)	{	m_Data.InsertAt( 0, pData, Length );	}	//	add data to array

private:
	void							Debug_ReadWritePointerError();		//	throw a break if we try to read or write a pointer

protected:
	s32								m_ReadPos;			//	current read position
	TArray<u8>						m_Data;				//	all the file binary data
	TRef							m_DataTypeHint;		//	this tells us what kind of data is stored. this is NOT required, but merely a hint for XML output; so if the data is declared as a float[s] then it'll be turned into a readable float in XML
};



//--------------------------------------------------------------------
//	function specialisations
//--------------------------------------------------------------------
template<> FORCEINLINE Bool TBinary::Read(TBinary& Data)				{	return ReadArray( Data.GetDataArray() );	}
template<> FORCEINLINE void TBinary::Write(const TBinary& Data)			{	WriteArray( Data.GetDataArray() );	}


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







TLBinary_DeclareDataTypeRef( u8,	"u8" );
TLBinary_DeclareDataTypeRef( u16,	"u16" );
TLBinary_DeclareDataTypeRef( u32,	"u32" );
TLBinary_DeclareDataTypeRef( u64,	"u64" );

TLBinary_DeclareDataTypeRef( s8,	"s8" );
TLBinary_DeclareDataTypeRef( s16,	"s16" );
TLBinary_DeclareDataTypeRef( s32,	"s32" );
TLBinary_DeclareDataTypeRef( s64,	"s64" );

TLBinary_DeclareDataTypeRef( float,	"flt" );
TLBinary_DeclareDataTypeRef( float2,	"flt2" );
TLBinary_DeclareDataTypeRef( float3,	"flt3" );
TLBinary_DeclareDataTypeRef( float4,	"flt4" );

#include "TLMaths.h"
#include "TColour.h"
TLBinary_DeclareDataTypeRef( TColour,	"col" );
TLBinary_DeclareDataTypeRef( TRef,		"TRef" );
TLBinary_DeclareDataTypeRef( TLMaths::TQuaternion,	"Quat" );
