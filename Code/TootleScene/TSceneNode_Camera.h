
#pragma once

#include <TootleCore/TStateMachine.h>

#include "TSceneNode_Transform.h"

namespace TLScene
{
	class TSceneNode_Camera;
};

/*
	TSceneNode_Camera class

	Where possible you will be able able to alter values which simulate a real world camera,
	so you will be able to change the magnification, f-stop, lens size etc.  
	These will then propagate as information that will be used for rendering such as depth of field, 
	field of view etc which will be calculated and won't be explicitly editable
*/
class TLScene::TSceneNode_Camera : public TLScene::TSceneNode_Transform, public TStateMachine
{
	class TCameraState_Auto;
	class TCameraState_Manual;
	
public:
	TSceneNode_Camera(TRefRef NodeRef,TRefRef TypeRef);

	// 'Real' camera value access
	float			GetMagnification()							{ return m_fMagnification; }
	void			SetMagnification(float fMagnification)		{ m_fMagnification = fMagnification; }

	float			GetLensSize()								{ return m_fLensSize; }
	void			SetLenSize(float fLensSize)					{ m_fLensSize = fLensSize; }

	float			GetAperture()								{ return m_fAperture; }
	void			SetAperture(float fAperture)				{ m_fAperture = fAperture; }

	// Target access
	TRefRef			GetTargetRef()								{ return m_TargetNodeRef; }
	TPtr<TLScene::TSceneNode_Transform> GetTargetObject();
 
	float			GetTargetDistance()							{ return m_fTargetDistance; }

protected:

	virtual void	Initialise(TLMessaging::TMessage& Message);	//	Initialise message - made into virtual func as it's so commonly used

	virtual void	Update(float fTimeStep);

	virtual void	ProcessMessage(TLMessaging::TMessage& Message);
	
	void			UpdateRenderTargetCamera(); // DB - TEMP UNTIL I ADD A CAMERA MANAGER.

private:

	void			CalculateFOV();
	void			CalculateDOF();

private:

	// Target information
	TRef			m_TargetNodeRef;		// Target object reference
	TRef			m_RenderTargetRef;		// Render target reference
	
	float			m_fTargetDistance;		// Calculated distance to target
	
	// 'Real' camera information
	float			m_fMagnification;		// Zoom
	float			m_fLensSize;			// Lens size - 35mm etc 
	float			m_fFNumber;				// f-number - ratio of focal length to aperture width
	float			m_fFocalLength;			// The focal length of the camera
	float			m_fAperture;			// Aperture size

	// Calculated FOV
	float			m_fHFOV;				// Horizontal field of view
	float			m_fVFOV;				// Vertical field of view
	float			m_fDFOV;				// Diagonal field of view

	// Calculated DOF
	float			m_fDOFNear;				// Depth of field - near
	float			m_fDOFFar;				// Depth of field - far
};


class TLScene::TSceneNode_Camera::TCameraState_Auto : public TStateMode
{
public:
	virtual Bool			OnBegin(TRefRef PreviousMode);
	virtual TRef			Update();			
};


class TLScene::TSceneNode_Camera::TCameraState_Manual : public TStateMode, public TLMessaging::TSubscriber
{
public:
	virtual Bool			OnBegin(TRefRef PreviousMode);
	virtual TRef			Update();
	virtual void			OnEnd(TRefRef NextMode);	
protected:
	virtual TRefRef		GetSubscriberRef() const		{	static TRef Ref("CamStateManual");	return Ref;	}
	virtual void		ProcessMessage(TLMessaging::TMessage& Message);

};




