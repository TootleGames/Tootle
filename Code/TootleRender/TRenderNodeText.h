/*------------------------------------------------------

	Text Render object

-------------------------------------------------------*/
#pragma once

#include "TRenderNode.h"


namespace TLRender
{
	class TRenderNodeText;
	class TRenderNodeVectorGlyph;
	class TRenderNodeVectorText;
	class TRenderNodeTextureText;
}



//----------------------------------------------------
//	child render object of TRenderNodeText - only difference is 
//	that it holds(per render) a direct pointer to the mesh for
//	the glyph, rather than looking it up from the ref
//----------------------------------------------------
class TLRender::TRenderNodeVectorGlyph : public TLRender::TRenderNode
{
	friend class TLRender::TRenderNodeVectorText;

protected:
	TRenderNodeVectorGlyph(TRefRef RenderNodeRef,TRefRef TypeRef);

	virtual TPtr<TLAsset::TMesh>&	GetMeshAsset() 	{	return m_pGlyphMesh;	}

public:
	TPtr<TLAsset::TMesh>	m_pGlyphMesh;
	u16						m_Character;		//	character in string
};



//--------------------------------------------------------
//	base class for text render object which renders a string with a given font
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
	virtual Bool			SetGlyphs()=0;			//	setup child glyph render objects

protected:
	TRef					m_FontRef;
	TString					m_Text;
	
private:
	Bool					m_GlyphsValid;			//	if TRUE we need to re-generate glyphs
};



//--------------------------------------------------------
//	base class for text render object which renders a string with a given font
//--------------------------------------------------------
class TLRender::TRenderNodeVectorText : public TLRender::TRenderNodeText
{
public:
	TRenderNodeVectorText(TRefRef RenderNodeRef,TRefRef TypeRef);

protected:
	virtual Bool			SetGlyphs();			//	setup child glyph render objects
	void					SetGlyph(TRenderNodeVectorGlyph& RenderGlyph,TLAsset::TFont& Font,float3& GlyphPos,u16 Char);
};



//--------------------------------------------------------
//	textured font text
//--------------------------------------------------------
class TLRender::TRenderNodeTextureText : public TLRender::TRenderNodeText
{
public:
	TRenderNodeTextureText(TRefRef RenderNodeRef,TRefRef TypeRef);

protected:
	virtual Bool					SetGlyphs();			//	setup geometry
	virtual TPtr<TLAsset::TMesh>&	GetMeshAsset()			{	return m_pMesh;	}
	virtual void					Initialise(TLMessaging::TMessage& Message);

protected:
	TPtr<TLAsset::TMesh>	m_pMesh;				//	mesh with our "sprites" in
};


