#include "TFileSimpleVector.h"
#include <TootleAsset/TMesh.h>
#include <TootleMaths/TTessellate.h>
#include "TLFile.h"





TLFileSys::TFileSimpleVector::TFileSimpleVector(TRefRef FileRef,TRefRef FileTypeRef) :
	TFileXml				( FileRef, FileTypeRef ),
	m_SvgPointScale			( 1.f ),
	m_SvgLayerZIncrement	( 0.5f ),
	m_VertexColoursEnabled	( TRUE ),
	m_ProjectUVsEnabled		( FALSE )
{
}



//--------------------------------------------------------
//	import the XML and convert from SVG to mesh
//--------------------------------------------------------
SyncBool TLFileSys::TFileSimpleVector::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType)
{
	if ( pAsset )
	{
		TLDebug_Break("Async export not supported yet. asset should be NULL");
		return SyncFalse;
	}

	//	import xml
	SyncBool ImportResult = TFileXml::Import();
	if ( ImportResult != SyncTrue )
		return ImportResult;

	//	get the root svg tag (all items are under this and contains scene info)
	TPtr<TXmlTag> pSvgTag = m_XmlData.GetChild("svg");

	//	malformed SVG maybe
	if ( !pSvgTag )
		return SyncFalse;

	s32 i;

	//	delete branches we dont care about
	for ( i=pSvgTag->GetChildren().GetLastIndex();	i>=0;	i-- )
	{
		TXmlTag& Tag = *pSvgTag->GetChildren().ElementAt(i);
	
		//	keep these kinds of tags
		if ( Tag.GetTagType() == TLXml::TagType_OpenClose ||
			 Tag.GetTagType() == TLXml::TagType_SelfClose )
		{
			continue;
		}
	
		//	delete this tag
		pSvgTag->GetChildren().RemoveAt(i);
	}

	//	no data
	if ( pSvgTag->GetChildren().GetSize() == 0 )
		return SyncFalse;

	//	create mesh asset
	TPtr<TLAsset::TMesh> pNewMesh = new TLAsset::TMesh( GetFileRef() );

	//	init scale
	m_SvgPointScale = 1.f;

	//	check for options
	const TString* pVertexColoursProperty = pSvgTag->GetProperty("TootleVertexColours");
	if ( pVertexColoursProperty )
		m_VertexColoursEnabled = !pVertexColoursProperty->IsEqual("false",FALSE);

	const TString* pProjectUVsProperty = pSvgTag->GetProperty("TootleProjectUVs");
	if ( pProjectUVsProperty )
		m_ProjectUVsEnabled = !pProjectUVsProperty->IsEqual("false",FALSE);

	//	old style defualt scalar was [document width] = [1 unit]
	float DimensionScalar = 1.f;

	//	find dimensions
	float Width = 100.f;
	const TString* pWidthString = pSvgTag->GetProperty("width");
	if ( pWidthString )
		pWidthString->GetFloat(Width);

	//	new scale format - if set then the width is relative to a ortho screen width (ie. 100 wide) instead of 1 unit wide
	//	gr: now uses document width, so 100 width assumes orhto camera of 100
	const TString* pOrthoScaleProperty = pSvgTag->GetProperty("TootleOrthoScale");
	if ( pOrthoScaleProperty )
		if ( !pOrthoScaleProperty->IsEqual("false",FALSE) )
			DimensionScalar = Width;

	//	set scale
	m_SvgPointScale = Width <= 0.f ? 1.f : DimensionScalar / Width;

	//	init offset
	m_SvgPointMove.Set( 0.f, 0.f, 0.f );

	//	parse xml to mesh (and it's children)
	if ( !ImportMesh( pNewMesh, *pSvgTag ) )
		return SyncFalse;

	//	set projection UVs
	if ( m_ProjectUVsEnabled )
		pNewMesh->CalcProjectionUVs();

	//	assign resulting asset
	pAsset = pNewMesh;
	pNewMesh->CalcBounds();

	return SyncTrue;
}


