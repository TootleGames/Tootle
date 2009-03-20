#include "TRef.h"
#include "TFixedArray.h"
#include "TString.h"
#include "TKeyArray.h"


#define ENABLE_DEBUG_REF_TRUNCATION		FALSE
//#define ENABLE_DEBUG_REF_TRUNCATION		TLDebug::IsEnabled()


//	much faster ref char lookup table - faster than keyarray as keyarray isnt sorted (but probbaly will be faster either way)
#define ENABLE_DUMB_LOOKUP

//----------------------------------------------------------
//	predefined alphabet for the characters that fit into ref's
//----------------------------------------------------------
namespace TLRef
{
	const u32	g_BitsPerRefChar		= 6;	//	32bits / TRef::g_CharsPerRef
	const u32	g_RefCharTableSizeMax	= 1<<g_BitsPerRefChar;	//	64
	const u32	g_RefCharTable_Size		= 41;	//	supported chars in a ref
	const u32	g_CharIndexBitMask		= g_RefCharTableSizeMax-1;

	//	table for string->ref conversions (+1 for terminator)
	const char	g_RefCharTable[g_RefCharTable_Size+1]		= {	" abcdefghijklmnopqrstuvwxyz0123456789?-#_"	};

	//	alternate table for caps conversion (+1 for terminator)
	const char	g_RefCharTableAlt[g_RefCharTable_Size+1]	= {	" ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789?-#_"	};

	Bool		g_RefCharLookupValid	= FALSE;
	u32			g_InvalidRefMask		= 0x0;			//	uninitialised! I can probably work out an actual number but calculations are more accurate

#ifdef ENABLE_DUMB_LOOKUP
	u32			g_RefCharLookupDumb[256];				//	lookup table for char->refchar
#endif

	TKeyArray<char,u32>	g_RefCharLookup;				//	lookup table for char->refchar

	inline u32	GetRefCharIndex(char c);					//	get a refchar index from a string character
	inline char	GetCharFromRefCharIndex(u32 Index,Bool Uppercase=FALSE);			//	get a string character from a refchar index
	inline u32	GetRefBitsFromChar(char Char,u32 Index,Bool CheckIndex);

	void		GenerateCharLookupTable();
	void		DestroyCharLookupTable();

	TLArray::SortResult		RefSort(const TRef& aRef,const TRef& bRef,const void* pTestVal);	//	simple ref-sort func - for arrays of TRef's
}



//////////////////////////////////////////////////////




//----------------------------------------------------
//	simple ref-sort func - for arrays of TRef's
//----------------------------------------------------
TLArray::SortResult TLRef::RefSort(const TRef& aRef,const TRef& bRef,const void* pTestVal)
{
	//	if pTestVal provided it's the ref
	const TRef& TestRef = pTestVal ? *(const TRef*)pTestVal : bRef;

	//	== turns into 0 (is greater) or 1(equals)
	return aRef < TestRef ? TLArray::IsLess : (TLArray::SortResult)(aRef==TestRef);	
}



void TLRef::GenerateCharLookupTable()
{
#ifdef ENABLE_DUMB_LOOKUP
	for ( u32 x=0;	x<sizeof(g_RefCharLookupDumb)/sizeof(g_RefCharLookupDumb[0]);	x++ )
		g_RefCharLookupDumb[x] = 0;
#endif

	g_RefCharLookup.Empty();

	//	loop through the alphabets and add the keys
	for ( u32 i=0;	i<g_RefCharTable_Size;	i++ )
	{
		//	slightly modified to ensure terminator has an index of zero
		char Char = g_RefCharTable[i];
		u32 Index = (Char == 0x0) ? 0 : i;
		char AltChar = g_RefCharTableAlt[i];
		u32 AltIndex = (AltChar == 0x0) ? 0 : i;

		g_RefCharLookup.Add( Char, Index );
		g_RefCharLookup.Add( AltChar, AltIndex );

#ifdef ENABLE_DUMB_LOOKUP
		g_RefCharLookupDumb[ Char ] = Index;
		g_RefCharLookupDumb[ AltChar ] = AltIndex;
#endif
	}

	//	work out a VALID bit mask
	u32 ValidMask = 0x0;
	for ( u32 i=0;	i<TRef::g_CharsPerRef;	i++ )
		ValidMask |= TLRef::g_CharIndexBitMask << (i*TLRef::g_BitsPerRefChar);
	//	get the invalid mask to test against
	g_InvalidRefMask = ~ValidMask;

	g_RefCharLookupValid = TRUE;
}

void TLRef::DestroyCharLookupTable()
{
	g_RefCharLookup.Empty(TRUE);
}


//---------------------------------------------------
//	get a refchar index from a string character
//---------------------------------------------------
inline u32 TLRef::GetRefCharIndex(char c)
{
	//	use key lookup table if it's been generated as it's faster
	if ( g_RefCharLookupValid )
	//if ( g_RefCharLookup.GetSize() )
	{
#ifdef ENABLE_DUMB_LOOKUP
		//	fast dumb version
		return g_RefCharLookupDumb[c];
#endif

		u32* pRefCharIndex = g_RefCharLookup.Find(c);
		if ( pRefCharIndex )
			return *pRefCharIndex;
	}
	else
	{
		//	slow version, but must be used before TLRef::Init()
		for ( u32 i=0;	i<g_RefCharTable_Size;	i++ )
		{
			if ( g_RefCharTable[i] == c )
				return i;

			if ( g_RefCharTableAlt[i] == c )
				return i;
		}	
	}

	//	unsupported character
	return GetRefCharIndex( '?' );
}


