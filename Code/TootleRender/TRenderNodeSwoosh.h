/*------------------------------------------------------

	Render node that renders a bezier line that can taper, wave etc

	todo:
		-	colour controllers (start, end, arbiritry colour points along)
		-	UV mapping (one long/projected mapping, repeat etc)
		-	proper swoosh dragging by moving the start pos

-------------------------------------------------------*/
#pragma once

#include "TRenderNode.h"
#include "TRenderNodeDebugMesh.h"



namespace TLRender
{
	class TRenderNodeSwoosh;
}


//----------------------------------------------------
//	gr: this is a 2D swoosh!
//	for a 3D implementation we should specify the correct axis and then
//	make it align to view or somethig
//----------------------------------------------------
class TLRender::TRenderNodeSwoosh : public TLRender::TRenderNodeDebugMesh
{
public:
	class TSwooshPoint
	{
	public:
		TSwooshPoint(Bool IsControlPoint=FALSE) :
			m_IsControlPoint	( IsControlPoint ),
			m_Width				( 1.f ),
			m_Offset			( 0.f )
		{
		}

	public:
		float3		m_Position;			//	
		float		m_Width;			//	width of the swoosh at this point - when changed reflected on render
		float		m_Offset;			//	+/- outset addition so we push the point out like a vertex shader. used for easy wobble generation
		Bool		m_IsControlPoint;	//	if true this point is manually set, otherwise it's one just positioned by bezier generation
		Type2<u16>	m_Vertexes;			//	west and east sides of the line
	};

public:
	TRenderNodeSwoosh(TRefRef NodeRef,TRefRef TypeRef);

	virtual void			Initialise(TLMessaging::TMessage& Message);

protected:
	virtual void			ProcessMessage(TLMessaging::TMessage& Message);
	virtual Bool			Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList);
	
	FORCEINLINE float		GetWidth(float AlongLine)											{	return m_Tapered ? TLMaths::Interp(m_Width,2.f,AlongLine) : m_Width;	}
	void					InitControlPoints(const TArray<float3>& Positions);					//	init swoosh points if not created or update points
	void					SetControlPoints(const TArray<float3>& Positions);					//	init swoosh points if not created or update points
	void					SetOffsets(const TArray<float>& Offsets);							//	init swoosh points if not created or update points

	FORCEINLINE void		SetControlPointPos(u32 ControlPointIndex,const float3& Position);
	FORCEINLINE void		SetControlPointOffset(u32 ControlPointIndex,const float& Outset);
	FORCEINLINE void		OnControlLineChanged()												{	m_LinePointsValid = FALSE;	}	//	the control line has changed, mark for re-generation
	void					UpdateLinePoints();		//	update the bezier points and update mesh
	
protected:
	float					m_Width;					//	line width; note runtime changes have no immediate effect
	Bool					m_Tapered;					//	line tapers to nothing at end; note runtime changes have no immediate effect
	u32						m_BezierSteps;				//	how many steps between control points we generate for the beziers
	TArray<TSwooshPoint>	m_LinePoints;				//	
	TArray<u32>				m_ControlPoints;			//	index in linepoints of the control points
	Bool					m_LinePointsValid;			//	if true the line points/bezier needs recalculating and mesh needs re-generating

	TArray<float3*>			m_LinePointPositions;		//	for faster line strip generation
	TArray<float*>			m_LinePointWidths;			//	for faster line strip generation
	TArray<float*>			m_LinePointOffsets;			//	for faster line strip generation
};



FORCEINLINE void TLRender::TRenderNodeSwoosh::SetControlPointPos(u32 ControlPointIndex,const float3& Position)	
{	
	u32 LinePointIndex = m_ControlPoints[ControlPointIndex];
	m_LinePoints[LinePointIndex].m_Position = Position;	
	OnControlLineChanged();
}


FORCEINLINE void TLRender::TRenderNodeSwoosh::SetControlPointOffset(u32 ControlPointIndex,const float& Offset)	
{	
	u32 LinePointIndex = m_ControlPoints[ControlPointIndex];
	m_LinePoints[LinePointIndex].m_Offset = Offset;	
	OnControlLineChanged();
}
