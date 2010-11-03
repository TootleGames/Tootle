/*------------------------------------------------------
	
	Binary data

-------------------------------------------------------*/
#pragma once
#include "TLTypes.h"
#include "TLBinary.h"
#include "TArray.h"
#include "TPtr.h"
#include "TString.h"
#include "TRef.h"



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


