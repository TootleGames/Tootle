#include "TLFile.h"
#include <TootleCore/TBinaryTree.h>



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
//	reads an integer out of a string, and does a min/max CheckInRange check. 
//	returns FALSE if out of range (in debug only, uses TLDebug_CHeckInRange)
//--------------------------------------------------------
Bool TLString::ReadIntegerInRange(const TString& String,s32& Integer,s32 Min,s32 Max)
{
	if ( !String.GetInteger( Integer ) )
		return FALSE;

	if ( !TLDebug_CheckInRange( Integer, Min, Max ) )
		return FALSE;

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
	//	turn string into a ref and check against the ref types...
	TRef StringRef( String );

	//	add "tootle data xml" supported types to this case statement
	switch ( StringRef.GetData() )
	{
		case TLBinary_TypeRef(TRef):
		case TLBinary_TypeRef(Bool):

		case TLBinary_TypeRef(float):
		case TLBinary_TypeRef(float2):
		case TLBinary_TypeRef(float3):
		case TLBinary_TypeRef(float4):

		case TLBinary_TypeRef(u8):
		case TLBinary_TypeRef(u16):
		case TLBinary_TypeRef(u32):
		case TLBinary_TypeRef(u64):

		case TLBinary_TypeRef(s8):
		case TLBinary_TypeRef(s16):
		case TLBinary_TypeRef(s32):
		case TLBinary_TypeRef(s64):

		case TLBinary_TypeRef_Hex8:
		case TLBinary_TypeRef_Hex16:
		case TLBinary_TypeRef_Hex32:
		case TLBinary_TypeRef_Hex64:

		case TLBinary_TypeRef(TQuaternion):
		case TLBinary_TypeRef(TEuler):
		case TLBinary_TypeRef(TAxisAngle):

		case TLBinary_TypeRef(TColour):
		case TLBinary_TypeRef(TColour24):
		case TLBinary_TypeRef(TColour32):
		case TLBinary_TypeRef(TColour64):
	
		case TLBinary_TypeRef_String:
		case TLBinary_TypeRef_WideString:
		{
			//	matches an existing data type ref
			return StringRef;
		}
		break;

		default:
			break;
	};

#ifdef _DEBUG
	TTempString Debug_String("Warning: using old data type name ");
	Debug_String.Append( String );
	TLDebug_Print( Debug_String );
#endif

	//	old string -> type detection
	if ( String == "float" )		return TLBinary::GetDataTypeRef<float>();
	if ( String == "float2" )		return TLBinary::GetDataTypeRef<float2>();
	if ( String == "float3" )		return TLBinary::GetDataTypeRef<float3>();
	if ( String == "float4" )		return TLBinary::GetDataTypeRef<float4>();
	if ( String == "quaternion" )	return TLBinary::GetDataTypeRef<TLMaths::TQuaternion>();
	if ( String == "colour" )		return TLBinary::GetDataTypeRef<TColour>();

	//	unknown type
#ifdef _DEBUG
	Debug_String.Set("Unsupported data type ");
	Debug_String.Append( String );
	TLDebug_Break( Debug_String );
#endif

	return TRef_Invalid;
}


