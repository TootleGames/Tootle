#include "TLine.h"



TLMaths::TLine::TLine(const float3& Start,const float3& End) : 
	m_Start	( Start ),
	m_End	( End )
{
}



	
//-----------------------------------------------------------
//	find the point along the line closest to Position
//-----------------------------------------------------------
float3 TLMaths::TLine::GetNearestPoint(const float3& Position) const
{
	float3 LineDir = GetDirection();
	float LineDirDotProduct = LineDir.DotProduct(LineDir);
	
	//	avoid div by zero
	if ( LineDirDotProduct == 0.f )
		return GetStart();

	float3 Dist = Position - GetStart();

	float LineDirDotProductDist = LineDir.DotProduct(Dist);

	float PointAlongLine = LineDirDotProductDist / LineDirDotProduct;

	if ( PointAlongLine <= 0.f )
		return GetStart();

	if ( PointAlongLine >= 1.f )
		return GetEnd();

	return GetStart() + LineDir * PointAlongLine;
}

//-----------------------------------------------------------
//	gr: this needs testing
//-----------------------------------------------------------
float TLMaths::TLine::GetDistanceSq(const float3& Position) const
{
	float3 vNearestPoint = GetNearestPoint(Position);

	// Vector between the position and the nearest point on the line
	float3 vAB = Position - vNearestPoint;

	// Use pythaguras to work out distance between the two points
	float fDist = (vAB.x * vAB.x) + (vAB.y * vAB.y) + (vAB.z * vAB.z); 

	return fDist;
}





TLMaths::TLine2D::TLine2D(const float2& Start,const float2& End) : 
	m_Start	( Start ),
	m_End	( End )
{
}



	
//-----------------------------------------------------------
//	find the point along the line closest to Position
//-----------------------------------------------------------
float2 TLMaths::TLine2D::GetNearestPoint(const float2& Position) const
{
	float2 LineDir = GetDirection();
	float LineDirDotProduct = LineDir.DotProduct(LineDir);
	
	//	avoid div by zero
	if ( LineDirDotProduct == 0.f )
		return GetStart();

	float2 Dist = Position - GetStart();

	float LineDirDotProductDist = LineDir.DotProduct(Dist);

	float PointAlongLine = LineDirDotProductDist / LineDirDotProduct;

	if ( PointAlongLine <= 0.f )
		return GetStart();

	if ( PointAlongLine >= 1.f )
		return GetEnd();

	return GetStart() + LineDir * PointAlongLine;
}

	
//-----------------------------------------------------------
//	find the point along the line closest to Position
//-----------------------------------------------------------
float2 TLMaths::TLine2D::GetNearestPoint(const float3& Position) const
{
	float2 LineDir = GetDirection();
	float LineDirDotProduct = LineDir.DotProduct(LineDir);
	
	//	avoid div by zero
	if ( LineDirDotProduct == 0.f )
		return GetStart();

	float2 Dist(Position.x, Position.y);
	Dist -= GetStart();

	float LineDirDotProductDist = LineDir.DotProduct(Dist);

	float PointAlongLine = LineDirDotProductDist / LineDirDotProduct;

	if ( PointAlongLine <= 0.f )
		return GetStart();

	if ( PointAlongLine >= 1.f )
		return GetEnd();

	return GetStart() + LineDir * PointAlongLine;
}

//-----------------------------------------------------------
//	gr: this needs testing
//-----------------------------------------------------------
float TLMaths::TLine2D::GetDistanceSq(const float2& Position) const
{
	float2 vNearestPoint = GetNearestPoint(Position);

	// Vector between the position and the nearest point on the line
	float2 vAB = Position - vNearestPoint;

	// Use pythaguras to work out distance between the two points
	//float fDist = vAB.DotProduct();
	float fDist = (vAB.x * vAB.x) + (vAB.y * vAB.y); 

	return fDist;
}



