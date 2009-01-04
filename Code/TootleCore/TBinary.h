/*------------------------------------------------------
	
	Binary data

-------------------------------------------------------*/
#pragma once
#include "TLTypes.h"
#include "TArray.h"
#include "TPtr.h"


//---------------------------------------------------------
//	
//---------------------------------------------------------
class TBinary
{
public:
	TBinary();
	TBinary(const u8* pData,u32 DataLength);
	TBinary(const TArray<u8>& Data);

	template<typename TYPE> FORCEINLINE const TYPE*	ReadNoCopy()						{	if ( !CheckDataAvailible( sizeof(TYPE) ) )	return NULL;	const TYPE* pData = (const TYPE*)GetData( m_ReadPos );	MoveReadPos( sizeof(TYPE) );	return pData;	}
	template<typename TYPE> FORCEINLINE Bool		Read(TYPE& Var)						{	return ReadData( (u8*)&Var, sizeof(TYPE) );		}
	template<typename TYPE> FORCEINLINE Bool		Read(TArray<TYPE>& Array)			{	return ReadArray( Array );	}
	template<typename TYPE> FORCEINLINE Bool		ReadAndCut(TYPE& Var)				{	return ReadData( (u8*)&Var, sizeof(TYPE), TRUE );	}
	template<typename TYPE> FORCEINLINE Bool		ReadArray(TArray<TYPE>& Array);		//	reads out the size of the array from our data then the array elements
	FORCEINLINE Bool								ReadAll(TBinary& Data)				{	return Read( Data, GetSizeUnread() );	}	//	read the remaining data into this binary data
	Bool											Read(TBinary& Data,u32 Length);		//	read a chunk of data into this binary data

	template<typename TYPE> FORCEINLINE void	Write(const TYPE& Var)				{	WriteData( (u8*)&Var, sizeof(TYPE) );		}
	template<typename TYPE> FORCEINLINE void	Write(const TArray<TYPE>& Array)	{	WriteArray( Array );	}
	template<typename TYPE> FORCEINLINE void	Write(const TPtr<TYPE>& Pointer)	{	Debug_ReadWritePointerError();	}	//	cant read/write a pointer
	template<typename TYPE> FORCEINLINE void	Write(const TYPE*& Pointer)			{	Debug_ReadWritePointerError();	}	//	cant read/write a pointer
	template<typename TYPE> FORCEINLINE void	WriteToStart(const TYPE& Var)		{	WriteDataToStart( (u8*)&Var, sizeof(TYPE) );	}
	template<typename TYPE> void				WriteArray(const TArray<TYPE>& Array);	//	write an array to the data. we write the element count into the data too to save doing it client side

	Bool							SetSize(u32 NewSize)				{	return m_Data.SetSize( NewSize );	}
	u32								GetSize() const						{	return m_Data.GetSize();	}
	void							ResetReadPos()						{	m_ReadPos = 0;	}
	s32								GetReadPos() const					{	return m_ReadPos;	}
	u32								GetSizeUnread() const;
	u8*								GetData(u32 Offset=0)				{	return &m_Data[Offset];	}
	const u8*						GetData(u32 Offset=0) const			{	return &m_Data[Offset];	}
	TArray<u8>&						GetDataArray()						{	return m_Data;	}
	const TArray<u8>&				GetDataArray() const				{	return m_Data;	}

	void							Empty(Bool Dealloc=FALSE)			{	m_Data.Empty(Dealloc);	m_ReadPos = -1;	}
	void							Compact()							{	m_Data.Compact();	}

	void							Copy(const TBinary& BinaryData)		{	GetDataArray().Copy( BinaryData.GetDataArray() );	}
	u32								GetChecksum() const;				//	get the checksum for the data
	SyncBool						Compress();							//	compress this data - data size should shrink
	SyncBool						Decompress();						//	decompress this data. data size should increase

protected:
	Bool							CheckDataAvailible(u32 DataSize) const;				//	see if this amount of data is readable
	void							MoveReadPos(u32 MoveAmount)							{	m_ReadPos += MoveAmount;	}	//	move read pos along
	Bool							ReadData(u8* pData,u32 Length,Bool CutData=FALSE);	//	read data into address - CutData cuts the read data out of the array
	void							WriteData(const u8* pData,u32 Length)			{	m_Data.Add( pData, Length );	}	//	add data to array
	void							WriteDataToStart(const u8* pData,u32 Length)	{	m_Data.Insert( 0, pData, Length );	}	//	add data to array

private:
	void							Debug_ReadWritePointerError();		//	throw a break if we try to read or write a pointer

protected:
	s32								m_ReadPos;			//	current read position
	TArray<u8>						m_Data;				//	all the file binary data
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