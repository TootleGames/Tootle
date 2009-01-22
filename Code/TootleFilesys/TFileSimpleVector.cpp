#include "TFileSimpleVector.h"
#include <TootleAsset/TMesh.h>
#include <TootleMaths/TTessellate.h>


namespace TLString
{
	Bool	ReadNextLetter(const TString& String,u32& CharIndex, char& Char);
	Bool	ReadNextFloatArray(const TString& String,u32& CharIndex,float* pFloats,u32 FloatSize);

	template<typename FLOATTYPE>
	Bool	ReadNextFloat(const TString& String,u32& CharIndex,FLOATTYPE& FloatType);
	template<>
	Bool	ReadNextFloat(const TString& String,u32& CharIndex,float& FloatType);
};




//--------------------------------------------------------
//	
//--------------------------------------------------------
Bool TLString::ReadNextLetter(const TString& String,u32& CharIndex, char& Char)
{
	//	step over whitespace
	s32 NonWhitespaceIndex = String.GetCharIndexNonWhitespace( CharIndex );
	if ( NonWhitespaceIndex == -1 )
		return FALSE;

	//	move char past whitespace
	CharIndex = (u32)NonWhitespaceIndex;
	const char& NextChar = String.GetCharAt(CharIndex);

	//	is next char a letter?
	if ( TLString::IsCharLetter( NextChar ) )
	{
		Char = NextChar;
			
		//	move string past this letter for next thing
		CharIndex++;

		return TRUE;
	}
	else
	{
		//	not a char, could be a number or summink
		return FALSE;
	}
}


template<typename FLOATTYPE>
Bool TLString::ReadNextFloat(const TString& String,u32& CharIndex,FLOATTYPE& FloatType)
{
	return ReadNextFloatArray( String, CharIndex, FloatType.GetData(), FloatType.GetSize() );
}

template<>
Bool TLString::ReadNextFloat(const TString& String,u32& CharIndex,float& FloatType)
{
	return ReadNextFloatArray( String, CharIndex, &FloatType, 1 );
}


//--------------------------------------------------------
//	
//--------------------------------------------------------
Bool TLString::ReadNextFloatArray(const TString& String,u32& CharIndex,float* pFloats,u32 FloatSize)
{
	//	loop through parsing seperators and floats
	u32 FloatIndex = 0;
	while ( FloatIndex < FloatSize )
	{
		//	step over whitespace
		s32 NonWhitespaceIndex = String.GetCharIndexNonWhitespace( CharIndex );

		//	no more non-whitespace? no more floats then
		if ( NonWhitespaceIndex == -1 )
			return FALSE;

		//	move char past whitespace
		CharIndex = (u32)NonWhitespaceIndex;

		s32 NextComma = String.GetCharIndex(',', CharIndex);
		s32 NextWhitespace = String.GetCharIndexWhitespace( CharIndex );
		s32 NextSplit = String.GetLength();

		if ( NextComma != -1 && NextComma < NextSplit )
			NextSplit = NextComma;
		if ( NextWhitespace != -1 && NextWhitespace < NextSplit )
			NextSplit = NextWhitespace;

		//	split
		TTempString Stringf;
		Stringf.Append( String, CharIndex, NextSplit-CharIndex );
		if ( !Stringf.GetFloat( pFloats[FloatIndex] ) )
		{
			TLDebug_Break("Failed to parse first float");
			return FALSE;
		}

		//	next float
		FloatIndex++;

		//	move string along past split
		CharIndex = NextSplit+1;

		//	out of string
		if ( CharIndex >= String.GetLength() )
		{
			CharIndex = String.GetLength();
			break;
		}
	}

	return TRUE;
}


