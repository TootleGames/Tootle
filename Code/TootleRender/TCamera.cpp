#include "TCamera.h"
#include <TootleCore/TString.h>
#include "TRenderTarget.h"

#include <TootleAudio/TLAudio.h>	// Needed to set the audio listener properties in CalculateViewVectors()


#ifdef _DEBUG
#define DEBUG_SCALE_FRUSTUM_BOX		0.8f	//	scale down the frustum boxes we generate to test culling/intersections etc
#endif



TLRender::TCamera::TCamera() :
	m_ViewLine	( float3(0.f, 0.f, 0.f), float3(0.f, 0.f, 1.f) ),
	m_NearZ		( 0.01f ),
	m_FarZ		( 100.f )
{
	//	init view vectors
	CalculateViewVectors();
}


//----------------------------------------------------
//	calculate new view vectors
//----------------------------------------------------
void TLRender::TCamera::CalculateViewVectors()
{
	//	get forward vector
	m_ViewForward = m_ViewLine.GetDirectionNormal();	

	//	rotate the worldup (Y axis) around z... to get the view up
	float3 ViewUp = GetWorldUp();
	ViewUp.RotateZ( m_CameraRoll.GetRadians() );

	//	camera forward against view up = view right
	m_ViewRight = m_ViewForward.CrossProduct( ViewUp );

	//	calculate the proper view up from the view right and view forward
	//	gr: is this the same as our rolled worldup, ViewUp?
	m_ViewUp = m_ViewRight.CrossProduct( m_ViewForward );

	// [03/02/09] DB - Set the lister details for the audio system
	// Not sure this is the right place for this but for now it should work as required.

	TLAudio::TListenerProperties Props;

	float3 vPosition = GetPosition();
	float3 vVelocity = vPosition - m_vPreviousPos;

	// Invert the X and Y due to differing coordinate system
	vPosition.x = -vPosition.x;
	vPosition.y = -vPosition.y;

	// Set the Z to be near the floor
	vPosition.z = 5.0f;

	Props.m_vPosition = vPosition;
	Props.m_vVelocity = vVelocity;
	Props.m_vUp = GetViewUp();
	Props.m_vLookAt = GetLookAt();

	if ( TLAudio::g_pAudiograph )
		TLAudio::g_pAudiograph->SetListener(Props);
}


//----------------------------------------------------
//	
//----------------------------------------------------
void TLRender::TCamera::OnCameraChanged()		
{	
	TLMaths::TQuadTreeNode::SetZoneOutOfDate(TRUE);	
	CalculateViewVectors();
}





TLRender::TProjectCamera::TProjectCamera() :
	m_HorzFov					( 45.f ),
	m_CameraLookAtMatrixValid	( FALSE ),
	m_FrustumShapeValid			( SyncFalse )
{
}


