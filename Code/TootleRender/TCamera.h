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
#include <TootleCore/TLMaths.h>
#include <TootleCore/TSubscriber.h>


namespace TLRender
{
	class TCamera;				//	base camera
	class TProjectCamera;		//	projection camera
	class TOrthoCamera;			//	orthographic camera

	class TRenderTarget;
}



//--------------------------------------------------------------
//	base camera type
//--------------------------------------------------------------
class TLRender::TCamera
{
public:
	TCamera();

	virtual const float3&	GetPosition() const		{	return m_Position;	}
	virtual float3			GetForward() const		{	return GetLookAt() - GetPosition();	}
	virtual const float3&	GetLookAt() const		{	return m_LookAt;	}
	virtual float3			GetUp() const			{	return GetWorldUp();	}

	virtual void			SetPosition(const float3& Position)	{	m_Position = Position;	}
	virtual void			SetLookAt(const float3& LookAt)		{	m_LookAt = LookAt;	}

	virtual Bool			IsOrtho() const			{	return FALSE;	}
	
	float3					GetWorldForward() const	{	return float3( 0.f, 0.f, 1.f );	}
	float3					GetWorldUp() const		{	return float3( 0.f, -1.f, 0.f );	}
	float3					GetWorldRight() const	{	return float3( 1.f, 0.f, 0.f );	}

	float					GetNearZ() const		{	return m_NearZ;	}
	float					GetFarZ() const			{	return m_FarZ;	}

	virtual Bool			GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize) const	{	return FALSE;	}	//	convert point on screen to a 3D ray

protected:
	float3					m_Position;	
	float3					m_LookAt;

	float					m_NearZ;
	float					m_FarZ;
};



//--------------------------------------------------------------
//	regular 3d projection camera
//--------------------------------------------------------------
class TLRender::TProjectCamera : public TCamera
{
public:
	TProjectCamera();

	const TLMaths::TAngle&	GetHorzFov() const													{	return m_HorzFov;	}
	const TLMaths::TAngle&	GetVertFov() const													{	return GetHorzFov();	}	//	gr: todo

	virtual void			SetPosition(const float3& Position)									{	m_CameraMatrixValid = FALSE;	TCamera::SetPosition( Position );	}
	virtual void			SetLookAt(const float3& LookAt)										{	m_CameraMatrixValid = FALSE;	TCamera::SetLookAt( LookAt );	}

	void					GetFrustumMatrix(TLMaths::TMatrix& Matrix,float AspectRatio);		//	work out a frustum matrix from this camera
	TLMaths::TMatrix&		GetCameraMatrix(TLRender::TRenderTarget* pRenderTarget)				{	if ( !m_CameraMatrixValid )	UpdateCameraMatrix(pRenderTarget);	return m_CameraMatrix;	}
	void					SetLastProjectionMatrix(const TLMaths::TMatrix& ProjectionMatrix)	{	m_LastProjectionMatrix = ProjectionMatrix;	}

	virtual Bool			GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize) const;	//	convert point on screen to a 3D ray

protected:
	void					UpdateCameraMatrix(TLRender::TRenderTarget* pRenderTarget);


protected:
	TLMaths::TAngle			m_HorzFov;				//	HORIZONTAL field of view
	TLMaths::TMatrix		m_CameraMatrix;			//	current camera matrix
	Bool					m_CameraMatrixValid;	//	is camera matrix out of date?

	TLMaths::TMatrix		m_LastProjectionMatrix;
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