/*

//--------------------------------------------------------
//	
//--------------------------------------------------------
Bool ReadNextFloat2(const TString& String,u32& CharIndex, float2& Float2)
{
	//	step over whitespace
	s32 NonWhitespaceIndex = String.GetCharIndexNonWhitespace( CharIndex );
	if ( NonWhitespaceIndex == -1 )
		return FALSE;

	//	move char past whitespace
	CharIndex = (u32)NonWhitespaceIndex;

	//	right, expecting a comma to seperate the two points
	//	if we find a whitespace first then there's a problem
	s32 NextComma = String.GetCharIndex(',', CharIndex);
	s32 NextWhitespace = String.GetCharIndexWhitespace( CharIndex );

	//	no comma at all
	if ( NextComma == -1 )
	{
		TLDebug_Break("Expected to find comma for float2");
		return FALSE;
	}

	//	no more whitespaces, make end of this as end of string
	if ( NextWhitespace == -1 )
		NextWhitespace = String.GetLength();

	//	whitespace before comma
	if ( NextWhitespace < NextComma )
	{
		TLDebug_Break("Found whitespace before comma.");
		return FALSE;
	}

	//	right. here->comma is x, comma->whitespace is y
	TTempString StringX;
	StringX.Append( String, CharIndex, NextComma-CharIndex );
	if ( !StringX.GetFloat( Float2.x ) )
	{
		TLDebug_Break("Failed to parse first float");
		return FALSE;
	}

	TTempString StringY;
	StringY.Append( String, NextComma+1, NextWhitespace-(NextComma+1) );
	if ( !StringY.GetFloat( Float2.y ) )
	{
		TLDebug_Break("Failed to parse second float");
		return FALSE;
	}

	//	all done! move along char index
	CharIndex = NextWhitespace;

	return TRUE;
}



//--------------------------------------------------------
//	
//--------------------------------------------------------
Bool ReadNextFloat(const TString& String,u32& CharIndex, float& Float)
{
	//	step over whitespace
	s32 NonWhitespaceIndex = String.GetCharIndexNonWhitespace( CharIndex );
	if ( NonWhitespaceIndex == -1 )
		return FALSE;

	//	move char past whitespace
	CharIndex = (u32)NonWhitespaceIndex;

	s32 NextWhitespace = String.GetCharIndexWhitespace( CharIndex );

	//	no more whitespaces, make end of this as end of string
	if ( NextWhitespace == -1 )
		NextWhitespace = String.GetLength();

	TTempString StringFloat;
	StringFloat.Append( String, CharIndex, NextWhitespace-CharIndex );
	if ( !StringFloat.GetFloat( Float ) )
	{
		TLDebug_Break("Failed to parse first float");
		return FALSE;
	}

	//	all done! move along char index
	CharIndex = NextWhitespace;

	return TRUE;
}
*/




TLFileSys::TFileSimpleVector::TFileSimpleVector(TRefRef FileRef,TRefRef FileTypeRef) :
	TFileXml				( FileRef, FileTypeRef ),
	m_SvgPointScale			( 1.f, 1.f, 1.f ),
	m_SvgLayerZIncrement	( 0.5f )
{
}



//--------------------------------------------------------
//	import the XML and convert from SVG to mesh
//--------------------------------------------------------
SyncBool TLFileSys::TFileSimpleVector::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported)
{
	if ( pAsset )
	{
		TLDebug_Break("Async export not supported yet. asset should be NULL");
		return SyncFalse;
	}

	Supported = TRUE;

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
		TPtr<TXmlTag>& pTag = pSvgTag->GetChildren().ElementAt(i);
	
		//	keep these kinds of tags
		if ( pTag->GetTagType() == TLXml::TagType_OpenClose ||
			 pTag->GetTagType() == TLXml::TagType_OpenClose )
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
	m_SvgPointScale.Set( 1.f, 1.f, 1.f );

	//	find scale dimensions
	const TString* pWidthString = pSvgTag->GetProperty("width");
	if ( pWidthString )
	{
		s32 Width;
		if ( pWidthString->GetInteger(Width) )
			if ( Width != 0 )
				m_SvgPointScale.x = 1.f / (float)Width;
	}

	const TString* pHeightString = pSvgTag->GetProperty("height");
	if ( pHeightString )
	{
		s32 Height;
		if ( pHeightString->GetInteger(Height) )
			if ( Height != 0 )
				m_SvgPointScale.y = 1.f / (float)Height;
	}

	//	init offset
	m_SvgPointMove.Set( 0.f, 0.f, 0.f );

	//	parse xml to mesh (and it's children)
	if ( !ImportMesh( pNewMesh, pSvgTag ) )
		return SyncFalse;

	//	assign resulting asset
	pAsset = pNewMesh;
	pNewMesh->CalcBoundsBox();

	return SyncTrue;
}