//--------------------------------------------------------
//	
//--------------------------------------------------------
SyncBool TLFile::ImportBinaryData(TPtr<TXmlTag>& pTag,TBinary& BinaryData,TRefRef DataType)
{
	//	grab data string
	const TString& DataString = pTag->GetDataString();
	u32 CharIndex = 0;

	switch ( DataType.GetData() )
	{
	case TLBinary_TypeRef(float):
	{
		float f;
		if ( !TLString::ReadNextFloatArray( DataString, CharIndex, &f, 1 ) )
			return SyncFalse;
		BinaryData.Write( f );
		return SyncTrue;
	}

	case TLBinary_TypeRef(float2):
	{
		float2 f;
		if ( !TLString::ReadNextFloatArray( DataString, CharIndex, f.GetData(), f.GetSize() ) )
			return SyncFalse;
		BinaryData.Write( f );
		return SyncTrue;
	}
	
	case TLBinary_TypeRef(float3):
	{
		float3 f;
		if ( !TLString::ReadNextFloatArray( DataString, CharIndex, f.GetData(), f.GetSize() ) )
			return SyncFalse;
		BinaryData.Write( f );
		return SyncTrue;
	}
	
	case TLBinary_TypeRef(float4):
	{
		float4 f;
		if ( !TLString::ReadNextFloatArray( DataString, CharIndex, f.GetData(), f.GetSize() ) )
			return SyncFalse;
		BinaryData.Write( f );
		return SyncTrue;
	}
	
	case TLBinary_TypeRef(TQuaternion):
	{
		float4 f;
		if ( !TLString::ReadNextFloatArray( DataString, CharIndex, f.GetData(), f.GetSize() ) )
			return SyncFalse;

		//	convert to normalised quaternion
		TLMaths::TQuaternion Quat( f );
		Quat.Normalise();
		BinaryData.Write( Quat );
		return SyncTrue;
	}
	
	case TLBinary_TypeRef(TEuler):
	{
		float3 f;
		if ( !TLString::ReadNextFloatArray( DataString, CharIndex, f.GetData(), f.GetSize() ) )
			return SyncFalse;

		//	convert to Euler type
		TLMaths::TEuler Euler( f );
		BinaryData.Write( Euler );
		return SyncTrue;
	}
	
	case TLBinary_TypeRef(TAxisAngle):
	{
		float4 f;
		if ( !TLString::ReadNextFloatArray( DataString, CharIndex, f.GetData(), f.GetSize() ) )
			return SyncFalse;

		//	convert to normalised quaternion
		TLMaths::TAxisAngle AxisAngle( f );
		BinaryData.Write( AxisAngle );
		return SyncTrue;
	}
	
	case TLBinary_TypeRef(TRef):
	{
		TRef Ref( DataString );
		BinaryData.Write( Ref );
		return SyncTrue;
	}
	
	case TLBinary_TypeRef_String:
	{
		//	do string cleanup, convert "\n" to a linefeed etc
		if ( TLString::IsStringDirty( DataString ) )
		{
			TString OutputString = DataString;
			TLString::CleanString( OutputString );
			BinaryData.WriteString( OutputString );
		}
		else
		{
			//	already clean, just write the original
			BinaryData.WriteString( DataString );
		}

		return SyncTrue;
	}
	
	case TLBinary_TypeRef(TColour):
	{
		float4 f;
		if ( !TLString::ReadNextFloatArray( DataString, CharIndex, f.GetData(), f.GetSize() ) )
			return SyncFalse;
		
		//	check range
		//	gr: use TLDebug_CheckInRange() ?
		if ( f.x > 1.0f || f.x < 0.0f ||
			f.y > 1.0f || f.y < 0.0f ||
			f.z > 1.0f || f.z < 0.0f ||
			f.w > 1.0f || f.w < 0.0f )
		{
			if ( !TLDebug_Break( TString("Colour float type has components out of range (0..1); %.3f,%.3f,%.3f,%.3f", f.x, f.y, f.z, f.w) ) )
				return SyncFalse;
		}

		TColour Colour( f );
		BinaryData.Write( Colour );
		return SyncTrue;
	}
	
	case TLBinary_TypeRef(TColour24):
	{
		Type3<s32> Colours;
		if ( !TLString::ReadNextInteger( DataString, CharIndex, Colours.x ) )		return SyncFalse;
		if ( !TLString::ReadNextInteger( DataString, CharIndex, Colours.y ) )		return SyncFalse;
		if ( !TLString::ReadNextInteger( DataString, CharIndex, Colours.z ) )		return SyncFalse;
		
		//	check range
		//	gr: use TLDebug_CheckInRange() ?
		if ( Colours.x > 255 || Colours.x < 0 ||
			Colours.y > 255 || Colours.y < 0 ||
			Colours.z > 255 || Colours.z < 0 )
		{
			if ( !TLDebug_Break( TString("Colour24 type has components out of range (0..255); %d,%d,%d", Colours.x, Colours.y, Colours.z ) ) )
				return SyncFalse;
		}

		TColour24 Colour( Colours.x, Colours.y, Colours.z );
		BinaryData.Write( Colour );
		return SyncTrue;
	}
	
	case TLBinary_TypeRef(TColour32):
	{
		Type4<s32> Colours;
		if ( !TLString::ReadNextInteger( DataString, CharIndex, Colours.x ) )		return SyncFalse;
		if ( !TLString::ReadNextInteger( DataString, CharIndex, Colours.y ) )		return SyncFalse;
		if ( !TLString::ReadNextInteger( DataString, CharIndex, Colours.z ) )		return SyncFalse;
		if ( !TLString::ReadNextInteger( DataString, CharIndex, Colours.w ) )		return SyncFalse;
		
		//	check range
		//	gr: use TLDebug_CheckInRange() ?
		if ( Colours.x > 255 || Colours.x < 0 ||
			Colours.y > 255 || Colours.y < 0 ||
			Colours.z > 255 || Colours.z < 0 ||
			Colours.w > 255 || Colours.w < 0 )
		{
			if ( !TLDebug_Break( TString("Colour32 type has components out of range (0..255); %d,%d,%d,%d", Colours.x, Colours.y, Colours.z, Colours.w ) ) )
				return SyncFalse;
		}

		TColour32 Colour( Colours.x, Colours.y, Colours.z, Colours.w );
		BinaryData.Write( Colour );
		return SyncTrue;
	}
	
	case TLBinary_TypeRef(TColour64):
	{
		Type4<s32> Colours;
		if ( !TLString::ReadNextInteger( DataString, CharIndex, Colours.x ) )		return SyncFalse;
		if ( !TLString::ReadNextInteger( DataString, CharIndex, Colours.y ) )		return SyncFalse;
		if ( !TLString::ReadNextInteger( DataString, CharIndex, Colours.z ) )		return SyncFalse;
		if ( !TLString::ReadNextInteger( DataString, CharIndex, Colours.w ) )		return SyncFalse;
		
		//	check range
		//	gr: use TLDebug_CheckInRange() ?
		if ( Colours.x > 65535 || Colours.x < 0 ||
			Colours.y > 65535 || Colours.y < 0 ||
			Colours.z > 65535 || Colours.z < 0 ||
			Colours.w > 65535 || Colours.w < 0 )
		{
			if ( !TLDebug_Break( TString("Colour64 type has components out of range (0..65535); %d,%d,%d,%d", Colours.x, Colours.y, Colours.z, Colours.w ) ) )
				return SyncFalse;
		}

		TColour64 Colour( Colours.x, Colours.y, Colours.z, Colours.w );
		BinaryData.Write( Colour );
		return SyncTrue;
	}

	case TLBinary_TypeRef(u8):
	{
		s32 Integer;
		if ( !TLString::ReadIntegerInRange( DataString, Integer, 0, 255 ) )
			return SyncFalse;
		BinaryData.Write( (u8)Integer );
		return SyncTrue;
	}

	case TLBinary_TypeRef(s8):
	{
		s32 Integer;
		if ( !TLString::ReadIntegerInRange( DataString, Integer, -127, 127 ) )
			return SyncFalse;
		BinaryData.Write( (s8)Integer );
		return SyncTrue;
	}

	case TLBinary_TypeRef(u16):
	{
		s32 Integer;
		if ( !TLString::ReadIntegerInRange( DataString, Integer, 0, 65535 ) )
			return SyncFalse;
		BinaryData.Write( (u16)Integer );
		return SyncTrue;
	}

	case TLBinary_TypeRef(s16):
	{
		s32 Integer;
		if ( !TLString::ReadIntegerInRange( DataString, Integer, -32767, 32767 ) )
			return SyncFalse;
		BinaryData.Write( (s16)Integer );
		return SyncTrue;
	}

	case TLBinary_TypeRef(u32):
	{
		s32 Integer;
		if ( !TLString::ReadIntegerInRange( DataString, Integer, 0, 4294967295 ) )
			return SyncFalse;
		BinaryData.Write( (u32)Integer );
		return SyncTrue;
	}

	case TLBinary_TypeRef(s32):
	{
		s32 Integer;
		if ( !TLString::ReadIntegerInRange( DataString, Integer, -2147483647, 2147483647 ) )
			return SyncFalse;
		BinaryData.Write( (s32)Integer );
		return SyncTrue;
	}

	case TLBinary_TypeRef(Bool):
	{
		//	read first char, we can work out true/false/0/1 from that
		if ( DataString.GetLength() == 0 )
			return SyncFalse;
		const char& BoolChar = DataString.GetCharAt(0);
		if ( BoolChar == 't' || BoolChar == 'T' || BoolChar == '1' )
		{
			BinaryData.Write( (Bool)TRUE );
			return SyncTrue;
		}
		else if ( BoolChar == 'f' || BoolChar == 'F' || BoolChar == '0' )
		{
			BinaryData.Write( (Bool)FALSE );
			return SyncTrue;
		}
		else
		{
			TLDebug_Break("Bool data is not True,False,0 or 1");
			return SyncFalse;
		}
	}

	default:
		break;
	};
	
#ifdef _DEBUG
	TTempString Debug_String("Unsupported/todo data type ");
	DataType.GetString( Debug_String );
	Debug_String.Append(". Data [");
	Debug_String.Append( DataString );
	Debug_String.Append("]");
	TLDebug_Break( Debug_String );
#endif

	return SyncFalse;
}



