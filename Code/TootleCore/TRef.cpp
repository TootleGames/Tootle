#include "TRef.h"
#include "TFixedArray.h"
#include "TString.h"
#include "TKeyArray.h"


#define ENABLE_DEBUG_REF_TRUNCATION		FALSE
//#define ENABLE_DEBUG_REF_TRUNCATION		TLDebug::IsEnabled()

namespace TLRef
{
	//	symbols allowed in urls
	//	-_.!*'() a-z A-Z 0-9

	//	not allowed for url enc as they are URI commands or often mistakenly converted/used for other things so unreliable
	//	$&+,/:;=?@<>#%{}|\^~[]` SPACE

	//	not allowed in win32 filenames
	//	\/:*?"<>|

	//	gr: new ref alphabet, some symbols have been repalced to aid URL-Encoding (so I don't need to encode a ref and can just send it as a string
	//	table for string->ref conversions (+1 for terminator) and an ALT table for capitalisation
	const char				g_RefAlphabet[TLRef_AlphabetSize+1]			= {" abcdefghijklmnopqrstuvwxyz0123456789'-!_"};	//	file system safe
	const char				g_RefAlphabetAlt[TLRef_AlphabetSize+1]		= {"*ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'-!_"};	//	web safe

	//	when url-encoding, we can't use a space, but we definatly want to use that in code as it's such a useful and common character. 
	const char&				g_RefCharUrlSpace		= g_RefAlphabetAlt[0];		//	'apostraphe
	
	Bool					g_RefCharLookupValid	= FALSE;
	u32						g_RefCharLookup[256];								//	lookup table for char->refchar
	u32						g_RefCharLookupSize = sizeofarray(g_RefCharLookup);	//	lookup table for char->refchar
	u32						g_InvalidRefMask		= 0x0;						//	uninitialised! I can probably work out an actual number but calculations are more accurate

	FORCEINLINE u32			GetRefCharIndex(TChar c);							//	get a refchar index from a string character
	FORCEINLINE TChar		GetCharFromRefCharIndex(u32 Index,Bool UseAltTable=FALSE);			//	get a string character from a refchar index
	FORCEINLINE u32			GetRefBitsFromChar(TChar Char,u32 Index,Bool CheckIndex);

	void					GenerateCharLookupTable();
}



//////////////////////////////////////////////////////



void TLRef::GenerateCharLookupTable()
{
	for ( u32 x=0;	x<g_RefCharLookupSize;	x++ )
		g_RefCharLookup[x] = 0;

	//	loop through the alphabets and add the keys
	for ( u32 i=0;	i<TLRef_AlphabetSize;	i++ )
	{
		//	slightly modified to ensure terminator has an index of zero
		const char& Char = g_RefAlphabet[i];
		u32 Index = (Char == 0x0) ? 0 : i;
		const char& AltChar = g_RefAlphabetAlt[i];
		u32 AltIndex = (AltChar == 0x0) ? 0 : i;

		g_RefCharLookup[ Char ] = Index;
		g_RefCharLookup[ AltChar ] = AltIndex;
	}

	//	work out a VALID bit mask
	u32 ValidMask = 0x0;
	for ( u32 i=0;	i<TLRef_CharsPerRef;	i++ )
		ValidMask |= TLRef_CharIndexBitMask << (i*TLRef_BitsPerChar);
	
	//	get the invalid mask to test against
	g_InvalidRefMask = ~ValidMask;

	g_RefCharLookupValid = TRUE;
}


//---------------------------------------------------
//	get a refchar index from a string character
//---------------------------------------------------
FORCEINLINE u32 TLRef::GetRefCharIndex(TChar c)
{
	//	use key lookup table if it's been generated as it's faster
	if ( g_RefCharLookupValid )
	{
		//	fast dumb version
		if ( c < g_RefCharLookupSize )
			return g_RefCharLookup[c];
	}
	else
	{
		//	gr: special case that won't cause an assert. Not neccessary when the lookup is valid.
		if ( c == 0x0 )
			c = TChar('_');

		//	slow version, but must be used before TLRef::Init()
		for ( u32 i=0;	i<TLRef_AlphabetSize;	i++ )
		{
			if ( g_RefAlphabet[i] == c )
				return i;

			if ( g_RefAlphabetAlt[i] == c )
				return i;
		}	
	}

	//	unsupported character
	TLDebug_Break( TString("unsupported character %c/%d/0x%20d provided for ref. Replacing with underscore", c,c,c ) );
	return GetRefCharIndex( TChar('_') );
}


//---------------------------------------------------
//	get a string character from a refchar index
//---------------------------------------------------
FORCEINLINE TChar TLRef::GetCharFromRefCharIndex(u32 Index,Bool UseAltTable)
{
	//	invalid index
	if ( Index >= TLRef_AlphabetSize )
		return 0;

	return UseAltTable ? g_RefAlphabetAlt[Index] : g_RefAlphabet[Index];
}


