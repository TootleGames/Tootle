/*------------------------------------------------------
	Our nice flexible engine string type

-------------------------------------------------------*/
#pragma once

#include "TArray.h"
#include "TFixedArray.h"


class TString;

#define TLString_Terminator		0x0

namespace TLString
{
	Bool		IsCharWhitespace(const char& Char);
	Bool		IsCharLetter(const char& Char);
	Bool		IsCharLowercase(const char& Char);
	Bool		IsCharUppercase(const char& Char);
	Bool		SetCharLowercase(char& Char);
	Bool		SetCharUppercase(char& Char);
	s32			GetCharInteger(const char& Char);
	s32			GetCharHexInteger(const char& Char);

	namespace Platform
	{
		u32		Strlen(const char* pString);
	}

	#define TLString_StringGrowBy	8
}


//------------------------------------------------------
//	todo: make templated for other string types? or for fixed lengths?
//		currently just dynamic char's
//------------------------------------------------------
class TString
{
protected:
	typedef char			CHARTYPE;

public:
	TString() : m_DataArray ( NULL, TLString_StringGrowBy )						{	}
	TString(const TString& String) : m_DataArray ( NULL, TLString_StringGrowBy ){	Append( String.GetData(), String.GetLength() );	}
	TString(const CHARTYPE* pString,...);										//	formatted constructor
	virtual ~TString()															{	}

	inline u32				GetLength() const					{	return GetStringArray().GetSize();	}
	inline u32				GetLengthWithoutTerminator() const	{	return GetCharLastIndex() + 1;	}
	inline const CHARTYPE*	GetData() const						{	return GetStringArray().GetData();	}

	//	gr: only temporary access - todo: revoke non-const accessor
	inline CHARTYPE*		GetData()				{	return GetStringArray().GetData();	}

	//	accessors
	void					SetLength(u32 NewLength)						{	GetStringArray().SetSize( NewLength );	AddTerminator(FALSE);	}	//	set the string to a certain size (usually for buffering)
	void					SetAllocSize(u32 AllocLength);					//	allocate buffer for string data so that it doesnt need to alloc more later
	u32						GetAllocSize() const							{	return GetStringArray().GetAllocSize();	}
	void					Empty(Bool Dealloc=FALSE)						{	GetStringArray().Empty(Dealloc);	}
	void					Set(const TString& String)						{	Empty();	Append( String );	}
	void					Set(const CHARTYPE* pString,s32 Length=-1)		{	Empty();	Append( pString, Length );	}
	void					Append(const CHARTYPE& Char);					//	append a single character onto the string
	void					Append(const TString& String,s32 Length=-1)		{	Append( String.GetData(), (Length<0 || Length > (s32)String.GetLength()) ? String.GetLength() : Length );	}
	void					Append(const CHARTYPE* pString,s32 Length=-1);	//	add string onto the end of the current string
	void					Append(const TString& String,u32 From,s32 Length);	//	append part of a string (use -1 as the length to copy everything FROM from)
	void					Appendf(const CHARTYPE* pString,...);			//	add string onto the end of the current string
	Bool					Trim();											//	trim whitespace from front and back of string - returns TRUE if changed
	Bool					SetLowercase();									//	change all uppercase characters to lowercase - returns TRUE if changed
	Bool					SetUppercase();									//	change all lowercase characters to uppercase - returns TRUE if changed

	//	string info functions
	s32						GetCharIndex(const CHARTYPE& Char,u32 From=0) const			{	return GetStringArray().FindIndex( Char, From );	}
	s32						GetCharIndexNonWhitespace(u32 From=0) const;				//	find the next non-whitespace char
	s32						GetCharIndexWhitespace(u32 From=0) const;					//	find the next whitespace char
	s32						GetLastCharIndex(const CHARTYPE& Char,s32 From=-1) const	{	return GetStringArray().FindIndexReverse( Char, From );	}
	Bool					GetCharExists(const CHARTYPE& Char) const					{	return GetStringArray().Exists( Char );	}
	Bool					IsEqual(const TString& String) const;						//	comparison to string
	Bool					IsEqual(const CHARTYPE* pString,s32 Length=-1) const;		//	comparison to string
	Bool					IsLessThan(const TString& String) const;					//	comparison to string
	template<class STRINGTYPE>
	Bool					Split(const CHARTYPE& SplitChar,TArray<STRINGTYPE>& StringArray) const;		//	split string by SplitChar into array. if no cases of SplitChar then FALSE is return and no strings are added to the array
	Bool					GetInteger(s32& Integer) const;					//	turn string into an integer - fails if some non-integer stuff in it (best to trim first where possible)
	Bool					GetFloat(float& Float) const;					//	turn string into a float
	Bool					GetHexInteger(u32& Integer) const;				//	turn hexidecimal string into an integer (best to trim first where possible)

	CHARTYPE&				GetCharAt(u32 Index)							{	return GetStringArray().ElementAt(Index);	}
	const CHARTYPE&			GetCharAt(u32 Index) const						{	return GetStringArray().ElementAtConst(Index);	}
	CHARTYPE				GetCharLast() const;							//	get the last char. if string is empty a terminator is returned. If the string ends with a terminator, it returns the last char before terminator (if any)
	s32						GetCharLastIndex() const;						//	get the index of last char. -1 if empty or all terminators
	void					RemoveCharAt(u32 Index,u32 Amount)				{	GetStringArray().RemoveAt(Index,Amount);	}

