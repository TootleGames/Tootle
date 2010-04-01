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
Bool TLString::IsCharWhitespace(const TChar& Char)
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
Bool TLString::IsCharLetter(const TChar& Char)
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
Bool TLString::IsCharLowercase(const TChar& Char)
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
Bool TLString::IsCharUppercase( const TChar& Char)
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
Bool TLString::SetCharLowercase(TChar& Char)
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
TChar TLString::GetCharLowercase(const TChar& Char)
{
	if ( Char >= TLCharString('A') && Char <= TLCharString('Z') )
	{
		TChar NewChar = Char;			//	A..Z
		NewChar -= TLCharString('A');	//	0..26
		NewChar += TLCharString('a');	//	a..z
		return NewChar;
	}

	return Char;
}

//-------------------------------------------------------------
//	test if a character is a whitespace
//-------------------------------------------------------------
Bool TLString::SetCharUppercase(TChar& Char)
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
s32 TLString::GetCharInteger(const TChar& Char)
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
s32 TLString::GetCharHexInteger(const TChar& Char)
{
	if ( Char >= '0' && Char <= '9' )
		return Char - '0';

	if ( Char >= 'a' && Char <= 'f' )
		return Char - 'a' + 10;

	if ( Char >= 'A' && Char <= 'F' )
		return Char - 'A' + 10;

	return -1;
}


//-------------------------------------------------------------
//	formatted constructor
//-------------------------------------------------------------
TString::TString(const TChar8* pString,...) : 
	m_DataArray ( NULL, TLString_StringGrowBy )
{
	if ( !pString )
		return;

#ifdef ENABLE_VA_LIST
	va_list v;
	va_start( v, pString );
	AppendVaList( pString, v );
	va_end( v );
#else
	Set( pString );

	//	string has been set
	OnStringChanged();
#endif
}


//-------------------------------------------------------------
//	formatted constructor
//-------------------------------------------------------------
TString::TString(const TChar16* pString,...) : 
	m_DataArray ( NULL, TLString_StringGrowBy )
{
	if ( !pString )
		return;

#ifdef ENABLE_VA_LIST
	va_list v;
	va_start( v, pString );
	AppendVaList( pString, v );
	va_end( v );
#else
	//	just set the format string
	Set( pString );

	//	string has been set
	OnStringChanged();
#endif
}


//-------------------------------------------------------------
//	
//-------------------------------------------------------------
TString& TString::Appendf(const TChar8* pString,...)
{
	if ( !pString )
		return *this;

#ifdef ENABLE_VA_LIST
	va_list v;
	va_start( v, pString );
	AppendVaList( pString, v );
	va_end( v );
#else
	Append( pString );
#endif

	return *this;
}

//------------------------------------------------------
//		currently just dynamic char's
//------------------------------------------------------
TString& TString::Appendf(const TChar16* pString,...)
{
	if ( !pString )
		return (*this);

#ifdef ENABLE_VA_LIST
	va_list v;
	va_start( v, pString );
	AppendVaList( pString, v );
	va_end( v );	
#else
	Append( pString );
#endif

	return (*this);
}


//------------------------------------------------------
//	
//------------------------------------------------------
void TString::AppendVaList(const TChar16* pString,va_list& v)
{
	//	format up a new string and append that
	TBufferString<512> Buffer;
	Buffer.SetLength(512);

#if defined(TL_TARGET_PC)
	int BufferStringLength = _vsnwprintf_s( Buffer.GetData(), Buffer.GetLength(), _TRUNCATE, pString, v );
#else
	// TODO: Not sure what the equivalent _vsnwprintf_s routine is on the Mac/Ipod :( 
	int BufferStringLength = vsnprintf( (char*)Buffer.GetData(), Buffer.GetLength(), (const char*)pString, v );
	//int BufferStringLength = vsprintf_s( Buffer.GetData(), Buffer.GetLength(), pString, v );
#endif

	Buffer.SetLength( BufferStringLength );
	Append( Buffer );
}

//------------------------------------------------------
//	
//------------------------------------------------------
void TString::AppendVaList(const TChar8* pString,va_list& v)
{
	//	format up a new string and append that
	TArray<TChar8> Buffer;
	Buffer.SetSize( TLString::Strlen( pString ) + 256 );

#if defined(TL_TARGET_PC)
	int BufferStringLength = vsprintf_s( Buffer.GetData(), Buffer.GetDataSize(), pString, v );
#else
	int BufferStringLength = vsprintf( (char*)Buffer.GetData(), (const char*)pString, v );
#endif

	Append( Buffer.GetData(), BufferStringLength );
}


