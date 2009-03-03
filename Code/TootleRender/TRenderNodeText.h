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
	TRenderNodeGlyph(TRefRef RenderNodeRef,TRefRef TypeRef);

	virtual TPtr<TLAsset::TMesh>&	GetMeshAsset() 	{	return m_pGlyphMesh;	}

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
	TRenderNodeText(TRefRef RenderNodeRef,TRefRef TypeRef);

	virtual void			Initialise(TLMessaging::TMessage& Message);	//	generic render node init

	TRefRef					GetFontRef() const					{	return m_FontRef;	}
	void					SetFontRef(TRefRef FontRef)			{	m_FontRef = FontRef;	}
	const TString&			GetText() const						{	return m_Text;	}
	void					SetText(const TString& Text);		//	setup new string

	virtual Bool			Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList);	//	our overloaded renderer

protected:
	void					SetGlyphs();			//	setup child glyph render objects
	void					SetGlyph(TRenderNodeGlyph& RenderGlyph,TLAsset::TFont& Font,float3& GlyphPos,u16 Char);

protected:
	TRef					m_FontRef;
	TString					m_Text;
	Bool					m_GlyphsChanged;		//	if TRUE we need to re-generate glyphs
};


