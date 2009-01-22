#include "TCamera.h"
#include <TootleCore/TString.h>
#include "TRenderTarget.h"


#include <TootleInput/TLInput.h>	// TEST
#include <TootleCore/TEventChannel.h> //TEST



TLRender::TCamera::TCamera() :
	m_Position	( 0.f, 0.f, 0.f ),
	m_LookAt	( 0.f, 0.f, 1.f ),
	m_NearZ		( 0.01f ),
	m_FarZ		( 100.f )
{
}



TLRender::TProjectCamera::TProjectCamera() :
	m_HorzFov			( 45.f ),
	m_CameraMatrixValid	( FALSE )
{
}


//----------------------------------------------------
//	update our camera matrix based on current view
//----------------------------------------------------
void TLRender::TProjectCamera::UpdateCameraMatrix(TLRender::TRenderTarget* pRenderTarget)
{
	const float3& eye = GetPosition();
	const float3& center = GetLookAt();

	if ( eye == center )
	{
		if ( !TLDebug_Break("Camera lookat and position are the same") )
			return;
	}

	float3 up = GetWorldUp();

//	gluLookAt(	eye.x, eye.y, eye.z,
//				center.x, center.y, center.z,
//				up.x, up.y, up.z );

	float mag;

	// Make rotation matrix 

	// Z vector
	float3 z = eye - center;
	mag = z.Length();

	//	gr: isnt this a normalise?
	if (mag) 
	{			// mpichler, 19950515 
	  z.x /= mag;
	  z.y /= mag;
	  z.z /= mag;
	}

	// Y vector
	float3 y = up;

	//	gr: could use the .CrossProduct(), but the .y computation is different?...
	// X vector = Y cross Z
	float3 x;
	x.x =  y.y * z.z - y.z * z.y;
	x.y = -y.x * z.z + y.z * z.x;
	x.z =  y.x * z.y - y.y * z.x;

	//	gr: could use the .CrossProduct(), but the .y computation is different?...
	// Recompute Y = Z cross X
	y.x =  z.y * x.z - z.z * x.y;
	y.y = -z.x * x.z + z.z * x.x;
	y.z =  z.x * x.y - z.y * x.x;

	// mpichler, 19950515 
	// cross product gives area of parallelogram, which is < 1.0 for
	// non-perpendicular unit-length vectors; so normalize x, y here
	
	mag = x.Length();
	//	gr: isnt this a normalise?
	if (mag) 
	{
	  x[0] /= mag;
	  x[1] /= mag;
	  x[2] /= mag;
	}

	mag = y.Length();
	//	gr: isnt this a normalise?
	if (mag)
	{
	  y[0] /= mag;
	  y[1] /= mag;
	  y[2] /= mag;
	}


	//	setup matrx

	TLMaths::TMatrix& M = m_CameraMatrix;
	M(0, 0) = x.x;
	M(0, 1) = x.y;
	M(0, 2) = x.z;
	M(0, 3) = 0.0f;
	M(1, 0) = y.x;
	M(1, 1) = y.y;
	M(1, 2) = y.z;
	M(1, 3) = 0.0f;
	M(2, 0) = z.x;
	M(2, 1) = z.y;
	M(2, 2) = z.z;
	M(2, 3) = 0.0f;
	M(3, 0) = 0.0f;
	M(3, 1) = 0.0f;
	M(3, 2) = 0.0f;
	M(3, 3) = 1.0f;


	//	quick hack!
	//	not sure why applying the translation doesnt work on the matrix...
//	pRenderTarget->PushMatrixMode();
	pRenderTarget->BeginSceneReset(FALSE);
	
	TLMaths::TTransform CamTransMat;
	CamTransMat.SetMatrix( m_CameraMatrix );
	pRenderTarget->Translate( CamTransMat );

	TLMaths::TTransform CamTransTrans;
	CamTransTrans.SetTranslate( float3( -eye.x, -eye.y, -eye.z ) );
	pRenderTarget->Translate( CamTransTrans );

	pRenderTarget->GetSceneMatrix( m_CameraMatrix );
	pRenderTarget->EndScene();
//	pRenderTarget->PopMatrixMode();

	//	camera matrix is now valid
	m_CameraMatrixValid = TRUE;
}


