#include <TootleCore/TLMaths.h>	//	gr: I don't know why but this needs to be included before "TFileFreetype.h" or "rand" isn't found
#include "TFileFreetype.h"
#include "TFileAsset.h"
#include <TootleAsset/TFont.h>
#include <TootleMaths/TTessellate.h>



namespace TLFileFreetype
{
	Bool		IsCharRequired(const char& Char);
	Bool		IsCharRequired(const u16& WChar);
	Bool		IsCharRequired(const u32& LongChar);
};



//--------------------------------------------------------------
//	gr: just to speed things up and make the assets a bit smaller I've made an alphabet of characters we convert
//		all other characters are ignored
//--------------------------------------------------------------
Bool TLFileFreetype::IsCharRequired(const char& Char)
{
	TString SupportedAlphabet(	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
									"abcdefghijklmnopqrstuvwxyz"
									"1234567890"
									" !£$%%^&*()_+-=[]{}#~'@:;/?\\|<>,.\"" );
	
	if ( SupportedAlphabet.GetCharExists( Char ) )
		return TRUE;

	return FALSE;
}


Bool TLFileFreetype::IsCharRequired(const u16& WChar)
{
	//	gr: currently not supporting any wide chars
	if ( WChar > 0xff )
		return FALSE;

	char Char = (char)(WChar&0xff);
	return IsCharRequired( Char );
}


Bool TLFileFreetype::IsCharRequired(const u32& LongChar)
{
	//	gr: currently not supporting any wide chars
	if ( LongChar > 0xff )
		return FALSE;

	char Char = (char)(LongChar&0xff);
	return IsCharRequired( Char );
}



TLFileSys::TFileFreetype::TFileFreetype(TRefRef FileRef,TRefRef FileTypeRef) :
	TFile					( FileRef, FileTypeRef ),
	m_pLibrary				( NULL ),
	m_pFace					( NULL ),
	m_CharHeight			( 0 ),
	m_NextGlyphIndex		( 0 ),
	m_NextGlyphCharacter	( 0 )
{
}


TLFileSys::TFileFreetype::~TFileFreetype()
{
	ShutdownExport(TRUE);
}


//--------------------------------------------------------------
//	turn this file into a font asset then turn that to a asset file
//--------------------------------------------------------------	
SyncBool TLFileSys::TFileFreetype::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported)
{
	Supported = TRUE;

	if ( !InitExport(pAsset) )
	{
		ShutdownExport(TRUE);
		return SyncFalse;
	}

	SyncBool ExportResult = UpdateExport(pAsset);
	if ( ExportResult == SyncWait )
		return SyncWait;

	//	failed, shutdown and return
	if ( ExportResult == SyncFalse )
	{
		ShutdownExport(TRUE);
		return SyncFalse;
	}

	//	print out some debug info
	TString FontRefString;
	pAsset->GetAssetRef().GetString( FontRefString );
	TLDebug_Print( TString("%d characters in the %s font", pAsset.GetObject<TLAsset::TFont>()->GetGlyphCount(), FontRefString.GetData() ) );

	//	cleanup, but dont delete font
	ShutdownExport( FALSE );

	return SyncTrue;

}	


//--------------------------------------------------------------
//	init export
//--------------------------------------------------------------
Bool TLFileSys::TFileFreetype::InitExport(TPtr<TLAsset::TAsset>& pAsset)
{
	if ( !pAsset )
	{
		//	create a new font asset
		pAsset = new TLAsset::TFont( this->GetFileRef() );
		if ( !pAsset )
			return FALSE;
	}

	
	//	graham's implementation
	if ( !m_pLibrary )
	{
		u32 Error = FT_Init_FreeType( &m_pLibrary );
		if ( Error )
			return FALSE;
	}

	if ( !m_pFace )
	{
		u32 Error = FT_New_Memory_Face( m_pLibrary, this->GetData().GetData(), this->GetSize(), 0, &m_pFace );
		if ( Error )
			return FALSE;
	}

	if ( m_CharHeight == 0 )
	{
		m_CharHeight = 100;	//	will turn into 16 metres high? - for our engine we'll probably want to scale back to 1/N
		u32 Error = FT_Set_Char_Size( m_pFace, // handle to face object 
										0, // char_width in 1/64th of points 
										m_CharHeight*64, // char_height in 1/64th of points 
										300, // horizontal device resolution   
										300 ); // vertical device resolution 
		if ( Error )
			return FALSE;

		//	set to process from first glyph
		m_NextGlyphIndex = 0;
		m_NextGlyphCharacter = FT_Get_First_Char( m_pFace, &m_NextGlyphIndex );                   
	}

	return TRUE;
}

	
//--------------------------------------------------------------
//	amount to scale points down to to be in relation to our engine units (about 1.f is a standard character height)
//--------------------------------------------------------------
float TLFileSys::TFileFreetype::GetPointScale()
{
	//	this is what i expect we should scale down by
	float ScaleDown = 1.f / ((float)m_CharHeight * 64.f);

	//	but it's still a bit big. 1 character height should be roughly 1.f
	ScaleDown *= 0.2f;

	return ScaleDown;
}



