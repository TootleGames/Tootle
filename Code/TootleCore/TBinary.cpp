#include "TBinary.h"
#include "TLMemory.h"
#include "TRef.h"
#include "TString.h"




//-----------------------------------------------------------
//	read data into address
//-----------------------------------------------------------
Bool TBinary::ReadData(u8* pData,u32 Length,Bool CutData)
{
	if ( !TBinary::CheckDataAvailible( Length ) )
		return FALSE;

	//	get our file data
	const u8* pFileData = GetData(m_ReadPos);

	//	copy into memory
	TLMemory::CopyData( pData, pFileData, Length );

	//	remove the data we just read
	if ( CutData )
	{
		m_Data.RemoveAt( m_ReadPos, Length );
	}
	else
	{
		//	keeping the data so move read pos along
		MoveReadPos( Length );
	}

	return TRUE;
}


//--------------------------------------------------------------------
//	read the remaining data into this binary data
//--------------------------------------------------------------------
Bool TBinary::Read(TBinary& Data,u32 Length)
{
	//	allocate additional mem in the data
	u32 OldSize = Data.GetSize();

	//	cannot go this large
	if ( !Data.SetSize( OldSize + Length ) )
		return FALSE;

	//	fetch our newly allocated data
	u8* pBinaryData = Data.GetData( OldSize );
	if ( !pBinaryData )
		return FALSE;

	//	copy to binary data
	if ( !ReadData( pBinaryData, Length ) )
	{
		//	resize binary back to size before we modified it
		Data.SetSize( OldSize );
		return FALSE;
	}

	return TRUE;
}


//--------------------------------------------------------------------
//	get the checksum for the data
//--------------------------------------------------------------------
u32 TBinary::GetChecksum() const
{
	//	really dumb checksum
	u32 Checksum = 0;
	for ( u32 i=0;	i<GetSize();	i++ )
	{
		Checksum += m_Data[i];
	}

	return Checksum;
}

//--------------------------------------------------------------------
//	compress this data - data size should shrink
//--------------------------------------------------------------------
SyncBool TBinary::Compress()
{
	return SyncFalse;
}

//--------------------------------------------------------------------
//	decompress this data. data size should increase
//--------------------------------------------------------------------
SyncBool TBinary::Decompress()
{
	return SyncFalse;
}



//--------------------------------------------------------------------
//	throw a break if we try to read or write a pointer
//--------------------------------------------------------------------
void TBinary::Debug_ReadWritePointerError()
{
	TLDebug_Break("Cannot read or write a pointer to a TBinary");
}

