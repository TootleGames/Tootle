/*------------------------------------------------------

	Text Render object

-------------------------------------------------------*/
#pragma once

#include "TRenderNode.h"


namespace TLRender
{
	class TRenderNode;
	class TRenderNodeGlyph;
	class TRenderNodeText;
}



//----------------------------------------------------
//	child render object of TRenderNodeText - only difference is 
//	that it holds(per render) a direct pointer to the mesh for
//	the glyph, rather than looking it up from the ref
//----------------------------------------------------
class TLRender::TRenderNodeGlyph : public TLRender::TRenderNode
{
	friend class TLRender::TRenderNodeText;

protected:
	TRenderNodeGlyph(TRefRef RenderNodeRef=TRef());

	virtual void			GetMeshAsset(TPtr<TLAsset::TMesh>& pMesh) 	{	pMesh = m_pGlyphMesh;	}

public:
	TPtr<TLAsset::TMesh>	m_pGlyphMesh;
	u16						m_Character;		//	character in string
};



//--------------------------------------------------------
//	text render object which renders a string with a given font
//--------------------------------------------------------
class TLRender::TRenderNodeText : public TLRender::TRenderNode
{
public:
	enum TextFlags
	{
		Centered,
		StretchToBoxHeight,	//	use render object's bounding box to stretch the height to. if StrethToBoxWidth not set then width is aspect
		StrethToBoxWidth,	//	use render object's bounding box to stretch the WIDTH to. if StrethToBoxHeight not set then height is aspect - if both are set then box is filled
	};

public:
	TRenderNodeText(TRefRef RenderNodeRef=TRef());

	TFlags<TextFlags>&		GetTextFlags()						{	return m_TextFlags;	}
	TRefRef					GetFontRef() const					{	return m_FontRef;	}
	void					SetFontRef(TRefRef FontRef)			{	m_FontRef = FontRef;	}
	const TString&			GetString() const					{	return m_String;	}
	void					SetString(const TString& String);	//	setup new string

	virtual Bool			Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList);	//	our overloaded renderer

protected:
	void					SetGlyphs();			//	setup child glyph render objects
	void					SetGlyph(TPtr<TRenderNodeGlyph>& pRenderGlyph,TPtr<TLAsset::TFont>& pFont,float3& GlyphPos,u16 Char);

protected:
	TFlags<TextFlags>		m_TextFlags;
	TRef					m_FontRef;
	TString					m_String;
	Bool					m_GlyphsChanged;					//	if TRUE we need to re-generate glyphs
};


