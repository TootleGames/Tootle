#include "TRenderNodeText.h"
#include <TootleAsset/TFont.h>
#include "TRenderGraph.h"




TLRender::TRenderNodeText::TRenderNodeText(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNode		( RenderNodeRef, TypeRef ),
	m_GlyphsChanged	( TRUE )
{
}


//---------------------------------------------------------
//	text render node init
//---------------------------------------------------------
void TLRender::TRenderNodeText::Initialise(TLMessaging::TMessage& Message)
{
	//	read init data
	//	import font - if one specified then load
	if ( Message.ImportData("FontRef", m_FontRef ) )
	{
		TLAsset::LoadAsset( m_FontRef );
	}

	//	import text - if we do, make sure we make a note the glyphs need building
	if ( Message.ImportDataString("Text", m_Text ) )
	{
		m_GlyphsChanged = TRUE;
	}

	//	do inherited init
	TRenderNode::Initialise( Message );
}

//--------------------------------------------------------------------
//	setup new string
//--------------------------------------------------------------------
void TLRender::TRenderNodeText::SetText(const TString& Text)
{
	//	no change
	if ( m_Text == Text )
		return;

	//	text changed, update glyphs
	m_Text = Text;
	m_GlyphsChanged = TRUE;
}


//--------------------------------------------------------------------
//	
//--------------------------------------------------------------------
void TLRender::TRenderNodeText::SetGlyph(TRenderNodeGlyph& RenderGlyph,TLAsset::TFont& Font,float3& GlyphPos,u16 Char)
{
	Bool Changed = FALSE;

	//	setup some flags on the render node
	//	don't do cull tests - parent's cull test is enough as it encapsulates all the children
	RenderGlyph.GetRenderFlags().Clear( TLRender::TRenderNode::RenderFlags::EnableCull );

	//	check for line feed
	Bool LineFeed = FALSE;
	if ( Char == '\n' )
		LineFeed = TRUE;

	//	set character
	Changed |= (RenderGlyph.m_Character != Char);

	//	update changed character
	if ( Changed )
	{
		RenderGlyph.m_Character = Char;
	
		//	update glyph mesh
		RenderGlyph.m_pGlyphMesh = LineFeed ? TLPtr::GetNullPtr<TLAsset::TMesh>() : Font.GetGlyph( Char );
	}

	//	get the mesh's lead in/out box
	const TLMaths::TBox* pLeadInOutBox = NULL;
	TLAsset::TMesh* pGlyphMesh = RenderGlyph.m_pGlyphMesh.GetObject();
	if ( pGlyphMesh )
	{
		TPtr<TBinaryTree>& pLeadInOutBoxData = pGlyphMesh->GetData("LeadBox");
		if ( pLeadInOutBoxData )
		{
			pLeadInOutBoxData->ResetReadPos();
			pLeadInOutBox = pLeadInOutBoxData->GetData().ReadNoCopy<TLMaths::TBox>();
			if ( pLeadInOutBox && !pLeadInOutBox->IsValid() )
				pLeadInOutBox = NULL;
		}
		
		//	no data, use our bounding box
		if ( !pLeadInOutBox )
			pLeadInOutBox = &pGlyphMesh->GetBoundsBox();
	}

	//	move back for lead in
	if ( pLeadInOutBox )
	{
		GlyphPos.x -= pLeadInOutBox->GetMin().x;
	}

	//	simple spacing atm - update node position
	if ( !RenderGlyph.GetTransform().HasTranslate() || RenderGlyph.GetTranslate() != GlyphPos )
	{
		RenderGlyph.SetTranslate( GlyphPos );
	}

	//	move along by the lead box
	if ( pLeadInOutBox )
	{
		GlyphPos.x += pLeadInOutBox->GetMax().x;
	}

	if ( LineFeed )
	{
		//	gr: need to find some way of getting an accurate line feed... currently use the bounds box height of 'A'
		TPtr<TLAsset::TMesh>& pLineFeedGlyph = Font.GetGlyph('A');
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
		RenderGlyph.OnBoundsChanged();
	}
}


//--------------------------------------------------------------------
//	setup child glyph render objects
//--------------------------------------------------------------------
void TLRender::TRenderNodeText::SetGlyphs()
{
	//	grab our font
	TPtr<TLAsset::TAsset>& pFontAsset = TLAsset::GetAsset( m_FontRef, TRUE );
	if ( !pFontAsset )
		return;
	TLAsset::TFont& Font = *pFontAsset.GetObject<TLAsset::TFont>();

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
		//	gr: just set not enabled?
		//	remove children we dont need any more
		if ( charindex >= m_Text.GetLengthWithoutTerminator() )
		{
			TLRender::g_pRendergraph->RemoveNode( pChild->GetNodeRef() );
			//RemoveChild( pRemoveChild );

			#ifndef TLGRAPH_OWN_CHILDREN
			pChild = pChild->GetNext();
			#endif
			continue;
		}

		//	setup existing child
		TRenderNodeGlyph& RenderGlyph = *pChild.GetObject<TRenderNodeGlyph>();

		//	update glyph
		SetGlyph( RenderGlyph, Font, GlyphPos, m_Text[charindex] );
		
		//	take parents render flags
		RenderGlyph.GetRenderFlags() = this->GetRenderFlags();

		#ifndef TLGRAPH_OWN_CHILDREN
		pChild = pChild->GetNext();
		#endif
		charindex++;
	}
	
	//	need to add more children
	while ( charindex<m_Text.GetLengthWithoutTerminator() )
	{
		//	cast to glyph
		TTempString GlyphName;
		GlyphName.Append( m_Text[charindex] );
		GlyphName.Append("glyph");
		TRef GlyphRef( GlyphName );
		GlyphRef = TLRender::g_pRendergraph->GetFreeNodeRef( GlyphRef );

		///////////////////////////////////////////////////////////////////////////////
		// This needs changing to use the following:
		//		TLMessaging::TMessage Message;
		//		Message.ExportData("Glyph", pRenderGlyph); // NOTE: Should be an ID rather than pointer
		//		Message.ExportData("Font", Font);
		//		Message.ExportData("Translate", GlyphPos);
		//		Message.ExportData("Char", m_Text[charindex]);
		//		TLRender::g_pRendergraph->CreateNode(GlyphRef, "Glyph", "Root");
		///////////////////////////////////////////////////////////////////////////////
		TPtr<TRenderNode> pRenderGlyphPtr = new TRenderNodeGlyph( GlyphRef, "Glyph" );
		TLRender::g_pRendergraph->AddNode( pRenderGlyphPtr, this->GetNodeRef() );

		TRenderNodeGlyph* pRenderGlyph = pRenderGlyphPtr.GetObject<TRenderNodeGlyph>();
		SetGlyph( *pRenderGlyph, Font, GlyphPos, m_Text[charindex] );

		///////////////////////////////////////////////////////////////////////////////
	
		charindex++;
	}


	m_GlyphsChanged = FALSE;
}


//--------------------------------------------------------------------
//	our overloaded renderer
//--------------------------------------------------------------------
Bool TLRender::TRenderNodeText::Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList)
{
	//	setup glyphs if they are out of date
	if ( m_GlyphsChanged )
		SetGlyphs();

	return TRUE;
}



TLRender::TRenderNodeGlyph::TRenderNodeGlyph(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNode		( RenderNodeRef, TypeRef ),
	m_Character		( 0 )
{
}
