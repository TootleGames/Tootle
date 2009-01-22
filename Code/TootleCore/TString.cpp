#include "TString.h"
#include <stdio.h>
#include <stdarg.h>


#if defined(_VA_LIST_DEFINED) || defined(_VA_LIST)
	#define ENABLE_VA_LIST
#else
	#error no val list?
#endif


//-------------------------------------------------------------
//	test if a character is a whitespace
//-------------------------------------------------------------
Bool TLString::IsCharWhitespace(const char& Char)
{
	//	gr: I *think* ! is the first char...
	if ( Char < '!' )
		return TRUE;

	switch ( Char )
	{
	case ' ':			//	space
		return TRUE;

	default:
		return FALSE;
	};
}


//-------------------------------------------------------------
//	test if a character is a whitespace
//-------------------------------------------------------------
Bool TLString::IsCharLetter(const char& Char)
{
	if ( ( Char >= 'A' && Char <= 'Z' ) || ( Char >= 'a' && Char <= 'z' ) )
	{
		return TRUE;
	}

	return FALSE;
}

//-------------------------------------------------------------
//	test if a character is a lowercase letter
//-------------------------------------------------------------
Bool TLString::IsCharLowercase(const char& Char)
{
	if ( Char >= 'a' && Char <= 'z' )
	{
		return TRUE;
	}

	return FALSE;
}

//-------------------------------------------------------------
//	test if a character is an uppercase letter
//-------------------------------------------------------------
Bool TLString::IsCharUppercase( const char& Char)
{
	if ( Char >= 'A' && Char <= 'Z' )
	{
		return TRUE;
	}

	return FALSE;
}

//-------------------------------------------------------------
//	test if a character is a whitespace
//-------------------------------------------------------------
Bool TLString::SetCharLowercase(char& Char)
{
	if ( Char >= 'A' && Char <= 'Z' )
	{
		Char -= 'A';
		Char += 'a';
		return TRUE;
	}

	return FALSE;
}

//-------------------------------------------------------------
//	return lowercase version of char
//-------------------------------------------------------------
char TLString::GetCharLowercase(const char& Char)
{
	if ( Char >= 'A' && Char <= 'Z' )
	{
		char NewChar = Char;
		NewChar -= 'A';
		NewChar += 'a';
		return NewChar;
	}

	return Char;
}

//-------------------------------------------------------------
//	test if a character is a whitespace
//-------------------------------------------------------------
Bool TLString::SetCharUppercase(char& Char)
{
	if ( Char >= 'a' && Char <= 'z' )
	{
		Char -= 'a';
		Char += 'A';
		return TRUE;
	}

	return FALSE;
}


//-------------------------------------------------------------
//	turn this character into an integer, -1 means its not a number
//-------------------------------------------------------------
s32 TLString::GetCharInteger(const char& Char)
{
	if ( Char >= '0' && Char <= '9' )
	{
		return Char - '0';
	}

	return -1;
}


//-------------------------------------------------------------
//	turn this hexidecimal character into an integer, -1 means its not a number
//-------------------------------------------------------------
s32 TLString::GetCharHexInteger(const char& Char)
{
	if ( Char >= '0' && Char <= '9' )
		return Char - '0';

	if ( Char >= 'a' && Char <= 'f' )
		return Char - 'a' + 10;

	return -1;
}


u32 TLString::Platform::Strlen(const char* pString)
{
#ifdef _MSC_EXTENSIONS
	return strlen( pString );
#else
	u32 Len = 0;
	while ( pString[Len] != 0x0 )
	{
		Len++;
	}
	return Len;
#endif
}