//--------------------------------------------------------
//	generate mesh data from this SVG tag
//--------------------------------------------------------
Bool TLFileSys::TFileSimpleVector::ImportMesh(TPtr<TLAsset::TMesh>& pMesh,TXmlTag& Tag)
{
	//	deal with child tags
	for ( u32 c=0;	c<Tag.GetChildren().GetSize();	c++ )
	{
		TXmlTag& ChildTag = *Tag.GetChildren().ElementAt(c);
		if ( ChildTag.GetTagName() == "polygon" )
		{
			if ( !ImportPolygonTag( pMesh, ChildTag ) )
				return FALSE;
		}
		else if ( ChildTag.GetTagName() == "path" )
		{
			if ( !ImportPathTag( pMesh, ChildTag ) )
				return FALSE;
		}
		else if ( ChildTag.GetTagName() == "rect" )
		{
			if ( !ImportRectTag( pMesh, ChildTag ) )
				return FALSE;
		}

		//	unknown type, make a new child out of it
		if ( !ChildTag.GetChildren().GetSize() )
			continue;

		float OldZ = m_SvgPointMove.z;

		//	every time we go deeper down the tree increment the z
		m_SvgPointMove.z += m_SvgLayerZIncrement;

		if ( !ImportMesh( pMesh, ChildTag ) )
			return FALSE;
		
		//	verify mesh
		if ( !pMesh->Debug_Verify() )
			return false;

		//	restore z
		//m_SvgPointMove.z = OldZ;
	}


	
	return TRUE;
}


//--------------------------------------------------------
//	convert Polygon tag to mesh info and add to mesh
//--------------------------------------------------------
Bool TLFileSys::TFileSimpleVector::ImportPolygonTag(TPtr<TLAsset::TMesh>& pMesh,TXmlTag& Tag)
{
	const TString* pString = Tag.GetProperty("points");
	if ( !pString )
	{
		if ( TLDebug_Break("Expected points property of svg for <polygon>") )
			return FALSE;

		return TRUE;
	}

	//	empty string.. empty polygon
	u32 StringLength = pString->GetLength();
	if ( StringLength == 0 )
		return TRUE;

	//	
	THeapArray<TTempString> NumberPairs;
	if ( !pString->Split(' ', NumberPairs ) )
	{
		TLDebug_Break("No numbers?");
		return FALSE;
	}

	//	split pairs into point numbers, then into vertex positions
	THeapArray<float3> OutlinePoints;

	for ( u32 i=0;	i<NumberPairs.GetSize();	i++ )
	{
		THeapArray<TTempString> PointStrings;
		if ( !NumberPairs[i].Split( ',', PointStrings ) )
		{
			if ( TLDebug_Break( TString("Error splitting pair of numbers: %s", NumberPairs[i].GetData() ) ) )
				return FALSE;

			continue;
		}

		//	turn first string into a number
		float2 Point;
		if ( !PointStrings[0].GetFloat( Point.x ) )
		{
			if ( TLDebug_Break("Invalid number found in number pair") )
				return FALSE;
			continue;
		}
		if ( !PointStrings[1].GetFloat( Point.y ) )
		{
			if ( TLDebug_Break("Invalid number found in number pair") )
				return FALSE;
			continue;
		}

		float3 VertexPos = CoordinateToVertexPos( Point );
		OutlinePoints.Add( VertexPos );
	}

	//	empty, but not invalid, nothing to create
	if ( OutlinePoints.GetSize() == 0 )
		return TRUE;

	//	turn into datum instead of geometry
	TRef DatumRef,DatumShapeType;
	Bool IsJustDatum=TRUE;
	if ( IsTagDatum( Tag, DatumRef, DatumShapeType, IsJustDatum ) )
	{
		pMesh->CreateDatum( OutlinePoints, DatumRef, DatumShapeType );

		//	just a datum, don't create geometry
		if ( IsJustDatum )
			return TRUE;
	}

	//	turn into a polygon in the mesh
	TLMaths::TTessellator* pTessellator = TLMaths::Platform::CreateTessellator( pMesh );
	if ( !pTessellator )
		return FALSE;
		
	TPtr<TLMaths::TContour> pContour = new TLMaths::TContour( OutlinePoints, NULL );
	pTessellator->AddContour(pContour);
		
	if ( !pTessellator->GenerateTessellations( TLMaths::TLTessellator::WindingMode_Odd ) )
		return FALSE;
	
	return TRUE;
}


