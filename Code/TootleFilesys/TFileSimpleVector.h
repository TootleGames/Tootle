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

	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType);
	virtual void		GetSupportedExportAssetTypes(TArray<TRef>& SupportedTypes) const	{	SupportedTypes << TRef_Static4(M,e,s,h);	}

protected:
	Bool				ImportMesh(TPtr<TLAsset::TMesh>& pMesh,TXmlTag& Tag);			//	generate mesh data from this SVG tag
	Bool				ImportPolygonTag(TPtr<TLAsset::TMesh>& pMesh,TXmlTag& Tag);	//	convert Polygon tag to mesh info and add to mesh
	Bool				ImportPathTag(TPtr<TLAsset::TMesh>& pMesh,TXmlTag& Tag);		//	convert path tag to mesh info and add to mesh
	Bool				ImportRectTag(TPtr<TLAsset::TMesh>& pMesh,TXmlTag& Tag);		//	convert rect tag to mesh info and add to mesh

	float3				CoordinateToVertexPos(const float2& Coordinate);			//	move & scale coordinate and convert to float3

	void				CreateMeshLines(TLAsset::TMesh& Mesh,TPtrArray<TLMaths::TContour>& Contours,Style& LineStyle);	//	create line strips on mesh from a list of contours
	void				CreateMeshLineStrip(TLAsset::TMesh& Mesh,TLMaths::TContour& Contour,Style& LineStyle);	//	create line strip on mesh from a contour

	Bool				IsTagDatum(TXmlTag& Tag,TRef& DatumRef,TRef& ShapeType,Bool& IsJustDatum);	//	check if tag marked as a datum

protected:
	float				m_SvgLayerZIncrement;	//	every layer we increase the Z position of meshes we generate to avoid Z fighting, this is how much we increment by
	float3				m_SvgPointMove;			//	when parsing xml we move all points by this amount
	//float3				m_SvgPointScale;		//	when parsing xml we scale all points by this
	float				m_SvgPointScale;		//	when parsing xml we scale all points by this

	Bool				m_VertexColoursEnabled;	//	adding TootleVertexColours="FALSE" to the <svg> tag disables generation of colours on vertexes (so the mesh can be coloured by the render node)
	Bool				m_ProjectUVsEnabled;	//	adding TootleProjectUVs="TRUE" to the <svg> tag enables projection UV mapping where the UV's of the vertexes are set relative to their position in their bounds. (most top-left vertex would be 0,0 uv)
	
};