//-------------------------------------------------------------
//	formatted constructor
//-------------------------------------------------------------
TString::TString(const CHARTYPE* pString,...) : 
	m_DataArray ( NULL, TLString_StringGrowBy )
{
	if ( !pString )
		return;

#ifdef ENABLE_VA_LIST
	va_list v;
	va_start( v, pString );

	u32 BufferSize = TLString::Platform::Strlen( pString ) + 100;
	if ( BufferSize < 512 )
		BufferSize = 512;
	
	SetLength( BufferSize );
#if defined(TL_TARGET_IPOD)
	int NewLength = vsprintf( GetData(), pString, v );
#else
	int NewLength = vsprintf_s( GetData(), GetLength(), pString, v );
#endif

	//	set the length of the string - but dont force terminator
	SetLength( NewLength, FALSE );

	va_end( v );
#else
	Set( pString );
#endif

	//	string has been set
	OnStringChanged();
}



//------------------------------------------------------
//		currently just dynamic char's
//------------------------------------------------------
void TString::Append(const CHARTYPE* pString,s32 Length)
{
	//	nothing to append...
	if ( Length == 0 || !pString )
		return;

	//	string starts with a terminator
	if ( pString[0] == TLString_Terminator )
		return;

	//	remove existing terminator to write following straight after
	RemoveTerminator();

	//	append string
	u32 Start = GetStringArray().GetSize();

	//	grow if we know the length
	if ( Length != -1 )
	{
		u32 NewLength = Start + Length;
		if ( !GetStringArray().SetSize( NewLength ) )
		{
			Length = GetLength() - Start;
			TTempString DebugString;
			DebugString.Appendf("Warning: String chopped - wanted to add %d - only appending %d chars (%d total)", NewLength-Start, Length, GetLength() );
			TLDebug_Print( DebugString );
		}

		TArray<char>& StringArray = GetStringArray();
		for ( u32 i=0;	i<(u32)Length;	i++ )
		{
			//	buffer allocated so write straight into buffer
			StringArray[Start+i] = pString[i];
		}
	}
	else
	{
		TArray<char>& StringArray = GetStringArray();
		const char* pChar = &pString[0];
		while ( (*pChar) != 0x0 )
		{
			//	buffer not allocated so do normal add routine
			s32 NewIndex = StringArray.Add( *pChar );

			//	run out of memory/array
			if ( NewIndex == -1 )
				break;

			pChar++;
		}
	}

	//	ensure there's a terminator on the end
	AddTerminator();

	OnStringChanged( Start );
}


//------------------------------------------------------
//	append a single character onto the string
//------------------------------------------------------
void TString::Append(const CHARTYPE& Char)
{
	s32 CharIndex = -1;
	TArray<char>& StringArray = GetStringArray();

	//	string is empty, just add character and terminator directly
	if ( GetLength() == 0 )
	{
		CharIndex = StringArray.Add( Char );
	}
	else
	{
		//	replace terminator
		s32 LastIndex = StringArray.GetLastIndex();
		if ( StringArray[LastIndex] == TLString_Terminator )
		{
			StringArray[LastIndex] = Char;
			CharIndex = LastIndex;
		}
		else
		{
			//	terminator is not last char, so just add this character
			CharIndex = StringArray.Add( Char );
		}
	}

	//	now add a terminator
	AddTerminator();

	OnStringChanged( CharIndex );
}



//------------------------------------------------------
//		currently just dynamic char's
//------------------------------------------------------
void TString::Appendf(const CHARTYPE* pString,...)
{
	if ( !pString )
		return;

#ifdef ENABLE_VA_LIST
	va_list v;
	va_start( v, pString );

	//	format up a new string and append that
	/*
	//	gr: stop recursive issues by not using a TString for the buffer
	char Buffer[512];
	int BufferStringLength = vsprintf_s( &Buffer[0], sizeof(Buffer), pString, v );
	Append( Buffer, BufferStringLength );
	*/
	TBufferString<512> Buffer;
	Buffer.SetLength(512);
#if defined(TL_TARGET_IPOD)
	int BufferStringLength = vsprintf( Buffer.GetData(), pString, v );
#else
	int BufferStringLength = vsprintf_s( Buffer.GetData(), Buffer.GetLength(), pString, v );
#endif
	Buffer.SetLength( BufferStringLength );
	Append( Buffer );

	va_end( v );	
#else
	Append( pString );
#endif
}