//--------------------------------------------------------
//	parse XML tag to Binary data[tree]
//--------------------------------------------------------
Bool TLFile::ParseXMLDataTree(TPtr<TXmlTag>& pTag,TBinaryTree& Data)
{
	/*
		XML examples
		
		//	root data
		<Data><u32>100</u32></Data>	

		// put in "translate" child
		<Data DataRef="translate"><float3>0,40,0</float3></Data> 

		//	"Node" data inside "NodeList" data
		<Data DataRef="NodeList">
			<Bool>TRUE</Bool>	//	written to "NodeList"
			<Data DataRef="Node"><TRef>ohai</TRef></Data> 
		</Data>
	*/

	//	read the data ref
	const TString* pDataRefString = pTag->GetProperty("DataRef");
	TRef DataRef( pDataRefString ? *pDataRefString : "" );

	//	establish the data we're writing data to
	TPtr<TBinaryTree> pDataChild;

	//	add a child to the node data if it has a ref, otherwise data is added to root of the node
	if ( DataRef.IsValid() )
	{
		pDataChild = Data.AddChild( DataRef );
		//	failed to add child data...
		if ( !pDataChild )
		{
			TLDebug_Break("failed to add child data");
			return FALSE;
		}
	}

	//	import contents of data
	TBinaryTree& NodeData = pDataChild ? *pDataChild.GetObjectPointer() : Data;

	//	if the tag has no children (eg. type like <float />) but DOES have data (eg. 1.0) throw up an error and fail
	//	assume the data is malformed and someone has forgotten to add the type specifier. 
	//	if something automated has output it and doesnt know the type it should still output it as hex raw data
	if ( !pTag->GetChildren().GetSize() && pTag->GetDataString().GetLengthWithoutTerminator() > 0 )
	{
		TTempString Debug_String("<Data ");
		DataRef.GetString( Debug_String );
		Debug_String.Append("> tag with no children, but DOES have data inside (eg. 1.0). Missing type specifier? (eg. <flt3>)\n");
		Debug_String.Append( pTag->GetDataString() );
		TLDebug_Break( Debug_String );
		return SyncFalse;
	}

	//	deal with child tags
	for ( u32 c=0;	c<pTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pTag->GetChildren().ElementAt(c);

		SyncBool TagImportResult = SyncFalse;

		if ( pChildTag->GetTagName() == "data" )
		{
			//	import child data
			if ( ParseXMLDataTree( pChildTag, NodeData ) )
				TagImportResult = SyncTrue;
			else
				TagImportResult = SyncFalse;

			if ( TagImportResult != SyncTrue )
				Data.Debug_PrintTree();
		}
		else
		{
			TRef DataTypeRef = TLFile::GetDataTypeFromString( pChildTag->GetTagName() );

			//	update type of data
			//	gr: DONT do this, if the type is mixed, this overwrites it. Setting the DataTypeHint should be automaticly done when we Write() in ImportBinaryData
			//NodeData.SetDataTypeHint( DataTypeRef );

			TagImportResult = TLFile::ImportBinaryData( pChildTag, NodeData, DataTypeRef );

			//	gr: just to check the data hint is being set from the above function...
			if ( TagImportResult == SyncTrue && !NodeData.GetDataTypeHint().IsValid() && NodeData.GetSize() > 0 )
			{
				TTempString Debug_String("Data imported is missing data type hint after successfull write? We just wrote data type ");
				DataTypeRef.GetString( Debug_String );
				Debug_String.Append(". This can be ignored if the data is mixed types");
				TLDebug_Break( Debug_String );
				Data.Debug_PrintTree();
			}
		}

		//	failed
		if ( TagImportResult == SyncFalse )
		{			
			TTempString str;
			str.Appendf("failed to import <data> tag \"%s\" in scheme", pChildTag->GetTagName().GetData() );
			
			TLDebug_Break( str );
			return FALSE;
		}

		//	async is unsupported
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async Scheme import");
			return FALSE;
		}
	}
	
	return TRUE;
}



