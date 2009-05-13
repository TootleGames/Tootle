#include "TRenderNodeText.h"
#include <TootleAsset/TFont.h>
#include <TootleAsset/TAtlas.h>
#include "TRenderGraph.h"
#include <TootleGame/TTextManager.h>


	

TLRender::TRenderNodeText::TRenderNodeText(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNode		( RenderNodeRef, TypeRef ),
	m_GlyphsValid	( FALSE ),
	m_AlignMode		( TLRenderText::HAlignLeft, TLRenderText::VAlignTop )
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

	// [07/05/09] DB - This shouldn't be used anymore.  Use a <TextRef> in place of the <Text> tag in any XML file
	if ( Message.ImportDataString("Text", m_Text ) )
	{
		TLDebug_Break("Setting text directly.  Should now use a text ref instead and lookup the text. If data string, use the STRING ref");
		OnStringChanged();
	}

	//	import text - if we do, make sure we make a note the glyphs need building
	//	gr: this IS allowed, for dynamic strings, eg, numbers, counters etc
	//		ref changed to "string" for clarity though. "String" being data, "text" being specific text.
	if ( Message.ImportDataString("String", m_Text ) )
	{
		OnStringChanged();
	}

	TRef TextRef;
	if ( Message.ImportData("TextRef", TextRef ) )
	{
		// Now get the string from the text manager
		if(TLText::g_pTextManager->GetText(TextRef, m_Text))
		{
			OnStringChanged();
		}
		else
		{
			#ifdef _DEBUG
			TTempString debugstr;
			TextRef.GetString(debugstr);
			TLDebug_Print(debugstr);
			TLDebug_Break("Failed to get text");
			#endif
		}
	}

	//	import scale/alignment modes
	Message.ImportData("HAlign", m_AlignMode.x );
	Message.ImportData("VAlign", m_AlignMode.y );
	Message.ImportData("SMode", m_ScaleMode );

	//	has text box shape
	TPtr<TBinaryTree>& pBoxData = Message.GetChild("Box");
	if ( pBoxData )
	{
		const TPtr<TLMaths::TShape>& pBoxShape = TLMaths::ImportShapeData( *pBoxData );
		if ( pBoxShape )
			SetTextBox( pBoxShape );
	}

	//	pull text box out of another node/mesh etc
	TRef TextBoxNode,TextBoxMesh,TextBoxDatum;
	Message.ImportData("BoxNode", TextBoxNode );
	Message.ImportData("BoxMesh", TextBoxMesh );

	if ( Message.ImportData("BoxDatum", TextBoxDatum ) )
	{
		const TLMaths::TShape* pBoxShape = NULL;

		if ( TextBoxNode.IsValid() )
		{
			//	get datum from node
			TLRender::TRenderNode* pRenderNode = TLRender::g_pRendergraph->FindNode( TextBoxNode );
			if ( pRenderNode )
			{
				//	todo: add ADDITIONAL world-transformed-datum support. For now best practise is to make this node a child of the one with the datum
				pBoxShape = pRenderNode->GetLocalDatum(TextBoxDatum);
				if ( !pBoxShape )
				{
					#ifdef _DEBUG
					TTempString Debug_String("Missing datum ");
					TextBoxDatum.GetString( Debug_String );
					Debug_String.Append(" on render node ");
					TextBoxNode.GetString( Debug_String );
					TLDebug_Break( Debug_String );
					#endif
				}
			}
			else
			{
				TLDebug_Break("Node to get text box datum from not found - add async support?");
			}
		}
		else if ( TextBoxMesh.IsValid() )
		{
			//	get datum directly from a mesh
			TLAsset::TMesh* pMesh = TLAsset::LoadAsset(TextBoxMesh,TRUE,"mesh").GetObject<TLAsset::TMesh>();
			if ( pMesh )
			{
				pBoxShape = pMesh->GetDatum(TextBoxDatum);
				if ( !pBoxShape )
				{
					#ifdef _DEBUG
					TTempString Debug_String("Missing datum ");
					TextBoxDatum.GetString( Debug_String );
					Debug_String.Append(" on mesh ");
					TextBoxMesh.GetString( Debug_String );
					TLDebug_Break( Debug_String );
					#endif
				}
			}
			else
			{
				TLDebug_Break("Mesh to get text box datum from not found - add async support?");
			}
		}
		else
		{
			TLDebug_Break("BoxDatum provided for text render node, but don't know where to get the datum from. (Missing render node/mesh ref's)");
		}

		//	set text box if we managed to get a shape
		if ( pBoxShape )
		{
			SetTextBox( pBoxShape );
		}
	}

	//	do inherited init
	TRenderNode::Initialise( Message );
}

