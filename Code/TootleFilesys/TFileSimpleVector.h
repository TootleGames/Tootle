/*------------------------------------------------------
	
	SVG file format (simple vector graphics)
	Converts to mesh

-------------------------------------------------------*/
#pragma once

#include "TFile.h"
#include "TFileXml.h"
#include <TootleCore/TColour.h>


namespace TLFileSys
{
	class TFileSimpleVector;
};

namespace TLAsset
{
	class TMesh;
}

namespace TLMaths
{
	class TContour;
}


//---------------------------------------------------------
//	SVG xml file that converts to a mesh
//---------------------------------------------------------
class TLFileSys::TFileSimpleVector : public TLFileSys::TFileXml
{
protected:
	class Style
	{
	public:
		Style(const TString* pStyleString=NULL);		

		Bool		Parse(const TString& StyleString);		//	parse string and get style elements
		Bool		Parse(const TString* pStyleString)		{	return pStyleString ? Parse( *pStyleString ) : FALSE;	}

		Bool		SetColourFromString(TColour& Colour,const TString& ColourString);		//	extracts a colour from a string. returns FALSE if no colour or failed to parse

	public:
		Bool		m_HasFill;					//	has fill
		TColour		m_FillColour;				//	fill colour
		Bool		m_HasStroke;				//	has an outline
		TColour		m_StrokeColour;				//	outline colour
		float		m_StrokeWidth;				//	thickness of outline
	};

public:
	TFileSimpleVector(TRefRef FileRef,TRefRef FileTypeRef);

	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported);			//	import the XML and convert from SVG to mesh

protected:
	Bool				ImportMesh(TPtr<TLAsset::TMesh>& pMesh,TPtr<TXmlTag>& pTag);		//	generate mesh data from this SVG tag
	Bool				ImportPolygonTag(TPtr<TLAsset::TMesh>& pMesh,TPtr<TXmlTag>& pTag);	//	convert Polygon tag to mesh info and add to mesh
	Bool				ImportPathTag(TPtr<TLAsset::TMesh>& pMesh,TPtr<TXmlTag>& pTag);		//	convert path tag to mesh info and add to mesh
	Bool				ImportRectTag(TPtr<TLAsset::TMesh>& pMesh,TPtr<TXmlTag>& pTag);		//	convert rect tag to mesh info and add to mesh

	float3				CoordinateToVertexPos(const float2& Coordinate);					//	move & scale coordinate and convert to float3

	void				CreateMeshLines(TPtr<TLAsset::TMesh>& pMesh,TPtrArray<TLMaths::TContour>& Contours,Style& LineStyle);	//	create line strips on mesh from a list of contours
	void				CreateMeshLineStrip(TPtr<TLAsset::TMesh>& pMesh,TPtr<TLMaths::TContour>& pContour,Style& LineStyle);	//	create line strip on mesh from a contour

protected:
	float				m_SvgLayerZIncrement;	//	every layer we increase the Z position of meshes we generate to avoid Z fighting, this is how much we increment by
	float3				m_SvgPointMove;			//	when parsing xml we move all points by this amount
	float3				m_SvgPointScale;		//	when parsing xml we scale all points by this
};