//------------------------------------------------------
//	remove terminator from the array, so the last element is a character
//------------------------------------------------------
void TString::RemoveTerminator()
{
	TArray<TChar>& StringArray = GetStringArray();
	
	//	trim string until we have no more terminators
	while ( StringArray.GetSize() > 0 )
	{
		if ( StringArray.ElementLast() != TLString_Terminator )
			break;

		//	error check so we don't get stuck in a loop
		if ( !StringArray.RemoveLast() )
			break;
	}
}

//------------------------------------------------------
//	make sure there's a terminator on the end
//------------------------------------------------------
void TString::SetTerminator()
{
	//	trim terminator[s] from the string
	RemoveTerminator();

	//	add a terminator
	//	if we can't add a terminator, resort to forcing a terminator
	TArray<TChar>& StringArray = GetStringArray();
	if ( StringArray.Add( TLString_Terminator ) == -1 )
	{
		StringArray.ElementLast() = TLString_Terminator;
	}

	//	check the string is terminated correctly
	TLDebug_Assert( GetLength() == GetLength( GetData() ), "String's length and null-terminated length don't match. Terminator improperly set");
}


//------------------------------------------------------
//	get the length from some other type of string
//------------------------------------------------------
u32 TString::GetLength(const TChar* pString)
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
		TChar thischar = (!CaseSensitive) ? GetLowercaseCharAt(i) : GetCharAt(i);
		TChar stringchar = (!CaseSensitive) ? String.GetLowercaseCharAt(i) : String.GetCharAt(i);

		//	different char found! abort!
		if ( thischar != stringchar )
			return FALSE;
	}

	//	all matched
	return TRUE;
}

/*
//------------------------------------------------------
//	comparison to string
//------------------------------------------------------
Bool TString::IsEqual(const TChar* pString,s32 Length,Bool CaseSensitive) const
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
		TChar thischar = (!CaseSensitive) ? GetLowercaseCharAt(i) : GetCharAt(i);
		TChar stringchar = (!CaseSensitive) ? TLString::GetCharLowercase( pString[i] ) : pString[i];

		if ( thischar != stringchar )
			return FALSE;
	}

	//	all matched
	return TRUE;
}
*/

