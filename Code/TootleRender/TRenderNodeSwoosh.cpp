#include "TRenderNodeSwoosh.h"






TLRender::TRenderNodeSwoosh::TRenderNodeSwoosh(TRefRef NodeRef,TRefRef TypeRef) :
	TRenderNodeDebugMesh	( NodeRef, TypeRef ),
	m_Width					( 1.f ),
	m_MinWidth				( 0.f ),
	m_Tapered				( FALSE ),
	m_LinePointsValid		( FALSE ),
	m_BezierSteps			( 0 )
{
}


//----------------------------------------------------
//	
//----------------------------------------------------
void TLRender::TRenderNodeSwoosh::Initialise(TLMessaging::TMessage& Message)
{
	//	do inherited init
	TLRender::TRenderNodeDebugMesh::Initialise( Message );

	//	read line properties
	Message.ImportData("Width", m_Width );
	Message.ImportData("MinWidth", m_MinWidth );
	Message.ImportData("Tapered", m_Tapered );
	Message.ImportData("BezSteps", m_BezierSteps );
	
	Bool InitControlPoints = FALSE;

	//	read initial line positions
	TFixedArray<float3,100> Positions;
	if ( Message.ImportArrays("Positions", Positions ) )
		SetControlPoints( Positions );

	TFixedArray<float,100> Offsets;
	if ( Message.ImportArrays("Offsets", Offsets ) )
		SetOffsets( Offsets );

}


//----------------------------------------------------
//	
//----------------------------------------------------
void TLRender::TRenderNodeSwoosh::ProcessMessage(TLMessaging::TMessage& Message)
{
	TLRender::TRenderNodeDebugMesh::ProcessMessage( Message );
}


//----------------------------------------------------
//	
//----------------------------------------------------
Bool TLRender::TRenderNodeSwoosh::Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList)
{
	//	nothing to draw
	if ( !m_LinePoints.GetSize() )
		return FALSE;

	//	update line as necessary
	if ( !m_LinePointsValid )
		UpdateLinePoints();
	
	//	render if okay
	return m_LinePointsValid;
}


//----------------------------------------------------
//	init swoosh points if not created or update points
//----------------------------------------------------
void TLRender::TRenderNodeSwoosh::SetControlPoints(const TArray<float3>& Positions)
{
	if ( m_LinePoints.GetSize() != Positions.GetSize() )
	{
		InitControlPoints( Positions );
		return;
	}

	//	set point positions
	for ( u32 i=0;	i<Positions.GetSize();	i++ )
		SetControlPointPos( i, Positions[i] );

	//	bezier/mesh out of date
	OnControlLineChanged();
}


//----------------------------------------------------
//	
//----------------------------------------------------
void TLRender::TRenderNodeSwoosh::SetOffsets(const TArray<float>& Offsets)
{
	//	set point positions
	for ( u32 i=0;	i<Offsets.GetSize();	i++ )
		SetControlPointOffset( i, Offsets[i] );

	//	bezier/mesh out of date
	OnControlLineChanged();
}

