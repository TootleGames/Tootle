#include "TCollisionShape.h"
#include "TPhysicsGraph.h"
#include <TootleAsset/TAsset.h>
#include <TootleAsset/TMesh.h>
#include <TootleMaths/TShape.h>


//	just for first project
#define FORCE_2D_COLLISION

#define LINEWIDTH(x)	0.f
//#define LINEWIDTH(x)	x


namespace TLPhysics
{
	Bool	GetIntersection_SphereLine(const TLMaths::TSphere& Sphere,const TLMaths::TLine& Line,float LineWidth,TLMaths::TIntersection& NodeAIntersection,TLMaths::TIntersection& NodeBIntersection);
	Bool	GetIntersection_SphereLineStrip(const TLMaths::TSphere& Sphere,const TArray<float3>& LineStrip,float LineWidth,TLMaths::TIntersection& NodeAIntersection,TLMaths::TIntersection& NodeBIntersection);
	Bool	GetIntersection_SphereLineStrip(const TLMaths::TSphere& Sphere,const TArray<float2>& LineStrip,float LineWidth,TLMaths::TIntersection& NodeAIntersection,TLMaths::TIntersection& NodeBIntersection);
	Bool	GetIntersection_SphereTriangle(const TLMaths::TSphere& Sphere,const TFixedArray<float3,3>& Triangle,TLMaths::TIntersection& NodeAIntersection,TLMaths::TIntersection& NodeBIntersection);
}




//----------------------------------------------------------
//	get intersection between sphere and line
//----------------------------------------------------------
Bool TLPhysics::GetIntersection_SphereLine(const TLMaths::TSphere& Sphere,const TLMaths::TLine& Line,float LineWidth,TLMaths::TIntersection& NodeAIntersection,TLMaths::TIntersection& NodeBIntersection)
{
	//	get the intersection point on the line...
	float3 IntersectionOnLine = Line.GetNearestPoint( Sphere.GetPos() );
	float3 DeltaToLine( IntersectionOnLine - Sphere.GetPos() );

	//	too far away?
	float Distance = DeltaToLine.LengthSq();
	float Radius = Sphere.GetRadius() + LINEWIDTH(LineWidth);

	if ( Distance > (Radius*Radius) )
		return FALSE;

	float3 DeltaToLineNormal = DeltaToLine;
	DeltaToLineNormal.Normalise( Radius );

	NodeAIntersection.m_Intersection = Sphere.GetPos() + DeltaToLineNormal;
	NodeBIntersection.m_Intersection = IntersectionOnLine;
	TLDebug_CheckFloat( NodeAIntersection.m_Intersection );
	TLDebug_CheckFloat( NodeBIntersection.m_Intersection );

	//	get normal of line
	NodeAIntersection.m_Normal = Line.GetDirectionNormal().CrossProduct( float3(0,0,1) );
	NodeAIntersection.m_HasNormal = TLDebug_CheckFloat( NodeAIntersection.m_Normal );

	return TRUE;
}



//----------------------------------------------------------
//	get intersection between sphere and line strip
//----------------------------------------------------------
Bool TLPhysics::GetIntersection_SphereLineStrip(const TLMaths::TSphere& Sphere,const TArray<float3>& LineStrip,float LineWidth,TLMaths::TIntersection& NodeAIntersection,TLMaths::TIntersection& NodeBIntersection)
{
	//	too short
	if ( LineStrip.GetSize() < 2 )
		return FALSE;

	//	just a single line, quicker...
	if ( LineStrip.GetSize() == 2 )
	{
		TLMaths::TLine Line;
		Line.Set( LineStrip[0], LineStrip[1] );
		return GetIntersection_SphereLine( Sphere, Line, LineWidth, NodeAIntersection, NodeBIntersection );
	}

	u32 IntersectionCount = 0;

	for ( u32 i=1;	i<LineStrip.GetSize();	i++ )
	{
		TLMaths::TLine Line;
		Line.Set( LineStrip[i-1], LineStrip[i] );

		//	tiny line!
		if ( Line.GetDirection().LengthSq() < TLMaths_NearZero )
		{
			TLDebug_Break("Tiny line");
			continue;
		}

		//	get the intersection point on the line...
		float3 IntersectionOnLine = Line.GetNearestPoint( Sphere.GetPos() );
		float3 DeltaToLine( IntersectionOnLine - Sphere.GetPos() );

		//	too far away?
		float Distance = DeltaToLine.LengthSq();
		float Radius = Sphere.GetRadius() + LINEWIDTH(LineWidth);

		if ( Distance > (Radius*Radius) )
			continue;

		float3 DeltaToLineNormal = DeltaToLine;
		DeltaToLineNormal.Normalise( Radius );
		float3 IntersectionOnSphere = Sphere.GetPos() + DeltaToLineNormal;

		//	first intersection, just set values
		if ( IntersectionCount == 0 )
		{
			NodeAIntersection.m_Intersection = IntersectionOnSphere;
			NodeBIntersection.m_Intersection = IntersectionOnLine;
			TLDebug_CheckFloat( NodeAIntersection.m_Intersection );
			TLDebug_CheckFloat( NodeBIntersection.m_Intersection );

			//	get normal of line
			NodeAIntersection.m_Normal = Line.GetDirectionNormal().CrossProduct( float3(0,0,1) );
			NodeAIntersection.m_HasNormal = TLDebug_CheckFloat( NodeAIntersection.m_Normal );
		}
		else
		{
			//	already intersected... merge intersections
			//	todo: probably need something more intelligent
			//		will probably replace with multiple-collision/intersection OnCollision's
			NodeAIntersection.m_Intersection += IntersectionOnSphere;
			NodeAIntersection.m_Intersection *= 0.5f;

			NodeBIntersection.m_Intersection += IntersectionOnLine;
			NodeBIntersection.m_Intersection *= 0.5f;

			TLDebug_CheckFloat( NodeAIntersection.m_Intersection );
			TLDebug_CheckFloat( NodeBIntersection.m_Intersection );

			//	get normal of line
			NodeAIntersection.m_Normal += Line.GetDirectionNormal().CrossProduct( float3(0,0,1) );
			NodeAIntersection.m_HasNormal = TLDebug_CheckFloat( NodeAIntersection.m_Normal );
		}

		IntersectionCount++;
	}

	if ( NodeAIntersection.m_HasNormal )
	{
		NodeAIntersection.m_Normal.Normalise();
		NodeAIntersection.m_HasNormal = TLDebug_CheckFloat( NodeAIntersection.m_Normal );
	}
	

	//	no intersections
	if ( IntersectionCount == 0 )
		return FALSE;

	return TRUE;
}