//--------------------------------------------------------
//	check if string marked as a datum
//--------------------------------------------------------
Bool TLString::IsDatumString(const TString& String,TRef& DatumRef,TRef& ShapeType,Bool& IsJustDatum)
{
	//	split the string - max at 4 splits, if it splits 4 times, there's too many parts
	TFixedArray<TStringLowercase<TTempString>, 4> StringParts;
	if ( !String.Split( '_', StringParts ) )
	{
		//	didn't split at all, can't be valid
		return FALSE;
	}

	//	check first part is named "datum"
	if ( StringParts[0] == "datum" )
	{
		//	just a datum
		IsJustDatum = TRUE;
	}
	else if ( StringParts[0] == "anddatum" )
	{
		//	not just a datum
		IsJustDatum = FALSE;
	}
	else
	{
		//	no kinda datum
		return FALSE;
	}

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


//--------------------------------------------------------
//	cleanup string. Convert "\n" to a linefeed, convert tabs, do other generic string-replace's etc, returns if any changes are made
//--------------------------------------------------------
Bool TLString::CleanString(TString& String)
{
	Bool Changes = FALSE;

	TArray<char>& StringCharArray = String.GetStringArray();

	//	run through the string until we find something we might want to change
	for ( u32 i=0;	i<StringCharArray.GetSize();	i++ )
	{
		char Prevc = (i==0) ? 0 : StringCharArray.ElementAt(i-1);
		char& c = StringCharArray.ElementAt(i);
		Bool IsLast = (i==StringCharArray.GetLastIndex());

		//	it's a slash, check the next control char - but ignore if previous char was also a slash
		if ( c == '\\' && !IsLast && Prevc != '\\' )
		{
			char& Nextc = StringCharArray.ElementAt(i+1);

			//	line feed, replace the 2 characters with one line feed
			if ( Nextc == 'n' ||  Nextc == 'N' )
			{
				c = '\n';
				StringCharArray.RemoveAt(i+1);
				Changes = TRUE;
				continue;
			}
		}
	}

	return Changes;
}