//------------------------------------------------------
//	make sure there's a terminator on the end
//	OverwriteTerminator tends to be used as a safety terminator
//	normally when we just add strings we dont need to force the 
//	last character to be a terminator
//------------------------------------------------------
void TString::AddTerminator(Bool ForceTerminator)
{
	if ( GetLength() <= 0 )
		return;

	TArray<char>& StringArray = GetStringArray();

	//	last element is NOT a terminator... append one
	if ( StringArray.ElementLast() != TLString_Terminator )
	{
		if ( !ForceTerminator )
		{
			//	if we can't add a terminator, resort to forcing a terminator
			if ( StringArray.Add( (CHARTYPE)TLString_Terminator ) == -1 )
				ForceTerminator = TRUE;
		}

		//	if overwrite terminator, then just set the last character as terminator
		if ( ForceTerminator )
		{
			StringArray.ElementLast() = TLString_Terminator;
		}
	}
}


//------------------------------------------------------
//	remove the terminator off the end
//------------------------------------------------------
void TString::RemoveTerminator()
{
	if ( GetLength() <= 0 )
		return;

	TArray<char>& StringArray = GetStringArray();

	//	last element is a terminator so cut it off
	while ( StringArray.ElementLast() == TLString_Terminator )
	{
		if ( !StringArray.SetSize( StringArray.GetSize() - 1 ) )
			break;

		if ( StringArray.GetSize() <= 0 )
			break;
	}
}


//------------------------------------------------------
//	get the length from some other type of string
//------------------------------------------------------
u32 TString::GetLength(const CHARTYPE* pString)
{
	if ( !pString )
		return 0;

	//	loop till we find a terminator
	u32 Length = 0;
	while ( pString[Length] != TLString_Terminator )
		Length++;

	return Length;
}


//------------------------------------------------------
//	comparison to string
//------------------------------------------------------
Bool TString::IsEqual(const TString& String,Bool CaseSensitive) const
{
	//	some string types can always be/not be case sensitive
	ForceCaseSensitivity( CaseSensitive );

	//	different length
	if ( GetLength() != String.GetLength() )
		return FALSE;

	//	loop through chars and compare
	for ( u32 i=0;	i<GetLength();	i++ )
	{
		char thischar = (!CaseSensitive) ? GetLowercaseCharAt(i) : GetCharAt(i);
		char stringchar = (!CaseSensitive) ? String.GetLowercaseCharAt(i) : String.GetCharAt(i);

		//	different char found! abort!
		if ( thischar != stringchar )
			return FALSE;
	}

	//	all matched
	return TRUE;
}


//------------------------------------------------------
//	comparison to string
//------------------------------------------------------
Bool TString::IsEqual(const CHARTYPE* pString,s32 Length,Bool CaseSensitive) const
{
	//	some string types can always be/not be case sensitive
	ForceCaseSensitivity( CaseSensitive );

	s32 ThisLength = this->GetLength();	//	includes terminator

	if ( Length <= 0 )
	{
		//	length will include terminator when using strlen
		Length = (s32)TLString::Platform::Strlen( pString );
		ThisLength--;	//	don't include terminator
	}
	else
	{
		//	if length is specified, see if it includes the length
		//	if the last character is NOT a terminator then ignore our terminator
		if ( pString[Length-1] != 0x0 )
			ThisLength--;
	}

	//	different length
	if ( ThisLength != Length || ThisLength < 0 )
		return FALSE;

	//	loop through chars and compare
	for ( u32 i=0;	i<(u32)ThisLength;	i++ )
	{
		//	different char found! abort!
		char thischar = (!CaseSensitive) ? GetLowercaseCharAt(i) : GetCharAt(i);
		char stringchar = (!CaseSensitive) ? TLString::GetCharLowercase( pString[i] ) : pString[i];

		if ( thischar != stringchar )
			return FALSE;
	}

	//	all matched
	return TRUE;
}


