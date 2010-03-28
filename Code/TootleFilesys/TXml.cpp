#include "TXml.h"


#define CULL_HIDDEN_TAGS	//	gr: i dont think we ever have a need to include hidden tags at runtime


namespace TLXml
{
	//	parse string to new tag
	TPtr<TXmlTag>	ParseTag(const TString& XmlString,u32 TagOpenIndex,u32 TagCloseIndex );
	Bool			IsCharText(TChar Char);			//	is character valid text for a tag
}



Bool TLXml::IsCharText(TChar Char)
{
	if ( Char >= 'a' && Char <= 'z' )
		return TRUE;

	if ( Char >= 'A' && Char <= 'Z' )
		return TRUE;

	if ( Char >= '0' && Char <= '9' )
		return TRUE;
	
	if ( Char == '-' )
		return TRUE;

	if ( Char == '_' )
		return TRUE;

	return FALSE;
}
	
//---------------------------------------------------------
//	parse string to new tag
//---------------------------------------------------------
TPtr<TXmlTag> TLXml::ParseTag(const TString& XmlString,u32 TagOpenIndex,u32 TagCloseIndex)
{
	//	work out contents of the tag
	s32 TagLength = TagCloseIndex + 1 - TagOpenIndex;
	s32 ContentLength = TagLength - 2;	//	cut off < and >

	//	not enough data inside to be valid
	if ( ContentLength <= 0 )
		return NULL;

	//	make string
	TString TagContents;
	TagContents.Append( XmlString, TagOpenIndex+1, ContentLength );

	//	by default step over first char as we've used it to work out the tag type
	u32 StepOver = 1;

	//	work out type
	TLXml::TagType Type = TLXml::TagType_Invalid;
	if ( TagContents[0] == '!' )
		Type = TLXml::TagType_Hidden;
	else if ( TagContents[0] == '/' )
		Type = TLXml::TagType_Close;
	else if ( TagContents[0] == '?' )
		Type = TLXml::TagType_QuestionMark;
	else
	{
		//	dont step over first char
		StepOver = 0;

		//	plain tag type, check if it's self closing though
		if ( TagContents.GetCharLast() == '/' )
		{
			Type = TLXml::TagType_SelfClose;

			//	remove the / off the end of the contents string otherwise the properties fail to parse
			TagContents.RemoveCharAt( TagContents.GetCharGetLastIndex() );
		}
		else
		{
			Type = TLXml::TagType_OpenClose;
		}
	}

	//	cut out character's we've parsed
	TagContents.RemoveCharAt( 0, StepOver );

	//	try and extract a tag name
	u32 TagNameLength = 0;
	while ( TagNameLength < TagContents.GetLength() )
	{
		//	break when we reach a character that can't be in a tag name
		if ( TLXml::IsCharText( TagContents[TagNameLength] ) )
			TagNameLength++;
		else
			break;
	}

	TTempString TagName;
	TagName.Append( TagContents, 0, TagNameLength );

	TString TagProperties;
	TagProperties.Append( TagContents, TagNameLength, -1 );

	//	make up tag
	TPtr<TXmlTag> pNewTag = new TXmlTag( TagName, Type );
	
	//	gr: don't parse properties for hidden tags - some characters can cause parse errors
	//		as it won't be formatted for property-like syntax
	//		in future we *could* put TagContents into the data string (m_Data) if we ever
	//		have a need for comments
	if ( Type != TLXml::TagType_Hidden )
	{
		//	error parsing properties, abort
		if ( !pNewTag->SetProperties( TagProperties ) )
			return NULL;
	}

	return pNewTag;
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
TXmlTag::TXmlTag() :
	m_TagType		( TLXml::TagType_Invalid )
{
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
TXmlTag::TXmlTag(const TString& TagName,TLXml::TagType Type) :
	m_TagName		( TagName ),
	m_TagType		( Type )
{
	m_TagName.SetLowercase();
}


//---------------------------------------------------------
//	set properties and parse the properties string
//---------------------------------------------------------
Bool TXmlTag::SetProperties(TString& PropertiesString)	
{
	//	cut off whitespace
	PropertiesString.Trim();

	//	empty
	u32 PropertiesStringLength = PropertiesString.GetLength();
	if ( PropertiesStringLength == 0 )
		return TRUE;

	u32 CharIndex = 0;
	s32 PropertyNameStart = 0;
	u32 PropertyNameLength = 0;
	while ( CharIndex < PropertiesStringLength )
	{
		//	loop till we find end of the property name
		if ( PropertyNameStart != -1 )
		{
			if ( TLXml::IsCharText( PropertiesString[CharIndex] ) )
			{
				CharIndex++;
				//	if we reached the end then process as if it were a non text char
				if ( CharIndex != PropertiesStringLength )
					continue;
			}

			//	found end of property name
			PropertyNameLength = CharIndex-PropertyNameStart;
			Bool HasValue = FALSE;

			//	end of string, no more stuff to check
			if ( CharIndex == PropertiesStringLength || CharIndex == PropertiesStringLength-1 )
			{
				HasValue = FALSE;
			}
			else
			{
				//	does this property have an equals after it? if so, pull out the value assigned to property
				TChar& NextChar = PropertiesString.GetCharAt(CharIndex);
				if ( NextChar == '=' )
					HasValue = TRUE;
			}
				
			TTempString PropertyName;
			PropertyName.Append( PropertiesString, PropertyNameStart, PropertyNameLength );
			PropertyNameStart = -1;

			if ( !HasValue )
			{
				//	no equals, just add property name
				m_Properties.Add( PropertyName, TTempString() );
				//	CharIndex++;	//	gr: maybe neeeded?
				continue;
			}
			
			//	find required pair of " after =
			s32 FirstQuoteIndex = PropertiesString.GetCharIndex('"', CharIndex);
			if ( FirstQuoteIndex == -1 )
			{
				TLDebug_Break( TString("Quote missing after property %s= in xml", PropertyName.GetData() ) );
				return FALSE;
			}
			s32 SecondQuoteIndex = PropertiesString.GetCharIndex('"', FirstQuoteIndex+1);
			if ( SecondQuoteIndex == -1 )
			{
				TLDebug_Break( TString("Unmatched Quote after property %s= in xml", PropertyName.GetData() ) );
				return FALSE;
			}

			//	make up data string
			u32 DataStringLength = SecondQuoteIndex - FirstQuoteIndex - 1;			
			TString DataString;
			DataString.Append( PropertiesString, FirstQuoteIndex+1, DataStringLength );

			//	add property
			m_Properties.Add( PropertyName, DataString );

			//	done! move char index
			CharIndex = SecondQuoteIndex+1;
			continue;
		}

		//	if non-whitespace we've found the start of another property name
		if ( !TLString::IsCharWhitespace( PropertiesString[CharIndex] ) )
		{
			PropertyNameStart = CharIndex;
		}

		CharIndex++;
	}
	
	//	remaining property name?
	if ( PropertyNameStart != -1 )
	{
		TTempString PropertyName;
		PropertyName.Append( PropertiesString, PropertyNameStart, -1 );
		m_Properties.Add( PropertyName, TTempString() );
	}

	return TRUE;	
}


//---------------------------------------------------------
//	debug_print the tree
//---------------------------------------------------------
void TXmlTag::Debug_PrintTree(u32 TreeLevel) const
{
	TTempString Indent;
	for ( u32 i=0;	i<TreeLevel;	i++ )
		Indent.Append('\t');

	//	print out name of branch
	TTempString TagString( Indent );
	TagString.Append("<");

	if ( m_TagType == TLXml::TagType_Hidden )
		TagString.Append('!');
	else if ( m_TagType == TLXml::TagType_QuestionMark )
		TagString.Append('?');

	TagString.Append( m_TagName );

	//	add properties
	for ( u32 p=0;	p<m_Properties.GetSize();	p++ )
	{
		const TKeyArray<TStringLowercase<TTempString>,TString>::PAIRTYPE& Property = m_Properties.GetPairAt(p);

		TagString.Append(' ');
		TagString.Append( Property.m_Key );

		if ( Property.m_Item.GetLength() > 0 )
		{
			TagString.Append("=\"");
			TagString.Append( Property.m_Item );
			TagString.Append('"');
		}
	}

	if ( m_TagType == TLXml::TagType_SelfClose )
		TagString.Append('/');
	TagString.Append('>');

	//	render open and close tags if we have children
	if ( m_Children.GetSize() )
	{
		//	print tag
		TLDebug_Print( TagString );

		//	render children
		for ( u32 c=0;	c<m_Children.GetSize();	c++ )
		{
			const TPtr<TXmlTag>& pChildTag = m_Children[c];
			pChildTag->Debug_PrintTree(TreeLevel+1);
		}
		
		//	print closing tag
		TTempString CloseTagString(Indent);
		CloseTagString.Append("</");
		CloseTagString.Append( m_TagName );
		CloseTagString.Append('>');
		TLDebug_Print( CloseTagString );
	}
	else if ( IsSelfClosed() )
	{
		//	just print the self-closed tag
		TLDebug_Print( TagString );
	}
	else
	{
		//	just print open and close on one line (if not self closing)
		TTempString TagLine;
		TagLine.Append( TagString );
		TagLine.Append("</");
		TagLine.Append( m_TagName );
		TagLine.Append(">");
		TLDebug_Print( TagLine );
	}
}

	
//---------------------------------------------------------
//	get children with mathcing tag
//---------------------------------------------------------
void TXmlTag::GetChildren(const TString& TagName,TPtrArray<TXmlTag>& Children)
{
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChild = m_Children[c];
		if ( pChild->GetTagName() == TagName )
		{
			Children.Add( pChild );
		}
	}
}



//---------------------------------------------------------
//	parse xml data
//---------------------------------------------------------
SyncBool TXml::Import(const TString& XmlString)
{
	//	create a stack of new tags as we work on them. 
	//	when a tag is opened we create a new tag in the array
	//	when one is closed we put it into it's parent in the stack 
	//	until we close the first, then we add it as a child
	//	if we run out of data and a tag is still open then parsing
	//	failed (invalid markup)
	TPtrArray<TXmlTag> TagStack;

	Bool SyntaxError = FALSE;
	s32 CharIndex = 0;
	s32 LastCharIndex = -1;	//	just to catch my mistakes :)
	while ( CharIndex < (s32)XmlString.GetLength() )
	{
		//	just catching grahams mistakes :)
		{
			if ( CharIndex == LastCharIndex )
			{
				if ( TLDebug_Break("Re-parsing same char index - mistake in grahams code?") )
				{
					SyntaxError = TRUE;
					break;
				}
			}
			LastCharIndex = CharIndex;
		}

		//	todo: walk over white space?
		s32 TagOpenIndex = XmlString.GetCharIndex('<',CharIndex);
		if ( TagOpenIndex == -1 )
		{
			//	no more tags
			break;
		}

		//	get close of new tag
		s32 TagCloseIndex = XmlString.GetCharIndex('>',TagOpenIndex);
		if ( TagCloseIndex == -1 )
		{
			SyntaxError = TRUE;
			break;
		}

		//	parse new tag
		TPtr<TXmlTag> pNewTag = TLXml::ParseTag( XmlString, TagOpenIndex, TagCloseIndex );
		
		//	failed to parse tag
		if ( !pNewTag )
		{
			SyntaxError = TRUE;
			break;
		}

		//	skip over hidden tags
#ifdef CULL_HIDDEN_TAGS
		if ( pNewTag->m_TagType == TLXml::TagType_Hidden )
		{
			//	goto next tag
			CharIndex = TagCloseIndex;
			continue;
		}
#endif

		//	closing tag for the tag on the end of the stack (tag names should match)
		if ( pNewTag->m_TagType == TLXml::TagType_Close )
		{
			//	nothing to close??
			if ( !TagStack.GetSize() )
			{
				TTempString Debug_String("XML Syntax error: closing tag with no previous open tag; ");
				TLDebug_Break( Debug_String );
				SyntaxError = TRUE;
				break;
			}

			TPtr<TXmlTag>& pLastTag = TagStack.ElementAt( TagStack.GetLastIndex() );

			//	wrong type?
			if ( pLastTag->m_TagType != TLXml::TagType_OpenClose )
			{
				TTempString Debug_String("XML Syntax error: mis matched type of tags; ");
				Debug_String.Append( pLastTag->GetTagName() );
				TLDebug_Break( Debug_String );
				SyntaxError = TRUE;
				break;
			}

			//	match tag names
			if ( pNewTag->GetTagName() != pLastTag->GetTagName() )
			{
				TTempString Debug_String("XML Syntax error: mis matched start/end tags; ");
				Debug_String.Append( pLastTag->GetTagName() );
				Debug_String.Append(" ... ");
				Debug_String.Append( pNewTag->GetTagName() );
				TLDebug_Break( Debug_String );
				SyntaxError = TRUE;
				break;
			}

			//	okay, checked, is the closing tag... close!
			//	does the last-open tag have a parent?
			if ( TagStack.GetSize() > 1 )
			{
				TPtr<TXmlTag>& pLastLastTag = TagStack.ElementAt( TagStack.GetLastIndex()-1 );
				pLastLastTag->AddChild( pLastTag );
			}
			else
			{
				//	open tag had no parent, add to root
				AddChild( pLastTag );
			}

			//	remove last tag from stack
			TagStack.RemoveLast();

			//	move to next char to find next tag
			CharIndex = TagCloseIndex;
			continue;
		}

		//	if tag is not open/close then just add to last open tag of the stack (or to root)
		if ( pNewTag->IsSelfClosed() )
		{
			//	add to last open tag in the stack
			if ( TagStack.GetSize() )
			{
				TagStack.ElementLast()->AddChild( pNewTag );
			}
			else
			{
				//	nothing in stack, add to root
				AddChild( pNewTag );
			}

			//	process next tag
			CharIndex = TagCloseIndex;
			continue;
		}

		//	is open/close tag - extract data up until the next tag
		s32 NextTagOpenIndex = XmlString.GetCharIndex('<',TagCloseIndex);
		if ( NextTagOpenIndex == -1 )
		{
			//	no more tags? means this open tag is broken
			TLDebug_Break("XML Syntax error: Expected another tag as previous tag is still open");
			SyntaxError = TRUE;
			break;
		}

		//	extract data into the new tag
		TString TagData;
		u32 DataStart = TagCloseIndex+1;
		TagData.Append( XmlString, DataStart, NextTagOpenIndex-DataStart );
		TagData.Trim();
		pNewTag->SetData( TagData );

		//	add this (open) tag to the stack
		TagStack.Add( pNewTag );

		//	move along to this new tag :)
		CharIndex = NextTagOpenIndex;
	}

	//	tags left in stack? error parsing then
	if ( TagStack.GetSize() > 0 )
	{
		Debug_PrintTree("xml filename");
		TLDebug_Break("XML Syntax error: Unprocessed tags (probably parser error)");
		SyntaxError = TRUE;
	}

	//	error parsing
	if ( SyntaxError )
		return SyncFalse;

	return SyncTrue;
}



//---------------------------------------------------------
//	debug_print the tree
//---------------------------------------------------------
void TXml::Debug_PrintTree(const TString& XmlFilename) const
{
	//	print out filename
	TLDebug_Print(XmlFilename);

	//	print tags
	if ( m_Children.GetSize() == 0 )
	{
		TLDebug_Print(TTempString("*empty*"));
		return;
	}

	//	render children
	for ( u32 c=0;	c<m_Children.GetSize();	c++ )
	{
		const TPtr<TXmlTag>& pChildTag = m_Children[c];
		pChildTag->Debug_PrintTree(0);
	}
}