//----------------------------------------------------------
//	get intersection between sphere and line strip
//----------------------------------------------------------
Bool TLPhysics::GetIntersection_SphereLineStrip(const TLMaths::TSphere& Sphere,const TArray<float2>& LineStrip,float LineWidth,TLMaths::TIntersection& NodeAIntersection,TLMaths::TIntersection& NodeBIntersection)
{
	//	too short
	if ( LineStrip.GetSize() < 2 )
		return FALSE;

	//	just a single line, quicker...
	if ( LineStrip.GetSize() == 2 )
	{
		TLMaths::TLine Line;
		Line.Set( LineStrip[0], LineStrip[1] );
		return GetIntersection_SphereLine( Sphere, Line, LineWidth, NodeAIntersection, NodeBIntersection );
	}

	u32 IntersectionCount = 0;
	float2 SpherePos2( Sphere.GetPos().x, Sphere.GetPos().y );

	for ( u32 i=1;	i<LineStrip.GetSize();	i++ )
	{
		TLMaths::TLine2D Line;
		Line.Set( LineStrip[i-1], LineStrip[i] );

		//	tiny line!
		if ( Line.GetDirection().LengthSq() < TLMaths_NearZero )
		{
			TLDebug_Print("Tiny line in linestrip");
			continue;
		}

		//	get the intersection point on the line...
		float2 IntersectionOnLine = Line.GetNearestPoint( SpherePos2 );
		float2 DeltaToLine( IntersectionOnLine - SpherePos2 );

		//	too far away?
		float Distance = DeltaToLine.LengthSq();
		float Radius = Sphere.GetRadius() + LINEWIDTH(LineWidth);

		if ( Distance > (Radius*Radius) )
			continue;

		float2 DeltaToLineNormal = DeltaToLine;
		DeltaToLineNormal.Normalise( Radius );
		float3 IntersectionOnSphere( SpherePos2.x + DeltaToLineNormal.x, SpherePos2.y + DeltaToLineNormal.y, 0.f );
		float3 LineDirNormal( Line.GetDirection(0.f).Normal() );

		//	first intersection, just set values
		if ( IntersectionCount == 0 )
		{
			NodeAIntersection.m_Intersection = IntersectionOnSphere;
			NodeBIntersection.m_Intersection = IntersectionOnLine;
			TLDebug_CheckFloat( NodeAIntersection.m_Intersection );
			TLDebug_CheckFloat( NodeBIntersection.m_Intersection );

			//	get normal of line
			NodeAIntersection.m_Normal = LineDirNormal.CrossProduct( float3(0,0,1) );
			NodeAIntersection.m_HasNormal = TLDebug_CheckFloat( NodeAIntersection.m_Normal );
		}
		else
		{
			//	already intersected... merge intersections
			//	todo: probably need something more intelligent
			//		will probably replace with multiple-collision/intersection OnCollision's
			NodeAIntersection.m_Intersection += IntersectionOnSphere;
			NodeAIntersection.m_Intersection *= 0.5f;

			NodeBIntersection.m_Intersection += IntersectionOnLine;
			NodeBIntersection.m_Intersection *= 0.5f;

			TLDebug_CheckFloat( NodeAIntersection.m_Intersection );
			TLDebug_CheckFloat( NodeBIntersection.m_Intersection );

			//	get normal of line
			NodeAIntersection.m_Normal += LineDirNormal.CrossProduct( float3(0,0,1) );
			NodeAIntersection.m_HasNormal = TLDebug_CheckFloat( NodeAIntersection.m_Normal );
		}

		IntersectionCount++;
	}

	if ( NodeAIntersection.m_HasNormal )
	{
		NodeAIntersection.m_Normal.Normalise();
		NodeAIntersection.m_HasNormal = TLDebug_CheckFloat( NodeAIntersection.m_Normal );
	}
	

	//	no intersections
	if ( IntersectionCount == 0 )
		return FALSE;

	return TRUE;
}