//--------------------------------------------------------
//	convert Polygon tag to mesh info and add to mesh
//	http://www.w3.org/TR/SVG/paths.html#PathElement
//--------------------------------------------------------
Bool TLFileSys::TFileSimpleVector::ImportPathTag(TPtr<TLAsset::TMesh>& pMesh,TXmlTag& Tag)
{
	const TString* pString = Tag.GetProperty("d");
	if ( !pString )
	{
		if ( TLDebug_Break("Expected d property of svg for <path>") )
			return FALSE;

		return TRUE;
	}

	const TString& String = *pString;

	//	empty string.. empty polygon
	u32 StringLength = String.GetLength();
	if ( StringLength == 0 )
		return TRUE;

	TPtrArray<TLMaths::TContour> Contours;
	THeapArray<TLMaths::TContourCurve> PathCurves;
	THeapArray<float3> PathPoints;
	TChar LastCommand = 0x0;
	float2 CurrentPosition;

	//	parse commands in string
	u32 CharIndex = 0;
	while ( CharIndex<String.GetLength() )
	{
		//	read next command
		TChar NewCommand;
		if ( TLString::ReadNextLetter( String, CharIndex, NewCommand ) )
		{
			Bool FindNewCommand = FALSE;

			//	process command
			switch ( NewCommand )
			{
				case 'z':	//	finish contour
				case 'Z':	//	finish contour
				{
					//	finish current contour
					if ( PathPoints.GetSize() )
					{
						TPtr<TLMaths::TContour> pNewContour = new TLMaths::TContour( PathPoints, &PathCurves );
						Contours.Add( pNewContour );
						PathPoints.Empty();
						PathCurves.Empty();
					}
					LastCommand = NewCommand;	//	gr: maybe get rid of this and expect an M next?
					FindNewCommand = TRUE;
					break;
				}

				case 'm':
				case 'M':	//	new contour
				{
					//	finish current contour
					if ( PathPoints.GetSize() )
					{
						TPtr<TLMaths::TContour> pNewContour = new TLMaths::TContour( PathPoints, &PathCurves );
						Contours.Add( pNewContour );
						PathPoints.Empty();
						PathCurves.Empty();
					}

					//	start new contour by fetching the initial pos
					float2 InitialPos;
					if ( !TLString::ReadNextFloat( String, CharIndex, InitialPos ) )
						return FALSE;
					PathPoints.Add( CoordinateToVertexPos( InitialPos ) );
					PathCurves.Add( TLMaths::ContourCurve_On );

					CurrentPosition = InitialPos;
					LastCommand = NewCommand;
					FindNewCommand = TRUE;
				}
				break;

				case 'l':
				case 'L':	//	line to here
				case 'c':
				case 'C':	//	line to here
				{
					//	set command and let code continue to do simple process
					LastCommand = NewCommand;
				}
				break;

				case 'a':	//	arc (relative)
				case 'A':	//	arc (absolute)
				{
					//	set command and let code continue to do simple process
					LastCommand = NewCommand;
				}
				break;	
					
				case 'S':	//	shorthand curve
				case 'Q':	//	quadratic bezier curve
				case 'T':	//	shorthand quadratic bezier curve
				{
					TLDebug_Print( TString("Unhandled <path> command: %c", NewCommand ) );
					//	no error with structure, we just cant import this path
					return TRUE;
				}
				break;

				default:
					TLDebug_Break( TString("Unknown <path> command: %c", NewCommand ) );
					//	no error with structure, we just cant import this path
					return TRUE;
					continue;
			}
	
			//	if we want to just go onto the next command then skip over continue-processing below
			if ( FindNewCommand )
				continue;
		}

		//	no command, continue last command
		
		//	line
		if ( LastCommand == 'L' || LastCommand == 'l' || LastCommand == 'H' || LastCommand == 'V' )
		{
			float2 NewPos = CurrentPosition;

			if  ( LastCommand == 'L' || LastCommand == 'l')
			{
				//	read new pos
				if ( !TLString::ReadNextFloat( String, CharIndex, NewPos ) )
					return FALSE;
				
				if ( TLString::IsCharLowercase( LastCommand ) )
					NewPos += CurrentPosition;
			}
			else if ( LastCommand == 'H' )
			{
				//	read new x
				float NewX;
				if ( !TLString::ReadNextFloat( String, CharIndex, NewX ) )
					return FALSE;
				if ( TLString::IsCharLowercase( LastCommand ) )
					NewPos.x += NewX;
				else
					NewPos.x = NewX;
			}
			else if ( LastCommand == 'V' )
			{
				//	read new x
				float NewY;
				if ( !TLString::ReadNextFloat( String, CharIndex, NewY ) )
					return FALSE;
				if ( TLString::IsCharLowercase( LastCommand ) )
					NewPos.y += NewY;
				else
					NewPos.y = NewY;
			}

			//	continue line
			PathPoints.Add( CoordinateToVertexPos( NewPos ) );
			PathCurves.Add( TLMaths::ContourCurve_On );
			CurrentPosition = NewPos;
		}
		else if(( LastCommand == 'C' ) || ( LastCommand == 'c' ))
		{
			//	cubic curve - 
			//	read control point A (before)
			float2 CubicControlPointA;
			if ( !TLString::ReadNextFloat( String, CharIndex, CubicControlPointA ) )
				return FALSE;

			//	read control point B (after)
			float2 CubicControlPointB;
			if ( !TLString::ReadNextFloat( String, CharIndex, CubicControlPointB ) )
				return FALSE;

			//	read end point (on)
			float2 CubicPointPosition;
			if ( !TLString::ReadNextFloat( String, CharIndex, CubicPointPosition ) )
				return FALSE;

			//	first cubic point is last point in points
			if ( !PathPoints.GetSize() )
			{
				TLDebug_Break("Expected a previous point to start the cubic curve from... maybe need to add the pos from M?");
				continue;
			}
			
			float2 NewPosition, NewCubicControlPointA, NewCubicControlPointB;
			
			if ( TLString::IsCharLowercase( LastCommand ) )
			{
				// Relative points
				NewPosition = CurrentPosition + CubicPointPosition;				
				NewCubicControlPointA = CurrentPosition + CubicControlPointA;
				NewCubicControlPointB = CurrentPosition + CubicControlPointB;
			}
			else 
			{
				// Absolute point
				NewPosition = CubicPointPosition;
				NewCubicControlPointA = CubicControlPointA;
				NewCubicControlPointB = CubicControlPointB;
			}

			//	add these in the right order (as per tesselator)
			PathPoints.Add( CoordinateToVertexPos( NewCubicControlPointA ) );
			PathCurves.Add( TLMaths::ContourCurve_Cubic );
			
			PathPoints.Add( CoordinateToVertexPos( NewCubicControlPointB ) );
			PathCurves.Add( TLMaths::ContourCurve_Cubic );
			
			PathPoints.Add( CoordinateToVertexPos( NewPosition ) );
			PathCurves.Add( TLMaths::ContourCurve_On );
			
			CurrentPosition = NewPosition;
		}
		else if(( LastCommand == 'A' ) || ( LastCommand == 'a' ))
		{
			//	arc - 
			// Read radii (rx,ry)
			float2 Radii;
			if ( !TLString::ReadNextFloat( String, CharIndex, Radii ) )
				return FALSE;
			
			//	read x-axis rotation
			float XAxisRot;
			if ( !TLString::ReadNextFloat( String, CharIndex, XAxisRot ) )
				return FALSE;
			
			// read sweep flag and arc flag
			float2 flags;
			if ( !TLString::ReadNextFloat( String, CharIndex, flags ) )
				return FALSE;
			
			//	read end point (on)
			float2 ArcEndPosition;
			if ( !TLString::ReadNextFloat( String, CharIndex, ArcEndPosition ) )
				return FALSE;
			
			//	first cubic point is last point in points
			if ( !PathPoints.GetSize() )
			{
				TLDebug_Break("Expected a previous point to start the cubic curve from... maybe need to add the pos from M?");
				continue;
			}
			
			float2 NewPosition;
			
			if ( TLString::IsCharLowercase( LastCommand ) )
			{
				// Relative points
				
				
				NewPosition = CurrentPosition + ArcEndPosition;				
			}
			else 
			{
				// Absolute point
				
				NewPosition = ArcEndPosition;
			}
			
			//	add these in the right order (as per tesselator)
			//PathPoints.Add( Radii);
			//PathCurves.Add( TLMaths::ContourCurve_Arc );
						
			PathPoints.Add( CoordinateToVertexPos( NewPosition ) );
			PathCurves.Add( TLMaths::ContourCurve_On );
			
			CurrentPosition = NewPosition;
		}		
		else if ( LastCommand == 'Z' || LastCommand == 'z' )
		{
			CharIndex++;
			continue;
		}
		else
		{
			TLDebug_Break("Unhandled last command ");
			return FALSE;
		}
	}

	//	finish current contour if wasn't ended by a command
	if ( PathPoints.GetSize() )
	{
		TPtr<TLMaths::TContour> pNewContour = new TLMaths::TContour( PathPoints, &PathCurves );
		Contours.Add( pNewContour );
		PathPoints.Empty();
		PathCurves.Empty();
	}


	if ( !Contours.GetSize() )
		return FALSE;

	//	turn into datum instead of geometry
	TRef DatumRef,DatumShapeType;
	Bool IsJustDatum = TRUE;
	if ( IsTagDatum( Tag, DatumRef, DatumShapeType, IsJustDatum ) )
	{
		if ( Contours.GetSize() == 1 )
		{
			pMesh->CreateDatum( Contours[0]->GetPoints(), DatumRef, DatumShapeType );
		}
		else
		{
			//	put all contour points into one array of points
			THeapArray<float3> ContourPoints;
			for ( u32 c=0;	c<Contours.GetSize();	c++ )
				ContourPoints.Add( Contours[c]->GetPoints() );
			
			pMesh->CreateDatum( ContourPoints, DatumRef, DatumShapeType );
		}

		//	skip creating geometry if datum is marked as just a datum
		if ( IsJustDatum )
			return TRUE;
	}
	
	//	create geometry

	//	get style
	Style TagStyle( Tag.GetProperty("style") );

	//	if a filled vector then create tesselations from the outline
	if ( TagStyle.m_HasFill )
	{
		//	tesselate those bad boys!
		TLMaths::TTessellator* pTessellator = TLMaths::Platform::CreateTessellator( pMesh );
		if ( pTessellator )
		{
			u32 ContourCount = Contours.GetSize();
			if ( ContourCount > 1 )
				ContourCount = 1;
			for ( u32 c=0;	c<ContourCount;	c++ )
			{
				pTessellator->AddContour( Contours[c] );
			}

			pTessellator->SetVertexColour( m_VertexColoursEnabled ? &TagStyle.m_FillColour : NULL );
			pTessellator->GenerateTessellations( TLMaths::TLTessellator::WindingMode_Odd );
		}
	}

	if ( TagStyle.m_HasStroke )
	{
		CreateMeshLines( *pMesh, Contours, TagStyle );
	}

	return TRUE;
}