//----------------------------------------------------
//	init swoosh points if not created or update points
//----------------------------------------------------
void TLRender::TRenderNodeSwoosh::InitControlPoints(const TArray<float3>& Positions)
{
	TLAsset::TMesh* pMesh = GetMeshAsset();

	//	cleanup
	m_LinePoints.Empty();
	m_ControlPoints.Empty();
	if ( pMesh )
		pMesh->Empty();
	m_LinePointPositions.Empty();
	m_LinePointWidths.Empty();
	m_LinePointOffsets.Empty();

	//	check params
	if ( !pMesh )
		return;

	if ( Positions.GetSize() < 2 )
	{
		TLDebug_Break("Need at least 2 points for a swoosh!");
		return;
	}

	//	pre-alloc swooshpoints so that pointers will be valid
	u32 TotalPoints = Positions.GetSize() + ( (Positions.GetSize()-1) * m_BezierSteps );
	m_LinePoints.SetAllocSize( TotalPoints );

	s32 PrevVertexOutside = -1;
	s32 PrevVertexInside = -1;

	//	generate new points
	for ( u32 i=0;	i<Positions.GetSize();	i++ )
	{
		float StepAlongLine = (float)m_LinePoints.GetSize() / (float)(TotalPoints-1);

		TSwooshPoint SwooshPoint( TRUE );
		SwooshPoint.m_Position = Positions[i];
		SwooshPoint.m_Vertexes.x = pMesh->AddVertex( Positions[i] );
		SwooshPoint.m_Vertexes.y = pMesh->AddVertex( Positions[i] );
		SwooshPoint.m_Width = GetWidth( StepAlongLine );
		
		//	build polygons
		if ( PrevVertexOutside != -1 )
			pMesh->GenerateQuad( PrevVertexOutside, SwooshPoint.m_Vertexes.x, SwooshPoint.m_Vertexes.y, PrevVertexInside );

		//	store "previous" vertexes
		PrevVertexOutside = SwooshPoint.m_Vertexes.x;
		PrevVertexInside = SwooshPoint.m_Vertexes.y;

		//	add to lists
		s32 Index = m_LinePoints.Add( SwooshPoint );
		m_LinePointPositions.Add( &m_LinePoints[Index].m_Position );
		m_LinePointWidths.Add( &m_LinePoints[Index].m_Width );
		m_LinePointOffsets.Add( &m_LinePoints[Index].m_Offset );
		m_ControlPoints.Add( Index );

		//	generate bezier step entries
		for ( u32 b=0;	b<m_BezierSteps;	b++ )
		{
			float StepAlongLine = (float)m_LinePoints.GetSize() / (float)(TotalPoints-1);

			TSwooshPoint StepSwooshPoint( FALSE );
			StepSwooshPoint.m_Position = Positions[i];
			StepSwooshPoint.m_Vertexes.x = pMesh->AddVertex( Positions[i] );
			StepSwooshPoint.m_Vertexes.y = pMesh->AddVertex( Positions[i] );
			StepSwooshPoint.m_Width = GetWidth( StepAlongLine );
			
			//	build polygons
			if ( PrevVertexOutside != -1 )
				pMesh->GenerateQuad( PrevVertexOutside, StepSwooshPoint.m_Vertexes.x, StepSwooshPoint.m_Vertexes.y, PrevVertexInside );

			//	store "previous" vertexes
			PrevVertexOutside = StepSwooshPoint.m_Vertexes.x;
			PrevVertexInside = StepSwooshPoint.m_Vertexes.y;

			//	add to lists
			s32 Index = m_LinePoints.Add( StepSwooshPoint );
			m_LinePointPositions.Add( &m_LinePoints[Index].m_Position );
			m_LinePointWidths.Add( &m_LinePoints[Index].m_Width );
			m_LinePointOffsets.Add( &m_LinePoints[Index].m_Offset );
		}
	}

	//	bezier/mesh out of date
	OnControlLineChanged();
}


//----------------------------------------------------
//	update the bezier points and update mesh
//----------------------------------------------------
void TLRender::TRenderNodeSwoosh::UpdateLinePoints()
{
	if ( m_LinePointsValid )
	{
		TLDebug_Break("Redundant line update?");
		return;
	}

	TLAsset::TMesh* pMesh = GetMeshAsset();
	if ( !pMesh )
	{
		TLDebug_Break("Mesh missing for swoosh, re-init here?");
		return;
	}

	//	todo: update bezier points between the control points
	

	//	generate new linestrip
	TFixedArray<float3,100> OutsideLineStrip;
	TFixedArray<float3,100> InsideLineStrip;
	TLMaths::ExpandLineStrip( m_LinePointPositions, m_LinePointWidths, m_LinePointOffsets, OutsideLineStrip, InsideLineStrip );

	//	update mesh
	TArray<float3>& Vertexes = pMesh->GetVertexes();

	for ( u32 i=0;	i<m_LinePoints.GetSize();	i++ )
	{
		Vertexes[ m_LinePoints[i].m_Vertexes.x ] = OutsideLineStrip[i];
		Vertexes[ m_LinePoints[i].m_Vertexes.y ] = InsideLineStrip[i];
	}

	//	all up to date
	m_LinePointsValid = TRUE;
}

	
