#include "TRenderNodeText.h"
#include <TootleAsset/TFont.h>
#include <TootleAsset/TAtlas.h>
#include "TRenderGraph.h"




TLRender::TRenderNodeText::TRenderNodeText(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNode		( RenderNodeRef, TypeRef ),
	m_GlyphsValid	( FALSE )
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
		m_GlyphsValid = FALSE;
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
	m_GlyphsValid = FALSE;
}




TLRender::TRenderNodeVectorText::TRenderNodeVectorText(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNodeText		( RenderNodeRef, TypeRef )
{
}


//--------------------------------------------------------------------
//	setup child glyph render objects
//--------------------------------------------------------------------
Bool TLRender::TRenderNodeVectorText::SetGlyphs()
{
	//	grab our font
	TPtr<TLAsset::TAsset>& pFontAsset = TLAsset::GetAsset( m_FontRef, TRUE );
	if ( !pFontAsset )
		return FALSE;
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
		TRenderNodeVectorGlyph& RenderGlyph = *pChild.GetObject<TRenderNodeVectorGlyph>();

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
		TPtr<TRenderNode> pRenderGlyphPtr = new TRenderNodeVectorGlyph( GlyphRef, "Glyph" );
		TLRender::g_pRendergraph->AddNode( pRenderGlyphPtr, this->GetNodeRef() );

		TRenderNodeVectorGlyph* pRenderGlyph = pRenderGlyphPtr.GetObject<TRenderNodeVectorGlyph>();
		SetGlyph( *pRenderGlyph, Font, GlyphPos, m_Text[charindex] );

		///////////////////////////////////////////////////////////////////////////////
	
		charindex++;
	}


	return TRUE;
}


//--------------------------------------------------------------------
//	our overloaded renderer
//--------------------------------------------------------------------
Bool TLRender::TRenderNodeText::Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList)
{
	//	setup glyphs if they are out of date
	if ( !m_GlyphsValid )
	{
		if ( SetGlyphs() )
			m_GlyphsValid = TRUE;
	}

	//	dont render if glyphs out of date
	return m_GlyphsValid;
}







//--------------------------------------------------------------------
//	
//--------------------------------------------------------------------
void TLRender::TRenderNodeVectorText::SetGlyph(TRenderNodeVectorGlyph& RenderGlyph,TLAsset::TFont& Font,float3& GlyphPos,u16 Char)
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