//--------------------------------------------------------------------
//	setup new string
//--------------------------------------------------------------------
void TLRender::TRenderNodeText::SetString(const TString& Text)
{
	//	no change
	if ( m_Text == Text )
		return;

	//	text changed
	m_Text = Text;

	OnStringChanged();
}


//--------------------------------------------------------------------
//	update box - returns TRUE if it's changed
//--------------------------------------------------------------------
Bool TLRender::TRenderNodeText::SetTextBox(const TLMaths::TBox2D& Box)
{
	//	todo: check for change
	m_TextBox = Box;

	//	gr: bit inefficient... need a seperate flag to just re-build alignment
	m_GlyphsValid = FALSE;

	return TRUE;
}


//--------------------------------------------------------------------
//	update box - returns TRUE if it's changed
//--------------------------------------------------------------------
Bool TLRender::TRenderNodeText::SetTextBox(const TLMaths::TShape& Shape)
{
	//	can only deal with 2D box's atm
	if ( Shape.GetShapeType() != TLMaths_ShapeRef_TBox2D )
	{
		TLDebug_Break("Currently TRenderNodeText's only support 2D box shapes for TextBox's");
		return FALSE;
	}

	return SetTextBox( static_cast<const TLMaths::TShapeBox2D&>( Shape ).GetBox() );
}


//--------------------------------------------------------------------
//	our overloaded renderer
//--------------------------------------------------------------------
Bool TLRender::TRenderNodeText::Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList)
{
	//	setup glyphs if they are out of date
	if ( !m_GlyphsValid )
	{
		TLMaths::TBox2D TextBounds;
		if ( SetGlyphs(TextBounds) )
		{
			//	nothing to align if no text
			if ( GetString().GetLength() > 0 )
				RealignGlyphs( TextBounds );

			m_GlyphsValid = TRUE;
		}
	}

	//	dont render if glyphs out of date
	return m_GlyphsValid;
}


