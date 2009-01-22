/*------------------------------------------------------
 
	Tesselation routines to turn an outline(points) into 
	polygons in a mesh - platform specific atm, a generic
	tesselation will be done at some point
 
 -------------------------------------------------------*/
#pragma once


#include <TootleCore/TLTypes.h>
#include <TootleCore/TPtrArray.h>
#include <TootleCore/TLMaths.h>
#include <TootleCore/TColour.h>



namespace TLAsset
{
	class TMesh;
};


namespace TLMaths
{
	enum TContourCurve
	{
		ContourCurve_On,		//	on/straight line
		
		ContourCurve_Cubic,		//	cubic bezier curve. A cubic BÈzier segment is defined by a start point, an end point, and two control points.
		//	if we find this, we expect an On before hand, another cubic to follow, then another On point after that
		
		ContourCurve_Quadratic,	//	Quadratic Bezier. A quadratic BÈzier segment is def	ined by a start point, an end point, and one control point.
		//ContourCurve_QuadraticPointA,	//	quadratic control point
		
		ContourCurve_Conic,		//	curve defined from 2 points and the midpoint between them (quadratic, but auto midpoint?)
		ContourCurve_Arc,		//	elliptical arc - "An elliptical arc segment draws a segment of an ellipse."
	};

	class TTessellator;			//	tesselation class
	class TSimpleTessellator;	//	grahams new simple tesselator that just creates triangles
	class TContour;				//	outline of a shape

	u32							GetBezierStepCount(float LineDistance);			//	based on a distance, work out how many bezier steps to produce
	
	namespace Platform 
	{
		extern TTessellator*	CreateTessellator(TPtr<TLAsset::TMesh>& pMesh);	//	create platform specific tessellator
	}
	
	
	namespace TLTessellator
	{
		//	http://pheatt.emporia.edu/courses/2003/cs410s03/hand19/windingrules.gif
		enum TWindingMode
		{
			WindingMode_Odd,
			WindingMode_NonZero,
			WindingMode_Positive,
		};
	};


	//	externs
	class TLine;
	class TLine2D;
}




//	FTContour class is a container of points that describe a vector font
//	outline. It is used as a container for the output of the bezier curve
//	evaluator in FTVectoriser.
class TLMaths::TContour
{
public:
	TContour(const TContour& Contour);
	TContour(const TArray<float3>& Contours,const TArray<TContourCurve>* pContourCurves);
	
	const TArray<float3>&	GetPoints() const				{	return m_Points;	}
	const float3&			Point(u32 Index) const			{	return m_Points[Index];	}

	//void					SetClockwise(Bool ClockWise)	{	m_Clockwise = ClockWise ? SyncTrue : SyncFalse;	}
	Bool					IsClockwise() const				{	return m_IsClockwise;	}
	void					SetParity(u32 parity);
	
	void					Grow(float OutsetDistance);		//	scale down the shape using outset calculations
	void					Shrink(float OutsetDistance);		//	scale down the shape using outset calculations
	
	void					GetEdgeLines(TArray<TLMaths::TLine>& EdgeLines) const;	//	get all the lines around the edge of the contour
	void					GetEdgeLines(TArray<TLMaths::TLine2D>& EdgeLines) const;	//	get all the lines around the edge of the contour
	Bool					HasIntersections() const;		//	check to make sure any lines along the contour dont intersect each other (a self intersecting polygon). returns TRUE if they do

	float					GetArea() const;				//	get area of the shape

private:
	void					evaluateQuadraticCurve(const float3&,const float3&,const float3&);
	void					evaluateCubicCurve(const float3&,const float3&,const float3&,const float3&);
	float					NormVector(const float3 &v);
	void					RotationMatrix(const float3 &a, const float3 &b, float *matRot, float *invRot);
	void					MultMatrixVect(float *mat, float3 &v);
	void					ComputeBisec(float3 &v);	//	Compute the vector bisecting from a vector 'v' and a distance 'd'

	static void				OutsetPoint(u32 Index,float Distance,TArray<float3>& NewPoints,const TArray<float3>& OriginalPoints,Bool ContourIsClockwise);			//	move point in/out with outset
	static float3			ComputeOutsetPoint(const float3& a,const float3& b,const float3& c,float Distance,Bool ContourIsClockwise);	//	Compute the outset point coordinates
	
private:
	TArray<float3>			m_Points;
	Bool					m_IsClockwise;
};


class TLMaths::TTessellator
{
public:
	TTessellator(TPtr<TLAsset::TMesh>& pMesh);
		
	void			AddContour(TPtr<TLMaths::TContour>& pContour)	{	m_Contours.Add( pContour );	}
		
	virtual Bool	GenerateTessellations(TLMaths::TLTessellator::TWindingMode WindingMode,float zNormal=-1.f)	{	return FALSE;	}			//	generate new polygons into this mesh
		
	void			SetVertexColour(const TColour& Colour)		{	m_VertexColour = Colour;	m_VertexColourValid = TRUE;	}

public:
	TPtrArray<TLMaths::TContour>	m_Contours;
	TPtr<TLAsset::TMesh>			m_pMesh;
	TColour							m_VertexColour;			//	vertexes get coloured to this
	Bool							m_VertexColourValid;	//	polygon HAS any colour
};







class TLMaths::TSimpleTessellator : public TLMaths::TTessellator
{
public:
	TSimpleTessellator(TPtr<TLAsset::TMesh>& pMesh);

	virtual Bool	GenerateTessellations(TLMaths::TLTessellator::TWindingMode WindingMode,float zNormal=-1.f);			//	generate new polygons into this mesh
};