//------------------------------------------------------
//	append part of a string (use -1 as the length to copy everything FROM from)
//------------------------------------------------------
TString& TString::Append(const TString& String,u32 From,s32 Length)
{
	if ( !TLDebug_CheckIndex( From, String.GetLength() ) )
		return *this;

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
	return Append( &String.GetCharAt(From), Length );
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
		const TChar& Char = GetCharAt(Index);

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
TChar TString::GetCharLast() const
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
Bool TString::Trim(Bool TrimLeft)
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

	//	if we went to far left, or not trimming left side, then abort now
	if ( NewLastIndex <= 0 || !TrimLeft )
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
//	turn string into an integer - fails if some non-integer stuff in it (best to trim first where possible). 
//	The extra pIsPositive param (if not null) stores the posititivy/negativity of the number. This is needed for -0.XYZ numbers. we still need the sign for floats, but as an int -0 is just 0.
//------------------------------------------------------
Bool TString::GetInteger(s32& Integer,Bool* pIsPositive) const
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

	//	default to positive
	if ( pIsPositive )
		*pIsPositive = TRUE;

	//	reset int
	Integer = 0;
	u32 Mult = 1;
	while ( CharIndex >= 0 )
	{
		const TChar& Char = GetCharAt( CharIndex );
		s32 CharInt = TLString::GetCharInteger( Char );

		//	char isnt an int
		if ( CharInt < 0 )
		{
			//	check if it's a negative
			if ( CharIndex == 0 && Char == '-' )
			{
				//	negate the result
				if ( pIsPositive )
					*pIsPositive = FALSE;

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
	Bool Positive = TRUE;
	if ( !FloatLargeString.GetInteger( IntegerLarge, &Positive ) )
		return FALSE;

	//	gr: as it's floating point, what we need to do is base the max decimal places based on the size of the 
	//	integer. todo: lookup schematics for a float. I think it's 8 in either direction?...
	//	parse through the decimal string
	u32 MaxDecPlaces = FloatDecimalString.GetLength();
	if ( MaxDecPlaces > 4 )
		MaxDecPlaces = 4;

	//	turn into positive float
	Float = (float)( Positive ? IntegerLarge : -IntegerLarge );

	float DecimalDiv = 1.f * 0.1f;
	for ( u32 i=0;	i<MaxDecPlaces;	i++ )
	{
		s32 CharInteger = TLString::GetCharInteger( FloatDecimalString[i] );
		
		//	not a number and we only want numbers
		if ( CharInteger == -1 )
		{
#ifdef _DEBUG
			TTempString Debug_String("Non-integer character in float-string decimal part: \"");
			Debug_String.Append( *this );			
			Debug_String.Append("\"");
			TLDebug_Break( Debug_String );
#endif
			return FALSE;
		}

		//	turn into decimal part and append to float
		float Decimal = ((float)CharInteger) * DecimalDiv;
		Float += Decimal;

		//	make decimal div smaller
		DecimalDiv *= 0.1f;
	}

	//	turn float back into a negative
	if ( !Positive )
		Float = -Float;

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
		const TChar& Char = GetCharAt( CharIndex );
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
		const TChar& Char = GetCharAt( CharIndex );
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
	for ( u32 i=From;	i<GetLength();	i++ )
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
	for ( u32 i=From;	i<GetLength();	i++ )
	{
		if ( TLString::IsCharWhitespace( GetCharAt(i) ) )
			return i;
	}

	return -1;
}


//------------------------------------------------------
//	insert a string into this string
//------------------------------------------------------
void TString::InsertAt(u32 Index,const TString& String)
{
	const TArray<TChar>& OtherStringArray = String.GetStringArray();
	TArray<TChar>& StringArray = this->GetStringArray();

	StringArray.InsertAt( Index, OtherStringArray );
}


//--------------------------------------------------------
//	get an array of floats from a string (expects just floats)
//--------------------------------------------------------
Bool TString::GetFloats(TArray<float>& Floats) const
{
	//	split the string
	TFixedArray<TChar,4> SplitChars;
	SplitChars << '\t' << '\n' << ' ' << ',' << ':';

	//	gr: I've made it so it ignore's f or F in floats so we can put in 1.f or 1.0f etc
	//		this may cause a problem if we're pulling out floats in commands (like SVG's but
	//		i don't think those cases will use this function anyway. If it is a problem, move
	//		the "acceptance" of a trailing f into GetFloat()
	SplitChars << 'f' << 'F';

	//	failed to split at all? must be empty string
	TArray<TTempString> FloatStrings;
	if ( !this->Split( SplitChars, FloatStrings ) )
		return false;

	//	now go through all the strings we split and pull out the floats
	for ( u32 s=0;	s<FloatStrings.GetSize();	s++ )
	{
		const TString& FloatString = FloatStrings[s];
		float f;

		//	one of these strings is not a float
		if ( !FloatString.GetFloat( f ) )
		{
			TTempString Debug_String;
			Debug_String << "Split string \"" << FloatString << "\" in GetFloats() is not a float";
			TLDebug_Break( Debug_String );
			return false;
		}

		//	add to float list
		s32 FloatIndex = Floats.Add( f );

		//	out of space for float... 
		//	gr: I've allowed this as success, just look out for this warning if we want an indetermined number of floats
		if ( FloatIndex == -1 )
		{
			TLDebug_Print("Warning: in GetFloats we ran out of space to add an additional float");
			return true;
		}
	}

	return true;	
}



//----------------------------------------------------------
//	read an array of integers from a string
//----------------------------------------------------------
Bool TString::GetIntegers(TArray<s32>& Integers) const
{
	//	split the string
	TFixedArray<TChar,4> SplitChars;
	SplitChars << '\t' << '\n' << ' ' << ',' << ':';

	TArray<TTempString> IntegerStrings;
	if ( !this->Split( SplitChars, IntegerStrings ) )
		return false;

	//	now go through all the strings we split and pull out the floats
	for ( u32 s=0;	s<IntegerStrings.GetSize();	s++ )
	{
		const TString& IntegerString = IntegerStrings[s];
		s32 i;

		//	one of these strings is not an integer
		if ( !IntegerString.GetInteger( i ) )
		{
			TTempString Debug_String;
			Debug_String << "Split string \"" << IntegerString << "\" in GetIntegers() is not an integer";
			TLDebug_Break( Debug_String );
			return false;
		}

		//	add to list
		Integers.Add( i );
	}

	return true;	
}
