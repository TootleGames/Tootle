#include "TRenderNodeText.h"
#include <TootleAsset/TFont.h>
#include "TRenderGraph.h"




TLRender::TRenderNodeText::TRenderNodeText(TRefRef RenderNodeRef) :
	TRenderNode	( RenderNodeRef ),
	m_GlyphsChanged	( TRUE )
{
}


//--------------------------------------------------------------------
//	setup new string
//--------------------------------------------------------------------
void TLRender::TRenderNodeText::SetString(const TString& String)
{
	//	no change
	if ( m_String == String )
		return;

	m_String = String;
	m_GlyphsChanged = TRUE;
}


//--------------------------------------------------------------------
//	
//--------------------------------------------------------------------
void TLRender::TRenderNodeText::SetGlyph(TPtr<TRenderNodeGlyph>& pRenderGlyph,TPtr<TLAsset::TFont>& pFont,float3& GlyphPos,u16 Char)
{
	if ( !pRenderGlyph )
	{
		TLDebug_Break("RenderGlyph expected");
		return;
	}

	Bool Changed = FALSE;

	//	check for line feed
	Bool LineFeed = FALSE;
	if ( Char == '\n' )
		LineFeed = TRUE;

	//	set character
	Changed |= (pRenderGlyph->m_Character != Char);
	pRenderGlyph->m_Character = Char;
	
	//	get the glyph for this character
	TPtr<TLAsset::TMesh> pGlyph = LineFeed ? TPtr<TLAsset::TMesh>(NULL) : pFont->GetGlyph( Char );

	//	get the mesh's lead in/out box
	const TLMaths::TBox* pLeadInOutBox = NULL;
	if( pGlyph )
	{
		TPtr<TBinaryTree> pLeadInOutBoxData = pGlyph->GetData("LeadBox");
		if ( pLeadInOutBoxData )
		{
			pLeadInOutBoxData->ResetReadPos();
			pLeadInOutBox = pLeadInOutBoxData->GetData().ReadNoCopy<TLMaths::TBox>();
			if ( pLeadInOutBox && !pLeadInOutBox->IsValid() )
				pLeadInOutBox = NULL;
		}
		
		//	no data, use our bounding box
		if ( !pLeadInOutBox )
			pLeadInOutBox = &pGlyph->GetBoundsBox();
	}

	//	move back for lead in
	if ( pLeadInOutBox )
	{
		GlyphPos.x -= pLeadInOutBox->GetMin().x;
	}

	//	simple spacing atm
	if ( pRenderGlyph->GetTranslate() != GlyphPos )
	{
		pRenderGlyph->SetTranslate( GlyphPos );
	}

	//	move along by the 
	if ( pLeadInOutBox )
	{
		GlyphPos.x += pLeadInOutBox->GetMax().x;
	}

	if ( LineFeed )
	{
		//	gr: need to find some way of getting an accurate line feed... currently use the bounds box height of 'A'
		TPtr<TLAsset::TMesh> pLineFeedGlyph = pFont->GetGlyph('A');
		if ( pLineFeedGlyph )
		{
			TLMaths::TBox& Bounds = pLineFeedGlyph->CalcBoundsBox();
			if ( Bounds.IsValid() )
			{
				float LineHeight = Bounds.GetMin().y + Bounds.GetMax().y;
				GlyphPos.y += LineHeight;
			}
		}
		GlyphPos.x = 0.f;
	}

	//	reset bounds if object has changed
	if ( Changed )
	{
		pRenderGlyph->OnBoundsChanged();
	}
}


//--------------------------------------------------------------------
//	setup child glyph render objects
//--------------------------------------------------------------------
void TLRender::TRenderNodeText::SetGlyphs()
{
	//	grab our font
	TPtr<TLAsset::TFont> pFont = TLAsset::GetAsset( m_FontRef, TRUE );
	if ( !pFont )
		return;

	//	relative to parent so start at 0,0,0
	float3 GlyphPos(0,0,0);

	//	setup RenderNodes
	u32 charindex=0;

#ifdef TLGRAPH_OWN_CHILDREN
	TPtrArray<TLRender::TRenderNode>& NodeChildren = GetChildren();
	for ( u32 c=0;	c<NodeChildren.GetSize();	c++ )
	{
		TPtr<TLRender::TRenderNode>& pChild = NodeChildren[c];
#else
	TPtr<TRenderNode> pChild = GetChildFirst();
	while ( pChild )
	{
#endif
		//	remove children we dont need any more
		if ( charindex >= m_String.GetLengthWithoutTerminator() )
		{
			TPtr<TRenderNode> pRemoveChild = pChild;

			TLRender::g_pRendergraph->RemoveNode( pRemoveChild );
			//RemoveChild( pRemoveChild );

			#ifndef TLGRAPH_OWN_CHILDREN
			pChild = pChild->GetNext();
			#endif
			continue;
		}

		//	setup existing child
		TPtr<TRenderNodeGlyph> pRenderGlyph = pChild;

		SetGlyph( pRenderGlyph, pFont, GlyphPos, m_String[charindex] );
	
		#ifndef TLGRAPH_OWN_CHILDREN
		pChild = pChild->GetNext();
		#endif
		charindex++;
	}
	
	//	need to add more children
	while ( charindex<m_String.GetLengthWithoutTerminator() )
	{
		//	cast to glyph
		TTempString GlyphName;
		GlyphName.Appendf("g- %c", m_String[charindex] );

		TPtr<TRenderNodeGlyph> pRenderGlyph = new TRenderNodeGlyph( GlyphName );
		TLRender::g_pRendergraph->AddNode( pRenderGlyph, this->GetNodeRef() );
		SetGlyph( pRenderGlyph, pFont, GlyphPos, m_String[charindex] );
	
		charindex++;
	}


	m_GlyphsChanged = FALSE;
}


//--------------------------------------------------------------------
//	our overloaded renderer
//--------------------------------------------------------------------
Bool TLRender::TRenderNodeText::Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList)
{
	//	grab our font
	TPtr<TLAsset::TFont> pFont = TLAsset::GetAsset( m_FontRef, TRUE );
	if ( !pFont )
		return FALSE;
	
	if ( m_GlyphsChanged )
		SetGlyphs();

	//	make sure each child glyph has correct mesh assigned
#ifdef TLGRAPH_OWN_CHILDREN
	TPtrArray<TLRender::TRenderNode>& NodeChildren = GetChildren();
	for ( u32 c=0;	c<NodeChildren.GetSize();	c++ )
	{
		TPtr<TLRender::TRenderNode>& pChild = NodeChildren[c];
#else
	TPtr<TRenderNode> pChild = GetChildFirst();
	while ( pChild )
	{
#endif
		TPtr<TRenderNodeGlyph> pRenderGlyph = pChild;

		//	get the glyph for this character
		TPtr<TLAsset::TMesh> pGlyph = pFont->GetGlyph( pRenderGlyph->m_Character );

		//	set the mesh ptr
		pRenderGlyph->m_pGlyphMesh = pGlyph;

		//	merge render flags
		pRenderGlyph->GetRenderFlags() = this->GetRenderFlags();
		
		#ifndef TLGRAPH_OWN_CHILDREN
		pChild = pChild->GetNext();
		#endif
	}

	//	do normal render... just renders children
	return TRUE;
}



TLRender::TRenderNodeGlyph::TRenderNodeGlyph(TRefRef RenderNodeRef) :
	TRenderNode	( RenderNodeRef ),
	m_Character		( 0 )
{
}