//--------------------------------------------------------------------
//	realign the glyphs according to our bounds box - the box provided is the box the glyphs take up
//--------------------------------------------------------------------
void TLRender::TRenderNodeText::RealignGlyphs(TLMaths::TBox2D& TextBounds)
{
	if ( !TextBounds.IsValid() )
	{
		TLDebug_Break("Invalid glyph bounds specified for glyph realignment");
		return;
	}

	//	build up a new transform.
	TLMaths::TTransform NewTransform;

	//	do scaling first
	if ( m_ScaleMode.IsValid() )
	{
		TTempString Debug_String("todo: handle text scale mode ");
		m_ScaleMode.GetString( Debug_String );
		TLDebug_Break( Debug_String );
		//	NewTransform.SetScale(xyz);
	}
	else
	{
		//	copy existing scale
		if ( GetTransform().HasScale() )
			NewTransform.SetScale( GetTransform().GetScale() );
	}

	//	if we don't have a text box we can still do alignment, just with our base point of 0,0
	TLMaths::TBox2D TempBox( float2(0.f,0.f), float2(0.f,0.f) );
	const TLMaths::TBox2D& AlignBox = m_TextBox.IsValid() ? m_TextBox : TempBox;

	//	scale the glyph bounds according to our scale
	//	note: ensure the original TextBounds isn't scaled according to our transform... it shouldn't be... it should be in local space
	if ( NewTransform.HasScale() )
	{
		//	gr: NewTransform should only have scale in it at the moment
		TextBounds.Transform( NewTransform );

		//	glyph bounds world space is now relative to our text box (which is local to our parent - regardless if we got the box from our parent or not)
	}

	if ( m_TextBox.IsValid() )
	{
		//	now get a vector which will align us with our text box as required
		//	defaults to top-left
		float2 Alignment( AlignBox.GetMin() );

		if ( m_AlignMode.x == TLRenderText::HAlignCenter )
			Alignment.x += (AlignBox.GetWidth() - TextBounds.GetWidth()) / 2.f;
		else if ( m_AlignMode.x == TLRenderText::HAlignRight )
			Alignment.x = (AlignBox.GetRight() - TextBounds.GetWidth());
		else if ( m_AlignMode.x != TLRenderText::HAlignLeft )
		{
			TTempString Debug_String("Unknown alignment mode ");
			m_AlignMode.x.GetString( Debug_String );
			TLDebug_Break( Debug_String );
		}

		if ( m_AlignMode.y == TLRenderText::VAlignMiddle )
			Alignment.y += (AlignBox.GetHeight() - TextBounds.GetHeight()) / 2.f;
		else if ( m_AlignMode.y == TLRenderText::VAlignBottom )
			Alignment.y = (AlignBox.GetBottom() - TextBounds.GetHeight());
		else if ( m_AlignMode.y != TLRenderText::VAlignTop )
		{
			TTempString Debug_String("Unknown alignment mode ");
			m_AlignMode.y.GetString( Debug_String );
			TLDebug_Break( Debug_String );
		}

		//	have a new alignment, apply it to the new transform...
		NewTransform.SetTranslate( Alignment.xyz( GetTransform().GetTranslate().z ) );
	}
	else
	{
		//	gr: currently the transform is reset to 0,0,0 if we have no text box... which is bad
		//	because usually if we dont have a text box we are setting a translate explicitly.
		//	we could use the existing translate for the TempBox but that's not working to work right
		//	for Center/Right align. Need to store an original offset I suppose... work this all
		//	into the next-revision where we have a different transform for each line
		NewTransform.SetTranslate( GetTransform().GetTranslate() );

		if ( m_AlignMode.x != TLRenderText::HAlignLeft || m_AlignMode.y != TLRenderText::VAlignTop )
		{
			TLDebug_Break("align modes without a text box aren't supported at the moment");
		}
	}

	//	and set our new transform
	SetTransform( NewTransform );
}








TLRender::TRenderNodeVectorText::TRenderNodeVectorText(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNodeText		( RenderNodeRef, TypeRef )
{
}


//--------------------------------------------------------------------
//	setup child glyph render objects
//--------------------------------------------------------------------
Bool TLRender::TRenderNodeVectorText::SetGlyphs(TLMaths::TBox2D& TextBounds)
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
		SetGlyph( RenderGlyph, Font, GlyphPos, m_Text[charindex], TextBounds );
		
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
		//		Message.ExportData(TRef_Static(T,r,a,n,s), GlyphPos);
		//		Message.ExportData("Char", m_Text[charindex]);
		//		TLRender::g_pRendergraph->CreateNode(GlyphRef, "Glyph", "Root");
		///////////////////////////////////////////////////////////////////////////////
		TPtr<TRenderNode> pRenderGlyphPtr = new TRenderNodeVectorGlyph( GlyphRef, "Glyph" );
		TLRender::g_pRendergraph->AddNode( pRenderGlyphPtr, this->GetNodeRef() );

		TRenderNodeVectorGlyph* pRenderGlyph = pRenderGlyphPtr.GetObject<TRenderNodeVectorGlyph>();
		SetGlyph( *pRenderGlyph, Font, GlyphPos, m_Text[charindex], TextBounds );

		///////////////////////////////////////////////////////////////////////////////
	
		charindex++;
	}


	return TRUE;
}