//------------------------------------------------------
//	append part of a string (use -1 as the length to copy everything FROM from)
//------------------------------------------------------
void TString::Append(const TString& String,u32 From,s32 Length)
{
	if ( !TLDebug_CheckIndex( From, String.GetLength() ) )
		return;

	if ( Length == -1 )
	{
		Length = String.GetLength() - From;
	}
	else
	{
		if ( !TLDebug_CheckInRange( From+Length, 0, String.GetLength() ) )
			Length = String.GetLength() - From;
	}

	//	append part
	Append( &String.GetCharAt(From), Length );
}




//------------------------------------------------------
//	less than comparison for array sorting
//------------------------------------------------------
Bool TString::IsLessThan(const TString& String) const
{
	u32 TestLength = GetLength() < String.GetLength() ? GetLength() : String.GetLength();

	for ( u32 i=0;	i<TestLength;	i++ )
	{
		if ( GetCharAt(i) < String.GetCharAt(i) )
			return TRUE;

		if ( GetCharAt(i) > String.GetCharAt(i) )
			return FALSE;
	}

	//	got here, run out of characters to compare, all matched so far
	//	so a shorter string is "less than" others
	if ( GetLength() < String.GetLength() )
		return TRUE;

	return FALSE;
}


//------------------------------------------------------
//	get the index of last char. -1 if empty or all terminators
//------------------------------------------------------
s32 TString::GetCharGetLastIndex() const
{
	//	empty string, return a terminator
	if ( !GetLength() )
		return -1;

	s32 Index = (s32)GetLength() - 1;
	while ( Index >= 0 )
	{
		const CHARTYPE& Char = GetCharAt(Index);

		//	if not a terminator, return it
		if ( Char != TLString_Terminator )
			return Index;

		//	back until we find a non-terminator
		Index--;
	}

	//	no non-terminators found, return a terminator
	return -1;
}


//------------------------------------------------------
//	get the last char. if string is empty a terminator is returned. 
//	If the string ends with a terminator, it returns the last char before terminator (if any)
//------------------------------------------------------
TString::CHARTYPE TString::GetCharLast() const
{
	s32 LastCharIndex = GetCharGetLastIndex();

	//	no chars (or all terminators)
	if ( LastCharIndex == -1 )
		return TLString_Terminator;

	//	return char
	return GetCharAt(LastCharIndex);
}


//------------------------------------------------------
//	allocate buffer for string data so that it doesnt need to alloc more later
//------------------------------------------------------
void TString::SetAllocSize(u32 AllocLength)					
{	
	//	size requested is less than current so cut string
	if ( AllocLength < GetLength() )	
	{	
		SetLength(AllocLength);	
		return;
	}	

	//	allocate extra data
	GetStringArray().SetAllocSize( AllocLength );
}


//------------------------------------------------------
//	trim whitespace from front and back of string - returns TRUE if changed
//------------------------------------------------------
Bool TString::Trim()
{
	u32 OldLength = GetLength();

	//	nothing to trim
	if ( !OldLength )
		return FALSE;

	s32 NewLastIndex = GetStringArray().GetLastIndex();
	while ( NewLastIndex >= 0 )
	{
		if ( !TLString::IsCharWhitespace( GetCharAt(NewLastIndex) ) )
			break;

		NewLastIndex--;
	}
	SetLength( NewLastIndex + 1 );
	u32 NewLength = GetLength();

	if ( NewLastIndex <= 0 )
		return (OldLength != NewLength);

	//	if we didnt go all the way to the start, trim the left too
	u32 NewFirstIndex = 0;
	while ( NewFirstIndex < NewLength )
	{
		if ( !TLString::IsCharWhitespace( GetCharAt(NewFirstIndex) ) )
			break;

		NewFirstIndex++;
	}

	//	nothing trimmed from front
	if ( NewFirstIndex == 0 )
		return (OldLength != NewLength);

	//	trim
	RemoveCharAt( 0, NewFirstIndex );

	return TRUE;
}


