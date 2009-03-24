
#include "TLFile.h"



//--------------------------------------------------------
//	
//--------------------------------------------------------
Bool TLString::ReadNextLetter(const TString& String,u32& CharIndex, char& Char)
{
	//	step over whitespace
	s32 NonWhitespaceIndex = String.GetCharIndexNonWhitespace( CharIndex );
	if ( NonWhitespaceIndex == -1 )
		return FALSE;

	//	move char past whitespace
	CharIndex = (u32)NonWhitespaceIndex;
	const char& NextChar = String.GetCharAt(CharIndex);

	//	is next char a letter?
	if ( TLString::IsCharLetter( NextChar ) )
	{
		Char = NextChar;
			
		//	move string past this letter for next thing
		CharIndex++;

		return TRUE;
	}
	else
	{
		//	not a char, could be a number or summink
		return FALSE;
	}
}


//--------------------------------------------------------
//	
//--------------------------------------------------------
Bool TLString::ReadNextInteger(const TString& String,u32& CharIndex,s32& Integer)
{
	//	step over whitespace
	s32 NonWhitespaceIndex = String.GetCharIndexNonWhitespace( CharIndex );

	//	no more non-whitespace? no more data then
	if ( NonWhitespaceIndex == -1 )
		return FALSE;

	//	move char past whitespace
	CharIndex = (u32)NonWhitespaceIndex;

	s32 NextComma = String.GetCharIndex(',', CharIndex);
	s32 NextWhitespace = String.GetCharIndexWhitespace( CharIndex );
	s32 NextSplit = String.GetLength();

	if ( NextComma != -1 && NextComma < NextSplit )
		NextSplit = NextComma;
	if ( NextWhitespace != -1 && NextWhitespace < NextSplit )
		NextSplit = NextWhitespace;

	//	split
	TTempString StringValue;
	StringValue.Append( String, CharIndex, NextSplit-CharIndex );
	if ( !StringValue.GetInteger( Integer ) )
	{
		TLDebug_Break("Failed to parse integer from string");
		return FALSE;
	}

	//	move string along past split
	CharIndex = NextSplit+1;

	//	out of string
	if ( CharIndex >= String.GetLength() )
		CharIndex = String.GetLength();

	return TRUE;
}


//--------------------------------------------------------
//	
//--------------------------------------------------------
Bool TLString::ReadNextFloatArray(const TString& String,u32& CharIndex,float* pFloats,u32 FloatSize,Bool ReturnInvalidFloatZero)
{
	//	loop through parsing seperators and floats
	u32 FloatIndex = 0;
	while ( FloatIndex < FloatSize )
	{
		//	step over whitespace
		s32 NonWhitespaceIndex = String.GetCharIndexNonWhitespace( CharIndex );

		//	no more non-whitespace? no more floats then
		if ( NonWhitespaceIndex == -1 )
			return FALSE;

		//	move char past whitespace
		CharIndex = (u32)NonWhitespaceIndex;

		s32 NextComma = String.GetCharIndex(',', CharIndex);
		s32 NextWhitespace = String.GetCharIndexWhitespace( CharIndex );
		s32 NextSplit = String.GetLength();

		if ( NextComma != -1 && NextComma < NextSplit )
			NextSplit = NextComma;
		if ( NextWhitespace != -1 && NextWhitespace < NextSplit )
			NextSplit = NextWhitespace;

		//	split
		TTempString Stringf;
		Stringf.Append( String, CharIndex, NextSplit-CharIndex );
		if ( !Stringf.GetFloat( pFloats[FloatIndex] ) )
		{
			//	gr: this is a work around when processing floats but encounter invalid floats in strings... say 1.1e07 (invalid floats from outputter)
			if ( ReturnInvalidFloatZero )
			{
				pFloats[FloatIndex] = 0.f;
			}
			else
			{
				TLDebug_Break("Failed to parse first float");
				return FALSE;
			}
		}

		//	next float
		FloatIndex++;

		//	move string along past split
		CharIndex = NextSplit+1;

		//	out of string
		if ( CharIndex >= String.GetLength() )
		{
			CharIndex = String.GetLength();
			break;
		}
	}

	return TRUE;
}





TRef TLFile::GetDataTypeFromString(const TString& String)
{
	//	cache predefined ref types for a simple match
	static TFixedArray<TRef,20> g_DataTypeRefCache;
	if ( g_DataTypeRefCache.GetSize() == 0 )
	{
		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef<TRef>() );

		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef<float>() );
		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef<float2>() );
		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef<float3>() );
		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef<float4>() );

		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef<u8>() );
		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef<u16>() );
		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef<u32>() );
		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef<u64>() );

		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef<s8>() );
		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef<s16>() );
		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef<s32>() );
		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef<s64>() );

		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef_Hex8() );
		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef_Hex16() );
		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef_Hex32() );
		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef_Hex64() );

		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef<TLMaths::TQuaternion>() );
		g_DataTypeRefCache.Add( TLBinary::GetDataTypeRef<TColour>() );
	}

	//	turn string into a ref and check against the ref types...
	TRef StringRef( String );
	if ( g_DataTypeRefCache.Exists( StringRef ) )
	{
		//	matches an existing data type ref
		return StringRef;
	}