//--------------------------------------------------------------------
//	
//--------------------------------------------------------------------
void TLRender::TRenderNodeVectorText::SetGlyph(TRenderNodeVectorGlyph& RenderGlyph,TLAsset::TFont& Font,float3& GlyphPos,u16 Char,TLMaths::TBox2D& TextBounds)
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
			pLeadInOutBox = &pGlyphMesh->GetBounds<TLMaths::TShapeBox>().GetBox();
	}

	//	move back for lead in
	if ( pLeadInOutBox && pLeadInOutBox->IsValid() )
	{
		GlyphPos.x -= pLeadInOutBox->GetMin().x;
	}

	//	simple spacing atm - update node position
	if ( !RenderGlyph.GetTransform().HasTranslate() || RenderGlyph.GetTranslate() != GlyphPos )
	{
		RenderGlyph.SetTranslate( GlyphPos );
	}

	//	move along by the lead box
	if ( pLeadInOutBox && pLeadInOutBox->IsValid() )
	{
		GlyphPos.x += pLeadInOutBox->GetMax().x;
	}

	//	accumulate size of glyph in the text
	if ( pGlyphMesh )
	{
		TLMaths::TBox2D GlyphBox = pGlyphMesh->GetBounds<TLMaths::TShapeBox2D>().GetBox();
		if ( GlyphBox.IsValid() )
		{
			GlyphBox.Transform( GlyphPos );
			TextBounds.Accumulate( GlyphBox );
		}
	}

	//	reset bounds if object has changed
	if ( Changed )
	{
		RenderGlyph.OnBoundsChanged();
	}

	//	not glyph related: return the glyph position on line feeds
	if ( LineFeed )
	{
		//	gr: need to find some way of getting an accurate line feed... currently use the bounds box height of 'A'
		TPtr<TLAsset::TMesh>& pLineFeedGlyph = Font.GetGlyph('A');
		if ( pLineFeedGlyph )
		{
			const TLMaths::TBox& Bounds = pLineFeedGlyph->GetBoundsBox();
			if ( Bounds.IsValid() )
			{
				float LineHeight = Bounds.GetMin().y + Bounds.GetMax().y;
				GlyphPos.y += LineHeight;
			}
		}
		GlyphPos.x = 0.f;
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
Bool TLRender::TRenderNodeTextureText::SetGlyphs(TLMaths::TBox2D& TextBounds)
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
	//	gr: just stored as a transform to ease some other things
	TLMaths::TTransform GlyphTransform;
	GlyphTransform.SetTranslate( float3( 0,0,0 ) );

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
				GlyphTransform.GetTranslate().y += LineHeight;
			}
			GlyphTransform.GetTranslate().x = 0.f;
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

		//	calculate the positioned box for this glyph
		TLMaths::TBox2D ThisGlyphBox = pGlyph->m_GlyphBox;
		ThisGlyphBox.Transform( GlyphTransform );

		//	accumulate this into the overall box
		TextBounds.Accumulate( ThisGlyphBox );

		TFixedArray<float3,4> VertPositions;
		VertPositions.Add( float3( ThisGlyphBox.GetLeft(), ThisGlyphBox.GetTop(), 0.f ) );
		VertPositions.Add( float3( ThisGlyphBox.GetRight(), ThisGlyphBox.GetTop(), 0.f ) );
		VertPositions.Add( float3( ThisGlyphBox.GetRight(), ThisGlyphBox.GetBottom(), 0.f ) );
		VertPositions.Add( float3( ThisGlyphBox.GetLeft(), ThisGlyphBox.GetBottom(), 0.f ) );
		
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
		GlyphTransform.GetTranslate().x += pGlyph->m_SpacingBox.GetWidth();

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

