/*------------------------------------------------------
	
	XML type

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TString.h>
#include <TootleCore/TPtrArray.h>
#include <TootleCore/TKeyArray.h>


class TXmlTag;	//	branch of xml
class TXml;		//	xml root - there is no root branch - kinda

namespace TLXml
{
	enum TagType
	{
		TagType_Invalid = 0,
		TagType_OpenClose,		//	opening of open/close tag <X></x> (first one)
		TagType_SelfClose,		//	self-closed tag <x />
		TagType_Close,			//	closing tag </x>
		TagType_QuestionMark,	//	markup info <? ?>
		TagType_Hidden,			//	always self closed? <! comment >
	};
}


//---------------------------------------------------------
//	XML branch - like binary tree, recursive type
//---------------------------------------------------------
class TXmlTag
{
	friend class TXml;
public:
	TXmlTag();
	TXmlTag(const TString& TagName,TLXml::TagType Type);

	void						SetData(const TString& Data)					{	m_Data = Data;	}
	Bool						SetProperties(TString& PropertiesString);		//	set properties and parse the properties string
	Bool						IsSelfClosed() const							{	return m_TagType != TLXml::TagType_OpenClose;	}
	const TString&				GetTagName() const								{	return m_TagName;	}
	const TLXml::TagType&		GetTagType() const								{	return m_TagType;	}
	TString*					GetProperty(const TString& PropertyName)		{	return m_Properties.Find( PropertyName );	}


	TPtr<TXmlTag>				GetChild(const TString& TagName)		{	return m_Children.FindPtr( TagName );	}
	TPtrArray<TXmlTag>&			GetChildren()							{	return m_Children;	}
	void						AddChild(TPtr<TXmlTag>& pTag)			{	m_Children.Add( pTag );	}

	inline Bool					operator<(const TXmlTag& Tag) const		{	return m_TagName < Tag.m_TagName;	}
	inline Bool					operator==(const TXmlTag& Tag) const	{	return m_TagName == Tag.m_TagName;	}
	inline Bool					operator==(const TString& TagName) const	{	return m_TagName == TagName;	}

	void						Debug_PrintTree(u32 TreeLevel) const;	//	debug_print the tree

protected:
	TLXml::TagType					m_TagType;			//	kind of tag
	TString							m_TagName;			//	branch's name (<TAG>)
	TKeyArray<TTempString,TString>	m_Properties;		//	list of properties in the tag (<... x="y" ...>)
	TString							m_Data;				//	contents of tag (<XYZ>Data</XYZ>)
	TPtrArray<TXmlTag>				m_Children;			//	child tags
};



//---------------------------------------------------------
//	XML parser
//---------------------------------------------------------
class TXml
{
public:
	TXml()				{}

	SyncBool			Import(const TString& XmlString);	//	parse xml data

	void				Empty(Bool Dealloc=FALSE)			{	m_Children.Empty(Dealloc);	}
	TPtr<TXmlTag>		GetChild(const TString& TagName)	{	return m_Children.FindPtr( TagName );	}
	void				AddChild(TPtr<TXmlTag>& pTag)		{	m_Children.Add( pTag );	}

	void				Debug_PrintTree(const TString& XmlFilename) const;			//	debug_print the tree

protected:
	TPtrArray<TXmlTag>	m_Children;							//	tags at the root
};