//--------------------------------------------------------------
//	continue export
//--------------------------------------------------------------
SyncBool TLFileSys::TFileFreetype::UpdateExport(TPtr<TLAsset::TAsset>& pAsset)
{
	TPtr<TLAsset::TFont> pFont = pAsset;
	if ( !pFont )
	{
		TLDebug_Break("Font expected");
		return SyncFalse;
	}

	float ScaleDown = GetPointScale();
	s32 CharactersToExport = 3;

	//	loop through all the characters in the font
	while ( m_NextGlyphIndex != 0 && --CharactersToExport > 0 )                                            
	{
		u16 Character = m_NextGlyphCharacter & 0xffff;
		u32 GlyphIndex = m_NextGlyphIndex;
		
		//	get next glyph now so we can easily use continue; in the loop
		m_NextGlyphCharacter = FT_Get_Next_Char( m_pFace, m_NextGlyphCharacter, &m_NextGlyphIndex );

		//	if this is a character we dont support... skip
		if ( !TLFileFreetype::IsCharRequired( m_NextGlyphCharacter ) )
			continue;

		//	load a glyph into the face's current-glyph
		//	always load vectors, never using bitmap fonts
		u32 LoadFlags = FT_LOAD_NO_BITMAP;
		u32 Error = FT_Load_Glyph( m_pFace, GlyphIndex, LoadFlags );
		if ( Error )
			continue;

		//	get current-glyph
		TLFreetype::FT_GlyphSlot& pGlyph = m_pFace->glyph;
		if ( !pGlyph )
			continue;

		//	add this glyph mesh to the font
		TPtr<TLAsset::TMesh> pGlyphMesh = pFont->AddGlyph( Character );
		if ( !pGlyphMesh )
			continue;

		//	get a leadin/out bounding box from the glyph info
		float3 LeadIn( 0.f, 0.f, 0.f );
		float3 LeadOut( (float)pGlyph->advance.x * ScaleDown, (float)pGlyph->advance.y * ScaleDown, 0.f );
		TLMaths::TBox LeadInOutBoundsBox( LeadIn, LeadOut );

		//	write into the mesh as arbirtry data
		TPtr<TBinaryTree> pLeadInOutBoxData = pGlyphMesh->GetData("LeadBox",TRUE);
		if ( pLeadInOutBoxData )
			pLeadInOutBoxData->Write( LeadInOutBoundsBox );

		//	vectorise glyph
//		if ( !pVectoriser->MakeMesh( pGlyphMesh, TLGlutTessellator::OutsetType_Front, 1.0f, 0.f ) )
		if ( !VectoriseGlyph( pGlyphMesh, pGlyph->outline ) )
		{
			//	if the glyph is empty, we add it anyway  for spacing (eg. space)
			TLDebug_Print( TString("Added empty character %c(0x%04x) to font", (char)Character, Character ) );

			//	the lead in/out bounding box is set, but we need to set an empty (but valid) bounds box
			//pGlyphMesh->GetBoundsBox().Set( float3(0,0,0), float3(0,0,0) );
			//pGlyphMesh->GetBoundsSphere().Set( float3(0,0,0), 0.f );
			continue;
		}

		//	 some debugging
		//TLDebug_Print( TString("Added character %c(0x%04x) to font", (char)Character, Character ) );
	}

	//	still got glyphs to import
	if ( m_NextGlyphIndex != 0 )
		return SyncWait;

	return SyncTrue;
}

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
void TLFileSys::TFileFreetype::ShutdownExport(Bool DeleteFont)
{
	if ( DeleteFont )
	{
//		m_pFont = NULL;
	}

	m_CharHeight = 0;
	m_NextGlyphIndex = 0;

	if ( m_pFace )
	{
		FT_Done_Face( m_pFace );
		m_pFace = NULL;
	}
	
	if ( m_pLibrary )
	{
		FT_Done_FreeType( m_pLibrary );
		m_pLibrary = NULL;
	}
}


