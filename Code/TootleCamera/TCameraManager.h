/*
 *  TCameraManager.h
 *  TootleCamera
 *
 *  Created by Duane Bradbury on 27/01/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include <TootleCore/TManager.h>
#include <TootleCore/TPtr.h>

#include <TootleAsset/TTexture.h>

namespace TLCamera
{
	class TCameraManager;
	
	extern 	TPtr<TCameraManager>	g_pCameraManager;	// The global camera manager
}


class TLCamera::TCameraManager : public TLCore::TManager
{
public:
	TCameraManager(TRefRef ManagerRef) :
		TManager(ManagerRef)
	{
	}

	// Camera access
	Bool	ConnectToCamera();
	Bool	DisconnectFromCamera();
		
	// Camera texture
	TRef				CreateCameraTexture();
	FORCEINLINE TRef	GetCameraTextureRef()		{ return (m_pCameraTexture.IsValid() ? m_pCameraTexture->GetAssetRef() : TRef()) ; }

protected:
	virtual SyncBool Initialise();
	virtual SyncBool Shutdown();
	
	
	virtual void	ProcessMessage(TLMessaging::TMessage& Message);

	
private:	
	TPtr<TLAsset::TTexture>		m_pCameraTexture;
};