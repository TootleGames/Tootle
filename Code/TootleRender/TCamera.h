/*------------------------------------------------------
	
	For future expansion; camera is contained in its own
	class.
	All the clever generic camera maths (clip tests etc)
	should be in here



	todo:
	3d world pos -> 2d rendertarget pos
	http://www.gamedev.net/community/forums/topic.asp?topic_id=426094



-------------------------------------------------------*/
#pragma once


#include <TootleCore/TLTypes.h>
#include <TootleCore/TSubscriber.h>
#include <TootleCore/TLMaths.h>
#include <TootleMaths/TQuadTree.h>
#include <TootleMaths/TFrustum.h>
#include <TootleMaths/TLine.h>
#include "TLRender.h"
#include "TScreenShape.h"

namespace TLRender
{
	class TCamera;				//	base camera
	class TProjectCamera;		//	projection camera
	class TOrthoCamera;			//	orthographic camera

	class TRenderTarget;
}

namespace TLMaths
{
	class TOblong;
}


//--------------------------------------------------------------
//	base camera type
//--------------------------------------------------------------
class TLRender::TCamera : public TLMaths::TQuadTreeNode
{
public:
	TCamera();
	
	virtual const float3&		GetPosition() const		{	return m_ViewLine.GetStart();	}
	virtual const float3&		GetLookAt() const		{	return m_ViewLine.GetEnd();	}
	virtual float3				GetViewForward() const	{	return m_ViewForward;	}//m_ViewLine.GetDirectionNormal();	}
	virtual float3				GetViewUp() const		{	return m_ViewUp;	}
	virtual float3				GetViewRight() const	{	return m_ViewRight;	}
	const TLMaths::TAngle&		GetCameraRoll() const	{	return m_CameraRoll;	}
	void						SetCameraRoll(const TLMaths::TAngle& RollAngle) {	m_CameraRoll = RollAngle;	OnCameraChanged();	}

	virtual void				SetPosition(const float3& Position)	{	m_vPreviousPos = GetPosition(); m_ViewLine.SetStart( Position );	OnCameraChanged();	}
	virtual void				SetLookAt(const float3& LookAt)		{	m_ViewLine.SetEnd( LookAt );	OnCameraChanged();	}
	virtual void				SetRenderTargetSize(const Type4<s32>& RenderTargetSize,const Type4<s32>& RenderTargetMaxSize,const Type4<s32>& ViewportMaxSize,TScreenShape ScreenShape)=0;

	const TLMaths::TBox2D&		GetViewportBox() const	{	return m_ViewportBox;	}
	const TLMaths::TBox2D&		GetScissorBox() const	{	return m_ScissorBox;	}

	virtual Bool				IsOrtho() const			{	return FALSE;	}

	float3						GetWorldForward() const	{	return float3( 0.f, 0.f, 1.f );	}
	float3						GetWorldUp() const		{	return float3( 0.f, -1.f, 0.f );	}
	float3						GetWorldRight() const	{	return float3( 1.f, 0.f, 0.f );	}
	float						GetNearZ() const		{	return m_NearZ;	}
	float						GetFarZ() const			{	return m_FarZ;	}

