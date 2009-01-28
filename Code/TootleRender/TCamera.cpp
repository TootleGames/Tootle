#include "TCamera.h"
#include <TootleCore/TString.h>
#include "TRenderTarget.h"


#include <TootleInput/TLInput.h>	// TEST
#include <TootleCore/TEventChannel.h> //TEST

//#define DEBUG_SCALE_FRUSTUM_BOX		0.2f	//	scale down the frustum boxes we generate to test culling/intersections etc


TLRender::TCamera::TCamera() :
	m_ViewLine	( float3(0.f, 0.f, 0.f), float3(0.f, 0.f, 1.f) ),
	m_NearZ		( 0.01f ),
	m_FarZ		( 100.f ),
	m_ProjectionMatrixValid	( FALSE ),
	m_ModelViewMatrixValid	( FALSE )
{
	//	init view vectors
	CalculateViewVectors();
}


//----------------------------------------------------
//	calculate new view vectors
//----------------------------------------------------
void TLRender::TCamera::CalculateViewVectors()
{
	m_Forward = m_ViewLine.GetDirectionNormal();	
	m_ViewRight = m_Forward.CrossProduct( GetWorldUp() );
	m_ViewUp = m_ViewRight.CrossProduct( m_Forward );
}


//----------------------------------------------------
//	
//----------------------------------------------------
void TLRender::TCamera::OnCameraChanged()		
{	
	m_ProjectionMatrixValid = FALSE;	
	m_ModelViewMatrixValid = FALSE;	
	TLMaths::TQuadTreeNode::SetZoneOutOfDate(TRUE);	
	CalculateViewVectors();
}






TLRender::TProjectCamera::TProjectCamera() :
	m_HorzFov					( 45.f ),
	m_CameraLookAtMatrixValid	( FALSE ),
	m_FrustumValid				( FALSE )
{
}