//--------------------------------------------------------
//	convert rect tag into a polygon
//--------------------------------------------------------
Bool TLFileSys::TFileSimpleVector::ImportRectTag(TPtr<TLAsset::TMesh>& pMesh,TXmlTag& Tag)
{
	//	get rect strings
	const TString* pLeftString = Tag.GetProperty("x");
	const TString* pTopString = Tag.GetProperty("y");
	const TString* pWidthString = Tag.GetProperty("width");
	const TString* pHeighthString = Tag.GetProperty("height");

	if ( !pWidthString || !pHeighthString || !pLeftString || !pTopString )
	{
		if ( TLDebug_Break("Expected x/y/w/h properties of svg for <rect>") )
			return FALSE;

		return TRUE;
	}

	//	get style
	Style TagStyle( Tag.GetProperty("style") );

	//	create rect
	float4 Rect;
	Bool AnyError = FALSE;
	AnyError |= !pLeftString->GetFloat( Rect.Left() );
	AnyError |= !pTopString->GetFloat( Rect.Top() );
	AnyError |= !pWidthString->GetFloat( Rect.Width() );
	AnyError |= !pHeighthString->GetFloat( Rect.Height() );

	if ( AnyError )
	{
		if ( TLDebug_Break("Failed to parse x/y/w/h string for dimensions of svg <rect>") )
			return FALSE;

		return TRUE;
	}

	float2 PointTopLeft( Rect.Left(), Rect.Top() );
	float2 PointTopRight( Rect.Right(), Rect.Top() );
	float2 PointBottomLeft( Rect.Left(), Rect.Bottom() );
	float2 PointBottomRight( Rect.Right(), Rect.Bottom() );

	//	vertex outline
	TFixedArray<float3,4> Vertexes;
	Vertexes.Add( CoordinateToVertexPos( PointTopLeft ) );
	Vertexes.Add( CoordinateToVertexPos( PointTopRight ) );
	Vertexes.Add( CoordinateToVertexPos( PointBottomRight ) );
	Vertexes.Add( CoordinateToVertexPos( PointBottomLeft ) );

	//	turn into datum instead of geometry
	TRef DatumRef,DatumShapeType;
	Bool IsJustDatum = TRUE;
	if ( IsTagDatum( Tag, DatumRef, DatumShapeType, IsJustDatum ) )
	{
		pMesh->CreateDatum( Vertexes, DatumRef, DatumShapeType );

		//	skip creating geometry if just a datum
		if ( IsJustDatum )
			return TRUE;
	}
	
	if ( TagStyle.m_HasFill || TagStyle.m_HasStroke )	//	create polygons
	{
		//	create quad
		if ( TagStyle.m_HasFill )
		{
			pMesh->GenerateQuad( Vertexes, m_VertexColoursEnabled ? &TagStyle.m_FillColour : NULL );
		}

		//	create line strip
		if ( TagStyle.m_HasStroke )
		{
			TLAsset::TMesh::Linestrip* pNewLinestrip = pMesh->GetLinestrips().AddNew();
			TLAsset::TMesh::Tristrip& NewLinestrip = *pNewLinestrip;
			NewLinestrip.Add( pMesh->AddVertex( Vertexes[0], m_VertexColoursEnabled ? &TagStyle.m_StrokeColour : NULL ) );
			NewLinestrip.Add( pMesh->AddVertex( Vertexes[1], m_VertexColoursEnabled ? &TagStyle.m_StrokeColour : NULL ) );
			NewLinestrip.Add( pMesh->AddVertex( Vertexes[2], m_VertexColoursEnabled ? &TagStyle.m_StrokeColour : NULL ) );
			NewLinestrip.Add( pMesh->AddVertex( Vertexes[3], m_VertexColoursEnabled ? &TagStyle.m_StrokeColour : NULL ) );
		}
	}


	return TRUE;
}