//---------------------------------------------------
//	get a string character from a refchar index
//---------------------------------------------------
FORCEINLINE u32 TLRef::GetRefBitsFromChar(TChar Char,u32 Index,Bool CheckIndex)
{
	if ( CheckIndex )
	{
		if ( !TLDebug_CheckIndex( Index, TLRef_CharsPerRef ) )
			return 0x0;
	}

	//	get the index
	u32 RefIndex = TLRef::GetRefCharIndex( Char );

	//	mask it off (just in case) so it doesnt overflow
	if ( CheckIndex && RefIndex > TLRef_CharIndexBitMask )
	{
		TLDebug_Break("RefCharIndex greater than allowed - lookup table is corrupt?");
		RefIndex &= TLRef_CharIndexBitMask;
	}

	//	add bits for this index to the Ref
	//	shift along to it's position in the 32 bits
	return RefIndex << (Index*TLRef_BitsPerChar);
}

//-----------------------------------------------------
//	pull out 5 characters and set from this string
//-----------------------------------------------------
void TRef::Set(const TString& RefString)
{
	Bool DebugTruncation = FALSE;

	//	pull out first 5 characters (or less) from our string
	u32 StringLength = RefString.GetLength();
	if ( StringLength > TLRef_CharsPerRef )
	{
		StringLength = TLRef_CharsPerRef;
		DebugTruncation = ENABLE_DEBUG_REF_TRUNCATION;
	}

	//	reset ref
	m_Ref = 0;
	Bool CheckIndex = TLDebug::IsEnabled();

	//	setup ref as we go through string
	//	NULL characters are switched to spaces, aside from the first one which is set to ?
	for ( u32 i=0;	i<StringLength;	i++ )
	{
		m_Ref |= TLRef::GetRefBitsFromChar( RefString[i], i, CheckIndex );
	}

	//	debug that string was truncated
	if ( DebugTruncation )
	{
		Debug_TruncatedRefString(RefString);
	}
}

//-----------------------------------------------------
//	pull out 5 characters and set from this string
//-----------------------------------------------------
void TRef::Set(const char* pRefString)
{
	if ( !pRefString )
	{
		TLDebug_Break("No string provided for Ref");
		SetInvalid();
		return;
	}

	//	reset ref
	m_Ref = 0;
	Bool CheckIndex = TLDebug::IsEnabled();

	//	extract chars up to terminator
	u32 i = 0;
	while ( i<TLRef_CharsPerRef && pRefString[i] != 0x0 )
	{
		m_Ref |= TLRef::GetRefBitsFromChar( pRefString[i], i, CheckIndex );
		i++;
	}

	Bool DebugTruncation = ENABLE_DEBUG_REF_TRUNCATION && (i >= TLRef_CharsPerRef) && (pRefString[i] != 0x0);

	//	debug that string was truncated
	if ( DebugTruncation )
	{
		TTempString RefString = pRefString;
		Debug_TruncatedRefString( RefString );
	}
}


//-----------------------------------------------------
//	set from array of 5 characters
//-----------------------------------------------------
void TRef::Set(const TArray<char>& RefStringChars)
{
	Bool CheckIndex = TLDebug::IsEnabled();

	//	reset ref
	m_Ref = 0;

	u32 Size = RefStringChars.GetSize();
	if ( Size > TLRef_CharsPerRef )
		Size = TLRef_CharsPerRef;

	//	convert the string characters to refalphabet indexes
	//	NULL characters are switched to spaces, aside from the first one which is set to ?
	for ( u32 i=0;	i<Size;	i++ )
	{
		m_Ref |= TLRef::GetRefBitsFromChar( RefStringChars[i], i, CheckIndex );
	}
}



//----------------------------------------------------
//	set from array of indexes
//----------------------------------------------------
void TRef::Set(const TArray<u32>& RefIndexes)
{
	//	reset ref
	m_Ref = 0;

	u32 Size = RefIndexes.GetSize();
	if ( Size > TLRef_CharsPerRef )
		Size = TLRef_CharsPerRef;

	//	convert the string characters to refalphabet indexes
	//	NULL characters are switched to spaces, aside from the first one which is set to ?
	for ( u32 i=0;	i<Size;	i++ )
	{
		//	get the index
		u32 RefIndex = RefIndexes[i];

		//	mask it off (just in case) so it doesnt overflow
		RefIndex &= TLRef_CharIndexBitMask;

		//	add bits for this index to the Ref
		//	shift along to it's position in the 32 bits
		m_Ref |= RefIndex << (i*TLRef_BitsPerChar);
	}
}