//----------------------------------------------------------
//	work out a frustum matrix from this camera
//----------------------------------------------------------
void TLRender::TProjectCamera::GetFrustumMatrix(TLMaths::TMatrix& Matrix,float AspectRatio)
{
	//	same as gluPerspective()
	float top = GetNearZ() * TLMaths::Tanf( GetHorzFov().GetRadians() );
	float bottom = -top;
	float left = bottom * AspectRatio;
	float right = top * AspectRatio;

	float x, y, a, b, c, d;

	x = (2.0f * GetNearZ()) / (right - left);
	y = (2.0f * GetNearZ()) / (top - bottom);
	a = (right + left) / (right - left);
	b = (top + bottom) / (top - bottom);
	c = -(GetFarZ() + GetNearZ()) / (GetFarZ() - GetNearZ());
	d = -(2.0f * GetFarZ() * GetNearZ()) / (GetFarZ() - GetNearZ());

	TLMaths::TMatrix& M = Matrix;
	//#define M(row,col)  Matrix.Data()[col*4+row]

	M(0,0) = x;     M(0,1) = 0.0f;  M(0,2) = a;      M(0,3) = 0.0f;
	M(1,0) = 0.0f;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0f;
	M(2,0) = 0.0f;  M(2,1) = 0.0f;  M(2,2) = c;      M(2,3) = d;
	M(3,0) = 0.0f;  M(3,1) = 0.0f;  M(3,2) = -1.0f;  M(3,3) = 0.0f;
}