TLRender::TRenderNodeVectorGlyph::TRenderNodeVectorGlyph(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNode		( RenderNodeRef, TypeRef ),
	m_Character		( 0 )
{
}







TLRender::TRenderNodeTextureText::TRenderNodeTextureText(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNodeText	( RenderNodeRef, TypeRef )
{
}


//--------------------------------------------------------
//	
//--------------------------------------------------------
void TLRender::TRenderNodeTextureText::Initialise(TLMessaging::TMessage& Message)
{
	TLRender::TRenderNodeText::Initialise( Message );
	
	//	block load the atlas so we can check the asset is the right type and assign the texture for the font
	TPtr<TLAsset::TAsset>& pAtlasAsset = TLAsset::LoadAsset( GetFontRef(), TRUE );
	if ( pAtlasAsset )
	{
		if ( pAtlasAsset->GetAssetType() == "Atlas" )
		{
			TLAsset::TAtlas& Atlas = *(pAtlasAsset.GetObject<TLAsset::TAtlas>());
			SetTextureRef( Atlas.GetTextureRef() );
			TLAsset::LoadAsset( Atlas.GetTextureRef() );
		}
		else
		{
			TLDebug_Break("TextureText render node assigned font asset which is not an atlas");
		}
	}
}


//--------------------------------------------------------
//	setup geometry
//--------------------------------------------------------
Bool TLRender::TRenderNodeTextureText::SetGlyphs()
{
	//	 create mesh
	if ( !m_pMesh )
	{
		m_pMesh = new TLAsset::TMesh("TxText");
		m_pMesh->SetLoadingState( TLAsset::LoadingState_Loaded );
	}

	//	grab atlas asset
	TPtr<TLAsset::TAsset>& pAsset = TLAsset::GetAsset( GetFontRef(), TRUE );

	//	wrong type/not loaded
	if ( ! (pAsset && pAsset->GetAssetType() == "Atlas") )
		return FALSE;

	TLAsset::TAtlas& Atlas = *(pAsset.GetObject<TLAsset::TAtlas>());

	//	current triangle index - maybe different from number of chars in the string because of missing glyphs
	u32 TriangleIndex = 0;
	TArray<TLAsset::TMesh::Triangle>& Triangles = m_pMesh->GetTriangles();

	//	relative to parent so start at 0,0,0
	float3 GlyphPos(0,0,0);

	//	setup geometry - currently rebuild the entire string
	for ( u32 i=0;	i<m_Text.GetLengthWithoutTerminator();	i++ )
	{
	//	check for line feed
		if ( m_Text[i] == '\n' )
		{
			const TLAsset::TAtlasGlyph* pGlyph = Atlas.GetGlyph('A');
			if ( pGlyph )
			{
				float LineHeight = pGlyph->m_SpacingBox.GetHeight();
				GlyphPos.y += LineHeight;
			}
			GlyphPos.x = 0.f;
			continue;
		}

		//	get glyph for this char
		const TLAsset::TAtlasGlyph* pGlyph = Atlas.GetGlyph( m_Text[i] );

	



		//	no character in font for this character
		if ( !pGlyph )
		{
			TLDebug_Warning( TString("Font atlas has no glyph for character: 0x%02x (%c)", m_Text[i], m_Text[i] ) );
			continue;
		}

		TFixedArray<float3,4> VertPositions;
		VertPositions.Add( GlyphPos + float3( pGlyph->m_GlyphBox.GetLeft(), pGlyph->m_GlyphBox.GetTop(), 0.f ) );
		VertPositions.Add( GlyphPos + float3( pGlyph->m_GlyphBox.GetRight(), pGlyph->m_GlyphBox.GetTop(), 0.f ) );
		VertPositions.Add( GlyphPos + float3( pGlyph->m_GlyphBox.GetRight(), pGlyph->m_GlyphBox.GetBottom(), 0.f ) );
		VertPositions.Add( GlyphPos + float3( pGlyph->m_GlyphBox.GetLeft(), pGlyph->m_GlyphBox.GetBottom(), 0.f ) );
		
		TFixedArray<const float2*,4> VertUVs;
		VertUVs.Add( &pGlyph->GetUV_TopLeft() );
		VertUVs.Add( &pGlyph->GetUV_TopRight() );
		VertUVs.Add( &pGlyph->GetUV_BottomRight() );
		VertUVs.Add( &pGlyph->GetUV_BottomLeft() );

		//	add new vertex/triangle
		if ( TriangleIndex >= Triangles.GetSize() )
		{
			TFixedArray<u16,4> Vertexes;
			Vertexes.Add( m_pMesh->AddVertex( VertPositions[0], NULL, VertUVs[0] ) );
			Vertexes.Add( m_pMesh->AddVertex( VertPositions[1], NULL, VertUVs[1] ) );
			Vertexes.Add( m_pMesh->AddVertex( VertPositions[2], NULL, VertUVs[2] ) );
			Vertexes.Add( m_pMesh->AddVertex( VertPositions[3], NULL, VertUVs[3] ) );
			Triangles.Add( TLAsset::TMesh::Triangle( Vertexes[0], Vertexes[1], Vertexes[2] ) );
			Triangles.Add( TLAsset::TMesh::Triangle( Vertexes[2], Vertexes[3], Vertexes[0] ) );
		}
		else
		{
			//	modify existing verts
			TLAsset::TMesh::Triangle& TriangleA = Triangles[TriangleIndex];		//	0 1 2
			m_pMesh->GetVertex( TriangleA.x ) = VertPositions[0];
			m_pMesh->GetVertex( TriangleA.y ) = VertPositions[1];
			m_pMesh->GetVertex( TriangleA.z ) = VertPositions[2];
			m_pMesh->GetVertexUV( TriangleA.x ) = *VertUVs[0];
			m_pMesh->GetVertexUV( TriangleA.y ) = *VertUVs[1];
			m_pMesh->GetVertexUV( TriangleA.z ) = *VertUVs[2];
			TLAsset::TMesh::Triangle& TriangleB = Triangles[TriangleIndex+1];	//	2 3 0
			m_pMesh->GetVertex( TriangleB.y ) = VertPositions[3];
			m_pMesh->GetVertexUV( TriangleB.y ) = *VertUVs[3];
		}

		//	move position along
		GlyphPos.x += pGlyph->m_SpacingBox.GetWidth();

		//	move to next triangles
		TriangleIndex += 2;
	}

	//	cull unwanted triangles & vertexes
	while ( Triangles.GetLastIndex() >= (s32)TriangleIndex )
	{
		m_pMesh->RemoveTriangle( Triangles.GetLastIndex(), TRUE, FALSE );
	}

	//	geometry changed - invalidate bounds
	m_pMesh->SetBoundsInvalid();

	return TRUE;
}