//----------------------------------------------------
//	update our camera matrix based on current view
//----------------------------------------------------
const TLMaths::TMatrix& TLRender::TProjectCamera::UpdateCameraLookAtMatrix()
{
	const float3& eye = GetPosition();
	const float3& center = GetLookAt();

	if ( eye == center )
	{
		if ( !TLDebug_Break("Camera lookat and position are the same") )
			return m_CameraLookAtMatrix;
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
	TLMaths::TMatrix& M = m_CameraLookAtMatrix;
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

	//	camera matrix is now valid
	m_CameraLookAtMatrixValid = TRUE;
	return m_CameraLookAtMatrix;
}



/*

/////////////////////////////////////////////////////////////////////////////
// Name:           CalcCullingInfo
// Arguments:      An eight element array of Points for each of the frustum 
//                 corners.
// Returns:        none
// Side Effects:   Stores the plane information (normal and d component) for
//                 each of the six frustum planes in the local frustumPlanes 
//                 array.
/////////////////////////////////////////////////////////////////////////////
void PerspectiveCam::CalcCullingInfo(Point3 frustumCorners[]) 
{
    Vector3 dir = Target - Eye;
    dir.Normalize();
    Vector3 up(0,1,0);
    Vector3 A;
    A.Cross(dir, up);
    Vector3 B;
    B.Cross(A, dir);
    float ch = tanf(FOV*M_PI/360);
    float cw = ch*(float)PixelWidth/PixelHeight;

    frustumCorners[0] = Eye + dir*NearClip + A*cw*NearClip + B*ch*NearClip;
    frustumCorners[1] = Eye + dir*NearClip - A*cw*NearClip + B*ch*NearClip;
    frustumCorners[2] = Eye + dir*NearClip - A*cw*NearClip - B*ch*NearClip;
    frustumCorners[3] = Eye + dir*NearClip + A*cw*NearClip - B*ch*NearClip;

    frustumCorners[4] = Eye + dir*FarClip + A*cw*FarClip + B*ch*FarClip;
    frustumCorners[5] = Eye + dir*FarClip - A*cw*FarClip + B*ch*FarClip;
    frustumCorners[6] = Eye + dir*FarClip - A*cw*FarClip - B*ch*FarClip;
    frustumCorners[7] = Eye + dir*FarClip + A*cw*FarClip - B*ch*FarClip;

    frustumPlanes[0].normal.Cross(frustumCorners[4]-frustumCorners[0], frustumCorners[5]-frustumCorners[0]);
    frustumPlanes[0].normal.Normalize();
    frustumPlanes[0].d = frustumPlanes[0].normal.Dot(frustumCorners[0].ToVector3());

    frustumPlanes[1].normal.Cross(frustumCorners[5]-frustumCorners[1], frustumCorners[6]-frustumCorners[1]);
    frustumPlanes[1].normal.Normalize();
    frustumPlanes[1].d = frustumPlanes[1].normal.Dot(frustumCorners[1].ToVector3());

    frustumPlanes[2].normal.Cross(frustumCorners[6]-frustumCorners[2], frustumCorners[7]-frustumCorners[2]);
    frustumPlanes[2].normal.Normalize();
    frustumPlanes[2].d = frustumPlanes[2].normal.Dot(frustumCorners[2].ToVector3());

    frustumPlanes[3].normal.Cross(frustumCorners[7]-frustumCorners[3], frustumCorners[4]-frustumCorners[3]);
    frustumPlanes[3].normal.Normalize();
    frustumPlanes[3].d = frustumPlanes[3].normal.Dot(frustumCorners[3].ToVector3());

    frustumPlanes[4].normal.Cross(frustumCorners[1]-frustumCorners[0], frustumCorners[2]-frustumCorners[0]);
    frustumPlanes[4].normal.Normalize();
    frustumPlanes[4].d = frustumPlanes[4].normal.Dot(frustumCorners[0].ToVector3());

    frustumPlanes[5].normal.Cross(frustumCorners[6]-frustumCorners[4], frustumCorners[5]-frustumCorners[4]);
    frustumPlanes[5].normal.Normalize();
    frustumPlanes[5].d = frustumPlanes[5].normal.Dot(frustumCorners[4].ToVector3());
}


/////////////////////////////////////////////////////////////////////////////
// Name:           CalcCullingInfo
// Arguments:      An eight element array of Points for each of the frustum 
//                 corners.
// Returns:        none
// Side Effects:   Stores the plane information (normal and d component) for
//                 each of the six frustum planes in the local frustumPlanes 
//                 array. Also fills frustumCorners with the eight corners
//                 of the frustum
/////////////////////////////////////////////////////////////////////////////
void OrthographicCam::CalcCullingInfo(Point3 frustumCorners[]) 
{
    Vector3 dir = Target - Eye;
    dir.Normalize();
    Vector3 up(0,1,0);
    Vector3 A;
    A.Cross(dir, up);
    Vector3 B;
    B.Cross(A, dir);
    float ch = Height/2;
    float cw = ch*(float)PixelWidth/PixelHeight;

    frustumCorners[0] = Eye + dir*NearClip + A*cw + B*ch;
    frustumCorners[1] = Eye + dir*NearClip - A*cw + B*ch;
    frustumCorners[2] = Eye + dir*NearClip - A*cw - B*ch;
    frustumCorners[3] = Eye + dir*NearClip + A*cw - B*ch;

    frustumCorners[4] = Eye + dir*FarClip + A*cw + B*ch;
    frustumCorners[5] = Eye + dir*FarClip - A*cw + B*ch;
    frustumCorners[6] = Eye + dir*FarClip - A*cw - B*ch;
    frustumCorners[7] = Eye + dir*FarClip + A*cw - B*ch;

    frustumPlanes[0].normal.Cross(frustumCorners[4]-frustumCorners[0], frustumCorners[5]-frustumCorners[0]);
    frustumPlanes[0].normal.Normalize();
    frustumPlanes[0].d = frustumPlanes[0].normal.Dot(frustumCorners[0].ToVector3());

    frustumPlanes[1].normal.Cross(frustumCorners[5]-frustumCorners[1], frustumCorners[6]-frustumCorners[1]);
    frustumPlanes[1].normal.Normalize();
    frustumPlanes[1].d = frustumPlanes[1].normal.Dot(frustumCorners[1].ToVector3());

    frustumPlanes[2].normal.Cross(frustumCorners[6]-frustumCorners[2], frustumCorners[7]-frustumCorners[2]);
    frustumPlanes[2].normal.Normalize();
    frustumPlanes[2].d = frustumPlanes[2].normal.Dot(frustumCorners[2].ToVector3());

    frustumPlanes[3].normal.Cross(frustumCorners[7]-frustumCorners[3], frustumCorners[4]-frustumCorners[3]);
    frustumPlanes[3].normal.Normalize();
    frustumPlanes[3].d = frustumPlanes[3].normal.Dot(frustumCorners[3].ToVector3());

    frustumPlanes[4].normal.Cross(frustumCorners[1]-frustumCorners[0], frustumCorners[2]-frustumCorners[0]);
    frustumPlanes[4].normal.Normalize();
    frustumPlanes[4].d = frustumPlanes[4].normal.Dot(frustumCorners[0].ToVector3());

    frustumPlanes[5].normal.Cross(frustumCorners[6]-frustumCorners[4], frustumCorners[5]-frustumCorners[4]);
    frustumPlanes[5].normal.Normalize();
    frustumPlanes[5].d = frustumPlanes[5].normal.Dot(frustumCorners[4].ToVector3());

}
*/

//-----------------------------------------------------------
//	extract the frustum from the current view matricies
//	http://www.crownandcutlass.com/features/technicaldetails/frustum.html
//-----------------------------------------------------------
void TLRender::TProjectCamera::CalcFrustum()
{
	if ( !m_ProjectionMatrixValid || !m_ModelViewMatrixValid )
	{
		TLDebug_Break("Can't calculate frustrum with invalid matricies");
		return;
	}

	float* proj = m_ProjectionMatrix.GetData();
	float* modl = m_ModelViewMatrix.GetData();

	float   clip[16];

	//	Combine the two matrices (multiply projection by modelview) 
	clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
	clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
	clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
	clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

	clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
	clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
	clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
	clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

	clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
	clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
	clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
	clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

	clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
	clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
	clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
	clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

	//	Extract the numbers for the RIGHT plane
	m_Frustum.GetRightPlane().x() = clip[ 3] - clip[ 0];
	m_Frustum.GetRightPlane().y() = clip[ 7] - clip[ 4];
	m_Frustum.GetRightPlane().z() = clip[11] - clip[ 8];
	m_Frustum.GetRightPlane().w() = clip[15] - clip[12];
	m_Frustum.GetRightPlane().xyzw().Normalise();

	// Extract the numbers for the LEFT plane
	m_Frustum.GetLeftPlane().x() = clip[ 3] + clip[ 0];
	m_Frustum.GetLeftPlane().y() = clip[ 7] + clip[ 4];
	m_Frustum.GetLeftPlane().z() = clip[11] + clip[ 8];
	m_Frustum.GetLeftPlane().w() = clip[15] + clip[12];
	m_Frustum.GetLeftPlane().xyzw().Normalise();

	// Extract the BOTTOM plane
	m_Frustum.GetBottomPlane().x() = clip[ 3] + clip[ 1];
	m_Frustum.GetBottomPlane().y() = clip[ 7] + clip[ 5];
	m_Frustum.GetBottomPlane().z() = clip[11] + clip[ 9];
	m_Frustum.GetBottomPlane().w() = clip[15] + clip[13];
	m_Frustum.GetBottomPlane().xyzw().Normalise();

	// Extract the TOP plane
	m_Frustum.GetTopPlane().x() = clip[ 3] - clip[ 1];
	m_Frustum.GetTopPlane().y() = clip[ 7] - clip[ 5];
	m_Frustum.GetTopPlane().z() = clip[11] - clip[ 9];
	m_Frustum.GetTopPlane().w() = clip[15] - clip[13];
	m_Frustum.GetTopPlane().xyzw().Normalise();

	// Extract the FAR plane
	m_Frustum.GetFarPlane().x() = clip[ 3] - clip[ 2];
	m_Frustum.GetFarPlane().y() = clip[ 7] - clip[ 6];
	m_Frustum.GetFarPlane().z() = clip[11] - clip[10];
	m_Frustum.GetFarPlane().w() = clip[15] - clip[14];
	m_Frustum.GetFarPlane().xyzw().Normalise();

	// Extract the NEAR plane
	m_Frustum.GetNearPlane().x() = clip[ 3] + clip[ 2];
	m_Frustum.GetNearPlane().y() = clip[ 7] + clip[ 6];
	m_Frustum.GetNearPlane().z() = clip[11] + clip[10];
	m_Frustum.GetNearPlane().w() = clip[15] + clip[14];
	m_Frustum.GetNearPlane().xyzw().Normalise();

	//	store off the near/far values
	m_Frustum.SetNearFar( m_NearZ, m_FarZ );


	//	calc the box
	TLMaths::TBoxOB& FrustumBox = m_Frustum.GetBox();
	FrustumBox.GetBoxCorners().SetSize(0);

	GetPlaneBoxCorners( m_NearZ, FrustumBox.GetBoxCorners() );
	GetPlaneBoxCorners( m_FarZ, FrustumBox.GetBoxCorners() );


	//	calc the shape for the zone from the frustum
	//	get the camera frustum box at a depth of zero (this will be a bit small as its far from the camera)
	TLMaths::TBoxOB PlaneBox;
	float worldz = 0.f;
	float viewz = GetPosition().z + worldz;
	GetPlaneBox( viewz, PlaneBox );
	//	turn this into a 2D box - note: if our box is rotated then the 2D box will be bigger than the screen
	m_FrustumZoneShape.SetInvalid();
	m_FrustumZoneShape.Accumulate( PlaneBox.GetBoxCorners() );



	m_FrustumValid = TRUE;
}



//----------------------------------------------------------
//	extract an oriented box from the frustum at a certain depth
//----------------------------------------------------------
void TLRender::TProjectCamera::GetPlaneBox(float ViewZDepth,TLMaths::TBoxOB& PlaneBox)
{
	PlaneBox.GetBoxCorners().SetSize(0);
	GetPlaneBoxCorners( ViewZDepth, PlaneBox.GetBoxCorners() );
}


//----------------------------------------------------------
//	extract an oriented box from the frustum at a certain depth
//----------------------------------------------------------
void TLRender::TProjectCamera::GetPlaneBoxCorners(float ViewZDepth,TArray<float3>& PlaneCorners)
{
    float ch = tanf( GetHorzFov().GetRadians() / 2.f );	//	half plane height scalar
    float cw = ch * GetAspectRatio();	//	half plane width scalar

	//	scale down the frustum boxes we generate to test culling/intersections etc
#ifdef DEBUG_SCALE_FRUSTUM_BOX
	ch *= DEBUG_SCALE_FRUSTUM_BOX;
	cw *= DEBUG_SCALE_FRUSTUM_BOX;
#endif

	//	precalc vectors
	//	center of plane
	//	gr: todo: again here... i have to negate x/y... my forward/backward numbers are wrong on the camera in a few places
	float3 Center = (GetPosition() * float3(-1.f,-1.f,1.f)) + (m_Forward * ViewZDepth);

	//	half vector extents across/up for near and far (cw/ch are half w/h)
	float3 Horz = m_ViewRight * (cw * ViewZDepth);
	float3 Vert = m_ViewUp * (ch * ViewZDepth);

	//	top left, top right, bottom right, bottom left
	PlaneCorners.Add( Center - Horz - Vert );
	PlaneCorners.Add( Center + Horz - Vert );
	PlaneCorners.Add( Center + Horz + Vert );
	PlaneCorners.Add( Center - Horz + Vert );
}


/*
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
*/


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


//----------------------------------------------------
//	does this camera's view frustrum intersect this zone's shape?
//----------------------------------------------------
const TLMaths::TBox2D& TLRender::TProjectCamera::GetZoneShape()
{
	if ( !m_FrustumValid )
	{
		CalcFrustum();
	}

	if ( !m_FrustumValid )
	{
		TLDebug_Break("frustum is invalid!");
		m_FrustumZoneShape.SetInvalid();
	}

	return m_FrustumZoneShape;
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