//--------------------------------------------------------------
//	convert point on screen to a 3D ray
//--------------------------------------------------------------
Bool TLRender::TProjectCamera::GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize) const
{
	/*
	gr: inverse view method, note: directx matrix order, which is left hand, opengl is right hand!
VOID Camera::GetScreenRay(INT sx,INT sy, Ray *ray) // Returns ray pointing from screen into world (aka - mouse)
{
	Vector v;
	v.x =  (((2.0f * sx) / ScreenWidth) - 1.0f) / Projection._11;
	v.y = -(((2.0f * sy) / ScreenHeight) - 1.0f) / Projection._22;
	v.z = 1.0f;

	Matrix mat = MatrixInvert( View );

	ray->Direction.x = (v.x * mat._11) + (v.y * mat._21) + (v.z * mat._31);
	ray->Direction.y = (v.x * mat._12) + (v.y * mat._22) + (v.z * mat._32);
	ray->Direction.z = (v.x * mat._13) + (v.y * mat._23) + (v.z * mat._33);
	ray->Direction.Normalize();

	ray->Position.x = mat._41;
	ray->Position.y = mat._42;
	ray->Position.z = mat._43;
}
*/




	//	gr: correct to use this article, but we might still be able to get
	//	away without the matrix usage using the right aspect etc
	//	i still think something needs to be done about rotation (lookat) and 
	//	some camera displacement though....
	//	http://www.mvps.org/directx/articles/rayproj.htm

	/*
	public static float ConvertPixelWidthTo3DWidth(Device device, int width)
		{
			return ConvertPixelWidthTo3DWidth(RenderTargetSize.Width(), width);
		}

		public static float ConvertPixelWidthTo3DWidth(int backBufferWidth, int width)
		{
			float dx = 5.0f * (float)Math.Tan((Math.PI /4) / 2.0f);
			float width3d = (dx*2 / (float)backBufferWidth * width);
			return width3d;
		}

		public static float ConvertPixelHeightTo3DHeight(Device device, int height)
		{
			return ConvertPixelHeightTo3DHeight(RenderTargetSize.Height(),height);
		}

		public static float ConvertPixelHeightTo3DHeight(int backBufferHeight, int height)
		{
			float dx = 5.0f * (float)Math.Tan((Math.PI /4) / 2.0f);
			float height3d = (dx*2 / (float)backBufferHeight * height);
			return height3d;
		}

	public static Vector3 ConvertTo3DPosition(Device device, int left, int top,int width, int height)
*/
	

	//	get the pos as a factor (0..1) in the render target
	float RenderW = (float)RenderTargetSize.Width();
	float RenderH = (float)RenderTargetSize.Height();
	float2 Posf( (float)RenderTargetPos.x / RenderW, (float)RenderTargetPos.y / RenderH );

	//	convert to -1..1
	Posf -= float2( 0.5f, 0.5f );
	Posf *= 2.f;

	// Half of screen in 3d coordinate
	// dx = D * tan(FOV/2)
   // 5 is the distance from camera (-5,0,0) to center 0,0,0
	//float CameraDistance = -5.f;
	float CameraDistance = m_FarZ - m_NearZ;
	//float dx = 5.f * (float)TLMaths::Tanf( (Math.PI /4.f) / 2.0f);
	float dx = -CameraDistance * (float)TLMaths::Tanf( GetHorzFov().GetRadians() / 2.0f );	//	gr: dx is the far right (from center) at the end of our projection
	float dy = -CameraDistance * (float)TLMaths::Tanf( GetVertFov().GetRadians() / 2.0f );	

	//	get the direction by scaling down the extents
	//	gr: i dont think this takes into account the lookat
	//	maybe this RayDir needs to align with the lookat direction (wouldn't have to negate the Z then...)
	float3 RayDir( dx * Posf.x, dy * Posf.y, -CameraDistance );

	//	ray starts at the camera...
	const float3& RayStart = GetPosition();

	/*
	//0.0f - 2.07f = 0,0 screen;
	float bitmapwidth = ConvertPixelWidthTo3DWidth(device,width) / 2.0f;
	float tw = (-1*dx)+ bitmapwidth + (dx*2 / device.PresentationParameters.BackBufferWidth * left);
	float bitmapheight = ConvertPixelHeightTo3DHeight(device,height) / 2.0f;
	float th = dx - bitmapheight - (dx*2 / device.PresentationParameters.BackBufferHeight * top);

	*/

	//	set ray
	WorldRay.SetDir( RayStart, RayDir );

	return TRUE;
}



//--------------------------------------------------------------
//	convert render target size (pixels) to game-screen dimensions (0-100.f)
//--------------------------------------------------------------
void TLRender::TOrthoCamera::GetOrthoSize(Type4<float>& OrthoSize,const Type4<s32>& RenderTargetSize)
{
	OrthoSize = RenderTargetSize;

	//	the screen is on a scale of 0-100 wide (height is relative to the width)
	float OrthoScale = GetOrthoRange() / (RenderTargetSize.Width() - RenderTargetSize.Left());
	
	OrthoSize *= OrthoScale;
}


//--------------------------------------------------------------
//	convert point on screen to a 3D ray
//--------------------------------------------------------------
Bool TLRender::TOrthoCamera::GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize) const
{
	//	the screen is on a scale of 0-100 wide (height is relative to the width)
	float OrthoScale = GetOrthoRange() / (RenderTargetSize.Width() - RenderTargetSize.Left());

	//	rememeber pos is in world space, not screen space so apply after scale
	float3 RayStart(	(RenderTargetPos.x * OrthoScale) - GetPosition().x, 
						(RenderTargetPos.y * OrthoScale) - GetPosition().y,
						m_NearZ + GetPosition().z );

	float ZLength = m_FarZ - m_NearZ;
	float3 RayEnd( RayStart + (GetWorldForward() * ZLength) );


	WorldRay.Set( RayStart, RayEnd );

	return TRUE;
}