//--------------------------------------------------------
//	generate mesh data from this SVG tag
//--------------------------------------------------------
Bool TLFileSys::TFileSimpleVector::ImportMesh(TPtr<TLAsset::TMesh>& pMesh,TPtr<TXmlTag>& pTag)
{
	//	deal with child tags
	for ( u32 c=0;	c<pTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pTag->GetChildren().ElementAt(c);
		if ( pChildTag->GetTagName() == "polygon" )
		{
			if ( !ImportPolygonTag( pMesh, pChildTag ) )
				return FALSE;
		}
		else if ( pChildTag->GetTagName() == "path" )
		{
			if ( !ImportPathTag( pMesh, pChildTag ) )
				return FALSE;
		}
		else if ( pChildTag->GetTagName() == "rect" )
		{
			if ( !ImportRectTag( pMesh, pChildTag ) )
				return FALSE;
		}

		//	unknown type, make a new child out of it
		if ( !pChildTag->GetChildren().GetSize() )
			continue;

		//	every time we go deeper down the tree increment the z
		m_SvgPointMove.z += m_SvgLayerZIncrement;

		if ( !ImportMesh( pMesh, pChildTag ) )
			return FALSE;
	}


	
	return TRUE;
}


//--------------------------------------------------------
//	convert Polygon tag to mesh info and add to mesh
//--------------------------------------------------------
Bool TLFileSys::TFileSimpleVector::ImportPolygonTag(TPtr<TLAsset::TMesh>& pMesh,TPtr<TXmlTag>& pTag)
{
	const TString* pString = pTag->GetProperty("points");
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
	TArray<TTempString> NumberPairs;
	if ( !pString->Split(' ', NumberPairs ) )
	{
		TLDebug_Break("No numbers?");
		return FALSE;
	}

	//	split pairs into point numbers, then into vertex positions
	TArray<float3> OutlinePoints;

	for ( u32 i=0;	i<NumberPairs.GetSize();	i++ )
	{
		TArray<TTempString> PointStrings;
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
Bool TLFileSys::TFileSimpleVector::ImportPathTag(TPtr<TLAsset::TMesh>& pMesh,TPtr<TXmlTag>& pTag)
{
	const TString* pString = pTag->GetProperty("d");
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
	TArray<TLMaths::TContourCurve> PathCurves;
	TArray<float3> PathPoints;
	char LastCommand = 0x0;
	float2 CurrentPosition;

	//	parse commands in string
	u32 CharIndex = 0;
	while ( CharIndex<String.GetLengthWithoutTerminator() )
	{
		//	read next command
		char NewCommand;
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

				case 'L':	//	line to here
				case 'C':	//	line to here
				{
					//	set command and let code continue to do simple process
					LastCommand = NewCommand;
				}
				break;

				case 'A':	//	arc
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
		if ( LastCommand == 'L' || LastCommand == 'H' || LastCommand == 'V' )
		{
			float2 NewPos = CurrentPosition;

			if  ( LastCommand == 'L' )
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
		else if ( LastCommand == 'C' )
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

			//	add these in the right order (as per tesselator)
			PathPoints.Add( CoordinateToVertexPos( CubicControlPointA ) );
			PathCurves.Add( TLMaths::ContourCurve_Cubic );

			PathPoints.Add( CoordinateToVertexPos( CubicControlPointB ) );
			PathCurves.Add( TLMaths::ContourCurve_Cubic );

			PathPoints.Add( CoordinateToVertexPos( CubicPointPosition ) );
			PathCurves.Add( TLMaths::ContourCurve_On );

			CurrentPosition = CubicPointPosition;
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

	//	get style
	Style TagStyle( pTag->GetProperty("style") );

	//	if a filled vector then create tesselations from the outline
	if ( TagStyle.m_HasFill )
	{
		//	tesselate those bad boys!
		TLMaths::TTessellator* pTessellator = TLMaths::Platform::CreateTessellator( pMesh );
		if ( pTessellator )
		{
			for ( u32 c=0;	c<Contours.GetSize();	c++ )
			{
				pTessellator->AddContour( Contours[c] );
			}
			pTessellator->SetVertexColour( TagStyle.m_FillColour );
			pTessellator->GenerateTessellations( TLMaths::TLTessellator::WindingMode_Odd );
		}
	}

	if ( TagStyle.m_HasStroke )
	{
		CreateMeshLines( pMesh, Contours, TagStyle );
	}
	return TRUE;
}


//--------------------------------------------------------
//	convert rect tag into a polygon
//--------------------------------------------------------
Bool TLFileSys::TFileSimpleVector::ImportRectTag(TPtr<TLAsset::TMesh>& pMesh,TPtr<TXmlTag>& pTag)
{
	//	get rect strings
	const TString* pLeftString = pTag->GetProperty("x");
	const TString* pTopString = pTag->GetProperty("y");
	const TString* pWidthString = pTag->GetProperty("width");
	const TString* pHeighthString = pTag->GetProperty("height");

	if ( !pWidthString || !pHeighthString || !pLeftString || !pTopString )
	{
		if ( TLDebug_Break("Expected x/y/w/h properties of svg for <rect>") )
			return FALSE;

		return TRUE;
	}

	//	get style
	Style TagStyle( pTag->GetProperty("style") );

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

	float3 VertexTopLeft = CoordinateToVertexPos( PointTopLeft );
	float3 VertexTopRight = CoordinateToVertexPos( PointTopRight );
	float3 VertexBottomLeft = CoordinateToVertexPos( PointBottomLeft );
	float3 VertexBottomRight = CoordinateToVertexPos( PointBottomRight );

	//	create polygons
	if ( TagStyle.m_HasFill || TagStyle.m_HasStroke )
	{
		Bool DifferentVertexes = (TagStyle.m_FillColour != TagStyle.m_StrokeColour) && (TagStyle.m_HasFill != TagStyle.m_HasStroke);

		//	add vertexes
		TFixedArray<s32,4> Vertexes;
		Vertexes[0] = pMesh->AddVertex( VertexBottomLeft,	TagStyle.m_HasFill ? TagStyle.m_FillColour : TagStyle.m_StrokeColour );
		Vertexes[1] = pMesh->AddVertex( VertexTopLeft,		TagStyle.m_HasFill ? TagStyle.m_FillColour : TagStyle.m_StrokeColour );
		Vertexes[2] = pMesh->AddVertex( VertexTopRight,	TagStyle.m_HasFill ? TagStyle.m_FillColour : TagStyle.m_StrokeColour );
		Vertexes[3] = pMesh->AddVertex( VertexBottomRight,	TagStyle.m_HasFill ? TagStyle.m_FillColour : TagStyle.m_StrokeColour );

		//	create tristrip
		if ( TagStyle.m_HasFill )
		{
			TLAsset::TMesh::Tristrip* pNewTristrip = pMesh->GetTristrips().AddNew();
			TLAsset::TMesh::Tristrip& NewTristrip = *pNewTristrip;
			NewTristrip.SetSize(4);
			NewTristrip[0] = Vertexes[0];
			NewTristrip[1] = Vertexes[1];
			NewTristrip[2] = Vertexes[3];
			NewTristrip[3] = Vertexes[2];
		}

		//	create line strip
		if ( TagStyle.m_HasStroke )
		{
			//	colours are different so we need to make up new verts
			if ( DifferentVertexes )
			{
				Vertexes[0] = pMesh->AddVertex( VertexBottomLeft,	TagStyle.m_StrokeColour );
				Vertexes[1] = pMesh->AddVertex( VertexTopLeft,		TagStyle.m_StrokeColour );
				Vertexes[2] = pMesh->AddVertex( VertexTopRight,		TagStyle.m_StrokeColour );
				Vertexes[3] = pMesh->AddVertex( VertexBottomRight,	TagStyle.m_StrokeColour );
			}

			TLAsset::TMesh::Line* pNewLinestrip = pMesh->GetLines().AddNew();
			TLAsset::TMesh::Tristrip& NewLinestrip = *pNewLinestrip;
			NewLinestrip.SetSize(4);
			NewLinestrip[0] = Vertexes[0];
			NewLinestrip[1] = Vertexes[1];
			NewLinestrip[2] = Vertexes[2];
			NewLinestrip[3] = Vertexes[3];
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
void TLFileSys::TFileSimpleVector::CreateMeshLines(TPtr<TLAsset::TMesh>& pMesh,TPtrArray<TLMaths::TContour>& Contours,Style& LineStyle)
{
	for ( u32 c=0;	c<Contours.GetSize();	c++ )
	{
		CreateMeshLineStrip( pMesh, Contours[c], LineStyle );
	}
}


//--------------------------------------------------------
//	create line strip on mesh from a contour
//--------------------------------------------------------
void TLFileSys::TFileSimpleVector::CreateMeshLineStrip(TPtr<TLAsset::TMesh>& pMesh,TPtr<TLMaths::TContour>& pContour,Style& LineStyle)
{
	const TArray<float3>& ContourPoints = pContour->GetPoints();

	TLAsset::TMesh::Line* pNewLine = pMesh->GetLines().AddNew();
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
			VertexIndex = pMesh->AddVertex( ContourPoints[p], LineStyle.m_StrokeColour );
		}

		pNewLine->Add( VertexIndex );

		if ( FirstVertex == -1 )
			FirstVertex = VertexIndex;

		//	gr: need to cut up the line
		if ( p < ContourPoints.GetSize() && pNewLine->GetSize() >= TLAsset::g_MaxLineStripSize )
		{
			//	make new line
			pNewLine = pMesh->GetLines().AddNew();
			if ( !pNewLine )
				break;
			
			//	start line at current vert so it doesnt break the line
			pNewLine->Add( VertexIndex );
		}
	}

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
	if ( StyleString.GetLengthWithoutTerminator() == 0 )
		return FALSE;

	//	example:
	//	style="fill:#192bed;fill-opacity:0.44767445;stroke:none;stroke-width:2.26799989000000000;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1"

	//	split string at semi colons
	TArray<TTempString> PropertyStrings;
	StyleString.Split( ';', PropertyStrings );

	//	make up properties
	TKeyArray<TTempString,TString> Properties;
	for ( u32 p=0;	p<PropertyStrings.GetSize();	p++ )
	{
		//	split property
		TFixedArray<TTempString,2> PropertyParts(0);
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
		pFillAlpha->GetFloat( m_FillColour.GetAlpha() );
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
		pStrokeAlpha->GetFloat( m_StrokeColour.GetAlpha() );
	}

	return TRUE;
}
	      


//--------------------------------------------------------
//	extracts a colour from a string. returns FALSE if no colour or failed to parse
//--------------------------------------------------------
Bool TLFileSys::TFileSimpleVector::Style::SetColourFromString(TColour& Colour,const TString& ColourString)
{
	u32 Length = ColourString.GetLengthWithoutTerminator();

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


