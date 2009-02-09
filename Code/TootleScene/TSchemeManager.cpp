/*
 *  TSchemeManager.cpp
 *  TootleScene
 *
 *  Created by Duane Bradbury on 06/02/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TSchemeManager.h"

#include <TootleAsset/TAsset.h>
#include <TootleCore/TEventChannel.h>

namespace TLScheme
{
	TPtr<TSchemeManager> g_pSchemeManager = NULL;
}


using namespace TLScheme;

SyncBool TSchemeManager::Initialise() 
{	
	return SyncTrue; 
}

void TSchemeManager::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{
	if(refPublisherID == "CORE")
	{
		// Subscribe to the update messages
		if(refChannelID == TLCore::UpdateRef)
			TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
	}
	
	// Super event channel routine
	TManager::OnEventChannelAdded(refPublisherID, refChannelID);
}


SyncBool TSchemeManager::Shutdown()
{ 
	// While we still have schemes loaded request them to be unloaded
	if(m_Schemes.GetSize())
	{
		//NOTE: We may not need to do this as we can assume the scenegraph will shutdown and remove all nodes
		UnloadAllSchemes();
		
		// Wait for all schemes to have been unloaded.
		return SyncWait;
	}	
	
	// Remove all request objects
	for(u32 uIndex = 0; uIndex < m_SchemeUpdateRequests.GetSize(); uIndex++)
	{
		m_SchemeUpdateRequests.ElementAt(uIndex) = NULL;
	}

	// Empty request list
	m_SchemeUpdateRequests.Empty();
	
	// Empty the list of remove requests
	m_SchemeUpdateRemove.Empty();
	
	return SyncTrue; 
}

SyncBool TSchemeManager::Update(float fTimeStep)		
{
	u32 uNumberOfRequests = m_SchemeUpdateRequests.GetSize();
	
	if(uNumberOfRequests > 0)
	{
		// Update all of the scheme update requests
		for(u32 uIndex = 0; uIndex < uNumberOfRequests; uIndex++)
		{
			TPtr<TSchemeUpdateRequest> pRequest = m_SchemeUpdateRequests.ElementAt(uIndex);			

			pRequest->Update();
		}
	}

	// Now go through the list of update requests that need removing if any
	uNumberOfRequests = m_SchemeUpdateRemove.GetSize();

	if(uNumberOfRequests)
	{
		for(u32 uIndex = 0; uIndex < uNumberOfRequests; uIndex++)
		{
			TRefRef SchemeRef = m_SchemeUpdateRemove.ElementAt(uIndex);
			
			s32 sIndex = m_SchemeUpdateRequests.FindIndex(SchemeRef);
			
			if(sIndex != -1)
			{
				// remove from the list
				m_SchemeUpdateRequests.RemoveAt(sIndex);				
			}
			else
			{
				TLDebug_Break("Failed to find scheme update request object for removal");
			}
		}

		// Empty the list of remove requests
		m_SchemeUpdateRemove.Empty();
	}
	
	return TManager::Update(fTimeStep);
}


Bool TSchemeManager::IsSchemeInState(TRefRef SchemeRef, TSchemeState SchemeState)
{
	if(SchemeState == Loaded)
	{
		return (m_Schemes.Find(SchemeRef) != NULL);
	}
	else if(SchemeState == UnLoaded)
	{
		return (m_Schemes.Find(SchemeRef) == NULL);
	}
	else
	{
		TPtr<TSchemeUpdateRequest> pRequest = m_SchemeUpdateRequests.FindPtr(SchemeRef);

		// Any request to load or unload? If not we are not loading or unloading the scheme
		if(!pRequest)
			return FALSE;

		// If the request is a load then return TRUE is the state check was Loading
		if((pRequest->GetUpdateType() == Load) && (SchemeState == Loading))
			return TRUE;
		else if(SchemeState == UnLoading)
			return TRUE;
			
		return FALSE;
	}
}

// Main request handling routine
Bool TSchemeManager::RequestUpdateScheme(TRefRef SchemeRef, TRefRef SchemeAssetRef, TSchemeUpdateType UpdateType)
{
	// Check to see if the scheme specified is alreayd in the state required
	if(UpdateType == Load)
	{
		// Laoded already?
		if(IsSchemeLoaded(SchemeRef))
			return TRUE;
		
		// Loading requested already?
		if(IsSchemeLoading(SchemeRef))
			return TRUE;
		
		// Unload requested?
		// Scheme loading?
		if(IsSchemeUnloading(SchemeRef))
		{
			// Cancel the unload and start the load
			TLDebug_Break("TODO");
			return FALSE;
		}
		
	}	
	else
	{
		// Unloaded already?
		if(IsSchemeUnLoaded(SchemeRef))
			return TRUE;

		// Unloading already?
		if(IsSchemeUnloading(SchemeRef))
			return TRUE;
		
		// Scheme loading?
		if(IsSchemeLoading(SchemeRef))
		{
			// Cancel the load and start the unload
			TLDebug_Break("TODO");
			return FALSE;
		}
		
	}
	
	
	// Begin the load/unload of the scheme
	TPtr<TSchemeUpdateRequest> pRequest = new TSchemeUpdateRequest(SchemeRef, SchemeAssetRef, UpdateType);
	
	if(pRequest)
	{
		if(m_SchemeUpdateRequests.Add(pRequest) != -1)
		{
			// Subscribe to the update request
			return SubscribeTo(pRequest.GetObject());
		}
	}
	

	
	return FALSE;
}

void TSchemeManager::UnloadAllSchemes()
{
	for(u32 uIndex = 0; uIndex < m_Schemes.GetSize(); uIndex++)
	{
		TRefRef SchemeRef = m_Schemes.ElementAt(uIndex);
		RequestUpdateScheme(SchemeRef, "", UnLoad);
	}
}

void TSchemeManager::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	TRef MessageRef = pMessage->GetMessageRef();

	// Handle request update messages
	if(MessageRef == "LOAD")
	{
		TRef SchemeRef;
		
		if(pMessage->ImportData("SCHEME", SchemeRef))
		{
			// If complete then add the scheme to our list of instanced schemes
			// and remove from the schemeupdate list
			m_Schemes.Add(SchemeRef);
			
			m_SchemeUpdateRemove.Add(SchemeRef);
				
		}
		
		return;
	}
	else if(MessageRef == "UNLOAD")
	{
		// If complete then remove the scheme from our list of instanced schemes
		// and remove from the schemeupdate list
		TRef SchemeRef;
		
		if(pMessage->ImportData("SCHEME", SchemeRef))
		{
			// If complete then add the scheme to our list of instanced schemes
			// and remove from the schemeupdate list			
			m_Schemes.Remove(SchemeRef);
			
			m_SchemeUpdateRemove.Add(SchemeRef);
		}
		
		return;
	}
	
	TManager::ProcessMessage(pMessage);
}




// Scheme update request class
TSchemeManager::TSchemeUpdateRequest::TSchemeUpdateRequest(TRefRef SchemeRef, TRefRef SchemeAssetRef, TSchemeUpdateType UpdateType) :
	m_SchemeRef(SchemeRef),
	m_SchemeAssetRef(SchemeAssetRef),
	m_Type(UpdateType)
{
	AddMode<TSchemeManager::TSchemeUpdateRequest::TSchemeState_Init>("Init");
	AddMode<TSchemeManager::TSchemeUpdateRequest::TSchemeState_Loading>("Loading");
	AddMode<TSchemeManager::TSchemeUpdateRequest::TSchemeState_UnLoading>("UnLoading");
	AddMode<TSchemeManager::TSchemeUpdateRequest::TSchemeState_Finished>("Finished");
	AddMode<TSchemeManager::TSchemeUpdateRequest::TSchemeState_Cancel>("Cancel");
	
	//SetMode("Init");
}


TRef TSchemeManager::TSchemeUpdateRequest::TSchemeState_Init::Update()
{	
	TPtr<TSchemeManager::TSchemeUpdateRequest> pRequest = GetStateMachine<TSchemeManager::TSchemeUpdateRequest>();

	if(pRequest->GetUpdateType() == Load)
	{
		// Begin load		
		TPtr<TLAsset::TAsset>& pAsset = TLAsset::GetAsset(pRequest->GetSchemeAssetRef(), TRUE);

		// Is the asset loaded?
		if(!pAsset)
		{
			// Request the asset to be loaded
			TLAsset::LoadAsset(pRequest->GetSchemeAssetRef());
			
			// Wait for asset to load
			return TRef();
		}

		// Asset is loaded
		return "Loading";
	}
	else
	{
		// Begin unload
		
		return "UnLoading";
	}
}	


TRef TSchemeManager::TSchemeUpdateRequest::TSchemeState_Loading::Update()
{	
	// Load the schemes required files
	// Wait for files to load
	// Instance the scheme
	// Wait for scheme to be instanced
	// Add scheme to scenegraph

	// Broadcast message to say we are done loading
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("LOAD");
	
	if(pMessage)
	{
		TPtr<TSchemeManager::TSchemeUpdateRequest> pRequest = GetStateMachine<TSchemeManager::TSchemeUpdateRequest>();
		
		if(pRequest)
		{
			pMessage->ExportData("SCHEME", pRequest->GetSchemeRef());
			pRequest->PublishMessage(pMessage);
		}
	}

	// All done	
	return "Finished";	
}	




TRef TSchemeManager::TSchemeUpdateRequest::TSchemeState_UnLoading::Update()
{	
	// Find scheme node in scenegraph
	// Shutdown the scheme node
	// and remove from graph
	
	
	// Broadcast message to say we are done loading
	TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("LOAD");
	
	if(pMessage)
	{
		TPtr<TSchemeManager::TSchemeUpdateRequest> pRequest = GetStateMachine<TSchemeManager::TSchemeUpdateRequest>();
		
		if(pRequest)
		{
			pMessage->ExportData("SCHEME", pRequest->GetSchemeRef());
			pRequest->PublishMessage(pMessage);
		}
	}
	
	// All done
	return "Finished";	
}	


TRef TSchemeManager::TSchemeUpdateRequest::TSchemeState_Cancel::Update()
{	
	return TRef();	
}	