//--------------------------------------------------------------
//	tessellate glyph and put into mesh
//	returns FALSE if nothing generated from outline
//--------------------------------------------------------------
Bool TLFileSys::TFileFreetype::VectoriseGlyph(TPtr<TLAsset::TMesh>& pMesh,const TLFreetype::FT_Outline& Outline)
{
	if ( !pMesh )
	{
		TLDebug_Break("Mesh expected");
		return FALSE;
	}

	TPtrArray<TLMaths::TContour>	Contours;

	s16 startIndex = 0;
	
	float ScaleDown = GetPointScale();

    for( s16 i=0;	i<Outline.n_contours;	i++ )
    {
        s16 endIndex = Outline.contours[i];
		TArray<float3> ContourPoints;
		TArray<TLMaths::TContourCurve> ContourCurves;

		if ( startIndex < 0 || endIndex < 0 )
		{
			TLDebug_Break("Invalid start/end index");
			break;
		}

		for ( u32 c=(u32)startIndex;	c<=(u32)endIndex;	c++ )
		{
			//	add point
			TLFreetype::FT_Vector& Vector = Outline.points[c];

			float3 Point3( (float)Vector.x, (float)Vector.y, 0.f );

			//	font coords are upside-down to us so negate y
			Point3.y = -Point3.y;

			//	scale down
			Point3 *= ScaleDown;

			//	and move so 0,0 is top-left
			//	gr: 3 seems to be a magic number... 100/64..*2?
			//	gr: logiccly if this is in TootleUnits then it should be +1...
			//Point3.y += 1.5f;
			Point3.y += 1.0f;

			ContourPoints.Add( Point3 );

			//	add curve tag
			char& Tag = Outline.tags[c];

			TLMaths::TContourCurve Curve = TLMaths::ContourCurve_On;
			char CurveTag = FT_CURVE_TAG( Tag );
			switch ( CurveTag )
			{
			case FT_Curve_Tag_On:		Curve = TLMaths::ContourCurve_On;	break;
			case FT_Curve_Tag_Conic:	Curve = TLMaths::ContourCurve_Conic;	break;
			case FT_Curve_Tag_Cubic:	Curve = TLMaths::ContourCurve_Cubic;	break;
			default:					Curve = TLMaths::ContourCurve_On;	break;
			}

			ContourCurves.Add( Curve );
		}

		TPtr<TLMaths::TContour> pContour = new TLMaths::TContour( ContourPoints, &ContourCurves );
		Contours.Add( pContour );

        startIndex = endIndex + 1;
    }

	//	no contours generated
	if ( !Contours.GetSize() )
		return FALSE;

    // Compute each contour's parity. FIXME: see if FT_Outline_Get_Orientation
    // can do it for us.
    for( u32 i=0;	i<Contours.GetSize(); i++ )
    {
        TPtr<TLMaths::TContour>& c1 = Contours[i];

        // 1. Find the leftmost point.
        const float3* pleftmost = &c1->Point(0);

        for(u32 n = 1; n < c1->GetPoints().GetSize(); n++)
        {
            const float3& p = c1->Point(n);
            if ( p.x < pleftmost->x )
            {
                pleftmost = &p;
            }
        }

        // 2. Count how many other contours we cross when going further to
        // the left.
        u32 parity = 0;

        for( u32 j=0;	j<Contours.GetSize();	j++)
        {
            if(j == i)
            {
                continue;
            }

            TPtr<TLMaths::TContour> c2 = Contours[j];

            for(u32 n = 0; n < c2->GetPoints().GetSize(); n++)
            {
                const float3& p1 = c2->Point(n);
                const float3& p2 = c2->Point((n + 1) % c2->GetPoints().GetSize());

                /* FIXME: combinations of >= > <= and < do not seem stable */
                if((p1.y < pleftmost->y && p2.y < pleftmost->y)
                    || (p1.y >= pleftmost->y && p2.y >= pleftmost->y)
                    || (p1.x > pleftmost->x && p2.x > pleftmost->x))
                {
                    continue;
                }
                else if(p1.x < pleftmost->x && p2.x < pleftmost->x)
                {
                    parity++;
                }
                else
                {
                    float3 a = p1 - (*pleftmost);
                    float3 b = p2 - (*pleftmost);
                    if(b.x * a.y > b.y * a.x)
                    {
                        parity++;
                    }
                }
            }
        }

        // 3. Make sure the glyph has the proper parity.
        c1->SetParity(parity);
    }


	
	float ZNormal = -1.f;

	TLMaths::TLTessellator::TWindingMode WindingMode = TLMaths::TLTessellator::WindingMode_NonZero;
	if ( (Outline.flags & ft_outline_even_odd_fill) != 0x0 )
	{
		WindingMode = TLMaths::TLTessellator::WindingMode_Odd;
	}

	//	create tessellator
	TLMaths::TTessellator* pTessellator = TLMaths::Platform::CreateTessellator( pMesh );
	if ( !pTessellator )
		return FALSE;
	
	//	add contours
	for ( u32 c=0;	c<Contours.GetSize();	c++ )
	{
		pTessellator->AddContour( Contours[c] );
	}

	//	tessellate!
	if ( !pTessellator->GenerateTessellations( WindingMode, ZNormal ) )
		return FALSE;

	return TRUE;
}


