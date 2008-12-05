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
	m_Fov				( 45.f ),
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
	float top = GetNearZ() * tanf( GetFov().GetRadians() );
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