//-----------------------------------------------------
//	pull out the alphabet index from a ref
//-----------------------------------------------------
u32 TRef::GetRefCharIndex(u32 Index) const
{
	if ( !TLDebug_CheckIndex( Index, TLRef_CharsPerRef ) )
		return 0;

	//	extract index
	u32 BitIndex = Index * TLRef_BitsPerChar;
	u32 RefCharIndex = ( m_Ref >> BitIndex ) & TLRef_CharIndexBitMask;

	return RefCharIndex;
}

//-----------------------------------------------------
//	convert ref to a string. 
//	if Trim then any white spaces are removed at the end of the string. 
//	If UrlSafe then we use the alternative symbol for Space instead of a space so the string doesnt need to be url encoded
//-----------------------------------------------------
void TRef::GetString(TString& RefString,Bool Capitalise,Bool Trim,Bool UrlSafe) const
{
	for ( u32 i=0;	i<TLRef_CharsPerRef;	i++ )
	{
		//	extract index
		u32 RefCharIndex = GetRefCharIndex( i );

		//	convert index to character
		Bool UseAltTable = UrlSafe || (Capitalise && (i==0));
		TChar RefChar = TLRef::GetCharFromRefCharIndex( RefCharIndex, UseAltTable );

		RefString.Append( RefChar );
	}

	//	trim whitespace at the end of the string (never does anything in url safe mode)
	if ( Trim && !UrlSafe )
		RefString.Trim(FALSE);
}


//---------------------------------------------------------
//	increment the reference - don't just increment the u32 though! do it systematticly
//---------------------------------------------------------
const TRef& TRef::Increment()
{
	//	gather indexes
	TFixedArray<u32,TLRef_CharsPerRef> CharIndexes;
	CharIndexes.SetSize(TLRef_CharsPerRef);
	CharIndexes[0] = GetRefCharIndex(0);
	CharIndexes[1] = GetRefCharIndex(1);
	CharIndexes[2] = GetRefCharIndex(2);
	CharIndexes[3] = GetRefCharIndex(3);
	CharIndexes[4] = GetRefCharIndex(4);

	s32 IncIndex = CharIndexes.GetLastIndex();
	
	//	increment last character
	while ( TRUE )
	{
		//	if character is last in the alphabet rollover and change the previous char too
		if ( CharIndexes[IncIndex] == TLRef_AlphabetSize-1 )
		{
			CharIndexes[IncIndex] = 0;
			IncIndex--;
			if ( IncIndex<0 )
				IncIndex = CharIndexes.GetLastIndex();
		}
		else
		{
			CharIndexes[IncIndex]++;
			break;
		}
	}

	//	now re-set the ref
	Set( CharIndexes );

	return *this;
}




//---------------------------------------------------------
//	debug that the specified string was truncated to fit in as a ref
//---------------------------------------------------------
void TRef::Debug_TruncatedRefString(const TString& RefString) const
{
	TTempString DebugString("Ref string truncated from ");
	DebugString.Append( RefString );
	DebugString.Append(" to ");
	GetString( DebugString );

	TLDebug_Print( DebugString );
}


//---------------------------------------------------------
//	break with invalid ref message
//---------------------------------------------------------
void TRef::Debug_BreakInvalidRef() const
{
	TLDebug_Break("Invalid ref - created at runtime and doesn't fit ref mask?");
}


//---------------------------------------------------------
//	set from string in the same format as GetString
//---------------------------------------------------------
void TTypedRef::Set(const TString& RefString)
{
	//	split at square brackets
	TFixedArray<TBufferString<10>,2 > RefParts;
	
	//	split by [ first
	//	if character not found then fail
	if ( !RefString.Split('[', RefParts ) )
	{
		TDebugString Debug_String;
		Debug_String << "Invalid TTypedRef string, no [ \"" << RefString << "\"";
		TLDebug_Break( Debug_String );
		Set( TTypedRef() );
		return;
	}
	
	//	now make sure the string ends with ]
	if ( RefParts[1].GetCharLast() != ']' )
	{
		TDebugString Debug_String;
		Debug_String << "Invalid TTypedRef string, does not end with ]\"" << RefString << "\"";
		TLDebug_Break( Debug_String );
		Set( TTypedRef() );
		return;
	}
	
	//	set each part
	m_Ref = RefParts[0];
	m_TypeRef = RefParts[1];
}


void TTypedRef::GetString(TString& RefString,Bool Capitalise) const
{
	GetRef().GetString( RefString, Capitalise );
	RefString << '[';
	GetTypeRef().GetString( RefString, Capitalise );
	RefString << ']';
}


void TTypedRef::GetString(TString& RefString) const
{
	RefString << GetRef() << '[' << GetTypeRef() << ']';
}