void TLRender::TProjectCamera::OnCameraChanged()		
{	
	TCamera::OnCameraChanged();	
	
	if ( m_FrustumShapeValid == SyncTrue ) 
		m_FrustumShapeValid = SyncWait;	
	
	m_CameraLookAtMatrixValid = FALSE;	
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


//-----------------------------------------------------------
//	extract the frustum from the current view matricies
//	http://www.crownandcutlass.com/features/technicaldetails/frustum.html
//-----------------------------------------------------------
void TLRender::TProjectCamera::CalcFrustum()
{
	//	calc the frustum oblong
	m_FrustumOblong.GetBoxCorners().SetSize(0);
	GetWorldFrustumPlaneBoxCorners( m_NearZ, m_FrustumOblong.GetBoxCorners() );
	GetWorldFrustumPlaneBoxCorners( m_FarZ, m_FrustumOblong.GetBoxCorners() );

	//	calc (cache) a 2D zone shape from the frustum
	float WorldDepth = 0.f;
	float ViewDepth = GetPosition().z + WorldDepth;
	GetWorldFrustumPlaneBox2D( ViewDepth, m_FrustumBox );

	//	shapes are now valid
	m_FrustumShapeValid = SyncTrue;
}


//----------------------------------------------------------
//	extract box and make 2D
//----------------------------------------------------------
void TLRender::TProjectCamera::GetWorldFrustumPlaneBox2D(float ViewDepth,TLMaths::TBox2D& PlaneBox2D) const
{
	//	calc the shape for the zone from the frustum
	//	get the camera frustum box at a depth of zero in the world (this will be a bit small as its far from the camera)
	TLMaths::TOblong PlaneBox;
	GetWorldFrustumPlaneBox( ViewDepth, PlaneBox );

	//	turn this into a 2D box - note: if our box is rotated then the 2D box will be bigger than the screen
	PlaneBox2D.SetInvalid();
	PlaneBox2D.Accumulate( PlaneBox.GetBoxCorners() );
}


//----------------------------------------------------------
//	extract an oriented box from the frustum at a certain depth
//----------------------------------------------------------
void TLRender::TProjectCamera::GetWorldFrustumPlaneBox(float ViewDepth,TLMaths::TOblong& PlaneBox) const
{
	PlaneBox.GetBoxCorners().SetSize(0);
	GetWorldFrustumPlaneBoxCorners( ViewDepth, PlaneBox.GetBoxCorners() );
}


//----------------------------------------------------------
//	calc new view sizes
//----------------------------------------------------------
void TLRender::TProjectCamera::SetRenderTargetSize(const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape)
{
	/*
	float AspectRatio = (float)RenderTargetSize.Width() / (float)RenderTargetSize.Height();

	//	calc the un-rotated screen view size
	float ViewHalfHeight = tanf( GetHorzFov().GetRadians() / 2.f );
	float ViewHalfWidth = ViewHalfHeight * AspectRatio;
	m_ScreenViewBox.SetMin( float2( -ViewHalfWidth, -ViewHalfHeight ) );
	m_ScreenViewBox.SetMax( float2( ViewHalfWidth, ViewHalfHeight ) );

	//	copy the screen view into the projection view...
	m_ProjectionViewBox = m_ScreenViewBox;

	//	rotate our projected view (same as glRotatef)
	if ( ScreenShape == TLRender::ScreenShape_WideRight || ScreenShape == TLRender::ScreenShape_WideLeft )
	{
		//	gr: bit of a bodge atm, just swap w and h
		TLMaths::SwapVars( m_ProjectionViewBox.GetMin().x, m_ProjectionViewBox.GetMin().y );
		TLMaths::SwapVars( m_ProjectionViewBox.GetMax().x, m_ProjectionViewBox.GetMax().y );
	}

	//	move the screen view forward to the nearz
	m_ScreenViewBox.GetMin() *= GetNearZ();
	m_ScreenViewBox.GetMax() *= GetNearZ();
	*/
}


//----------------------------------------------------------
//	calc new view sizes
//----------------------------------------------------------
void TLRender::TProjectCamera::SetViewportSize(const Type4<s32>& ViewportSize,TScreenShape ScreenShape)
{
	float AspectRatio = (float)ViewportSize.Width() / (float)ViewportSize.Height();

	//	calc the un-rotated screen view size
	float ViewHalfHeight = tanf( GetHorzFov().GetRadians() / 2.f );
	float ViewHalfWidth = ViewHalfHeight * AspectRatio;
	m_ScreenViewBox.SetMin( float2( -ViewHalfWidth, -ViewHalfHeight ) );
	m_ScreenViewBox.SetMax( float2( ViewHalfWidth, ViewHalfHeight ) );

	//	copy the screen view into the projection view...
	m_ProjectionViewBox = m_ScreenViewBox;

	//	rotate our projected view (same as glRotatef)
	if ( ScreenShape == TLRender::ScreenShape_WideRight || ScreenShape == TLRender::ScreenShape_WideLeft )
	{
		//	gr: bit of a bodge atm, just swap w and h
		TLMaths::SwapVars( m_ProjectionViewBox.GetMin().x, m_ProjectionViewBox.GetMin().y );
		TLMaths::SwapVars( m_ProjectionViewBox.GetMax().x, m_ProjectionViewBox.GetMax().y );
	}

	//	move the screen view forward to the nearz
	m_ScreenViewBox.GetMin() *= GetNearZ();
	m_ScreenViewBox.GetMax() *= GetNearZ();
}


//----------------------------------------------------------
//	extract an oriented box from the frustum at a certain depth
//----------------------------------------------------------
void TLRender::TProjectCamera::GetWorldFrustumPlaneBoxCorners(float ViewDepth,TArray<float3>& PlaneCorners) const
{
	//	scale down the frustum boxes we generate to test culling/intersections etc
#ifdef DEBUG_SCALE_FRUSTUM_BOX
	TLMaths::TBox2D ViewBox = GetProjectionViewBox();
	ViewBox.GrowBox( DEBUG_SCALE_FRUSTUM_BOX );
#else
	const TLMaths::TBox2D& ViewBox = GetProjectionViewBox();
#endif

#ifdef _DEBUG
	if ( ViewDepth < GetNearZ() )
	{
		TLDebug_Break("Do not request a frustum plane behind the nearz, if ZERO will be invalid.");
		ViewDepth = GetNearZ();
	}
#endif

	//	precalc vectors
	//	center of plane
	//	gr: todo: again here... i have to negate x/y... my forward/backward numbers are wrong on the camera in a few places
	float3 Center = (GetPosition() * float3(-1.f,-1.f,1.f)) + (m_ViewForward * ViewDepth);

	//	half vector extents across/up for near and far (cw/ch are half w/h)
	float3 Horz = m_ViewRight * (ViewBox.GetHalfWidth() * ViewDepth);
	float3 Vert = m_ViewUp * (ViewBox.GetHalfHeight() * ViewDepth);

	//	top left, top right, bottom right, bottom left
	PlaneCorners.Add( Center - Horz - Vert );
	PlaneCorners.Add( Center + Horz - Vert );
	PlaneCorners.Add( Center + Horz + Vert );
	PlaneCorners.Add( Center - Horz + Vert );
}



//--------------------------------------------------------------
//	convert point on screen to a 3D ray
//--------------------------------------------------------------
Bool TLRender::TProjectCamera::GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const
{
	TLDebug_Break("GR: todo");
	
	//	gr: do like; get box for near and far planes
	//		get 0..1 of x and y from RenderTargetPos/RenderTargetSize
	//		get that 0..1,0..1 as point in the near plane box, and far plane box...
	//		tada! vectors in near and far planes, turn into line!
	
	return FALSE;
}


//----------------------------------------------------
//	does this camera's view frustrum intersect this zone's shape?
//----------------------------------------------------
const TLMaths::TBox2D& TLRender::TProjectCamera::GetZoneShape()
{
	if ( m_FrustumShapeValid != SyncTrue )
	{
		CalcFrustum();
	}

	//	state is invalid, so ensure the shape we return is invalid
	if ( m_FrustumShapeValid == SyncFalse )
	{
		TLDebug_Break("frustum is invalid!");
		m_FrustumBox.SetInvalid();
	}

	return m_FrustumBox;
}


//--------------------------------------------------------------
//	convert render target size (pixels) to game-screen dimensions (0-100.f)
//	calc new view sizes
//--------------------------------------------------------------
void TLRender::TOrthoCamera::SetRenderTargetSize(const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape)
{
	//	gr: "cache" the world -> pixel scalars
	m_WorldToPixelScale = RenderTargetSize.Width() / GetOrthoRange();
	m_PixelToWorldScale = GetOrthoRange() / RenderTargetSize.Width();

	m_OrthoRenderTargetBox.GetLeft()	= (float)RenderTargetSize.Left();
	m_OrthoRenderTargetBox.GetRight()	= (float)RenderTargetSize.Right();
	m_OrthoRenderTargetBox.GetTop()		= (float)RenderTargetSize.Top();
	m_OrthoRenderTargetBox.GetBottom()	= (float)RenderTargetSize.Bottom();

	//	the screen is on a scale of 0-100 wide (height is relative to the width)
	//	so scale the dimensions
	float OrthoScale = GetOrthoRange() / m_OrthoRenderTargetBox.GetWidth();
	m_OrthoRenderTargetBox *= OrthoScale;

	//	altering values directly means it wont be marked as valid, so do that manually too
	m_OrthoRenderTargetBox.SetValid();
}


//--------------------------------------------------------------
//	convert render target size (pixels) to game-screen dimensions (0-100.f)
//	calc new view sizes
//--------------------------------------------------------------
void TLRender::TOrthoCamera::SetViewportSize(const Type4<s32>& ViewportSize,TScreenShape ScreenShape)
{
	/*
	//	set box dimensions according to screen orientation
	if ( ScreenShape == TLRender::ScreenShape_WideRight || ScreenShape == TLRender::ScreenShape_WideLeft )
	{
		//	gr: currently this is the same rotation of dimensions either way around, it makes the box the right size
		//		the actual rotation is done at projection
		m_OrthoViewportBox.GetLeft()	= (float)ViewportSize.Top();
		m_OrthoViewportBox.GetRight()	= (float)ViewportSize.Bottom();
		m_OrthoViewportBox.GetTop()		= (float)ViewportSize.Left();
		m_OrthoViewportBox.GetBottom()	= (float)ViewportSize.Right();
	}
	else
	{
		//	gr: currently this is the same rotation of dimensions
		m_OrthoViewportBox.GetLeft()	= (float)ViewportSize.Left();
		m_OrthoViewportBox.GetRight()	= (float)ViewportSize.Right();
		m_OrthoViewportBox.GetTop()		= (float)ViewportSize.Top();
		m_OrthoViewportBox.GetBottom()	= (float)ViewportSize.Bottom();
	}

	//	gr: left and top is always zero as ortho coordinates are a scale. The RenderTargetSize dictates the position as well as dimensions,
	//		but the viewport/scissoring does the actual screen-positioning
	m_OrthoViewportBox.GetLeft() = 0.f;
	m_OrthoViewportBox.GetTop() = 0.f;
	*/

	m_OrthoViewportBox.GetLeft()	= (float)ViewportSize.Left();
	m_OrthoViewportBox.GetRight()	= (float)ViewportSize.Right();
	m_OrthoViewportBox.GetTop()		= (float)ViewportSize.Top();
	m_OrthoViewportBox.GetBottom()	= (float)ViewportSize.Bottom();

	//	the screen is on a scale of 0-100 wide (height is relative to the width)
	//	so scale the dimensions
	float OrthoScale = GetOrthoRange() / m_OrthoViewportBox.GetWidth();
	m_OrthoViewportBox *= OrthoScale;

	//	altering values directly means it wont be marked as valid, so do that manually too
	m_OrthoViewportBox.SetValid();
}


//--------------------------------------------------------------
//	convert point on screen to a 3D ray
//--------------------------------------------------------------
Bool TLRender::TOrthoCamera::GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const
{
	//	re-use the get pos code to get the start of the ray
	float RayStartDepth = m_NearZ + GetPosition().z;
	float3 RayStart;
	
	if ( !GetWorldPos( RayStart, RayStartDepth, RenderTargetPos, RenderTargetSize, ScreenShape ) )
		return FALSE;

	//	max extent of the line is the viewable distance...
	float ZLength = m_FarZ - m_NearZ;
	float3 RayEnd( RayStart + (GetWorldForward() * ZLength) );

	WorldRay.Set( RayStart, RayEnd );

	return TRUE;
}


//--------------------------------------------------------------
//	convert point on screen to a 3D pos
//--------------------------------------------------------------
Bool TLRender::TOrthoCamera::GetWorldPos(float3& WorldPos,float WorldDepth,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const
{
	//	screen -> view
	float2 xy( (float)RenderTargetPos.x, (float)RenderTargetPos.y );
	float2 wh( (float)RenderTargetSize.Width(), (float)RenderTargetSize.Height() );

	//	scale the rendertarget pos to a pos between 0..100 (100 being GetOrthoRange - and scale is always based on width)
	float OrthoX = (xy.x / wh.x) * GetOrthoRange();
	float OrthoY = (xy.y / wh.x) * GetOrthoRange();

	//	"ortho" x/y is now world space
	OrthoX -= GetPosition().x;
	OrthoY -= GetPosition().y;

	//	new world pos -
	//	gr: camera z is right? or wrong... world space cam pos + nearz.. must be right
	//	gr: no complex z stuff in ortho...
	WorldPos.Set( OrthoX, OrthoY, WorldDepth );

	return TRUE;
}

//--------------------------------------------------------------
//	convert point on screen to a 3D pos
// NOTE: Works the opposite of the GetWorldPos() routine
//--------------------------------------------------------------
Bool TLRender::TOrthoCamera::GetRenderTargetPos(Type2<s32>& RenderTargetPos, const float3& WorldPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const
{
	// world -> screen

	//	Move pos to be in at origin of camera
	float OrthoX = WorldPos.x + GetPosition().x;
	float OrthoY = WorldPos.y + GetPosition().y;

	//	now in view space

	//	scale by ortho range to be 0..1 in the view
	OrthoX /= GetOrthoRange();
	OrthoY /= GetOrthoRange();

	float2 wh( (float)RenderTargetSize.Width(), (float)RenderTargetSize.Height() );

	// Scale by the render target width to be in render target space
	OrthoX *= wh.x;
	OrthoY *= wh.x;

	// Limit results to (0,0,rendertargetwidth, rendertargetheight)
	//	gr: should allow out-of-bounds values so we can tell if something is off rendetarget/offscreen
	//TLMaths::Limit(OrthoX, 0.0f, wh.x);
	//TLMaths::Limit(OrthoY, 0.0f, wh.y);

	//	now set 
	RenderTargetPos.Set((s32)OrthoX, (s32)OrthoY);

	return TRUE;
}