//------------------------------------------------------
//	change all uppercase characters to lowercase - returns TRUE if changed
//------------------------------------------------------
Bool TString::SetLowercase()
{
	Bool Changed = FALSE;
	for ( u32 i=0;	i<GetLength();	i++ )
	{
		Changed |= TLString::SetCharLowercase( GetCharAt(i) );
	}
	return Changed;
}


//------------------------------------------------------
//	change all lowercase characters to uppercase - returns TRUE if changed
//------------------------------------------------------
Bool TString::SetUppercase()
{
	Bool Changed = FALSE;
	for ( u32 i=0;	i<GetLength();	i++ )
	{
		Changed |= TLString::SetCharUppercase( GetCharAt(i) );
	}
	return Changed;
}


//------------------------------------------------------
//	turn string into an integer - fails if some non-integer stuff in it
//------------------------------------------------------
Bool TString::GetInteger(s32& Integer) const
{
	//	just a routine to see if we could correct bad cases (hence debug only)
	if ( TLDebug::IsEnabled() )
	{
		TTempString Trimmed( *this );
		if ( Trimmed.Trim() )
		{
			if ( TLDebug_Break("This string needs trimming before trying to extract integer. Best to trim the string we're passing in") )
				return FALSE;
		}
	}

	//	work backwards
	s32 CharIndex = GetCharGetLastIndex();
	if ( CharIndex == -1 )
		return FALSE;

	//	reset int
	Integer = 0;
	u32 Mult = 1;
	while ( CharIndex >= 0 )
	{
		const char& Char = GetCharAt( CharIndex );
		s32 CharInt = TLString::GetCharInteger( Char );

		//	char isnt an int
		if ( CharInt < 0 )
		{
			//	check if it's a negative
			if ( CharIndex == 0 && Char == '-' )
			{
				Integer = -Integer;
				break;
			}
			else 
			{
				return FALSE;
			}
		}

		//	mult our integer as required (x*10, x*100)
		Integer += CharInt * Mult;

		CharIndex--;
		Mult *= 10;
	}

	return TRUE;
}


//------------------------------------------------------
//	turn string into a float
//------------------------------------------------------
Bool TString::GetFloat(float& Float) const
{
	//	look for a dot, if there isn't one, then treat as an int
	s32 DotCharIndex = GetCharIndex('.');
	if ( DotCharIndex == -1 )
	{
		s32 Integer;
		if ( !GetInteger(Integer) )
			return FALSE;

		//	turn to float
		Float = (float)Integer;
		return TRUE;
	}

	//	split at dot
	TTempString FloatLargeString, FloatDecimalString;
	FloatLargeString.Append( *this, DotCharIndex );
	FloatDecimalString.Append( *this, DotCharIndex+1, -1 );

	//	turn these parts into integers
	s32 IntegerLarge;
	if ( !FloatLargeString.GetInteger( IntegerLarge ) )
		return FALSE;

	//	turn into float
	Float = (float)IntegerLarge;

	//	gr: as it's floating point, what we need to do is base the max decimal places based on the size of the 
	//	integer. todo: lookup schematics for a float. I think it's 8 in either direction?...
	//	parse through the decimal string
	u32 MaxDecPlaces = FloatDecimalString.GetLengthWithoutTerminator();
	if ( MaxDecPlaces > 4 )
		MaxDecPlaces = 4;

	float DecimalDiv = 1.f * 0.1f;
	for ( u32 i=0;	i<MaxDecPlaces;	i++ )
	{
		s32 CharInteger = TLString::GetCharInteger( FloatDecimalString[i] );
		
		//	not a number and we only want numbers
		if ( CharInteger == -1 )
		{
			TLDebug_Break("Non-integer character in float-string decimal part");
			return FALSE;
		}

		//	turn into decimal part and append to float
		float Decimal = ((float)CharInteger) * DecimalDiv;
		Float += Decimal;

		//	make decimal div smaller
		DecimalDiv *= 0.1f;
	}

	return TRUE;
}