//--------------------------------------------------------
//	move & scale coordinate and convert to float3
//--------------------------------------------------------
float3 TLFileSys::TFileSimpleVector::CoordinateToVertexPos(const float2& Coordinate)
{
	float3 NewPos( Coordinate.x, Coordinate.y, 0.f );

	NewPos += m_SvgPointMove;
	NewPos *= m_SvgPointScale;

	return NewPos;
}

	
//--------------------------------------------------------
//	create line strips on mesh from a list of contours
//--------------------------------------------------------
void TLFileSys::TFileSimpleVector::CreateMeshLines(TLAsset::TMesh& Mesh,TPtrArray<TLMaths::TContour>& Contours,Style& LineStyle)
{
	for ( u32 c=0;	c<Contours.GetSize();	c++ )
	{
		CreateMeshLineStrip( Mesh, *Contours[c], LineStyle );
	}
}


//--------------------------------------------------------
//	create line strip on mesh from a contour
//--------------------------------------------------------
void TLFileSys::TFileSimpleVector::CreateMeshLineStrip(TLAsset::TMesh& Mesh,TLMaths::TContour& Contour,Style& LineStyle)
{
	const TArray<float3>& ContourPoints = Contour.GetPoints();

	TLAsset::TMesh::Linestrip* pNewLine = Mesh.GetLinestrips().AddNew();
	if ( !pNewLine )
		return;

	s32 FirstVertex = -1;

	for ( u32 p=0;	p<=ContourPoints.GetSize();	p++ )
	{
		s32 VertexIndex = -1;

		//	need to add the first vert to the last line to keep a loop if we split it up
		if ( p >= ContourPoints.GetSize() )
		{
			//	still the original line
			if ( p < TLAsset::g_MaxLineStripSize )
				break;
			
			VertexIndex = FirstVertex;
		}
		else
		{
			VertexIndex = Mesh.AddVertex( ContourPoints[p], m_VertexColoursEnabled ? &LineStyle.m_StrokeColour : NULL );
		}

		pNewLine->Add( VertexIndex );

		if ( FirstVertex == -1 )
			FirstVertex = VertexIndex;

		//	gr: need to cut up the line
		if ( p < ContourPoints.GetSize() && pNewLine->GetSize() >= TLAsset::g_MaxLineStripSize )
		{
			//	make new line
			pNewLine = Mesh.GetLinestrips().AddNew();
			if ( !pNewLine )
				break;
			
			//	start line at current vert so it doesnt break the line
			pNewLine->Add( VertexIndex );
		}
	}

}