//---------------------------------------------------
//	get a string character from a refchar index
//---------------------------------------------------
inline char TLRef::GetCharFromRefCharIndex(u32 Index,Bool Uppercase)
{
	//	invalid index
	if ( Index >= g_RefCharTable_Size )
		return 0;

	return Uppercase ? g_RefCharTableAlt[Index] : g_RefCharTable[Index];
}


//---------------------------------------------------
//	get a string character from a refchar index
//---------------------------------------------------
inline u32 TLRef::GetRefBitsFromChar(char Char,u32 Index,Bool CheckIndex)
{
	if ( CheckIndex )
	{
		if ( !TLDebug_CheckIndex( Index, TRef::g_CharsPerRef ) )
			return 0x0;
	}

	//	get the index
	u32 RefIndex = TLRef::GetRefCharIndex( Char );

	//	mask it off (just in case) so it doesnt overflow
	if ( CheckIndex && RefIndex > TLRef::g_CharIndexBitMask )
	{
		TLDebug_Break("RefCharIndex greater than allowed - lookup table is corrupt?");
		RefIndex &= TLRef::g_CharIndexBitMask;
	}

	//	add bits for this index to the Ref
	//	shift along to it's position in the 32 bits
	return RefIndex << (Index*TLRef::g_BitsPerRefChar);
}

//-----------------------------------------------------
//	pull out 5 characters and set from this string
//-----------------------------------------------------
void TRef::Set(const TString& RefString)
{
	Bool DebugTruncation = FALSE;

	//	pull out first 5 characters (or less) from our string
	u32 StringLength = RefString.GetLength();
	if ( StringLength > TRef::g_CharsPerRef )
	{
		StringLength = TRef::g_CharsPerRef;
		DebugTruncation = ENABLE_DEBUG_REF_TRUNCATION;
	}

	//	reset ref
	m_Ref = 0x0;
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
	m_Ref = 0x0;
	Bool CheckIndex = TLDebug::IsEnabled();

	//	extract chars up to terminator
	u32 i = 0;
	while ( i<TRef::g_CharsPerRef && pRefString[i] != 0x0 )
	{
		m_Ref |= TLRef::GetRefBitsFromChar( pRefString[i], i, CheckIndex );
		i++;
	}

	Bool DebugTruncation = ENABLE_DEBUG_REF_TRUNCATION && (i >= TRef::g_CharsPerRef) && (pRefString[i] != 0x0);

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
	m_Ref = 0x0;

	u32 Size = RefStringChars.GetSize();
	if ( Size > TRef::g_CharsPerRef )
		Size = TRef::g_CharsPerRef;

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
	m_Ref = 0x0;

	u32 Size = RefIndexes.GetSize();
	if ( Size > TRef::g_CharsPerRef )
		Size = TRef::g_CharsPerRef;

	//	convert the string characters to refalphabet indexes
	//	NULL characters are switched to spaces, aside from the first one which is set to ?
	for ( u32 i=0;	i<Size;	i++ )
	{
		//	get the index
		u32 RefIndex = RefIndexes[i];

		//	mask it off (just in case) so it doesnt overflow
		RefIndex &= TLRef::g_CharIndexBitMask;

		//	add bits for this index to the Ref
		//	shift along to it's position in the 32 bits
		m_Ref |= RefIndex << (i*TLRef::g_BitsPerRefChar);
	}
}

//-----------------------------------------------------
//	pull out the alphabet index from a ref
//-----------------------------------------------------
u32 TRef::GetRefCharIndex(u32 Index) const
{
	if ( !TLDebug_CheckIndex( Index, TRef::g_CharsPerRef ) )
		return 0;

	//	extract index
	u32 BitIndex = Index * TLRef::g_BitsPerRefChar;
	u32 RefCharIndex = ( m_Ref >> BitIndex ) & TLRef::g_CharIndexBitMask;

	return RefCharIndex;
}

//-----------------------------------------------------
//	convert ref to a string
//-----------------------------------------------------
void TRef::GetString(TString& RefString,Bool Capitalise) const
{
	for ( u32 i=0;	i<TRef::g_CharsPerRef;	i++ )
	{
		//	extract index
		u32 RefCharIndex = GetRefCharIndex( i );

		//	convert index to character
		//	gr: just to make them a little nicer to read, I've made the first character in the string uppercase
		char RefChar = TLRef::GetCharFromRefCharIndex( RefCharIndex, Capitalise && (i==0) );
		RefString.Append( RefChar );
	}
}


//---------------------------------------------------------
//	increment the reference - don't just increment the u32 though! do it systematticly
//---------------------------------------------------------
const TRef& TRef::Increment()
{
	//	gather indexes
	TFixedArray<u32,TRef::g_CharsPerRef> CharIndexes(TRef::g_CharsPerRef);

	for ( u32 i=0;	i<CharIndexes.GetSize();	i++ )
	{
		CharIndexes[i] = GetRefCharIndex(i);
	}

	s32 IncIndex = CharIndexes.GetSize()-1;
	
	//	increment last character
	while ( TRUE )
	{
		//	if character is last in the alphabet rollover and change the previous char too
		if ( CharIndexes[IncIndex] == TLRef::g_RefCharTable_Size-1 )
		{
			CharIndexes[IncIndex] = 0;
			IncIndex--;
			if ( IncIndex<0 )
				IncIndex = CharIndexes.GetSize()-1;
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