//------------------------------------------------------
//	turn hexidecimal string into an integer
//------------------------------------------------------
Bool TString::GetHexInteger(u32& Integer) const
{
	//	just a routine to see if we could correct bad cases (hence debug only)
	if ( TLDebug::IsEnabled() )
	{
		TTempString Trimmed( *this );
		if ( Trimmed.Trim() )
		{
			if ( TLDebug_Break("This string needs trimming before trying to extract integer. Best to trim the string we're passing in") )
				return FALSE;
		}
	}

	//	work backwards
	s32 CharIndex = GetCharGetLastIndex();
	if ( CharIndex == -1 )
		return FALSE;

	//	too big to be a 32bit hex number
	if ( CharIndex >= 8 )
	{
		if ( !TLDebug_Break("String, if converted to hex could be bigger than 32 bits. Retry to continue, cancel to fail") )
			return FALSE;
		CharIndex = 7;
	}

	//	reset int
	Integer = 0;
	u32 Shift = 0;
	while ( CharIndex >= 0 )
	{
		const char& Char = GetCharAt( CharIndex );
		s32 CharHexInt = TLString::GetCharHexInteger( Char );

		//	char isnt an int
		if ( CharHexInt < 0 )
		{
			return FALSE;
		}

		//	mult our integer as required (x*10, x*100)
		Integer |= CharHexInt << Shift;

		CharIndex--;

		//	shift increments per character, a character (0..f) is 0..16, so each character is 4 bits
		Shift += 4;
	}

	return TRUE;
}



//------------------------------------------------------
//	turn hexidecimal string into an array of bytes
//	so string is expected to be like 0011223344aabbff etc
//------------------------------------------------------
Bool TString::GetHexBytes(TArray<u8>& ParsedBytes) const
{
	//	just a routine to see if we could correct bad cases (hence debug only)
	if ( TLDebug::IsEnabled() )
	{
		TTempString Trimmed( *this );
		if ( Trimmed.Trim() )
		{
			if ( TLDebug_Break("This string needs trimming before trying to extract integer. Best to trim the string we're passing in") )
				return FALSE;
		}
	}

	//	work backwards
	s32 CharIndex = GetCharGetLastIndex();
	if ( CharIndex == -1 )
		return FALSE;

	//	should be in pairs, so no odd lengths
	if ( CharIndex % 2 == 1 )
	{
		if ( !TLDebug_Break("String is odd(not even) length, if converted to an array of hex bytes would be offset by a half byte. Retry to continue(trims last char), cancel to fail") )
			return FALSE;
		CharIndex--;
	}

	//	reset int
	u8 Byte = 0;
	Bool FirstHalfByte = TRUE;
	while ( CharIndex >= 0 )
	{
		const char& Char = GetCharAt( CharIndex );
		s32 CharHexInt = TLString::GetCharHexInteger( Char );

		//	char isnt an int
		if ( CharHexInt < 0 )
		{
			return FALSE;
		}

		//	mult our integer as required (x*10, x*100)
		u8 Shift = FirstHalfByte ? 0 : 4;
		Byte |= CharHexInt << Shift;

		//	next char
		CharIndex--;

		//	first half? next get 2nd half
		if ( FirstHalfByte )
		{
			FirstHalfByte = FALSE;
		}
		else
		{
			//	just done 2nd half, save byte and start on next one
			ParsedBytes.Add( Byte );
			FirstHalfByte = TRUE;
			Byte = 0;
		}
	}

	return TRUE;
}


//------------------------------------------------------
//	find the next non-whitespace char
//------------------------------------------------------
s32 TString::GetCharIndexNonWhitespace(u32 From) const
{
	for ( u32 i=From;	i<GetLengthWithoutTerminator();	i++ )
	{
		if ( !TLString::IsCharWhitespace( GetCharAt(i) ) )
			return i;
	}

	return -1;
}

//------------------------------------------------------
//	find the next whitespace char
//------------------------------------------------------
s32 TString::GetCharIndexWhitespace(u32 From) const
{
	for ( u32 i=From;	i<GetLengthWithoutTerminator();	i++ )
	{
		if ( TLString::IsCharWhitespace( GetCharAt(i) ) )
			return i;
	}

	return -1;
}