//--------------------------------------------------------
//	check if tag marked as a datum
//--------------------------------------------------------
Bool TLFileSys::TFileSimpleVector::IsTagDatum(TXmlTag& Tag,TRef& DatumRef,TRef& ShapeType,Bool& IsJustDatum)
{
	//	get the "id" property
	const TString* pIDString = Tag.GetProperty("id");
	if ( !pIDString )
		return FALSE;

	//	check if string marked as a datum
	return TLString::IsDatumString( *pIDString, DatumRef, ShapeType, IsJustDatum );
}


TLFileSys::TFileSimpleVector::Style::Style(const TString* pStyleString) :
	m_HasFill		( TRUE ),					//	by default we fill the polygon
	m_FillColour	( 1.f, 1.f, 1.f, 1.f ),
	m_HasStroke		( FALSE ),
	m_StrokeColour	( 1.f, 1.f, 1.f, 1.f ),
	m_StrokeWidth	( 0.f )
{
	//	parse string
	Parse( pStyleString );
}

	
//--------------------------------------------------------
//	parse string and get style elements
//--------------------------------------------------------
Bool TLFileSys::TFileSimpleVector::Style::Parse(const TString& StyleString)
{
	//	empty string
	if ( StyleString.GetLength() == 0 )
		return FALSE;

	//	example:
	//	style="fill:#192bed;fill-opacity:0.44767445;stroke:none;stroke-width:2.26799989000000000;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1"

	//	split string at semi colons
	THeapArray<TTempString> PropertyStrings;
	StyleString.Split( ';', PropertyStrings );

	//	make up properties
	TKeyArray<TTempString,TString> Properties;
	for ( u32 p=0;	p<PropertyStrings.GetSize();	p++ )
	{
		//	split property
		TFixedArray<TTempString,2> PropertyParts;
		PropertyStrings[p].Split(':', PropertyParts);
		
		Properties.Add( PropertyParts[0], PropertyParts[1] );
	}

	//	get specific properties

	//	get fill colour
	TString* pFillColour = Properties.Find("fill");
	if ( pFillColour )
	{
		m_HasFill = SetColourFromString( m_FillColour, *pFillColour );
	}

	//	get fill alpha
	TString* pFillAlpha = Properties.Find("fill-opacity");
	if ( pFillAlpha )
	{
		pFillAlpha->GetFloat( m_FillColour.GetAlphaf() );
	}

	//	see if it has a stroke or not
	TString* pStrokeColour = Properties.Find("stroke");
	if ( pStrokeColour )
	{
		m_HasStroke = SetColourFromString( m_StrokeColour, *pStrokeColour );
	}

	//	get stroke line size
	TString* pStrokeWidth = Properties.Find("stroke-width");
	if ( pStrokeWidth )
	{
		pStrokeWidth->GetFloat( m_StrokeWidth );
	}

	//	get stroke alpha
	TString* pStrokeAlpha = Properties.Find("stroke-opacity");
	if ( pStrokeAlpha )
	{
		pStrokeAlpha->GetFloat( m_StrokeColour.GetAlphaf() );
	}

	return TRUE;
}
	      