#ifdef _DEBUG
	TTempString Debug_String("Warning: using old data type name ");
	Debug_String.Append( String );
	//Debug_String.Append(" in Scheme ");
	//this->GetFileRef().GetString( Debug_String );
	TLDebug_Print( Debug_String );
#endif

	//	old string -> type detection
	if ( String == "float" )		return TLBinary::GetDataTypeRef<float>();
	if ( String == "float2" )		return TLBinary::GetDataTypeRef<float2>();
	if ( String == "float3" )		return TLBinary::GetDataTypeRef<float3>();
	if ( String == "float4" )		return TLBinary::GetDataTypeRef<float4>();
	if ( String == "quaternion" )	return TLBinary::GetDataTypeRef<TLMaths::TQuaternion>();
	if ( String == "colour" )		return TLBinary::GetDataTypeRef<TColour>();
	if ( String == "string" )		return TLBinary::GetDataTypeRef_String();
	if ( String == "widestring" )	return TLBinary::GetDataTypeRef_WideString();

	//	unknown type
#ifdef _DEBUG
	Debug_String.Set("Unsupported data type ");
	Debug_String.Append( String );
	//Debug_String.Append(" in Scheme ");
	//this->GetFileRef().GetString( Debug_String );
	TLDebug_Break( Debug_String );
#endif

	return TRef();
}


//--------------------------------------------------------
//	
//--------------------------------------------------------
SyncBool TLFile::ImportBinaryData(TPtr<TXmlTag>& pTag,TBinary& BinaryData,TRefRef DataType)
{
	//	grab data string
	const TString& DataString = pTag->GetDataString();
	u32 CharIndex = 0;

	if ( DataType == TLBinary::GetDataTypeRef<float>() )
	{
		float f;
		if ( !TLString::ReadNextFloatArray( DataString, CharIndex, &f, 1 ) )
			return SyncFalse;
		BinaryData.Write( f );
		return SyncTrue;
	}
	else if ( DataType == TLBinary::GetDataTypeRef<float2>() )
	{
		float2 f;
		if ( !TLString::ReadNextFloatArray( DataString, CharIndex, f.GetData(), f.GetSize() ) )
			return SyncFalse;
		BinaryData.Write( f );
		return SyncTrue;
	}
	else if ( DataType == TLBinary::GetDataTypeRef<float3>() )
	{
		float3 f;
		if ( !TLString::ReadNextFloatArray( DataString, CharIndex, f.GetData(), f.GetSize() ) )
			return SyncFalse;
		BinaryData.Write( f );
		return SyncTrue;
	}
	else if ( DataType == TLBinary::GetDataTypeRef<float4>() || DataType == TLBinary::GetDataTypeRef<TColour>() )
	{
		float4 f;
		if ( !TLString::ReadNextFloatArray( DataString, CharIndex, f.GetData(), f.GetSize() ) )
			return SyncFalse;
		BinaryData.Write( f );
		return SyncTrue;
	}
	else if ( DataType == TLBinary::GetDataTypeRef<TRef>() )
	{
		TRef Ref( DataString );
		BinaryData.Write( Ref );
		return SyncTrue;
	}
	else if ( DataType == TLBinary::GetDataTypeRef_String() )
	{
		BinaryData.WriteString( DataString );
		return SyncTrue;
	}

#ifdef _DEBUG
	TTempString Debug_String("Unsupported/todo data type ");
	Debug_String.Append( DataString );
	//Debug_String.Append(" in Scheme ");
	//this->GetFileRef().GetString( Debug_String );
	TLDebug_Break( Debug_String );
#endif

	return SyncFalse;
}



//--------------------------------------------------------
//	check if string marked as a datum
//--------------------------------------------------------
Bool TLString::IsDatumString(const TString& String,TRef& DatumRef,TRef& ShapeType)
{
	//	split the string - max at 4 splits, if it splits 4 times, there's too many parts
	TFixedArray<TStringLowercase<TTempString>, 4> StringParts;
	if ( !String.Split( '_', StringParts ) )
	{
		//	didn't split at all, can't be valid
		return FALSE;
	}

	//	check first part is named "datum"
	if ( StringParts[0] != "datum" )
		return FALSE;

	//	should be 3 parts
	if ( StringParts.GetSize() != 3 )
	{
		TLDebug_Break( TString("Malformed Datum name (%s) on SVG geometry. Should be Datum_SHAPEREF_DATUMREF", String.GetData() ) );
		return FALSE;
	}

	//	make shape ref from 2nd string
	ShapeType.Set( StringParts[1] );
	DatumRef.Set( StringParts[2] );
	
	//	if either are invalid, fail
	if ( !ShapeType.IsValid() || !DatumRef.IsValid() )
	{
		TLDebug_Break( TString("Failed to set valid Ref's from Datum identifier: %s", String.GetData() ) );
		return FALSE;
	}

	return TRUE;
}