	//	camera virtual
	virtual Bool					GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const	{	return FALSE;	}	//	convert point on screen to a 3D ray
	virtual Bool					GetWorldPos(float3& WorldPos,float WorldDepth,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const	{	return FALSE;	}	//	convert point on screen to a 3D position
	virtual Bool					GetRenderTargetPos(Type2<s32>& RenderTargetPos, const float3& WorldPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const { return FALSE; } // convert 3d pos into screen 2d point
	virtual float					GetScreenSizeFromWorldSize(float WorldUnit,float Depth)		{	return WorldUnit;	};	//	convert a world unit to pixel size
	virtual const TLMaths::TBox2D&	GetWorldViewBox(float WorldDepth) const=0;					//	the world-space box for the extents at the edges of the screen.

protected:
	virtual void				OnCameraChanged();			//	
	void						CalculateViewVectors();		//	calculate new view vectors

protected:
	TLMaths::TLine			m_ViewLine;		//	pos=start, lookat=end

	//	we calculate these on-change so that we dont need to do it all the time - these are in world space still
	float3					m_ViewUp;		//	world dir cross view right = view up
	float3					m_ViewRight;	//	world dir coss world up = view right
	float3					m_ViewForward;	//	direction of Pos->lookat
	float3					m_vPreviousPos;	// Previous camera position

	float					m_NearZ;
	float					m_FarZ;

	TLMaths::TAngle			m_CameraRoll;	//	camera rotation
	TLMaths::TBox2D			m_ViewportBox;	//	viewport size - screen rotated
	TLMaths::TBox2D			m_ScissorBox;	//	scissor size - screen rotated
};



//--------------------------------------------------------------
//	regular 3d projection camera
//--------------------------------------------------------------
class TLRender::TProjectCamera : public TLRender::TCamera
{
	friend class TRenderTarget;
public:
	TProjectCamera();

	const TLMaths::TAngle&	GetHorzFov() const													{	return m_HorzFov;	}
//	const TLMaths::TAngle&	GetVertFov() const													{	return GetHorzFov();	}	//	gr: todo
	virtual void			SetRenderTargetSize(const Type4<s32>& RenderTargetSize,const Type4<s32>& RenderTargetMaxSize,const Type4<s32>& ViewportMaxSize,TScreenShape ScreenShape);	//	calc new view sizes

	const TLMaths::TBox2D&	GetScreenViewBox() const											{	return m_ScreenViewBox;	}		//	view dimensions - NOT rotated
	const TLMaths::TBox2D&	GetProjectionViewBox() const										{	return m_ProjectionViewBox;	}	//	view dimensions - rotated!

	const TLMaths::TMatrix&	GetCameraLookAtMatrix()												{	return (!m_CameraLookAtMatrixValid) ? UpdateCameraLookAtMatrix() : m_CameraLookAtMatrix;	}
	void					GetWorldFrustumPlaneBox(float ViewZDepth,TLMaths::TOblong& PlaneBox) const;			//	extract an oriented box from the frustum at a certain depth
	void					GetWorldFrustumPlaneBox2D(float ViewZDepth,TLMaths::TBox2D& PlaneBox2D) const;		//	extract box and make 2D

	virtual Bool			GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const;	//	convert point on screen to a 3D ray
	virtual const TLMaths::TBox2D&	GetWorldViewBox(float WorldDepth) const;					//	the world-space box for the extents at the edges of the screen.

	//	quad tree node virtual - only gets used if render target has a root zone
	virtual const TLMaths::TBox2D&	GetZoneShape();												//	get the shape of this node (frustum shape)

protected:
	const TLMaths::TMatrix&	UpdateCameraLookAtMatrix();

	virtual void			OnCameraChanged();
	void					CalcFrustum();			//	extract the frustum from the current view matricies

	void					GetWorldFrustumPlaneBoxCorners(float ViewZDepth,TArray<float3>& PlaneCorners) const;			//	extract an oriented box from the frustum at a certain depth

protected:
	TLMaths::TAngle			m_HorzFov;				//	HORIZONTAL field of view

	TLMaths::TMatrix		m_CameraLookAtMatrix;		
	Bool					m_CameraLookAtMatrixValid;	

	TLMaths::TBox2D			m_ScreenViewBox;		//	screen's frustum view box in view/screen space, unrotated and moved to nearZ
	TLMaths::TBox2D			m_ProjectionViewBox;	//	screen view, but rotated as per screen orientation

	TLMaths::TOblong		m_FrustumOblong;		//	current camera frustum shape. Most accurate shape
	TLMaths::TBox2D			m_FrustumBox;			//	frustum shape in a box for 2D culling - less accurate but infinately faster
	SyncBool				m_FrustumShapeValid;	//	is the frustum out of date? (camera moved) - FALSE if invalid, syncwait if out of date
};



//--------------------------------------------------------------
//	orthographic camera
//--------------------------------------------------------------
class TLRender::TOrthoCamera : public TCamera
{
public:
	TOrthoCamera(Bool SquareProjection=FALSE) :
		m_WorldToPixelScale	( 0.f ),
		m_PixelToWorldScale	( 0.f ),
		m_SquareProjection	( SquareProjection )
	{
	}

	virtual Bool			IsOrtho() const			{	return TRUE;	}
	virtual void			SetRenderTargetSize(const Type4<s32>& RenderTargetSize,const Type4<s32>& RenderTargetMaxSize,const Type4<s32>& ViewportMaxSize,TScreenShape ScreenShape);	//	calc new view sizes

	const TLMaths::TBox2D&				GetOrthoCoordinateBox() const		{	return m_OrthoCoorindateBox;	}	
	DEPRECATED const TLMaths::TBox2D&	GetOrthoRenderTargetBox() const		{	return m_OrthoWorldBox;	}	//	use TLRender::TRenderTarget::GetWorldViewBox (or GetWorldViewBox if you have a camera pointer already)
	float								GetOrthoRange() const				{	return 100.f;	}

	virtual Bool						GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const;			//	convert point on screen to a 3D ray
	virtual Bool						GetWorldPos(float3& WorldPos,float WorldDepth,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const;	//	convert point on screen to a 3D position
	virtual const TLMaths::TBox2D&		GetWorldViewBox(float WorldDepth) const	{	return m_OrthoWorldBox;	}

	virtual Bool			GetRenderTargetPos(Type2<s32>& RenderTargetPos, const float3& WorldPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const; // convert 3d pos into rendertarget-space 2d point
	virtual float			GetScreenSizeFromWorldSize(float WorldUnit,float Depth)		{	return WorldUnit * m_WorldToPixelScale;	};	//	convert a world unit to pixel size

protected:
	float					m_WorldToPixelScale;		//	world -> pixel scalar
	float					m_PixelToWorldScale;		//	pixel -> world scalar
	TLMaths::TBox2D			m_OrthoCoorindateBox;		//	ortho coorindate box. this will match the viewport box but be in ortho scale
	TLMaths::TBox2D			m_OrthoWorldBox;			//	ortho render target dimensions as a box
	Bool					m_SquareProjection;			//	if true then the projection becomes square which allows us to roll the camera without skewing it
};

