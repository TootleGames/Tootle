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

namespace TLRender
{
	class TCamera;				//	base camera
	class TProjectCamera;		//	projection camera
	class TOrthoCamera;			//	orthographic camera

	class TRenderTarget;
}

namespace TLMaths
{
	class TBoxOB;
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
	virtual void				SetRenderTargetSize(const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape)	{	}	//	calc new view sizes
	virtual void				SetViewportSize(const Type4<s32>& ViewportSize,TScreenShape ScreenShape)	{	}	//	calc new view sizes

	virtual Bool				IsOrtho() const			{	return FALSE;	}

	float3						GetWorldForward() const	{	return float3( 0.f, 0.f, 1.f );	}
	float3						GetWorldUp() const		{	return float3( 0.f, -1.f, 0.f );	}
	float3						GetWorldRight() const	{	return float3( 1.f, 0.f, 0.f );	}
	float						GetNearZ() const		{	return m_NearZ;	}
	float						GetFarZ() const			{	return m_FarZ;	}

	//	camera virtual
	virtual Bool				GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const	{	return FALSE;	}	//	convert point on screen to a 3D ray
	virtual Bool				GetWorldPos(float3& WorldPos,float WorldDepth,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const	{	return FALSE;	}	//	convert point on screen to a 3D position

	//	only for render targets
	TLMaths::TMatrix&			GetModelViewMatrix(Bool SetValid)	{	m_ModelViewMatrixValid |= SetValid;	return m_ModelViewMatrix;	}
	TLMaths::TMatrix&			GetProjectionMatrix(Bool SetValid)	{	m_ProjectionMatrixValid |= SetValid;	return m_ProjectionMatrix;	}

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
	TLMaths::TAngle			m_CameraRoll;				//	camera rotation

	TLMaths::TMatrix		m_ProjectionMatrix;			//	projection matrix
	Bool					m_ProjectionMatrixValid;	//	projection matrix is valid
	TLMaths::TMatrix		m_ModelViewMatrix;			//	modelview matrix
	Bool					m_ModelViewMatrixValid;		//	modelview matrix is valid
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
	virtual void			SetRenderTargetSize(const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape);	//	calc new view sizes
	virtual void			SetViewportSize(const Type4<s32>& ViewportSize,TScreenShape ScreenShape);	//	calc new view sizes

	const TLMaths::TBox2D&	GetScreenViewBox() const											{	return m_ScreenViewBox;	}		//	view dimensions - NOT rotated
	const TLMaths::TBox2D&	GetProjectionViewBox() const										{	return m_ProjectionViewBox;	}	//	view dimensions - rotated!

	const TLMaths::TMatrix&	GetCameraLookAtMatrix()												{	return (!m_CameraLookAtMatrixValid) ? UpdateCameraLookAtMatrix() : m_CameraLookAtMatrix;	}
	void					GetWorldFrustumPlaneBox(float ViewZDepth,TLMaths::TBoxOB& PlaneBox) const;			//	extract an oriented box from the frustum at a certain depth
	void					GetWorldFrustumPlaneBox2D(float ViewZDepth,TLMaths::TBox2D& PlaneBox2D) const;		//	extract box and make 2D

	virtual Bool			GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const;	//	convert point on screen to a 3D ray

	//	quad tree node virtual - only gets used if render target has a root zone
	virtual const TLMaths::TBox2D&	GetZoneShape();												//	get the shape of this node (frustum shape)

protected:
	const TLMaths::TMatrix&	UpdateCameraLookAtMatrix();

	virtual void			OnCameraChanged()		{	TCamera::OnCameraChanged();	m_FrustumValid = FALSE;	m_CameraLookAtMatrixValid = FALSE;	}
	void					CalcFrustum();			//	extract the frustum from the current view matricies

	void					GetWorldFrustumPlaneBoxCorners(float ViewZDepth,TArray<float3>& PlaneCorners) const;			//	extract an oriented box from the frustum at a certain depth

protected:
	TLMaths::TAngle			m_HorzFov;				//	HORIZONTAL field of view

	TLMaths::TMatrix		m_CameraLookAtMatrix;		
	Bool					m_CameraLookAtMatrixValid;	

	TLMaths::TBox2D			m_ScreenViewBox;		//	screen's frustum view box in view/screen space, unrotated and moved to nearZ
	TLMaths::TBox2D			m_ProjectionViewBox;	//	screen view, but rotated as per screen orientation
	TLMaths::TFrustum		m_Frustum;				//	current camera frustum
	Bool					m_FrustumValid;			//	is the frustum out of date? (camera moved)
	TLMaths::TBox2D			m_FrustumZoneShape;		//	
};



//--------------------------------------------------------------
//	orthographic camera
//--------------------------------------------------------------
class TLRender::TOrthoCamera : public TCamera
{
public:
	TOrthoCamera()			{}

	virtual Bool			IsOrtho() const			{	return TRUE;	}
	virtual void			SetRenderTargetSize(const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape);	//	calc new view sizes
	virtual void			SetViewportSize(const Type4<s32>& ViewportSize,TScreenShape ScreenShape);		//	calc new view sizes

	const TLMaths::TBox2D&	GetOrthoViewportBox() const			{	return m_OrthoViewportBox;	}
	const TLMaths::TBox2D&	GetOrthoRenderTargetBox() const		{	return m_OrthoRenderTargetBox;	}
	float					GetOrthoRange() const	{	return 100.f;	}

	virtual Bool			GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const;			//	convert point on screen to a 3D ray
	virtual Bool			GetWorldPos(float3& WorldPos,float WorldDepth,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const;	//	convert point on screen to a 3D position

protected:
	TLMaths::TBox2D			m_OrthoViewportBox;		//	ortho viewport dimensions as a box
	TLMaths::TBox2D			m_OrthoRenderTargetBox;	//	ortho render target dimensions as a box
};

