/*------------------------------------------------------

	Text Render object

-------------------------------------------------------*/
#pragma once

#include "TRenderNode.h"
#include <TootleAsset/TText.h>


namespace TLRender
{
	class TRenderNodeText;
	class TRenderNodeVectorGlyph;
	class TRenderNodeVectorText;
	class TRenderNodeTextureText;
}

namespace TLRenderText
{
	static const TRef		HAlignLeft		=	TRef_Static4(L,e,f,t);		//	default
	static const TRef		HAlignCenter	=	TRef_Static(C,e,n,t,e);
	static const TRef		HAlignRight		=	TRef_Static(R,i,g,h,t);
	
	static const TRef		VAlignTop		=	TRef_Static3(T,o,p);		//	default
	static const TRef		VAlignMiddle	=	TRef_Static(M,i,d,d,l);
	static const TRef		VAlignBottom	=	TRef_Static(B,o,t,t,o);

	static const TRef		ScaleFit		=	TRef_Static3(F,i,t);		//	scale DOWN so the text fits inside the box if it goes past the extents
	static const TRef		ScaleWidth		=	TRef_Static(W,i,d,t,h);		//	scale so it fits to the width of the box
	static const TRef		ScaleHeight		=	TRef_Static(H,e,i,g,h);		//	scale so it fits to the height of the box
};


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

	virtual TPtr<TLAsset::TMesh>&	GetMeshAsset(Bool BlockLoad=FALSE) 	{	return m_pGlyphMesh;	}
	virtual Bool					Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList)	{	return TRUE;	}

public:
	TPtr<TLAsset::TMesh>	m_pGlyphMesh;
	u16						m_Character;		//	character in string
};



//--------------------------------------------------------
//	base class for text render object which renders a string with a given font
//--------------------------------------------------------
class TLRender::TRenderNodeText : public TLRender::TRenderNode
{
protected:
	/*
	class TGlyphLine
	{
	public:
		TLMaths::TBox2D		m_Bounds;				//	bounds of this LINE of glyphs
		TArray<TRef>		m_GlyphNodes;			//	node refs of the glyph children
		//TLMaths::TTransform	m_LineTransform;	//	transform for the line.
	};
	*/

public:
	TRenderNodeText(TRefRef RenderNodeRef,TRefRef TypeRef);

	virtual void			Initialise(TLMessaging::TMessage& Message);	//	generic render node init
	virtual void			OnTransformChanged(u8 TransformChangedBits=TLMaths_TransformBitAll);	//	when the transform changes, merge with the alignment transform

	TRefRef					GetFontRef() const							{	return m_FontRef;	}
	void					SetFontRef(TRefRef FontRef)					{	m_FontRef = FontRef;	}
	const TString&			GetString() const							{	return m_Text;	}
	DEPRECATED const TString&	GetText() const							{	return m_Text;	}
	void					SetString(const TString& String);			//	setup new string
	DEPRECATED void			SetText(const TString& String)				{	SetString( String );	}
	void					SetText(TRefRef TextRef,TLAsset::TText::TTextReplaceTable* pReplaceTable=NULL);
	FORCEINLINE void		SetText(TRefRef TextRef,TLAsset::TText::TTextReplaceTable& ReplaceTable)			{	SetText( TextRef, &ReplaceTable );	}
	Bool					SetTextBox(const TLMaths::TBox2D& Box);		//	update box - returns TRUE if it's changed
	Bool					SetTextBox(const TLMaths::TShape& Shape);	//	update box - returns TRUE if it's changed
	Bool					SetTextBox(const TLMaths::TShape* pShape)	{	return pShape ? SetTextBox( *pShape ) : SetTextBoxInvalid();	}
	Bool					SetTextBoxInvalid()							{	Bool WasValid = m_TextBox.IsValid();	m_TextBox.SetInvalid();	return (WasValid!=FALSE);	}

	virtual Bool			Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList);	//	our overloaded renderer
	
protected:
	virtual Bool			SetGlyphs(TLMaths::TBox2D& TextBounds)=0;		//	setup child glyph render objects
	void					RealignGlyphs(TLMaths::TBox2D& TextBounds);		//	realign the glyphs according to our bounds box - the box provided is the box the glyphs take up

	FORCEINLINE float		GetLineHeight() const						{	return m_LineHeight;	}

	void					OnStringChanged()							{	m_GlyphsValid = FALSE;	}
	void					OnStringFormatChanged()						{	m_GlyphsValid = FALSE;	}
	void					OnLayoutChanged()							{	m_GlyphsValid = FALSE;	}
	
	virtual void			ProcessMessage(TLMessaging::TMessage& Message);

protected:
	TRef					m_FontRef;
	TString					m_Text;

	TLMaths::TBox2D			m_TextBox;				//	currently only a box is supported
	Type2<TRef>				m_AlignMode;			//	horizontal and vertical alignment mode
	TRef					m_ScaleMode;			//	scale mode, if invalid no scaling applied. Note: if you scale the text, and then set a scale mode you will lose your original scale info (or at least on whatever axis is affected by the mode)
	float					m_LineHeight;			//	scalar to the line height for extra line spacing. 1.0 is default

	TLMaths::TTransform		m_BaseTransform;		//	this is the "offset" transform. whenever the local transform is set it's copied here, then the alignment transform is put on top
	TLMaths::TTransform		m_AlignmentTransform;	//	transform for the alignment

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
	virtual Bool			SetGlyphs(TLMaths::TBox2D& TextBounds);			//	setup child glyph render objects
	void					SetGlyph(TRenderNodeVectorGlyph& RenderGlyph,TLAsset::TFont& Font,float3& GlyphPos,u16 Char,TLMaths::TBox2D& GlyphBounds);
};



//--------------------------------------------------------
//	textured font text
//--------------------------------------------------------
class TLRender::TRenderNodeTextureText : public TLRender::TRenderNodeText
{
public:
	TRenderNodeTextureText(TRefRef RenderNodeRef,TRefRef TypeRef);

protected:
	virtual Bool					SetGlyphs(TLMaths::TBox2D& TextBounds);	//	setup geometry
	virtual TPtr<TLAsset::TMesh>&	GetMeshAsset(Bool BlockLoad=FALSE)			{	return m_pMesh;	}
	virtual void					Initialise(TLMessaging::TMessage& Message);

protected:
	TPtr<TLAsset::TMesh>			m_pMesh;				//	mesh with our "sprites" in
};


