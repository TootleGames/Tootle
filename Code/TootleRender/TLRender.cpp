#include "TLRender.h"




namespace TLRender
{
	const TColour	g_DebugMissingColoursColour( 1.f, 0.f, 1.f );	//	pink
	u32				g_PolyCount = 0;
	u32				g_VertexCount = 0;
}


//-----------------------------------------------------------
//	draw vertexes as points
//-----------------------------------------------------------
void TLRender::Opengl::DrawPrimitivePoints(const TArray<float3>* pVertexes)
{
	//	have a static array of indexes and grow it as required
	static TArray<u16> g_AllPoints;

	//	grow index array as required
	if ( g_AllPoints.GetSize() < pVertexes->GetSize() )
	{
		u32 OldSize = g_AllPoints.GetSize();
		
		//	alloc points
		g_AllPoints.SetSize( pVertexes->GetSize() );

		//	set new entries
		for ( u32 i=OldSize;	i<g_AllPoints.GetSize();	i++ )
			g_AllPoints[i] = i;
	}

	//	draw points
	Platform::DrawPrimitives( Platform::GetPrimTypePoint(), pVertexes->GetSize(), g_AllPoints.GetData() );
}



//---------------------------------------------------
//	unbind everything
//---------------------------------------------------
void TLRender::Opengl::Unbind()
{
	//	unbind attribs
	//Platform::BindFixedVertexes( NULL );
	Platform::BindVertexes( NULL );
	Platform::BindColours( NULL );
}


//---------------------------------------------------
//	transform scene
//---------------------------------------------------
void TLRender::Opengl::SceneTransform(const TLMaths::TTransform& Transform)
{
	//	gr: do this in the same order as the Transform() functions?
	//		currently in old-render-code-order
	if ( Transform.HasTranslate() )
	{
		const float3& Trans = Transform.GetTranslate();
		glTranslatef( Trans.x, Trans.y, Trans.z );
	}

	if ( Transform.HasScale() )
	{
		const float3& Scale = Transform.GetScale();
		glScalef( Scale.x, Scale.y, Scale.z );
	}

	if ( Transform.HasMatrix() )
	{
		glMultMatrixf( Transform.GetMatrix().GetData() );
	}

	if ( Transform.HasRotation() )
	{
		TLMaths::TMatrix RotMatrix;
		TLMaths::QuaternionToMatrix( Transform.GetRotation(), RotMatrix );
		glMultMatrixf( RotMatrix.GetData() );
	}
}


//---------------------------------------------------
//	eular rotation on the scene - wrapper for glRotatef
//---------------------------------------------------
void TLRender::Opengl::SceneRotate(const TLMaths::TAngle& Rotation,const float3& Axis)
{
	glRotatef( Rotation.GetDegrees(), Axis.x, Axis.y, Axis.z );
}


