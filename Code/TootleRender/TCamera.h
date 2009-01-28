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
	virtual float3				GetForward() const		{	return m_Forward;	}//m_ViewLine.GetDirectionNormal();	}
	virtual const float3&		GetLookAt() const		{	return m_ViewLine.GetEnd();	}
	virtual float3				GetUp() const			{	return GetWorldUp();	}
	virtual float3				GetRight() const		{	return GetWorldUp();	}

	virtual void				SetPosition(const float3& Position)	{	m_ViewLine.SetStart( Position );	OnCameraChanged();	}
	virtual void				SetLookAt(const float3& LookAt)		{	m_ViewLine.SetEnd( LookAt );	OnCameraChanged();	}

	virtual Bool				IsOrtho() const			{	return FALSE;	}
	
	float3						GetWorldForward() const	{	return float3( 0.f, 0.f, 1.f );	}
	float3						GetWorldUp() const		{	return float3( 0.f, -1.f, 0.f );	}
	float3						GetWorldRight() const	{	return float3( 1.f, 0.f, 0.f );	}
	float						GetNearZ() const		{	return m_NearZ;	}
	float						GetFarZ() const			{	return m_FarZ;	}

	//	camera virtual
	virtual Bool				GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize) const	{	return FALSE;	}	//	convert point on screen to a 3D ray

	//	only for render targets
	TLMaths::TMatrix&			GetModelViewMatrix(Bool SetValid)						{	m_ModelViewMatrixValid |= SetValid;	return m_ModelViewMatrix;	}
	TLMaths::TMatrix&			GetProjectionMatrix(Bool SetValid)						{	m_ProjectionMatrixValid |= SetValid;	return m_ProjectionMatrix;	}

protected:
	virtual void				OnCameraChanged();			//	
	void						CalculateViewVectors();		//	calculate new view vectors

protected:
	TLMaths::TLine			m_ViewLine;		//	pos=start, lookat=end

	//	we calculate these on-change so that we dont need to do it all the time
	float3					m_ViewUp;		//	world dir cross view right = view up
	float3					m_ViewRight;	//	world dir coss world up = view right
	float3					m_Forward;		//	direction of Pos->lookat (world and view)

	float					m_NearZ;
	float					m_FarZ;

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

	float					GetAspectRatio() const												{	return (4.f / 3.f);	}	//	gr: fixed aspect ratio now, might need to be reversed for widescreen
	const TLMaths::TAngle&	GetHorzFov() const													{	return m_HorzFov;	}
	const TLMaths::TAngle&	GetVertFov() const													{	return GetHorzFov();	}	//	gr: todo

//	void					GetFrustumMatrix(TLMaths::TMatrix& Matrix,float AspectRatio);		//	work out a frustum matrix from this camera
	const TLMaths::TMatrix&	GetCameraLookAtMatrix()												{	return (!m_CameraLookAtMatrixValid) ? UpdateCameraLookAtMatrix() : m_CameraLookAtMatrix;	}
	void					GetPlaneBox(float ViewZDepth,TLMaths::TBoxOB& PlaneBox);			//	extract an oriented box from the frustum at a certain depth

	virtual Bool			GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize) const;	//	convert point on screen to a 3D ray

	//	quad tree node virtual - only gets used if render target has a root zone
	virtual const TLMaths::TBox2D&	GetZoneShape();												//	get the shape of this node (frustum shape)

protected:
	const TLMaths::TMatrix&	UpdateCameraLookAtMatrix();

	virtual void			OnCameraChanged()		{	TCamera::OnCameraChanged();	m_FrustumValid = FALSE;	m_CameraLookAtMatrixValid = FALSE;	}
	void					CalcFrustum();			//	extract the frustum from the current view matricies

	void					GetPlaneBoxCorners(float ViewZDepth,TArray<float3>& PlaneCorners);			//	extract an oriented box from the frustum at a certain depth

protected:
	TLMaths::TAngle			m_HorzFov;				//	HORIZONTAL field of view

	TLMaths::TMatrix		m_CameraLookAtMatrix;		
	Bool					m_CameraLookAtMatrixValid;	

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
	void					GetOrthoSize(Type4<float>& OrthoSize,const Type4<s32>& RenderTargetSize);	//	convert render target size (pixels) to game-screen dimensions (0-100.f)

	virtual Bool			GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize) const;			//	convert point on screen to a 3D ray

protected:
	float					GetOrthoRange() const	{	return 100.f;	}
};

