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
void TLRender::Opengl::SceneTransform(const TLMaths::TTransform& Transform,const TLMaths::TMatrix* pMatrix)
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

	if ( pMatrix )
	{
		glMultMatrixf( pMatrix->GetData() );
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


//---------------------------------------------------
//	get render target's viewport size from the size and the screen size
//---------------------------------------------------
void TLRender::Opengl::GetViewportSize(Type4<s32>& ViewportSize,const Type4<s32>& ViewportTargetMaxSize,const Type4<s32>& RenderTargetSize,const Type4<s32>& RenderTargetMaxSize,TScreenShape ScreenShape)
{
	//	rotate render target size to be in "viewport" space
	Type4<s32> RotatedRenderTargetSize = RenderTargetSize;

	if ( ScreenShape == ScreenShape_WideLeft )
	{
		//	gr: rendertarget is rotated left, so to get viewport, rotate it right again
		//	rotate right
		RotatedRenderTargetSize.x = RenderTargetSize.Top();
		RotatedRenderTargetSize.y = RenderTargetMaxSize.Width() - RenderTargetSize.Right();
		RotatedRenderTargetSize.Width() = RenderTargetSize.Height();
		RotatedRenderTargetSize.Height() = RenderTargetSize.Width();
	}
	else if ( ScreenShape == ScreenShape_WideRight )
	{
		//	gr: rendertarget is rotated right, so to get viewport, rotate it left again
		//	rotate left
		RotatedRenderTargetSize.x = RenderTargetMaxSize.Height() - RenderTargetSize.Bottom();
		RotatedRenderTargetSize.y = RenderTargetSize.Left();
		RotatedRenderTargetSize.Width() = RenderTargetSize.Height();
		RotatedRenderTargetSize.Height() = RenderTargetSize.Width();
	}

	//	position for opengl viewport
	//	0,0 is bottom left, next two sizes in Scissor() and Viewport() are still widht and height, just upside down
	ViewportSize.Left() = RotatedRenderTargetSize.Left();
	ViewportSize.Top() = ViewportTargetMaxSize.Height() - RotatedRenderTargetSize.Top() - RotatedRenderTargetSize.Height();

	//	no change in dimensions
	ViewportSize.Width() = RotatedRenderTargetSize.Width();
	ViewportSize.Height() = RotatedRenderTargetSize.Height();
}