	inline const CHARTYPE&	operator[](u32 Index) const						{	return GetCharAt(Index);	}

	inline TString&			operator=(const CHARTYPE* pString)				{	Set( pString );	return *this;	}
	inline TString&			operator=(const TString& String)				{	Set( String );	return *this;	}
	inline Bool				operator==(const TString& String) const			{	return IsEqual( String );	}
	inline Bool				operator==(const CHARTYPE* pString) const		{	return IsEqual( pString );	}
	inline Bool				operator!=(const TString& String) const			{	return !IsEqual( String );	}
	inline Bool				operator!=(const CHARTYPE* pString) const		{	return !IsEqual( pString );	}
	inline Bool				operator<(const TString& String) const			{	return IsLessThan( String );	}

protected:
	virtual TArray<CHARTYPE>&		GetStringArray()						{	return m_DataArray;	}
	virtual const TArray<CHARTYPE>&	GetStringArray() const					{	return m_DataArray;	}

	u32							GetLength(const CHARTYPE* pString);			//	get the length from some other type of string

	//	internal manipulation of buffer only!
	void						AddTerminator(Bool ForceTerminator=FALSE);		//	make sure there's a terminator on the end. ForceTerminator will overwrite the last character with a terminator if there isn't one (string wont grow)
	void						RemoveTerminator();								//	remove terminators from end of string
	void						SetLength(u32 NewLength,Bool ForceTerminator)	{	GetStringArray().SetSize( NewLength );	AddTerminator(ForceTerminator);	}	//	set the string to a certain size (usually for buffering)

protected:
	TArray<CHARTYPE>			m_DataArray;
};
	

//---------------------------------------------------------
//	same as TString but the array isn't dynamiccaly allocated, instead
//	it's a fixed array, much more effecient CPU wise, but more
//	expensvie memory wise
//---------------------------------------------------------
template<int SIZE>
class TBufferString : public TString
{
public:
	//	gr: note: do not use TString constructors as VTable isn't setup so doesn't use the TFixedArray properly
	TBufferString() : m_FixedDataArray(0)									{	}
	TBufferString(const TString& String) : m_FixedDataArray(0)				{	Append( String );	}
	TBufferString(const CHARTYPE* pString) : m_FixedDataArray(0)			{	Append( pString );	}

	inline const CHARTYPE&	operator[](u32 Index) const						{	return GetCharAt(Index);	}

	inline TBufferString&	operator=(const CHARTYPE* pString)				{	Set( pString );	return *this;	}
	inline TBufferString&	operator=(const TString& String)				{	Set( String );	return *this;	}
	inline Bool				operator==(const TString& String) const			{	return IsEqual( String );	}
	inline Bool				operator==(const CHARTYPE* pString) const		{	return IsEqual( pString );	}
	inline Bool				operator!=(const TString& String) const			{	return !IsEqual( String );	}
	inline Bool				operator!=(const CHARTYPE* pString) const		{	return !IsEqual( pString );	}
	inline Bool				operator<(const TString& String) const			{	return IsLessThan( String );	}

protected:
	virtual TArray<CHARTYPE>&		GetStringArray()		{	return m_FixedDataArray;	}
	virtual const TArray<CHARTYPE>&	GetStringArray() const	{	return m_FixedDataArray;	}

protected:
	TFixedArray<TString::CHARTYPE,SIZE>		m_FixedDataArray;
};





typedef TBufferString<512> TTempString;








//------------------------------------------------------
//	split string by SplitChar into array. if no cases of 
//	SplitChar then FALSE is return and no strings are added to the array
//------------------------------------------------------
template<class STRINGTYPE>
Bool TString::Split(const CHARTYPE& SplitChar,TArray<STRINGTYPE>& StringArray) const
{
	//	find first split point
	u32 SplitFrom = 0;
	s32 SplitTo = GetCharIndex( SplitChar );

	//	SplitChar not found
	if ( SplitTo == -1 )
		return FALSE;

	u32 LastCharIndex = GetCharLastIndex();

	while ( SplitFrom <= LastCharIndex )
	{
		if ( (s32)SplitFrom >= SplitTo )
		{
			TLDebug_Break( TString("String split has got confused; SplitFrom: %d, SplitTo: %d", SplitFrom, SplitTo ) );
			break;
		}

		//	make up new string
		STRINGTYPE SplitString;
		SplitString.Append( *this, SplitFrom, SplitTo-SplitFrom );

		//	add to array
		s32 AddIndex = StringArray.Add( SplitString );
		if ( AddIndex == -1 )
		{
			//	cannot fit any more strings into this array
			return TRUE;
		}

		//	step over the character we split at, so +1
		SplitFrom = SplitTo + 1;

		//	find next split
		SplitTo = GetCharIndex( SplitChar, SplitFrom );

		//	split at end of the string
		if ( SplitTo == -1 )
			SplitTo = LastCharIndex+1;
	}

	return TRUE;
}

