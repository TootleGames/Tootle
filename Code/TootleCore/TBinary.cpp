#include "TBinary.h"
#include "TLMemory.h"
#include "TRef.h"
#include "TString.h"
#include "TColour.h"




//-----------------------------------------------------------
//	get the size of a type from a ref - like sizeof() for a ref
//-----------------------------------------------------------
u32	TLBinary::GetDataTypeSize(TRefRef TypeRef)
{
	//	gr: see if we can find some way to automate this in TLBinary_DeclareDataTypeRef...
#define case_TLBinary_TypeToSize(PodType)	\
case TLBinary_TypeRef(PodType):			return GetDataTypeSize<TLBinary_TypeRef(PodType)>();	\
case TLBinary_TypeNRef(Type2,PodType):	return GetDataTypeSize<TLBinary_TypeNRef(Type2,PodType)>();	\
case TLBinary_TypeNRef(Type3,PodType):	return GetDataTypeSize<TLBinary_TypeNRef(Type3,PodType)>();	\
case TLBinary_TypeNRef(Type4,PodType):	return GetDataTypeSize<TLBinary_TypeNRef(Type4,PodType)>();	
	
	switch ( TypeRef.GetData() )
	{
			case_TLBinary_TypeToSize( float );
			case_TLBinary_TypeToSize( u8 );
			case_TLBinary_TypeToSize( s8 );
			case_TLBinary_TypeToSize( u16 );
			case_TLBinary_TypeToSize( s16 );
			case_TLBinary_TypeToSize( u32 );
			case_TLBinary_TypeToSize( s32 );
			case_TLBinary_TypeToSize( TRef );
			case_TLBinary_TypeToSize( TColour24 );
			case_TLBinary_TypeToSize( TColour32 );
			case_TLBinary_TypeToSize( TColour64 );
	}
	
#undef case_TLBinary_TypeToSize
	
	TDebugString Debug_String;
	Debug_String << TypeRef << " is an unhandled case";
	TLDebug_Break( Debug_String );
	return 0;
}


//-----------------------------------------------------------
//	get the number of elements of a type from a ref - like sizeofarray() for a ref
//-----------------------------------------------------------
u32	TLBinary::GetDataTypeElementCount(TRefRef TypeRef)
{
	//	gr: see if we can find some way to automate this in TLBinary_DeclareDataTypeRef...
#define case_TLBinary_TypeToElementCount(PodType)	\
case TLBinary_TypeRef(PodType):			return GetDataTypeElementCount<TLBinary_TypeRef(PodType)>();	\
case TLBinary_TypeNRef(Type2,PodType):	return GetDataTypeElementCount<TLBinary_TypeNRef(Type2,PodType)>();	\
case TLBinary_TypeNRef(Type3,PodType):	return GetDataTypeElementCount<TLBinary_TypeNRef(Type3,PodType)>();	\
case TLBinary_TypeNRef(Type4,PodType):	return GetDataTypeElementCount<TLBinary_TypeNRef(Type4,PodType)>();	
	
	switch ( TypeRef.GetData() )
	{
		case_TLBinary_TypeToElementCount( float );
		case_TLBinary_TypeToElementCount( u8 );
		case_TLBinary_TypeToElementCount( s8 );
		case_TLBinary_TypeToElementCount( u16 );
		case_TLBinary_TypeToElementCount( s16 );
		case_TLBinary_TypeToElementCount( u32 );
		case_TLBinary_TypeToElementCount( s32 );
		case TLBinary_TypeRef(TColour24):	return GetDataTypeElementCount<TLBinary_TypeRef(TColour24)>();
		case TLBinary_TypeRef(TColour32):	return GetDataTypeElementCount<TLBinary_TypeRef(TColour32)>();
		case TLBinary_TypeRef(TColour64):	return GetDataTypeElementCount<TLBinary_TypeRef(TColour64)>();
		case TLBinary_TypeRef(TColour):		return GetDataTypeElementCount<TLBinary_TypeRef(TColour)>();
	}
	
#undef case_TLBinary_TypeToElementCount
	
	TDebugString Debug_String;
	Debug_String << TypeRef << " is an unhandled case";
	TLDebug_Break( Debug_String );
	return 0;
}

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


//--------------------------------------------------------------------
//	get all the data in the form of a hex string (does not include size, type hint or read info)
//--------------------------------------------------------------------
void TBinary::GetDataHexString(TString& String,bool FromWideString,bool ToWideString) const
{
	TLDebug_Break("gr: untested with unicode");

	//	don't allow cropping of ascii strings
	if ( FromWideString && !ToWideString )
	{
		if ( !TLDebug_Break("Converting from wide-string to non-widestring, this will lose data, so not allowed - retry to allow...") )
		{
			return;
		}
	}

	const char HexChars[16+1] = "0123456789ABCDEF";
	for ( u32 i=0;	i<m_Data.GetSize();	i++ )
	{
		const u8& Data = m_Data[i];

		//	if source is missing first byte, add zeros
		if ( ToWideString && !FromWideString )
		{
			String.Append('0');
			String.Append('0');
		}

		u8 PartAIndex = (Data >> 4) & 0x0F;
		String.Append( HexChars[PartAIndex] );

		u8 PartBIndex = (Data) & 0x0F;
		String.Append( HexChars[PartBIndex] );
	}
}

//--------------------------------------------------------------------
//	write data from hex string "00112233aabbccff"
//	each character (ascii or unicode) in the string is half a byte
//--------------------------------------------------------------------
Bool TBinary::WriteDataHexString(const TString& String,TRef TypeHint)
{
	TLDebug_Break("gr: untested with unicode");

	u32 StringDataLength = String.GetLength();
	const TArray<TChar>& StringArray = String.GetStringArray();

	//	string shouldn't have an odd character left over
	if ( StringDataLength % 2 == 1 )
	{
		TLDebug_Break("Hex string provided doesn't have even number of characters");
		return FALSE;
	}

	u32 OrigDataSize = m_Data.GetSize();

	for ( u32 i=0;	i<StringDataLength;	i+=2 )
	{
		//	work out the numbers from the hex characters
		s32 PartA = TLString::GetCharHexInteger( StringArray[i+0] );
		s32 PartB = TLString::GetCharHexInteger( StringArray[i+1] );
		if ( PartA == -1 || PartB == -1 )
		{
			TLDebug_Break("Non-hex character found in hex-string");
			m_Data.SetSize( OrigDataSize );
			return FALSE;
		}

		//	make byte
		u8 PartA8 = (u8)PartA;
		u8 PartB8 = (u8)PartB;
		u8 Byte = (PartA8 << 4) & 0xF0;
		Byte |= (PartB8) & 0x0F;

		//	save byte
		m_Data.Add( Byte );
	}

	return TRUE;
}


//--------------------------------------------------------------------
//	
//--------------------------------------------------------------------
Bool TBinary::ReadString(TString& String)			
{
	//	if data is known as a widestring we can just do a simple read-array
	if ( GetDataTypeHint() == TLBinary_TypeRef_WideString )
		return ReadArray( String.GetStringArray() );

	//	read into ascii array
	THeapArray<TChar8> AsciiString;
	if ( !ReadArray( AsciiString ) )
		return FALSE;

	//	set string (does our conversion)
	String.Set( AsciiString.GetData(), AsciiString.GetSize() );
	
	return TRUE;
}

