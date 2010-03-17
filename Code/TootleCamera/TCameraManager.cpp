/*
 *  TCameraManager.cpp
 *  TootleCamera
 *
 *  Created by Duane Bradbury on 27/01/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#include "TCameraManager.h"

#include "TLCamera.h"

#include <TootleAsset/TLAsset.h>


namespace TLCamera
{
	TPtr<TCameraManager>	g_pCameraManager = NULL;	// The global camera manager
}

using namespace TLCamera;

SyncBool TCameraManager::Initialise()
{
	// Initialise the low level device
	return TLCamera::Initialise();
}

SyncBool TCameraManager::Shutdown()
{
	TLDebug_Print("Cameramanager shutdown");

	m_pCameraTexture = NULL;
	
	return TLCamera::Shutdown();
}



void TCameraManager::ProcessMessage(TLMessaging::TMessage& Message)
{
	// Process image data from the camera and update the camera texture
	if(Message.GetMessageRef() == "image")
	{
		if(m_pCameraTexture.IsValid())
		{
			// Update the texture data
			//m_pCameraTexture->ImportData(Message);
			
			Bool bAlpha;
			u32 uWidth, uHeight;
			Message.ImportData("Alpha", bAlpha );
			Message.ImportData("Width", uWidth );
			Message.ImportData("Height", uHeight );	

			Type2<u16> ImageSize(uWidth, uHeight);
			m_pCameraTexture->SetSize(ImageSize, bAlpha, TRUE);
			
			TPtr<TBinaryTree>& pChild = Message.GetChild("TexData");
			
			if(pChild.IsValid())
			{
				TBinary& ImageData = pChild->GetData();
				
				m_pCameraTexture->SetTextureData(ImageSize, ImageData);

			}
		}
	}
	
	TManager::ProcessMessage(Message);
}


Bool TCameraManager::ConnectToCamera()
{
	if(TLCamera::ConnectToCamera())
	{
		TLCamera::SubscribeToCamera(this);
		return TRUE;
	}
	
	return FALSE;
}

Bool TCameraManager::DisconnectFromCamera()
{
	if(TLCamera::DisconnectFromCamera())
	{
		//TLCamera::UnSubscribeFromCamera(this);
		return TRUE;
	}

	return FALSE;
}

TRef TCameraManager::CreateCameraTexture()
{
	m_pCameraTexture = TLAsset::CreateAsset("camtexture","texture");
	
	if(!m_pCameraTexture)
		return TRef();
	
	// Set the texture to be loaded and initialise the image data
	m_pCameraTexture->SetLoadingState( TLAsset::LoadingState_Loaded );
	
	// OpenGL ES doesn;t like square textures so we need to use one bigger than what would actually like to display
	// for now
	//Type2<u16> texturesize(320,480);
	
	Type2<u16> texturesize(512,512);
	m_pCameraTexture->SetSize(texturesize, FALSE);
	
	return m_pCameraTexture->GetAssetRef();
}