//--------------------------------------------------------
//	extracts a colour from a string. returns FALSE if no colour or failed to parse
//--------------------------------------------------------
Bool TLFileSys::TFileSimpleVector::Style::SetColourFromString(TColour& Colour,const TString& ColourString)
{
	u32 Length = ColourString.GetLength();

	//	if string is empty, no colour
	if ( Length < 1 )
		return FALSE;

	//	colour is "none" so no colour
	if ( ColourString == "none" )
		return FALSE;

	//	todo: get string colours, "red", "blue", etc

	//	is a hex colour?
	if ( ColourString[0] == '#' )
	{
		//	specific length expected
		if ( Length != 6 + 1 )
		{
			TLDebug_Break("String not long enough to be a hex colour e.g. #ffffff");
			return FALSE;
		}

		//	split into individual strings for each hex part of the colour
		TTempString RedString;		RedString.Append( ColourString, 1, 2 );
		TTempString GreenString;	GreenString.Append( ColourString, 3, 2 );
		TTempString BlueString;		BlueString.Append( ColourString, 5, 2 );
		u32 r,g,b;
		Bool AnyError = FALSE;
		AnyError |= !RedString.GetHexInteger( r );
		AnyError |= !GreenString.GetHexInteger( g );
		AnyError |= !BlueString.GetHexInteger( b );

		if ( AnyError || r>255 || g>255 || b>255 )
		{
			TLDebug_Break("Failed to parse red, green or blue" );
			return FALSE;
		}
		
		//	set colour
		Colour.Set( r, g, b, Colour.GetAlpha8() );
		return TRUE;
	}
	else
	{
		//	not a hex colour... dunno how to parse this - please write all examples here in the comments :)
		TLDebug_Break("Don't know how to parse colour string in style");
		return FALSE;
	}
}


